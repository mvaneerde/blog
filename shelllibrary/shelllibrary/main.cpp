// main.cpp

#include <initguid.h>
#include <windows.h>
#include <cguid.h>
#include <atlbase.h>
#include <knownfolders.h>
#include <shobjidl.h>
#include <stdio.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

struct Library {
    LPCWSTR name;
    GUID guid;
};

Library known[] = {
    { L"documents",   FOLDERID_DocumentsLibrary  },
    { L"music",       FOLDERID_MusicLibrary      },
    { L"pictures",    FOLDERID_PicturesLibrary   },
    { L"videos",      FOLDERID_VideosLibrary     },
    { L"recorded tv", FOLDERID_RecordedTVLibrary },
};

void usage() {
    LOG(
        L"shelllibrary add <path> to <library>\n"
        L"    <path> must already exist\n"
        L"    <library> must be one of:"
    );
    
    for (int i = 0; i < ARRAYSIZE(known); i++) {
        LOG(L"        %s", known[i].name);
    }
}

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() {
        CoUninitialize();
    }
};

int _cdecl wmain(int argc, _In_reads_(argc) LPCWSTR argv[]) {

    if (0 == argc - 1) {
        usage();
        return 0;
    }

    if (4 != argc - 1) {
        LOG(L"Expected four arguments, not %d", argc - 1);
        return -__LINE__;
    }
    
    if (0 != _wcsicmp(L"add", argv[1])) {
        LOG(L"First argument must be \"add\", not %s", argv[1]);
        return -__LINE__;
    }
    
    LPCWSTR path = argv[2];
    
    if (0 != _wcsicmp(L"to", argv[3])) {
        LOG(L"Third argument must be \"to\", not %s", argv[3]);
        return -__LINE__;
    }
    
    GUID library = {0};
    bool found = false;
    LPCWSTR key = argv[4];
    for (int i = 0; i < ARRAYSIZE(known); i++) {
        if (0 == _wcsicmp(key, known[i].name)) {
            library = known[i].guid;
            found = true;
            break;
        }
    }
    
    if (!found) {
        LOG(L"Unrecognized library %s", key);
        return -__LINE__;
    }
    
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        LOG(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;
    
    CComPtr<IShellLibrary> pShellLibrary;
    hr = SHLoadLibraryFromKnownFolder(
        library,
        STGM_READWRITE,
        IID_PPV_ARGS(&pShellLibrary)
    );
    if (FAILED(hr)) {
        LOG(L"SHLoadLibraryFromKnownFolder failed: hr = 0x%08x", hr);
        return -__LINE__;
    }

    hr = SHAddFolderPathToLibrary(
        pShellLibrary,
        path
    );
    if (FAILED(hr)) {
        LOG(L"SHAddFolderPathToLibrary failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    hr = pShellLibrary->Commit();
    if (FAILED(hr)) {
        LOG(L"IShellLibrary::Commit failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    LOG(L"Added %s to %s library", path, key);
    
    return 0;
}