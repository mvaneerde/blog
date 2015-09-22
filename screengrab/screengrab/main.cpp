// main.cpp
// most code cribbed from
// "Capturing an Image"
// http://msdn.microsoft.com/en-us/library/dd183402(v=VS.85).aspx

#include <windows.h>
#include <stdio.h>

#include <functional>

#define LOG(format, ...) wprintf( format L"\n", __VA_ARGS__)
#define ERR(format, ...) LOG( L"ERROR: " format, __VA_ARGS__)

// Transparent RAII cleanup
class OnExit {
public:
    OnExit(std::function<void(void)> f) : m_f(f) {}
    ~OnExit() { m_f(); }
    
private:
    std::function<void(void)> m_f;
};

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    if (1 != argc - 1) {
        ERR(L"Expected one argument (the .bmp filename to save to), not %u", argc - 1);
        return -__LINE__;
    }
    
    LPCWSTR szFilename = argv[1];
    
    // grab a device context for the screen
    // we will read from this
    HDC hdcScreen = GetDC(nullptr);
    if (nullptr == hdcScreen) {
        ERR(L"GetDC failed: error %u", GetLastError());
        return -__LINE__;
    }
    OnExit oeReleaseDc([=](){ ReleaseDC(nullptr, hdcScreen); });
    
    // create an in-memory device context
    // we will write to this
    HDC hdcMemory = CreateCompatibleDC(hdcScreen);
    if (nullptr == hdcMemory) {
        ERR(L"CreateCompatibleDC failed: error %u", GetLastError());
        return -__LINE__;
    }
    OnExit oeDeleteDC([=](){ DeleteDC(hdcMemory); });
    
    // get the size of the screen
    // (actually, just the primary monitor)
    //
    // potential for further development: create a bitmap for each monitor
    // or a bitmap which contains the entire virtual screen
    // see MonitorEnumProc on MSDN for details
    HWND hwndDesktop = GetDesktopWindow();
    RECT rectScreen;
    if (!GetClientRect(hwndDesktop, &rectScreen)) {
        ERR(L"GetClientRect(GetDesktopWindow) failed: error %u", GetLastError());
        return -__LINE__;
    }

    LOG(
        L"Desktop dimensions are %u wide by %u high",
        rectScreen.right - rectScreen.left,
        rectScreen.bottom - rectScreen.top
    );

    // create a compatible bitmap using the screen device context
    HBITMAP hbmScreen = CreateCompatibleBitmap(
        hdcScreen,
        rectScreen.right - rectScreen.left,
        rectScreen.bottom - rectScreen.top
    );
    if (nullptr == hbmScreen) {
        ERR(L"CreateCompatibleBitmap failed: error %u", GetLastError());
        return -__LINE__;
    }
    OnExit oeDeleteBitmap([=](){ DeleteObject(hbmScreen); });
    
    // select the compatible bitmap into the compatible dc
    SelectObject(hdcMemory, hbmScreen);
    
    // copy the bits from the screen into memory
    if (!BitBlt(
        hdcMemory, // destination
        0, 0, // upper left corner
        rectScreen.right - rectScreen.left, rectScreen.bottom - rectScreen.top, // width and height
        hdcScreen, // source
        0, 0, // upper left corner
        // width and height are assumed
        SRCCOPY
    )) {
        ERR(L"BitBlt failed: error = %u", GetLastError());
        return -__LINE__;
    }
    
    // put the bits into a bitmap shape
    BITMAP bmp = {0};
    GetObject(hbmScreen, sizeof(bmp), &bmp);

    BITMAPINFOHEADER bmih = {0};
    bmih.biSize = sizeof(bmih);
    bmih.biWidth = bmp.bmWidth;
    bmih.biHeight = bmp.bmHeight;
    bmih.biPlanes = 1;
    bmih.biBitCount = 32;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = 0;
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    // allocate the necessary memory
    DWORD bytesBmp = (bmp.bmWidth * bmih.biBitCount + 31) / 32 * 4 * bmp.bmHeight;
    HANDLE hDIB = GlobalAlloc(GHND, bytesBmp);
    if (nullptr == hDIB) {
        ERR(L"Could not allocate %u bytes", bytesBmp);
        return -__LINE__;
    }
    OnExit oeFreeMemory([=](){ GlobalFree(hDIB); });

    BYTE *pbBitmap = (BYTE*)GlobalLock(hDIB);
    OnExit oeUnlock([=](){ GlobalUnlock(hDIB); });
    
    // get the DIB data
    GetDIBits(
        hdcScreen,
        hbmScreen,
        0, // start at line 0
        bmp.bmHeight,
        pbBitmap,
        (BITMAPINFO *)&bmih,
        DIB_RGB_COLORS
    );
    
    // write the file
    HANDLE hFile = CreateFile(
        szFilename,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (nullptr == hFile) {
        ERR(L"CreateFile failed: err = %u", GetLastError());
        return -__LINE__;
    }
    OnExit oeCloseFile([=](){ CloseHandle(hFile); });
    
    BITMAPFILEHEADER bmfh = {0};
    bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih);
    bmfh.bfSize = bytesBmp + bmfh.bfOffBits;
    bmfh.bfType = 0x4D42; // BM
    
    DWORD dwBytesWritten;
    if (!WriteFile(hFile, &bmfh, sizeof(bmfh), &dwBytesWritten, nullptr)) {
        ERR(L"WriteFile failed: %u", GetLastError());
        return -__LINE__;
    }
    if (sizeof(bmfh) != dwBytesWritten) {
        ERR(L"Expected %Iu bytes to be written, not %u", sizeof(bmfh), dwBytesWritten);
        return -__LINE__;
    }
    
    if (!WriteFile(hFile, &bmih, sizeof(bmih), &dwBytesWritten, nullptr)) {
        ERR(L"WriteFile failed: %u", GetLastError());
        return -__LINE__;
    }
    if (sizeof(bmih) != dwBytesWritten) {
        ERR(L"Expected %Iu bytes to be written, not %u", sizeof(bmih), dwBytesWritten);
        return -__LINE__;
    }
    
    if (!WriteFile(hFile, pbBitmap, bytesBmp, &dwBytesWritten, nullptr)) {
        ERR(L"WriteFile failed: %u", GetLastError());
        return -__LINE__;
    }
    if (bytesBmp != dwBytesWritten) {
        ERR(L"Expected %u bytes to be written, not %u", bytesBmp, dwBytesWritten);
        return -__LINE__;
    }

    return 0;
}