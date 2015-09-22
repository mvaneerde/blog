// main.cpp

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>

#include "prefs.h"
#include "loopback-capture.h"

int do_everything(int argc, LPCWSTR argv[]);

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    HRESULT hr = S_OK;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("CoInitialize failed: hr = 0x%08x", hr);
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
        printf("CPrefs::CPrefs constructor failed: hr = 0x%08x\n", hr);
        return -__LINE__;
    }
    if (S_FALSE == hr) {
        // nothing to do
        return 0;
    }

    // create a "loopback capture has started" event
    HANDLE hStartedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hStartedEvent) {
        printf("CreateEvent failed: last error is %u\n", GetLastError());
        return -__LINE__;
    }

    // create a "stop capturing now" event
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hStopEvent) {
        printf("CreateEvent failed: last error is %u\n", GetLastError());
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
        printf("CreateThread failed: last error is %u\n", GetLastError());
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
        printf("Thread aborted before starting to loopback capture: hr = 0x%08x\n", threadArgs.hr);
        CloseHandle(hStartedEvent);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    if (WAIT_OBJECT_0 != dwWaitResult) {
        printf("Unexpected WaitForMultipleObjects return value %u", dwWaitResult);
        CloseHandle(hStartedEvent);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    CloseHandle(hStartedEvent);

    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    if (INVALID_HANDLE_VALUE == hStdIn) {
        printf("GetStdHandle returned INVALID_HANDLE_VALUE: last error is %u\n", GetLastError());
        SetEvent(hStopEvent);
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hStartedEvent);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    printf("Press Enter to quit...\n");

    // wait for the thread to terminate early
    // or for the user to press (and release) Enter
    HANDLE rhHandles[2] = { hThread, hStdIn };

    bool bKeepWaiting = true;
    while (bKeepWaiting) {

        dwWaitResult = WaitForMultipleObjects(2, rhHandles, FALSE, INFINITE);

        switch (dwWaitResult) {

            case WAIT_OBJECT_0: // hThread
                printf("The thread terminated early - something bad happened\n");
                bKeepWaiting = false;
                break;

            case WAIT_OBJECT_0 + 1: // hStdIn
                // see if any of them was an Enter key-up event
                INPUT_RECORD rInput[128];
                DWORD nEvents;
                if (!ReadConsoleInput(hStdIn, rInput, ARRAYSIZE(rInput), &nEvents)) {
                    printf("ReadConsoleInput failed: last error is %u\n", GetLastError());
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
                            printf("Stopping capture...\n");
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
                printf("WaitForMultipleObjects returned unexpected value 0x%08x\n", dwWaitResult);
                SetEvent(hStopEvent);
                WaitForSingleObject(hThread, INFINITE);
                bKeepWaiting = false;
                break;
        }
    }

    DWORD exitCode;
    if (!GetExitCodeThread(hThread, &exitCode)) {
        printf("GetExitCodeThread failed: last error is %u\n", GetLastError());
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    if (0 != exitCode) {
        printf("Loopback capture thread exit code is %u; expected 0\n", exitCode);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return -__LINE__;
    }

    if (S_OK != threadArgs.hr) {
        printf("Thread HRESULT is 0x%08x\n", threadArgs.hr);
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
        printf("mmioClose failed: MMSYSERR = %u\n", result);
        return -__LINE__;
    }

    // reopen the file in read/write mode
    MMIOINFO mi = {0};
    prefs.m_hFile = mmioOpen(const_cast<LPWSTR>(prefs.m_szFilename), &mi, MMIO_READWRITE);
    if (NULL == prefs.m_hFile) {
        printf("mmioOpen(\"%ls\", ...) failed. wErrorRet == %u\n", prefs.m_szFilename, mi.wErrorRet);
        return -__LINE__;
    }

    // descend into the RIFF/WAVE chunk
    MMCKINFO ckRIFF = {0};
    ckRIFF.ckid = MAKEFOURCC('W', 'A', 'V', 'E'); // this is right for mmioDescend
    result = mmioDescend(prefs.m_hFile, &ckRIFF, NULL, MMIO_FINDRIFF);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioDescend(\"WAVE\") failed: MMSYSERR = %u\n", result);
        return -__LINE__;
    }

    // descend into the fact chunk
    MMCKINFO ckFact = {0};
    ckFact.ckid = MAKEFOURCC('f', 'a', 'c', 't');
    result = mmioDescend(prefs.m_hFile, &ckFact, &ckRIFF, MMIO_FINDCHUNK);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioDescend(\"fact\") failed: MMSYSERR = %u\n", result);
        return -__LINE__;
    }

    // write the correct data to the fact chunk
    LONG lBytesWritten = mmioWrite(
        prefs.m_hFile,
        reinterpret_cast<PCHAR>(&threadArgs.nFrames),
        sizeof(threadArgs.nFrames)
    );
    if (lBytesWritten != sizeof(threadArgs.nFrames)) {
        printf("Updating the fact chunk wrote %u bytes; expected %u\n", lBytesWritten, (UINT32)sizeof(threadArgs.nFrames));
        return -__LINE__;
    }

    // ascend out of the fact chunk
    result = mmioAscend(prefs.m_hFile, &ckFact, 0);
    if (MMSYSERR_NOERROR != result) {
        printf("mmioAscend(\"fact\") failed: MMSYSERR = %u\n", result);
        return -__LINE__;
    }

    // let prefs' destructor call mmioClose
    
    return 0;
}
