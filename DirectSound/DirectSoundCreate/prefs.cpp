#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <dsound.h>

#include "log.h"
#include "prefs.h"
#include "cleanup.h"

bool Prefs::V8() {
    return _v8;
}

bool Prefs::Render() {
    return !_capture;
}

LPCGUID Prefs::DeviceId() {
    return (_useDeviceId ? &_deviceId : nullptr);
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
            L"DirectSoundCreate [-8] [-capture] [-device default | voice | <deviceId>]\r\n"
            L"    -8: uses DirectSoundCreate8 or DirectSoundCapture8\r\n"
            L"    -capture: opens a recording stream\r\n"
            L"    -device: specify which speaker or microphone to use"
        );
        return S_FALSE;
    }

    // read parameters
    bool seenV8 = false;
    bool seenCapture = false;
    bool seenDevice = false;
    
    DeviceOption deviceOption = useNull;
    
    for (int i = 1; i < argc; i++) {
        // -8
        if (
            0 == _wcsicmp(argv[i], L"-8") ||
            0 == _wcsicmp(argv[i], L"/8")
        ) {
            if (seenV8) {
                ERR(L"Specify -8 at most once");
                return E_INVALIDARG;
            }

            seenV8 = true;
            _v8 = true;
            continue;
        }

        // -capture
        if (
            0 == _wcsicmp(argv[i], L"-capture") ||
            0 == _wcsicmp(argv[i], L"/capture")
        ) {
            if (seenCapture) {
                ERR(L"Specify -capture at most once");
                return E_INVALIDARG;
            }

            seenCapture = true;
            _capture = true;
            continue;
        }

        // -device
        if (
            0 == _wcsicmp(argv[i], L"-device") ||
            0 == _wcsicmp(argv[i], L"/device")
        ) {
            if (seenDevice) {
                ERR(L"Specify -device at most once");
                return E_INVALIDARG;
            }

            seenDevice = true;
            _useDeviceId = true;

            i++;
            if (i == argc) {
                ERR(L"-device requires a device ID");
                return E_INVALIDARG;
            }

            if (0 == _wcsicmp(argv[i], L"default")) {
                deviceOption = useDefault;
            } else if (0 == _wcsicmp(argv[i], L"voice")) {
                deviceOption = useVoice;
            } else {
                deviceOption = useSpecific;

                HRESULT hr = CLSIDFromString(argv[i], &_deviceId);
                if (FAILED(hr)) {
                    ERR(L"CLSIDFromString returned failure 0x%08x", hr);
                    return hr;
                }
            }
            continue;
        }

        ERR(L"Unrecognized argument %s", argv[i]);
        return E_INVALIDARG;
    }

    switch (deviceOption) {
    case useNull:
        break;
    case useDefault:
        _deviceId = (_capture ? DSDEVID_DefaultCapture : DSDEVID_DefaultPlayback);
        break;
    case useVoice:
        _deviceId = (_capture ? DSDEVID_DefaultVoiceCapture : DSDEVID_DefaultVoicePlayback);
        break;
    case useSpecific:
        break;
    }

    return S_OK;
}
