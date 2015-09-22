// main.cpp

#include <windows.h>
#include <shobjidl.h>
#include <stdio.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

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

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    if (1 != argc - 1) {
        LOG(L"expected a single argument, not %d\n", argc - 1);
        return -__LINE__;
    }
    
    // coinitialize
    // cocreate a desktop wallpaper object
    // call IDesktopWallpaper->SetWallpaper
    
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        LOG(L"CoInitialize returned 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;

    IDesktopWallpaper *pDesktopWallpaper = nullptr;
    hr = CoCreateInstance(__uuidof(DesktopWallpaper), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pDesktopWallpaper));
    if (FAILED(hr)) {
        LOG(L"CoCreateInstance(__uuidof(DesktopWallpaper)) returned 0x%08x", hr);
        return -__LINE__;
    }
    ReleaseOnExit releaseDesktopWallpaper(pDesktopWallpaper);
    
    LPCWSTR wallpaper = argv[1];
    hr = pDesktopWallpaper->SetWallpaper(nullptr /* all monitors */, wallpaper);
    if (FAILED(hr)) {
        LOG(L"IDesktopWallpaper::SetWallpaper returned 0x%08x", hr);
        return -__LINE__;
    }
        
    LOG(L"Setting the desktop wallpaper to %s succeeded.\n", wallpaper);
    return 0;
}