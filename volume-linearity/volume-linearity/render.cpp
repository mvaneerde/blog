// render.cpp

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include "log.h"
#include "cleanup.h"
#include "vary.h"
#include "render.h"
#include "stop.h"

HRESULT Render(IAudioMeterInformation *pAudioMeterInformation, EVary which) {
    if (NULL == pAudioMeterInformation) {
        ERR(L"Render was passed a NULL pointer");
        return E_POINTER;
    }
        
    // CoCreate an IMMDeviceEnumerator
    IMMDeviceEnumerator *pMMDeviceEnumerator = NULL;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        ERR(L"CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseMMDeviceEnumerator(pMMDeviceEnumerator);
    
    // Get the default console render endpoint
    IMMDevice *pMMDevice = NULL;
    hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pMMDevice);
    if (FAILED(hr)) {
        ERR(L"IMMDeviceEnumerator::GetDefaultAudioEndpoint failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseMMDevice(pMMDevice);

    // activate an IAudioClient
    IAudioClient *pAudioClient = NULL;
    hr = pMMDevice->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        NULL,
        reinterpret_cast<void**>(&pAudioClient)
    );
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IAudioClient) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseAudioClient(pAudioClient);

    // get the mix format
    LPWAVEFORMATEX pWfx = NULL;
    hr = pAudioClient->GetMixFormat(&pWfx);
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetMixFormat failed: hr = 0x%08x", hr);
        return hr;
    }
    CoTaskMemFreeOnExit freeMixFormat(pWfx);
    
    // is it floating-point?
    if (WAVE_FORMAT_IEEE_FLOAT == pWfx->wFormatTag) {
        LOG(L"Mix format is WAVE_FORMAT_IEEE_FLOAT");
    } else if (WAVE_FORMAT_EXTENSIBLE == pWfx->wFormatTag) {
        // at this point I should really check pWfx->cbSize
        WAVEFORMATEXTENSIBLE *pWfxEx = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(pWfx);
        if (KSDATAFORMAT_SUBTYPE_IEEE_FLOAT == pWfxEx->SubFormat) {
            LOG(L"Mix format is WAVE_FORMAT_EXTENSIBLE/KSDATAFORMAT_SUBTYPE_IEEE_FLOAT");
        } else {
            ERR(L"Mix format is WAVE_FORMAT_EXTENSIBLE but has unexpected subformat");
            return E_UNEXPECTED;
        }
    } else {
        ERR(L"Mix format has unexpected format tag %u", pWfx->wFormatTag);
        return E_UNEXPECTED;
    }
    
    // at this point I should really check that it's 32-bit floating-point
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hEvent) {
        DWORD dwErr = GetLastError();
        ERR(L"CreateEvent failed with error %d", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }
    CloseHandleOnExit closeEvent(hEvent);
    
    // event-driven shared-mode render
    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        0,
        0,
        pWfx,
        NULL
    );
    if (FAILED(hr)) {
        ERR(L"IAudioClient::Initialize failed: hr = 0x%08x", hr);
        return hr;
    }
    
    hr = pAudioClient->SetEventHandle(hEvent);
    if (FAILED(hr)) {
        ERR(L"IAudioClient::SetEventHandle failed: hr = 0x%08x", hr);
        return hr;
    }
    
    IAudioRenderClient *pAudioRenderClient = NULL;
    hr = pAudioClient->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(&pAudioRenderClient));
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetService(IAudioRenderClient) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseAudioRenderClient(pAudioRenderClient);

    UINT32 nFramesInBuffer;
    hr = pAudioClient->GetBufferSize(&nFramesInBuffer);
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetBufferSize failed: hr = 0x%08x", hr);
        return hr;
    }
    LOG(L"Frames in buffer: %u", nFramesInBuffer);
    
    // prefill a buffer's worth of silence
    BYTE *pData = NULL;
    hr = pAudioRenderClient->GetBuffer(nFramesInBuffer, &pData);
    if (FAILED(hr)) {
        ERR(L"Initial IAudioRenderClient::GetBuffer failed: hr = 0x%08x", hr);
        return hr;
    }
    
    hr = pAudioRenderClient->ReleaseBuffer(nFramesInBuffer, AUDCLNT_BUFFERFLAGS_SILENT);
    if (FAILED(hr)) {
        ERR(L"Initial IAudioRenderClient::ReleaseBuffer failed: hr = 0x%08x", hr);
        return hr;
    }
    
    // get an audio stream volume pointer
    IAudioStreamVolume *pAudioStreamVolume = NULL;
    hr = pAudioClient->GetService(__uuidof(IAudioStreamVolume), reinterpret_cast<void**>(&pAudioStreamVolume));
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetService(IAudioStreamVolume) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseAudioStreamVolume(pAudioStreamVolume);

    UINT32 nAudioStreamVolumeChannels = 0;
    hr = pAudioStreamVolume->GetChannelCount(&nAudioStreamVolumeChannels);
    if (FAILED(hr)) {
        ERR(L"IAudioStreamVolume::GetChannelCount failed: hr = 0x%08x", hr);
        return hr;
    }
    float *pAudioStreamVolumeChannels = reinterpret_cast<float*>(CoTaskMemAlloc(sizeof(float) * nAudioStreamVolumeChannels));
    if (NULL == pAudioStreamVolumeChannels) {
        ERR(L"Could not allocate space for %u floats for audio stream volume", nAudioStreamVolumeChannels);
        return E_OUTOFMEMORY;
    }
    CoTaskMemFreeOnExit freeAudioStreamVolumeChannels(pAudioStreamVolumeChannels);
    
    // get a simple audio volume pointer
    ISimpleAudioVolume *pSimpleAudioVolume = NULL;
    hr = pAudioClient->GetService(__uuidof(ISimpleAudioVolume), reinterpret_cast<void**>(&pSimpleAudioVolume));
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetService(ISimpleAudioVolume) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseSimpleAudioVolume(pSimpleAudioVolume);
    
    // get a channel audio volume pointer
    IChannelAudioVolume *pChannelAudioVolume = NULL;
    hr = pAudioClient->GetService(__uuidof(IChannelAudioVolume), reinterpret_cast<void**>(&pChannelAudioVolume));
    if (FAILED(hr)) {
        ERR(L"IAudioClient::GetService(IChannelAudioVolume) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseChannelAudioVolume(pChannelAudioVolume);

    UINT32 nChannelAudioVolumeChannels = 0;
    hr = pChannelAudioVolume->GetChannelCount(&nChannelAudioVolumeChannels);
    if (FAILED(hr)) {
        ERR(L"IChannelAudioVolume::GetChannelCount failed: hr = 0x%08x", hr);
        return hr;
    }
    float *pChannelAudioVolumeChannels = reinterpret_cast<float*>(CoTaskMemAlloc(sizeof(float) * nChannelAudioVolumeChannels));
    if (NULL == pChannelAudioVolumeChannels) {
        ERR(L"Could not allocate space for %u floats for audio stream volume", nChannelAudioVolumeChannels);
        return E_OUTOFMEMORY;
    }
    CoTaskMemFreeOnExit freeChannelAudioVolumeChannels(pChannelAudioVolumeChannels);

    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        ERR(L"IAudioClient::Start failed: hr = 0x%08x", hr);
        return hr;
    }
    
    StopAudioClientOnExit stopAudioClient(pAudioClient);
    
    LPCWSTR szIndependentVariable = L"Signal Magnitude";
    double independent = 0.0;
    
    const float hzSquareWaveFrequency = 100.0f; // frequency of the square wave
    const double pi = 4.0 * atan(1.0);
    double phase = 0.0;
    double amplitude = 0.5; // stop the limiter from kicking in
    
    // on each pass we'll bump up the amplitude by 1/512 until it's 1.0
    UINT32 nLatencyInBufferLengths = 3;
    UINT32 nFramesBetweenChanges = nLatencyInBufferLengths * nFramesInBuffer;
    UINT32 nTotalChanges = 512; // except for eAudioEndpointVolume
    float valMin = 0.0f;
    float valMax = 1.0f;
    float valStep = (valMax - valMin) / nTotalChanges;
    
    switch (which) {
        case eSignal:
            szIndependentVariable = L"Signal Magnitude";
            independent = valMin;
            amplitude = independent;
            break;
            
        case eAudioStreamVolume:
            szIndependentVariable = L"AudioStreamVolume";
            independent = valMin;
            for (UINT32 i = 0; i < nAudioStreamVolumeChannels; i++) {
                pAudioStreamVolumeChannels[i] = static_cast<float>(independent);
            }
            hr = pAudioStreamVolume->SetAllVolumes(nAudioStreamVolumeChannels, pAudioStreamVolumeChannels);
            if (FAILED(hr)) {
                ERR(L"IAudioStreamVolume::SetAllVolumes failed: hr = 0x%08x", hr);
                return hr;
            }
            break;
        
        case eSimpleAudioVolume:
            szIndependentVariable = L"SimpleAudioVolume";
            independent = valMin;
            hr = pSimpleAudioVolume->SetMasterVolume(static_cast<float>(independent), NULL);
            if (FAILED(hr)) {
                ERR(L"ISimpleAudioVolume::SetMasterVolume failed: hr = 0x%08x", hr);
                return hr;
            }
            break;
        
        case eChannelAudioVolume:
            szIndependentVariable = L"ChannelAudioVolume";
            independent = valMin;
            for (UINT32 i = 0; i < nChannelAudioVolumeChannels; i++) {
                pChannelAudioVolumeChannels[i] = static_cast<float>(independent);
            }
            hr = pChannelAudioVolume->SetAllVolumes(nChannelAudioVolumeChannels, pChannelAudioVolumeChannels, NULL);
            if (FAILED(hr)) {
                ERR(L"IChannelAudioVolume::SetAllVolumes failed: hr = 0x%08x", hr);
                return hr;
            }
            break;

        default:
            ERR(L"EVary type %u is not recognized", which);
            return E_INVALIDARG;
    }

    // print headers
    LOG(L"Frames\t\"%s\"\tMeter", szIndependentVariable);
    
    UINT32 nTotalFrames = nFramesBetweenChanges * (nTotalChanges + nLatencyInBufferLengths);
    UINT32 nLastChange = 0;
    
    for (UINT32 nFramesWritten = 0; nFramesWritten < nTotalFrames; /* below */) {
            
        DWORD dwWaitResult = WaitForSingleObject(hEvent, INFINITE);
        if (WAIT_OBJECT_0 != dwWaitResult) {
            DWORD dwErr = GetLastError();
            ERR(L"WaitForSingleObject returned %u (expected WAIT_OBJECT_0) with error %u", dwWaitResult, dwErr);
            return HRESULT_FROM_WIN32(dwErr);
        }
        
        UINT32 nCurrentPadding;
        hr = pAudioClient->GetCurrentPadding(&nCurrentPadding);
        if (FAILED(hr)) {
            ERR(L"IAudioClient::GetCurrentPadding failed: hr = 0x%08x", hr);
            return hr;
        }
        
        if (nCurrentPadding > nFramesInBuffer) {
            ERR(L"Current padding (%u) exceeds buffer size (%u)", nCurrentPadding, nFramesInBuffer);
            return E_UNEXPECTED;
        }
        
        UINT32 nFramesToGet = nFramesInBuffer - nCurrentPadding;
        if (0 == nFramesToGet) {
            continue;
        }
    
        // we're about to write some data
        if (nFramesWritten - nLastChange >= nFramesBetweenChanges) {
            double old = independent;
            UINT32 nChangesSoFar = nFramesWritten / nFramesBetweenChanges;
            independent = min(valMax, valMin + valStep * nChangesSoFar);
            switch (which) {
                case eSignal:
                    amplitude = independent;
                    break;
                
                case eAudioStreamVolume:
                    for (UINT32 i = 0; i < nAudioStreamVolumeChannels; i++) {
                        pAudioStreamVolumeChannels[i] = static_cast<float>(independent);
                    }
                    hr = pAudioStreamVolume->SetAllVolumes(nAudioStreamVolumeChannels, pAudioStreamVolumeChannels);
                    if (FAILED(hr)) {
                        ERR(L"IAudioStreamVolume::SetAllVolumes failed: hr = 0x%08x", hr);
                        return hr;
                    }
                    break;
                    
                case eSimpleAudioVolume:
                    hr = pSimpleAudioVolume->SetMasterVolume(static_cast<float>(independent), NULL);
                    if (FAILED(hr)) {
                        ERR(L"ISimpleAudioVolume::SetMasterVolume failed: hr = 0x%08x", hr);
                        return hr;
                    }
                    break;

                case eChannelAudioVolume:
                    for (UINT32 i = 0; i < nChannelAudioVolumeChannels; i++) {
                        pChannelAudioVolumeChannels[i] = static_cast<float>(independent);
                    }
                    hr = pChannelAudioVolume->SetAllVolumes(nChannelAudioVolumeChannels, pChannelAudioVolumeChannels, NULL);
                    if (FAILED(hr)) {
                        ERR(L"IChannelAudioVolume::SetAllVolumes failed: hr = 0x%08x", hr);
                        return hr;
                    }
                    break;

                default:
                    ERR(L"EVary type %u is not recognized", which);
                    return E_INVALIDARG;
            }
            
            float peak;
            hr = pAudioMeterInformation->GetPeakValue(&peak);
            if (FAILED(hr)) {
                ERR(L"IAudioMeterInformation::GetPeakValue failed: hr = 0x%08x", hr);
                return hr;
            }
            LOG(L"%u\t%g\t%g", nFramesWritten, old, peak);
            nLastChange = nFramesWritten;
        }
        
        hr = pAudioRenderClient->GetBuffer(nFramesToGet, &pData);
        if (FAILED(hr)) {
            ERR(L"IAudioRenderClient::GetBuffer failed after %u frames written: hr = 0x%08x", nFramesWritten, hr);
            return hr;
        }
        
        float *pFloatData = reinterpret_cast<float *>(pData);
        for (UINT32 f = 0; f < nFramesToGet; f++) {
            float value;
            phase += 2 * pi * hzSquareWaveFrequency / pWfx->nSamplesPerSec;
            while (phase > 2 * pi) {
                    phase -= 2 * pi;
            }
            
            if (phase > pi) {
                value = static_cast<float>(amplitude);
            } else {
                value = static_cast<float>(-amplitude);
            }
            
            for (UINT32 c = 0; c < pWfx->nChannels; c++) {
                
                
                pFloatData[ f * pWfx->nChannels + c ] = value;
            }
        }
        
        hr = pAudioRenderClient->ReleaseBuffer(nFramesToGet, 0);
        if (FAILED(hr)) {
            ERR(L"Initial IAudioRenderClient::ReleaseBuffer failed after %u frames written: hr = 0x%08x", nFramesWritten, hr);
            return hr;
        }
        
        nFramesWritten += nFramesToGet;
    }
    
    return S_OK;
}
