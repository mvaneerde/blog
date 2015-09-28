// main.cpp

#include <windows.h>
#include <stdio.h>
#include <mmdeviceapi.h>

#include "prefs.h"
#include "silence.h"

int do_everything(int argc, LPCWSTR argv[]);

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    HRESULT hr = S_OK;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("CoInitialize failed: hr = 0x%08x", hr);
        return __LINE__;
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
        return __LINE__;
    }
    if (S_FALSE == hr) {
        // nothing to do
        return 0;
    }

    // create a "silence has started playing" event
    HANDLE hStartedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hStartedEvent) {
        printf("CreateEvent failed: last error is %u\n", GetLastError());
        return __LINE__;
    }

    // create a "stop playing silence now" event
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hStopEvent) {
        printf("CreateEvent failed: last error is %u\n", GetLastError());
        CloseHandle(hStartedEvent);
        return __LINE__;
    }

    // create arguments for silence-playing thread
    PlaySilenceThreadFunctionArguments threadArgs;
    threadArgs.hr = E_UNEXPECTED; // thread will overwrite this
    threadArgs.hStartedEvent = hStartedEvent;
    threadArgs.hStopEvent = hStopEvent;
    threadArgs.pMMDevice = prefs.m_pMMDevice;

    HANDLE hThread = CreateThread(
        NULL, 0,
        PlaySilenceThreadFunction, &threadArgs,
        0, NULL
    );
    if (NULL == hThread) {
        printf("CreateThread failed: last error is %u\n", GetLastError());
        CloseHandle(hStopEvent);
        CloseHandle(hStartedEvent);
        return __LINE__;
    }

    // wait for either silence to start or the thread to end
    HANDLE waitArray[2] = { hStartedEvent, hThread };
    DWORD dwWaitResult;
    dwWaitResult = WaitForMultipleObjects(
        ARRAYSIZE(waitArray), waitArray,
        FALSE, INFINITE
    );

    if (WAIT_OBJECT_0 + 1 == dwWaitResult) {
        printf("Thread aborted before starting to play silence: hr = 0x%08x\n", threadArgs.hr);
        CloseHandle(hStartedEvent);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return __LINE__;
    }

    if (WAIT_OBJECT_0 != dwWaitResult) {
        printf("Unexpected WaitForMultipleObjects return value %u: last error is %u\n", dwWaitResult, GetLastError());
        CloseHandle(hStartedEvent);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return __LINE__;
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
        return __LINE__;
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
                            printf("Aborting playback...\n");
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
        return __LINE__;
    }

    if (0 != exitCode) {
        printf("Silence thread exit code is %u; expected 0\n", exitCode);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return __LINE__;
    }

    if (S_OK != threadArgs.hr) {
        printf("Thread HRESULT is 0x%08x\n", threadArgs.hr);
        CloseHandle(hThread);
        CloseHandle(hStopEvent);
        return __LINE__;
    }

    CloseHandle(hThread);
    CloseHandle(hStopEvent);
    return 0;
}
