// devices.cpp
#include <windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <mmdeviceapi.h>
#include <devicetopology.h>

#include "log.h"
#include "cleanup.h"
#include "property.h"
#include "stringify.h"
#include "devices.h"

HRESULT AudioAdapterIdFromEndpoint(
    IMMDevice *pMMDevice,
    __out LPWSTR *ppAdapterId
);

HRESULT AudioDevices() {
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
    LPWSTR szzAdapterIds = (LPWSTR)CoTaskMemAlloc(sizeof(WCHAR));
    if (NULL == szzAdapterIds) {
        ERR(L"Could not allocate enough memory for a WCHAR");
        return E_OUTOFMEMORY;
    }
    szzAdapterIds[0] = L'\0';
    CoTaskMemFreeOnExit freeAdapterIds(szzAdapterIds);
    UINT nAdapterIdsLength = 1;
    
    for (UINT i = 0; i < nDevices; i++) {
        IMMDevice *pEndpoint = NULL;
        hr = pMMDeviceCollection->Item(i, &pEndpoint);
        if (FAILED(hr)) {
            ERR(L"IMMDeviceCollection::Item failed: hr = 0x%08x", hr);
            return hr;
        }
        ReleaseOnExit releaseMMDevice(pEndpoint);
               
        // have we seen an endpoint on this adapter already?
        LPWSTR pszId = NULL;
        hr = AudioAdapterIdFromEndpoint(pEndpoint, &pszId);
        if (FAILED(hr)) {
            return hr;
        }
        CoTaskMemFreeOnExit freeId(pszId);
        
        bool bSeen = false;
        UINT32 nNewAdapterIdsLength = (UINT32)wcslen(pszId) + 1;
#pragma prefast(suppress: __WARNING_BUFFER_OVERFLOW, "empty-string-terminated list of strings")
        for (LPWSTR szOldId = szzAdapterIds; wcslen(szOldId); szOldId += wcslen(szOldId) + 1) {
            nNewAdapterIdsLength += (UINT32)wcslen(szOldId) + 1;
            if (0 == _wcsicmp(szOldId, pszId)) {
                bSeen = true;
            }
        }
        nNewAdapterIdsLength++;
        
        if (bSeen) {
            // don't need to log it again
            continue;
        }
        
        // now add it to the list of "seen" adapter IDs
        LPWSTR szzNewAdapterIds = (LPWSTR)CoTaskMemAlloc(sizeof(WCHAR) * nNewAdapterIdsLength);
        if (NULL == szzNewAdapterIds) {
            ERR(L"Could not allocate %u bytes", (UINT32)sizeof(WCHAR) * nNewAdapterIdsLength);
            return E_OUTOFMEMORY;
        }
        CoTaskMemFreeOnExit freeNew(szzNewAdapterIds);
        
        // copy over the old
        errno_t err = memcpy_s(
            szzNewAdapterIds,
                sizeof(WCHAR) * nNewAdapterIdsLength,
            szzAdapterIds,
                sizeof(WCHAR) * nAdapterIdsLength
        );
        if (0 != err) {
            ERR(L"Could not copy old IDs: err = %u", err);
            return E_FAIL;
        }
        
        // copy over the new
        err = memcpy_s(
            szzNewAdapterIds + nAdapterIdsLength - 1, // write over the final \0
                sizeof(WCHAR) * (nNewAdapterIdsLength - nAdapterIdsLength + 1),
            pszId,
                sizeof(WCHAR) * (wcslen(pszId) + 1)
        );
        if (0 != err) {
            ERR(L"Could not copy new ID: err = %u", err);
            return E_FAIL;
        }
        
        // set the new terminator
        szzNewAdapterIds[nNewAdapterIdsLength - 1] = L'\0';
        freeNew.NewTarget(szzAdapterIds);
        freeAdapterIds.NewTarget(szzNewAdapterIds);
        szzAdapterIds = szzNewAdapterIds;
        
        IMMDevice *pAdapter = NULL;
        hr = pMMDeviceEnumerator->GetDevice(pszId, &pAdapter);
        if (FAILED(hr)) {
            ERR(L"IMMDeviceEnumerator::GetDevice(%s) failed: hr = 0x%08x", pszId, hr);
            return hr;
        }
        
        hr = AudioDevice(pAdapter);
        if (FAILED(hr)) {
            hrLastFailure = hr;
        }
        
        LOG(L"");
    }
    
    return hrLastFailure;
}

HRESULT AudioDevice(IMMDevice *pMMDevice) {
    // log the ID
    LPWSTR pszId = NULL;
    HRESULT hr = pMMDevice->GetId(&pszId);
    if (FAILED(hr)) {
        ERR(L"IMMDevice::GetId failed: hr = 0x%08x", hr);
        return hr;
    }
    CoTaskMemFreeOnExit freeId(pszId);
    LOG(L"ID: %s", pszId);
    
    // log the state
    DWORD dwState = 0;
    hr = pMMDevice->GetState(&dwState);
    if (FAILED(hr)) {
        ERR(L"IMMDevice::GetState failed: hr = 0x%08x", hr);
        return hr;
    }
    LOG(L"State: %u (%s)", dwState, StringFromState(dwState));

    // log all the properties
    IPropertyStore *pPropertyStore;
    hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
    if (FAILED(hr)) {
        ERR(L"IMMDevice::OpenPropertyStore failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releasePropertyStore(pPropertyStore);
    
    DWORD nProperties = 0;
    hr = pPropertyStore->GetCount(&nProperties);
    if (FAILED(hr)) {
        ERR(L"IPropertyStore::GetCount failed: hr = 0x%08x", hr);
        return hr;
    }    
    LOG(L"-- Properties (%u) --", nProperties);
    
    HRESULT hrLastFailure = S_OK;
    for (DWORD i = 0; i < nProperties; i++) {
        PROPERTYKEY key = {0};
        
        hr = pPropertyStore->GetAt(i, &key);
        if (FAILED(hr)) {
            hrLastFailure = hr;
            ERR(L"IPropertyStore::GetAt failed: hr = 0x%08x", hr);
            continue;
        }
        
        PROPVARIANT var; PropVariantInit(&var);
        PropVariantClearOnExit clearPropVariant(&var);
        hr = pPropertyStore->GetValue(key, &var);
        if (FAILED(hr)) {
            hrLastFailure = hr;
            ERR(L"IPropertyStore::GetAt failed: hr = 0x%08x", hr);
            continue;
        }
        
        hr = LogProperty(key, var);
        if (FAILED(hr)) {
            hrLastFailure = hr;
            continue;
        }
    }
    
    return hrLastFailure;
}

HRESULT AudioAdapterIdFromEndpoint(
    IMMDevice *pMMDevice,
    __out LPWSTR *ppAdapterId
) {
    // need to do the connector dance
    IDeviceTopology *pDeviceTopology = NULL;
    HRESULT hr = pMMDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDeviceTopology);
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IDeviceTopology) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseDeviceTopology(pDeviceTopology);
    
    IConnector *pConnector = NULL;
    hr = pDeviceTopology->GetConnector(0, &pConnector);
    if (FAILED(hr)) {
        ERR(L"IDeviceTopology::GetConnector(0) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseConnector(pConnector);
    
    hr = pConnector->GetDeviceIdConnectedTo(ppAdapterId);
    if (FAILED(hr)) {
        ERR(L"IConnector::GetDeviceIdConnectedTo failed: hr = 0x%08x", hr);
        return hr;
    }
    

    return S_OK;
}