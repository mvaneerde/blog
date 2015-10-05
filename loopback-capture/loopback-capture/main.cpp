// main.cpp

#include "common.h"

int do_everything(int argc, LPCWSTR argv[]);

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    HRESULT hr = S_OK;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }

    int result = do_everything(argc, argv);
    
    CoUninitialize();
    return result;
}

int do_everything(int argc, LPCWSTR argv[]) {
    HRESULT hr = S_OK;

    // parse command line
    CPrefs prefs(argc, argv, hr);
    if (FAILED(hr)) {
        ERR(L"CPrefs::CPrefs constructor failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    if (S_FALSE == hr) {
        // nothing to do
        return 0;
    }

    // create a "loopback capture has started" event
    HANDLE hStartedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hStartedEvent) {
        ERR(L"CreateEvent failed: last error is %u", GetLastError());
        return -__LINE__;
    }

    // create a "stop capturing now" event
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hStopEvent) {
        ERR(L"CreateEvent failed: last error is %u", GetLastError());
        CloseHandle(hStartedEvent);
        return -__LINE__;
    }

    // create arguments for loopback capture thread
    LoopbackCaptureThreadFunctionArguments threadArgs;
    threadArgs.hr = E_UNEXPECTED; // thread will overwrite this
    threadArgs.pMMDevice = prefs.m_pMMDevice;
    threadArgs.bInt16 = prefs.m_bInt16;
    threadArgs.hFile = prefs.m_hFile;
    threadArgs.hStartedEvent = hStartedEvent;
    threadArgs.hStopEvent = hStopEvent;
    threadArgs.nFrames = 0;

    HANDLE hThread = CreateThread(
        NULL, 0,
        LoopbackCaptureThreadFunction, &threadArgs,
        0, NULL
    );
    if (NULL == hThread) {
        ERR(L"CreateThread failed: last error is %u", GetLastError());
        CloseHandle(hStopEvent);
        CloseHandle(hStartedEvent);
        return -__LINE__;
    }

    // wait for either capture to start or the thread to end
    HANDLE waitArray[2] = { hStartedEvent, hThread };
    DWORD dwWaitResult;
    dwWaitResult = WaitForMultipleObjects(
        ARRAYSIZE(waitArray), waitArray,
        FALSE, INFINITE
    );

    if (WAIT_OBJECT_0 + 1 == dwWaitResult) {
        ERR(L"Thread aborted before starting to loopback capture: hr = 0x%08x", threadArgs.hr);
        CloseHandle(hStartedEvent);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    if (WAIT_OBJECT_0 != dwWaitResult) {
        ERR(L"Unexpected WaitForMultipleObjects return value %u", dwWaitResult);
        CloseHandle(hStartedEvent);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    CloseHandle(hStartedEvent);

    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    if (INVALID_HANDLE_VALUE == hStdIn) {
        ERR(L"GetStdHandle returned INVALID_HANDLE_VALUE: last error is %u", GetLastError());
        SetEvent(hStopEvent);
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hStartedEvent);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    LOG(L"%s", L"Press Enter to quit...");

    // wait for the thread to terminate early
    // or for the user to press (and release) Enter
    HANDLE rhHandles[2] = { hThread, hStdIn };

    bool bKeepWaiting = true;
    while (bKeepWaiting) {

        dwWaitResult = WaitForMultipleObjects(2, rhHandles, FALSE, INFINITE);

        switch (dwWaitResult) {

            case WAIT_OBJECT_0: // hThread
                ERR(L"%s", L"The thread terminated early - something bad happened");
                bKeepWaiting = false;
                break;

            case WAIT_OBJECT_0 + 1: // hStdIn
                // see if any of them was an Enter key-up event
                INPUT_RECORD rInput[128];
                DWORD nEvents;
                if (!ReadConsoleInput(hStdIn, rInput, ARRAYSIZE(rInput), &nEvents)) {
                    ERR(L"ReadConsoleInput failed: last error is %u", GetLastError());
                    SetEvent(hStopEvent);
                    WaitForSingleObject(hThread, INFINITE);
                    bKeepWaiting = false;
                } else {
                    for (DWORD i = 0; i < nEvents; i++) {
                        if (
                            KEY_EVENT == rInput[i].EventType &&
                            VK_RETURN == rInput[i].Event.KeyEvent.wVirtualKeyCode &&
                            !rInput[i].Event.KeyEvent.bKeyDown
                         ) {
                            LOG(L"%s", L"Stopping capture...");
                            SetEvent(hStopEvent);
                            WaitForSingleObject(hThread, INFINITE);
                            bKeepWaiting = false;
                            break;
                        }
                    }
                    // if none of them were Enter key-up events,
                    // continue waiting
                }
                break;

            default:
                ERR(L"WaitForMultipleObjects returned unexpected value 0x%08x", dwWaitResult);
                SetEvent(hStopEvent);
                WaitForSingleObject(hThread, INFINITE);
                bKeepWaiting = false;
                break;
        }
    }

    DWORD exitCode;
    if (!GetExitCodeThread(hThread, &exitCode)) {
        ERR(L"GetExitCodeThread failed: last error is %u", GetLastError());
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    if (0 != exitCode) {
        ERR(L"Loopback capture thread exit code is %u; expected 0", exitCode);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    if (S_OK != threadArgs.hr) {
        ERR(L"Thread HRESULT is 0x%08x", threadArgs.hr);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    CloseHandle(hThread);
    CloseHandle(hStopEvent);

    // everything went well... fixup the fact chunk in the file
    MMRESULT result = mmioClose(prefs.m_hFile, 0);
    prefs.m_hFile = NULL;
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioClose failed: MMSYSERR = %u", result);
        return -__LINE__;
    }

    // reopen the file in read/write mode
    MMIOINFO mi = {0};
    prefs.m_hFile = mmioOpen(const_cast<LPWSTR>(prefs.m_szFilename), &mi, MMIO_READWRITE);
    if (NULL == prefs.m_hFile) {
        ERR(L"mmioOpen(\"%ls\", ...) failed. wErrorRet == %u", prefs.m_szFilename, mi.wErrorRet);
        return -__LINE__;
    }

    // descend into the RIFF/WAVE chunk
    MMCKINFO ckRIFF = {0};
    ckRIFF.ckid = MAKEFOURCC('W', 'A', 'V', 'E'); // this is right for mmioDescend
    result = mmioDescend(prefs.m_hFile, &ckRIFF, NULL, MMIO_FINDRIFF);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioDescend(\"WAVE\") failed: MMSYSERR = %u", result);
        return -__LINE__;
    }

    // descend into the fact chunk
    MMCKINFO ckFact = {0};
    ckFact.ckid = MAKEFOURCC('f', 'a', 'c', 't');
    result = mmioDescend(prefs.m_hFile, &ckFact, &ckRIFF, MMIO_FINDCHUNK);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioDescend(\"fact\") failed: MMSYSERR = %u", result);
        return -__LINE__;
    }

    // write the correct data to the fact chunk
    LONG lBytesWritten = mmioWrite(
        prefs.m_hFile,
        reinterpret_cast<PCHAR>(&threadArgs.nFrames),
        sizeof(threadArgs.nFrames)
    );
    if (lBytesWritten != sizeof(threadArgs.nFrames)) {
        ERR(L"Updating the fact chunk wrote %u bytes; expected %u", lBytesWritten, (UINT32)sizeof(threadArgs.nFrames));
        return -__LINE__;
    }

    // ascend out of the fact chunk
    result = mmioAscend(prefs.m_hFile, &ckFact, 0);
    if (MMSYSERR_NOERROR != result) {
        ERR(L"mmioAscend(\"fact\") failed: MMSYSERR = %u", result);
        return -__LINE__;
    }

    // let prefs' destructor call mmioClose
    
    return 0;
}
