// main.cpp

#include <windows.h>
#include <stdio.h>

#include <sapi.h>

#pragma warning ( push )
#pragma warning ( disable : 4996 ) // SAPI uses deprecated GetVersionEx
#include <sphelper.h>
#pragma warning ( pop )

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) wprintf(L"ERROR:" format L"\n", __VA_ARGS__)

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() { CoUninitialize(); }
};

class ReleaseOnExit {
public:
    ReleaseOnExit(IUnknown *p) : m_p(p) {}
    ~ReleaseOnExit() { if (nullptr != m_p) { m_p->Release(); } }
private:
    IUnknown *m_p;
};

class CoTaskMemFreeOnExit {
public:
    CoTaskMemFreeOnExit(PVOID p) : m_p(p) {}
    ~CoTaskMemFreeOnExit() { CoTaskMemFree(m_p); }
private:
    PVOID m_p;
};

class SpClearEventOnExit {
public:
    SpClearEventOnExit(SPEVENT *pEvent) : m_pEvent(pEvent) {}
    ~SpClearEventOnExit() { SpClearEvent(m_pEvent); }
private:
    SPEVENT *m_pEvent;
};

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;
    
    ISpStream *pSpStream = nullptr;

    switch (argc - 1) {
        case 0: // fine
            break;
        
        case 1:
            if (0 == wcscmp(L"-?", argv[1]) || 0 == wcscmp(L"/?", argv[1])) {
                LOG(L"listen.exe [--file filename]\nConverts speech to text");
                return 0;
            } else {
                ERR(L"Unrecognized argument %s", argv[1]);
                return -__LINE__;
            }
            
        case 2:
            if (0 == _wcsicmp(L"--file", argv[1])) {
                hr = CoCreateInstance(
                    CLSID_SpStream, nullptr, CLSCTX_ALL, 
                    __uuidof(ISpStream),
                    (void**)&pSpStream
                );
                if (FAILED(hr)) {
                    ERR(L"CoCreateInstance(ISpVoice) failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                hr = pSpStream->BindToFile(
                    argv[2],
                    SPFM_OPEN_READONLY,
                    nullptr,
                    nullptr,
                    0
                );
                if (FAILED(hr)) {
                    ERR(L"ISpStream::BindToFile failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
            } else {
                ERR(L"Unrecognized argument %s", argv[1]);
                return -__LINE__;
            }
            break;
            
        default:
            ERR(L"Expected no arguments or two, not %d", argc - 1);
            return -__LINE__;
    }
    
    ReleaseOnExit rSpStream(pSpStream);

    // grab either the shared or the inproc recording context
    ISpRecognizer *pSpRecognizer = nullptr;
    hr = CoCreateInstance(
        (nullptr == pSpStream ? CLSID_SpSharedRecoContext : CLSID_SpInprocRecognizer),
        nullptr, CLSCTX_ALL, 
        __uuidof(ISpRecognizer),
        (void**)&pSpRecognizer
    );
    if (FAILED(hr)) {
        ERR(L"CoCreateInstance(ISpRecognizer) failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    ReleaseOnExit rSpRecognizer(pSpRecognizer);

    // pull data from a file if one was passed in
    if (nullptr != pSpStream) {
        hr = pSpRecognizer->SetInput(pSpStream, TRUE); // allow format conversion
        if (FAILED(hr)) {
            ERR(L"ISpRecognizer::SetInput failed: hr = 0x%08x", hr);
            return -__LINE__;
        }
    }

    // create a recording context
    ISpRecoContext *pSpRecoContext = nullptr;
    hr = pSpRecognizer->CreateRecoContext(&pSpRecoContext);
    if (FAILED(hr)) {
        ERR(L"ISpRecognizer::CreateRecoContext failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    ReleaseOnExit rSpRecoContext(pSpRecoContext);
        
    // tell it what we're interested in
    ULONGLONG events =
        SPFEI(SPEI_RECOGNITION) |
        SPFEI(SPEI_END_SR_STREAM);
    ULONGLONG queues = events;
    hr = pSpRecoContext->SetInterest(
        events,
        queues
    );
    if (FAILED(hr)) {
        ERR(L"ISpRecoContext::SetInterest failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    // set it to send notification events
    hr = pSpRecoContext->SetNotifyWin32Event();
    if (FAILED(hr)) {
        ERR(L"ISpRecoContext::SetNotifyWin32Event failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    // create a grammar
    ULONGLONG idListenGrammar = 0x3BC8AA80;
    ISpRecoGrammar *pSpRecoGrammar = nullptr;
    hr = pSpRecoContext->CreateGrammar(idListenGrammar, &pSpRecoGrammar);
    if (FAILED(hr)) {
        ERR(L"ISpRecoContext::CreateGrammar failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    ReleaseOnExit rSpRecoGrammar(pSpRecoGrammar);
    
    // set the grammar for dictation
    hr = pSpRecoGrammar->LoadDictation(
        nullptr, // general dictation
        SPLO_STATIC
    );
    if (FAILED(hr)) {
        ERR(L"ISpRecoGrammar::LoadDictation failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    // activate dictation
    hr = pSpRecoGrammar->SetDictationState(SPRS_ACTIVE);
    if (FAILED(hr)) {
        ERR(L"ISpRecoGrammar::SetDictationState failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    // we will listen for console input (Enter to exit) and for speech input
    HANDLE handles[] = {
        GetStdHandle(STD_INPUT_HANDLE),
        pSpRecoContext->GetNotifyEventHandle(),
    };

    if (nullptr == pSpStream) {
        LOG(L"Speak into the microphone naturally; I will print what I understand.");
        LOG(L"Press ENTER to quit.");
    }

    bool done = false;
    while (!done) {
        DWORD dwResult = WaitForMultipleObjectsEx(
            ARRAYSIZE(handles), handles,
            FALSE, // any handle will wake us up
            INFINITE,
            FALSE // not alertable
        );
        switch (dwResult) {
            case WAIT_OBJECT_0: { // console input
                // see if any of them was an Enter key-up event
                INPUT_RECORD rInput[128] = {};
                DWORD nEvents = 0;
                if (!ReadConsoleInput(handles[0], rInput, ARRAYSIZE(rInput), &nEvents)) {
                    ERR(L"ReadConsoleInput failed: error %u\n", GetLastError());
                    return -__LINE__;
                }
                
                for (DWORD i = 0; i < nEvents; i++) {
                    if (
                        KEY_EVENT == rInput[i].EventType &&
                        VK_RETURN == rInput[i].Event.KeyEvent.wVirtualKeyCode &&
                        !rInput[i].Event.KeyEvent.bKeyDown
                    ) {
                        done = true;
                    }
                }
                
                break;
            }
                
            case WAIT_OBJECT_0 + 1: { // speech input
                // there are speech events available; grab them
                for (;;) {
                    SPEVENT event; SpInitEvent(&event);
                    SpClearEventOnExit sceoe(&event);
                    
                    hr = pSpRecoContext->GetEvents(1, &event, nullptr);
                    if (S_FALSE == hr) {
                        break;
                    }
                    
                    if (FAILED(hr)) {
                        ERR(L"ISpRecoContext::GetEvents failed: hr = 0x%08x", hr);
                        return -__LINE__;
                    }
                    
                    switch (event.eEventId) {
                        case SPEI_END_SR_STREAM:
                            done = true;
                            break;

                        case SPEI_RECOGNITION: {
                            if (0 == event.lParam) {
                                ERR(L"SPEI_RECOGNITION event received with null .lparam");
                                break;
                            }
                            
                            // we release this in SpClearEvent
                            ISpRecoResult *pSpRecoResult = reinterpret_cast<ISpRecoResult *>(event.lParam);
                            WCHAR *szText = nullptr;
                            BYTE attributes = 0;
                            HRESULT hr = pSpRecoResult->GetText(
                                (ULONG)SP_GETWHOLEPHRASE, (ULONG)SP_GETWHOLEPHRASE,
                                TRUE, // replace symbols: e.g. "20 dollars" => "$20"
                                &szText,
                                &attributes
                            );
                            if (FAILED(hr)) {
                                ERR(L"ISpRecoResult::GetText failed: hr = 0x%08x", hr);
                                return -__LINE__;
                            }
                            CoTaskMemFreeOnExit ctmfoe(szText);
                            
                            LOG(L"%s", szText);
                            break;
                        }
                            
                        default:
                            ERR(L"Unexpected event raised: event.eEventId = %u", event.eEventId);
                            break;
                    }
                } // for each event
                break;
            }
                
            default: {
                ERR(L"Wait returned 0x%08x; last error is %u", dwResult, GetLastError());
                return -__LINE__;
            }
        }

    } // wait on handles again
    
    return 0;
}