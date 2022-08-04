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

    const WORD channels = 2;
    const DWORD samplesPerSecond = 48000;
    const WORD bitsPerSample = 16;
    const DWORD BITS_PER_BYTE = 8;

    WAVEFORMATEX format = {
        WAVE_FORMAT_PCM,
        channels, // nChannels
        samplesPerSecond, // nSamplesPerSec
        channels * bitsPerSample * samplesPerSecond / BITS_PER_BYTE, // nAvgBytesPerSec
        channels * bitsPerSample / BITS_PER_BYTE, // nBlockAlign
        bitsPerSample, // wBitsPerSample
        0 // cbSize
    };

    const DWORD MILLISECONDS_PER_SECOND = 1000;
    const DWORD bufferMilliseconds = 30;
    const DWORD bufferBytes = (
        bufferMilliseconds *
        samplesPerSecond /
        MILLISECONDS_PER_SECOND
    ) * format.nBlockAlign;

    if (v8) {
        DSCBUFFERDESC captureBufferDescription = {
            sizeof(DSCBUFFERDESC), // dwSize
            0, // dwFlags
            bufferBytes, // dwBufferBytes
            0, // dwReserved
            &format, // lpwfxFormat
            0, // dwFXCount
            nullptr // lpDSCFXDesc
        };
        DSBUFFERDESC renderBufferDescription = {
            sizeof(DSBUFFERDESC), // dwSize
            0, // dwFlags
            bufferBytes, // dwBufferBytes
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
