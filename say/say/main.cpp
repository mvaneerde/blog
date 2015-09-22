// main.cpp

#include <windows.h>
#include <stdio.h>
#include <sapi.h>
#include <shlwapi.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) wprintf(L"ERROR:" format L"\n", __VA_ARGS__)

#define CHANNELS 2
#define SAMPLE_RATE 16000
#define BIT_DEPTH 16
#define FRAMES_PER_PASS 4

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() { CoUninitialize(); }
};

class ReleaseOnExit {
public:
    ReleaseOnExit(IUnknown *p) : m_p(p) {}
    ~ReleaseOnExit() {
        if (m_p) {
            m_p->Release();
        }
    }
private:
    IUnknown *m_p;
};

int _cdecl wmain(int argc, LPCWSTR argv[]) {

    if (1 == argc) {
        LOG(
            L"say \"phrase\" [--file <filename> | --stream]\n"
            L"runs phrase through text-to-speech engine\n"
            L"if --file is specified, writes to .wav file\n"
            L"if --stream is specified, captures to a stream\n"
            L"if neither is specified, plays to default output\n"
        );
        return 0;
    }
    
    enum Output {
        Speakers,
        File,
        Stream
    };
    
    Output where;
    LPCWSTR file = nullptr;
    
    if (2 == argc) {
        where = Speakers;
    } else if (4 == argc && 0 == _wcsicmp(argv[2], L"--file")) {
        where = File;
        file = argv[3];
    } else if (3 == argc && 0 == _wcsicmp(argv[2], L"--stream")) {
        where = Stream;
    } else {
        ERR(L"Invalid command line");
        return -__LINE__;
    }
    
    LPCWSTR text = argv[1];

    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;
    
    ISpVoice *pSpVoice = nullptr;
    hr = CoCreateInstance(
        CLSID_SpVoice, nullptr, CLSCTX_ALL, 
        __uuidof(ISpVoice),
        (void**)&pSpVoice
    );
    if (FAILED(hr)) {
        ERR(L"CoCreateInstance(ISpVoice) failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    ReleaseOnExit rSpVoice(pSpVoice);
    
    WAVEFORMATEX fmt = {};
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = CHANNELS;
    fmt.nSamplesPerSec = SAMPLE_RATE;
    // nAvgBytesPerSec is calculated
    // nBlockAlign is calculated
    fmt.wBitsPerSample = BIT_DEPTH;
    fmt.cbSize = 0;

    fmt.nBlockAlign = fmt.nChannels * fmt.wBitsPerSample / 8;
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

    IStream *pStream = NULL;
    
    switch (where) {
    
        case Speakers: {
            break;
        }
        
        case File:
        case Stream: {
            ISpStream *pSpStream = nullptr;
            hr = CoCreateInstance(
                CLSID_SpStream, nullptr, CLSCTX_ALL, 
                __uuidof(ISpStream),
                (void**)&pSpStream
            );
            if (FAILED(hr)) {
                ERR(L"CoCreateInstance(ISpVoice) failed: hr = 0x%08x", hr);
                return -__LINE__;
            }
            ReleaseOnExit rSpStream(pSpStream);
            
            if (File == where) {
                hr = pSpStream->BindToFile(
                    file,
                    SPFM_CREATE_ALWAYS,
                    &SPDFID_WaveFormatEx,
                    &fmt,
                    0
                );
                if (FAILED(hr)) {
                    ERR(L"ISpStream::BindToFile failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
            } else {
                // stream
                pStream = SHCreateMemStream(NULL, 0);
                if (nullptr == pStream) {
                    ERR(L"SHCreateMemStream failed");
                    return -__LINE__;
                }
                
                hr = pSpStream->SetBaseStream(
                    pStream,
                    SPDFID_WaveFormatEx,
                    &fmt
                );
                if (FAILED(hr)) {
                    ERR(L"ISpStream::SetBaseStream failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
            }
            
            hr = pSpVoice->SetOutput(pSpStream, TRUE);
            if (FAILED(hr)) {
                ERR(L"ISpVoice::SetOutput failed: hr = 0x%08x", hr);
                return -__LINE__;
            }

            break;
        }
    }
    
    ReleaseOnExit rStream(pStream);
    
    ULONG stream = 0;
    hr = pSpVoice->Speak(text, SPF_DEFAULT, &stream);
    if (FAILED(hr)) {
        ERR(L"ISpVoice::Speak failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    LOG(L"Stream is %u", stream);

    if (Stream == where) {
        // the stream has stereo 16-bit data
        // read 128 samples at a time
        static_assert(BIT_DEPTH == 16, "assuming int16");
        INT16 chunk[128 * CHANNELS];
        ULONG read = 0;
        
        // SAPI wrote to the stream and filled it up
        // but the position is still at the end
        // so we need to rewind back to the beginning
        // before we can read it
        LARGE_INTEGER zero = {0};
        hr = pStream->Seek(zero, STREAM_SEEK_SET, NULL);
        if (FAILED(hr)) {
            ERR(L"IStream::Seek failed: hr = 0x%08x", hr);
            return -__LINE__;
        }

        for (bool eos = false; !eos; ) {
            hr = pStream->Read(chunk, sizeof(chunk), &read);
            if (FAILED(hr)) {
                ERR(L"IStream::Read failed: hr = 0x%08x", hr);
                return -__LINE__;
            }
            
            // LOG(L"IStream::Read returned 0x%08x and read %u bytes", hr, read);
            // Be careful - some stream implementations signal EOS differently
            eos = (S_FALSE == hr);

            if (0 != read % fmt.nBlockAlign) {
                ERR(L"IStream::Read returned a non-aligned chunk: %u bytes", read);
                return -__LINE__;
            }
            
            int i = 0;
            ULONG logged = 0;
            while (logged < read) {
                ULONG framesLeft = (read - logged) / fmt.nBlockAlign;
                ULONG framesLoggedThisPass = framesLeft;
                
                if (framesLeft > FRAMES_PER_PASS) {
                    framesLoggedThisPass = FRAMES_PER_PASS;
                }
                
                static_assert(CHANNELS == 2, "assuming stereo");
                switch (framesLoggedThisPass) {
                    case 1:
                        LOG(L"% 8d % 8d", chunk[i], chunk[i + 1]);
                        break;

                    case 2:
                        LOG(L"% 8d % 8d; % 8d % 8d",
                            chunk[i], chunk[i + 1],
                            chunk[i + 2], chunk[i + 3]
                        );
                        break;
                        
                    case 3:
                        LOG(L"% 8d % 8d; % 8d % 8d; % 8d % 8d",
                            chunk[i], chunk[i + 1],
                            chunk[i + 2], chunk[i + 3],
                            chunk[i + 4], chunk[i + 5]
                        );
                        break;

                    static_assert(FRAMES_PER_PASS == 4, "assuming four frames per pass");
                    case 4:
                        LOG(L"% 8d % 8d; % 8d % 8d; % 8d % 8d; % 8d % 8d",
                            chunk[i], chunk[i + 1],
                            chunk[i + 2], chunk[i + 3],
                            chunk[i + 4], chunk[i + 5],
                            chunk[i + 6], chunk[i + 7]
                        );
                        break;
                }
                
                i += framesLoggedThisPass * CHANNELS;
                logged += framesLoggedThisPass * fmt.nBlockAlign;
            }
        }
    }
    
    return 0;
}