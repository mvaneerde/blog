// capture.cpp

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <endpointvolume.h>
#include <stdio.h>
#include "cleanup.h"
#include "log.h"
#include "vary.h"
#include "capture.h"
#include "stop.h"

HRESULT Capture() {
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
    
    // Get the default console capture endpoint
    IMMDevice *pMMDevice = NULL;
    hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pMMDevice);
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

    // need to call initialize to get a stream volume
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hEvent) {
        DWORD dwErr = GetLastError();
        ERR(L"CreateEvent failed with error %d", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }
    CloseHandleOnExit closeEvent(hEvent);
    
    // timer-driven shared-mode capture
    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        0,
        0,
        pWfx,
        NULL
    );
    if (FAILED(hr)) {
        ERR(L"IAudioClient::Initialize failed: hr = 0x%08x", hr);
        return hr;
    }

    // we'll just dump the samples on the floor... won't even bother creating a capture client
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        ERR(L"IAudioClient::Start failed: hr = 0x%08x", hr);
        return hr;
    }    
    
    StopAudioClientOnExit stopAudioClient(pAudioClient);
    
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

    // get an audio endpoint volume pointer
    IAudioEndpointVolume *pAudioEndpointVolume = NULL;
    hr = pMMDevice->Activate(
        __uuidof(IAudioEndpointVolume),
        CLSCTX_ALL,
        NULL,
        reinterpret_cast<void**>(&pAudioEndpointVolume)
    );
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IAudioEndpointVolume) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseAudioEndpointVolume(pAudioEndpointVolume);

    UINT nAudioEndpointVolumeChannels;
    hr = pAudioEndpointVolume->GetChannelCount(&nAudioEndpointVolumeChannels);
    if (FAILED(hr)) {
        ERR(L"IAudioEndpointVolume::GetChannelCount() failed: hr = 0x%08x", hr);
        return hr;
    }
    
    if (nChannelAudioVolumeChannels != nAudioEndpointVolumeChannels) {
        ERR(L"IChannelAudioVolume has %u channels but IAudioEndpointVolume has %u", nChannelAudioVolumeChannels, nAudioEndpointVolumeChannels);
        return E_UNEXPECTED;
    }
    
    if (nChannelAudioVolumeChannels < 2) {
        ERR(L"This tool assumes an endpoint volume with at least two channels, not %u", nChannelAudioVolumeChannels);
        return E_UNEXPECTED;
    }
    
    float dbEndpointVolumeMin;
    float dbEndpointVolumeMax;
    float dbEndpointVolumeStep;
    hr = pAudioEndpointVolume->GetVolumeRange(&dbEndpointVolumeMin, &dbEndpointVolumeMax, &dbEndpointVolumeStep);
    if (FAILED(hr)) {
        ERR(L"IAudioEndpointVolume::GetVolumeRange failed: hr = 0x%08x", hr);
        return hr;
    }
    LOG(
        L"Endpoint volume ranges from %g dB to %g dB in steps of %g dB",
        dbEndpointVolumeMin,
        dbEndpointVolumeMax,
        dbEndpointVolumeStep
    );
 
    LOG(
        L"\"Master dB\"\t"
        L"\"Master Scalar\"\t"
        L"\"Channel 1 dB\"\t"
        L"\"Channel 1 Scalar\"\t"
        L"\"Session\"\t"
        L"\"Channel 0\"\t"
        L"\"Channel 1\""
    );
    for (int i = 0; dbEndpointVolumeMin + i * dbEndpointVolumeStep <= dbEndpointVolumeMax; i++) {
        float dbMaster = dbEndpointVolumeMin + i * dbEndpointVolumeStep;
        hr = pAudioEndpointVolume->SetMasterVolumeLevel(dbMaster, NULL);
        if (FAILED(hr)) {
            ERR(L"IAudioEndpointVolume::SetMasterVolumeLevel failed: hr = 0x%08x", hr);
            return hr;
        }

        float scalarMaster;
        hr = pAudioEndpointVolume->GetMasterVolumeLevelScalar(&scalarMaster);
        if (FAILED(hr)) {
            ERR(L"IAudioEndpointVolume::GetMasterVolumeLevelScalar failed: hr = 0x%08x", hr);
            return hr;
        }        
        
        for (int j = 0; j <= i; j++) {
            float dbChannel1 = dbEndpointVolumeMin + j * dbEndpointVolumeStep;
            hr = pAudioEndpointVolume->SetChannelVolumeLevel(1, dbChannel1, NULL);
            if (FAILED(hr)) {
                ERR(L"IAudioEndpointVolume::SetChannelVolumeLevel failed: hr = 0x%08x", hr);
                return hr;
            }

            float scalarChannel1;
            hr = pAudioEndpointVolume->GetChannelVolumeLevelScalar(1, &scalarChannel1);
            if (FAILED(hr)) {
                ERR(L"IAudioEndpointVolume::GetChannelVolumeLevelScalar failed: hr = 0x%08x", hr);
                return hr;
            }

            float session;
            hr = pSimpleAudioVolume->GetMasterVolume(&session);
            if (FAILED(hr)) {
                ERR(L"ISimpleAudioVolume::GetMasterVolume failed: hr = 0x%08x", hr);
                return hr;
            }        
        
            hr = pChannelAudioVolume->GetAllVolumes(nChannelAudioVolumeChannels, pChannelAudioVolumeChannels);
            if (FAILED(hr)) {
                ERR(L"IChannelAudioVolume::GetAllVolumes failed: hr = 0x%08x", hr);
                return hr;
            }
            float channel0 = pChannelAudioVolumeChannels[0];
            float channel1 = pChannelAudioVolumeChannels[1];

            LOG(
                L"%g\t%g\t%g\t%g\t%g\t%g\t%g",
                dbMaster,
                scalarMaster,
                dbChannel1,
                scalarChannel1,
                session,
                channel0,
                channel1
            );
        } // channel 1 volumes
    } // master volumes
 
    return S_OK;
}