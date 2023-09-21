// create.cpp

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <dsound.h>

#include "log.h"
#include "cleanup.h"
#include "create.h"

HRESULT Create(bool v8, bool render, LPCGUID deviceId) {
    HRESULT hr;

    if (v8) {
        if (render) {
            LPDIRECTSOUND8 directSound = nullptr;
            hr = DirectSoundCreate8(deviceId, &directSound, nullptr);
            if (FAILED(hr)) {
                ERR(L"DirectSoundCreate8 failed: 0x%08x", hr);
            }
        } else {
            LPDIRECTSOUNDCAPTURE8 directSound = nullptr;
            hr = DirectSoundCaptureCreate8(deviceId, &directSound, nullptr);
            if (FAILED(hr)) {
                ERR(L"DirectSoundCaptureCreate8 failed: 0x%08x", hr);
            }
        }
    } else {
        if (render) {
            LPDIRECTSOUND directSound = nullptr;
            hr = DirectSoundCreate(deviceId, &directSound, nullptr);
            if (FAILED(hr)) {
                ERR(L"DirectSoundCreate failed: 0x%08x", hr);
            }
        }
        else {
            LPDIRECTSOUNDCAPTURE directSound = nullptr;
            hr = DirectSoundCaptureCreate(deviceId, &directSound, nullptr);
            if (FAILED(hr)) {
                ERR(L"DirectSoundCaptureCreate failed: 0x%08x", hr);
            }
        }
    }

    return hr;
}
