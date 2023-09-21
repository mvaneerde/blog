// enumerate.cpp

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <dsound.h>

#include "log.h"
#include "enumerate.h"
#include "cleanup.h"

BOOL CALLBACK LogDirectSoundDevice(
    LPGUID guid,
    LPCWSTR description,
    LPCWSTR module,
    LPVOID /*context*/
) {
    LPWSTR szGuid = nullptr;

    if (guid != nullptr) {
        HRESULT hr = StringFromIID(*guid, &szGuid);
        if (FAILED(hr)) {
            ERR(L"StringFromIID failed: hr = 0x%08x", hr);
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }
    }

    CoTaskMemFreeOnExit freeGuid(szGuid);

    LOG(
        L"    DirectSound device address: %s\r\n"
        L"    Description: %s\r\n"
        L"    Module: %s\r\n",
        (guid == nullptr ? L"(null pointer)" : szGuid),
        description,
        module
    );
    return TRUE;
}

HRESULT EnumerateDirectSound() {
    LOG(L"DirectSound playback devices:");
    HRESULT hr = DirectSoundEnumerate(LogDirectSoundDevice, nullptr);
    if (FAILED(hr)) {
        ERR(L"DirectSoundEnumerate failed: 0x%08x", hr);
        return hr;
    }

    LOG(L"DirectSound recording devices:");
    hr = DirectSoundCaptureEnumerate(LogDirectSoundDevice, nullptr);
    if (FAILED(hr)) {
        ERR(L"DirectSoundCaptureEnumerate failed: 0x%08x", hr);
        return hr;
    }

    return S_OK;
}