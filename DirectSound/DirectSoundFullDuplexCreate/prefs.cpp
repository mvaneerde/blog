// prefs.cpp

#include <windows.h>
#include <objbase.h>
#include <stdio.h>

#include "log.h"
#include "prefs.h"
#include "cleanup.h"

LPCGUID Prefs::RenderDeviceId() {
    return (_specificRenderDevice ? &_renderDeviceId : nullptr);
}

LPCGUID Prefs::CaptureDeviceId() {
    return (_specificCaptureDevice ? &_captureDeviceId : nullptr);
}

HRESULT Prefs::Set(int argc, LPCWSTR argv[]) {
    // usage statement
    if (
        argc == 2 &&
        (
            0 == _wcsicmp(argv[1], L"-?") ||
            0 == _wcsicmp(argv[1], L"/?")
            )
        ) {
        LOG(
            L"DirectSoundFullDuplexCreate [-renderDevice <deviceId>] [-captureDevice <deviceId>]\r\n"
            L"    -renderDevice: specify a DirectSound device ID as returned by DirectSoundEnumerate\r\n"
            L"    -captureDevice: specify a DirectSound device ID as returned by DirectSoundEnumerate"
        );
        return S_FALSE;
    }

    // read parameters
    bool seenRenderDevice = false;
    bool seenCaptureDevice = false;

    for (int i = 1; i < argc; i++) {
        // -renderDevice
        if (
            0 == _wcsicmp(argv[i], L"-renderDevice") ||
            0 == _wcsicmp(argv[i], L"/renderDevice")
            ) {
            if (seenRenderDevice) {
                ERR(L"Specify -renderDevice at most once");
                return E_INVALIDARG;
            }

            seenRenderDevice = true;
            _specificRenderDevice = true;

            i++;
            if (i == argc) {
                ERR(L"-renderDevice requires a device ID");
                return E_INVALIDARG;
            }

            HRESULT hr = CLSIDFromString(argv[i], &_renderDeviceId);
            if (FAILED(hr)) {
                ERR(L"CLSIDFromString returned failure 0x%08x", hr);
                return hr;
            }
            continue;
        }

        // -captureDevice
        if (
            0 == _wcsicmp(argv[i], L"-captureDevice") ||
            0 == _wcsicmp(argv[i], L"/captureDevice")
            ) {
            if (seenCaptureDevice) {
                ERR(L"Specify -captureDevice at most once");
                return E_INVALIDARG;
            }

            seenCaptureDevice = true;
            _specificCaptureDevice = true;

            i++;
            if (i == argc) {
                ERR(L"-captureDevice requires a device ID");
                return E_INVALIDARG;
            }

            HRESULT hr = CLSIDFromString(argv[i], &_captureDeviceId);
            if (FAILED(hr)) {
                ERR(L"CLSIDFromString returned failure 0x%08x", hr);
                return hr;
            }
            continue;
        }

        ERR(L"Unrecognized argument %s", argv[i]);
        return E_INVALIDARG;
    }

    return S_OK;
}
