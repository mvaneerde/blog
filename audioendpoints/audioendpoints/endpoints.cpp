// endpoints.cpp
#include <windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <mmdeviceapi.h>

#include "log.h"
#include "cleanup.h"
#include "stringify.h"
#include "property.h"
#include "devices.h"
#include "endpoints.h"

HRESULT AudioEndpoints() {
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

    // get a list of all endpoints on the system
    IMMDeviceCollection *pMMDeviceCollection = NULL;
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATEMASK_ALL, &pMMDeviceCollection);
    if (FAILED(hr)) {
        ERR(L"IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseMMDeviceCollection(pMMDeviceCollection);
    
    UINT nDevices = 0;
    hr = pMMDeviceCollection->GetCount(&nDevices);
    if (FAILED(hr)) {
        ERR(L"IMMDeviceCollection::GetCount failed: hr = 0x%08x", hr);
        return hr;
    }
    
    HRESULT hrLastFailure = S_OK;
    for (UINT i = 0; i < nDevices; i++) {
        IMMDevice *pMMDevice = NULL;
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            ERR(L"IMMDeviceCollection::Item failed: hr = 0x%08x", hr);
            return hr;
        }
        ReleaseOnExit releaseMMDevice(pMMDevice);
        
        hr = AudioDevice(pMMDevice);
        if (FAILED(hr)) {
            hrLastFailure = hr;
        }
        
        LOG(L"");
    }
    
    return hrLastFailure;
}
