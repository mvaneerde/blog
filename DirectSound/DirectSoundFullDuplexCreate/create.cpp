// create.cpp

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <dsound.h>

#include "log.h"
#include "cleanup.h"
#include "create.h"

HRESULT Create(bool v8, LPCGUID renderDeviceId, LPCGUID captureDeviceId) {
    HRESULT hr;

    if (v8) {
        WAVEFORMATEX format = {
            WAVE_FORMAT_PCM, // nChannels
            2, // nChannels
            48000, // nSamplesPerSec
            2 * 16 * 48000 / 8, // nAvgBytesPerSec
            2 * 16 / 8, // nBlockAlign
            16, // wBitsPerSample
            0 // cbSize
        };
        DSCBUFFERDESC captureBufferDescription = {
            sizeof(DSCBUFFERDESC), // dwSize
            0, // dwFlags
            0, // dwBufferBytes
            0, // dwReserved
            &format, // lpwfxFormat
            0, // dwFXCount
            nullptr // lpDSCFXDesc
        };
        DSBUFFERDESC renderBufferDescription = {
            sizeof(DSBUFFERDESC), // dwSize
            0, // dwFlags
            0, // dwBufferBytes
            0, // dwReserved
            &format, // lpwfxFormat
            GUID_NULL // guid3DAlgorithm
        };
        LPDIRECTSOUNDFULLDUPLEX fullDuplex = nullptr;
        LPDIRECTSOUNDCAPTUREBUFFER8 captureBuffer = nullptr;
        LPDIRECTSOUNDBUFFER8 buffer = nullptr;

        hr = DirectSoundFullDuplexCreate8(
            captureDeviceId,
            renderDeviceId,
            &captureBufferDescription,
            &renderBufferDescription,
            GetConsoleWindow(),
            DSSCL_NORMAL,
            &fullDuplex,
            &captureBuffer,
            &buffer,
            nullptr // outer unknown
        );
        if (FAILED(hr)) {
            ERR(L"DirectSoundFullDuplexCreate8 failed with 0x%08x", hr);
            return hr;
        }
    } else {
        hr = DirectSoundFullDuplexCreate(
            captureDeviceId,
            renderDeviceId,
            nullptr,
            nullptr,
            nullptr,
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        if (FAILED(hr)) {
            ERR(L"DirectSoundFullDuplexCreate failed with 0x%08x", hr);
            return hr;
        }
    }

    return hr;
}
