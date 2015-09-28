// meter.cpp

#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <stdio.h>
#include "cleanup.h"
#include "log.h"

HRESULT GetAudioMeterInformation(IAudioMeterInformation **ppAudioMeterInformation) {
    if (NULL == ppAudioMeterInformation) {
        ERR(L"GetAudioMeterInformation was passed a NULL pointer");
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
    
    // Activate an IAudioMeterInformation
    IAudioMeterInformation *pAudioMeterInformation = NULL;
    hr = pMMDevice->Activate(
        __uuidof(IAudioMeterInformation),
        CLSCTX_ALL,
        NULL,
        reinterpret_cast<void**>(&pAudioMeterInformation)
    );
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IAudioMeterInformation) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseAudioMeterInformation(pAudioMeterInformation);

    pAudioMeterInformation->AddRef();
    *ppAudioMeterInformation = pAudioMeterInformation;
 
    return S_OK;
}