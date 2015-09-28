// endpoint.cpp
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <stdio.h>
#include "cleanup.h"
#include "log.h"
#include "endpoint.h"

HRESULT Endpoint() {

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

    LOG(L"dB\tScalar");
    for (int i = 0; dbEndpointVolumeMin + i * dbEndpointVolumeStep <= dbEndpointVolumeMax; i++) {
        float db = dbEndpointVolumeMin + i * dbEndpointVolumeStep;
        hr = pAudioEndpointVolume->SetMasterVolumeLevel(db, NULL);
        if (FAILED(hr)) {
            ERR(L"IAudioEndpointVolume::SetMasterVolumeLevel failed: hr = 0x%08x", hr);
            return hr;
        }
        
        float scalar;
        hr = pAudioEndpointVolume->GetMasterVolumeLevelScalar(&scalar);
        if (FAILED(hr)) {
            ERR(L"IAudioEndpointVolume::GetMasterVolumeLevelScalar failed: hr = 0x%08x", hr);
            return hr;
        }        
        
        LOG(L"%g\t%g", db, scalar);
    }
    
    return S_OK;
}