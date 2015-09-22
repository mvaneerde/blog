// main.cpp

#include <initguid.h>
#include <windows.h>
#include <cguid.h>
#include <atlbase.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <stdio.h>
#include <functiondiscoverykeys_devpkey.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() {
        CoUninitialize();
    }
};

class PropVariantClearOnExit {
public:
    PropVariantClearOnExit(PROPVARIANT *p) : m_p(p) {}
    ~PropVariantClearOnExit() {
        PropVariantClear(m_p);
    }
    
private:
    PROPVARIANT *m_p;
};

int _cdecl wmain() {

    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        LOG(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;
    
    CComPtr<IMMDeviceEnumerator> pMMDeviceEnumerator;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void **>(&pMMDeviceEnumerator)
    );
    if (FAILED(hr)) {
        LOG(L"CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    CComPtr<IMMDeviceCollection> pMMDeviceCollection;
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(
        eRender,
        DEVICE_STATE_ACTIVE,
        &pMMDeviceCollection
    );
    if (FAILED(hr)) {
        LOG(L"IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    UINT count;
    hr = pMMDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        LOG(L"IMMDeviceCollection::GetCount failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    for (UINT i = 0; i < count; i++) {
        CComPtr<IMMDevice> pMMDevice;
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            LOG(L"IMMDeviceCollection::Item(%u) failed: hr = 0x%08x", i, hr);
            continue;
        }
        
        CComPtr<IAudioEndpointVolume> pAudioEndpointVolume;
        hr = pMMDevice->Activate(
            __uuidof(IAudioEndpointVolume),
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<void **>(&pAudioEndpointVolume)
        );
        if (FAILED(hr)) {
            LOG(L"IMMDevice(%u)::Activate(IAudioEndpointVolume) failed: hr = 0x%08x", i, hr);
            continue;
        }
        
        BOOL mute;
        hr = pAudioEndpointVolume->GetMute(&mute);
        if (FAILED(hr)) {
            LOG(L"IAudioEndpointVolume(%u)::GetMute failed: hr = 0x%08x", i, hr);
            continue;
        }
        
        if (mute) {
            // device already muted
            continue;
        }
        
        // get the name of the endpoint
        CComPtr<IPropertyStore> pPropertyStore;
        hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        if (FAILED(hr)) {
            LOG(L"IMMDevice(%u)::OpenPropertyStore failed: hr = 0x%08x", i, hr);
            continue;
        }

        PROPVARIANT v; PropVariantInit(&v);
        PropVariantClearOnExit pvcoe(&v);
        hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &v);
        if (FAILED(hr)) {
            LOG(L"IPropertyStore(%u)::GetValue(PKEY_Device_FriendlyName) failed: hr = 0x%08x", i, hr);
            continue;
        }
        
        if (VT_LPWSTR != v.vt) {
            LOG(L"Device %u property PKEY_Device_FriendlyName has unexpected vartype %u", i, v.vt);
            continue;
        }

        // mute it
        hr = pAudioEndpointVolume->SetMute(TRUE, nullptr);
        if (FAILED(hr)) {
            LOG(L"IAudioEndpointVolume(%u)::SetMute failed: hr = 0x%08x", i, hr);
            continue;
        }
        
        LOG(L"Muted %s", v.pwszVal);
    }
    
    return 0;
}