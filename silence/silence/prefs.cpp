// prefs.cpp

#include <windows.h>
#include <stdio.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#include "prefs.h"

HRESULT get_default_device(IMMDevice **ppMMDevice);
HRESULT list_devices();
HRESULT get_specific_device(LPCWSTR szLongName, IMMDevice **ppMMDevice);
void usage(LPCWSTR exe);

CPrefs::CPrefs(int argc, LPCWSTR argv[], HRESULT &hr)
: m_pMMDevice(NULL)
{
    switch (argc) {
        case 1:
            // get default console render device
            hr = get_default_device(&m_pMMDevice);
            break;
            
        case 2:
            if (0 == _wcsicmp(argv[1], L"-?") || 0 == _wcsicmp(argv[1], L"/?")) {
                // print usage but don't actually play
                hr = S_FALSE;
                usage(argv[0]);
            } else if (0 == _wcsicmp(argv[1], L"--list-devices")) {
                // list the devices but don't actually play
                hr = list_devices();

                // don't actually play
                if (S_OK == hr) {
                    hr = S_FALSE;
                }
            } else {
                printf("Unexpected argument %ls\n", argv[1]);
                hr = E_INVALIDARG;
                usage(argv[0]);
            }
            break;

        case 3:
            if (0 == _wcsicmp(argv[1], L"--device")) {
                // play sound to the specific device
                hr = get_specific_device(argv[2], &m_pMMDevice);
            } else {
                printf("Unexpected argument %ls\n", argv[1]);
                hr = E_INVALIDARG;
                usage(argv[0]);
            }
            break;
            
        default:
            printf("Unexpected argument count %u\n", argc);
            hr = E_INVALIDARG;
            usage(argv[0]);
    }
}

CPrefs::~CPrefs() {
    if (NULL != m_pMMDevice) {
        m_pMMDevice->Release();
    }
}

HRESULT get_default_device(IMMDevice **ppMMDevice) {
    HRESULT hr = S_OK;
    IMMDeviceEnumerator *pMMDeviceEnumerator;

    // activate a device enumerator
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        printf("CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x\n", hr);
        return hr;
    }

    // get the default render endpoint
    hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, ppMMDevice);
    pMMDeviceEnumerator->Release();
    if (FAILED(hr)) {
        printf("IMMDeviceEnumerator::GetDefaultAudioEndpoint failed: hr = 0x%08x\n", hr);
        return hr;
    }

    return S_OK;
}

HRESULT list_devices() {
    HRESULT hr = S_OK;

    // get an enumerator
    IMMDeviceEnumerator *pMMDeviceEnumerator;

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        printf("CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x\n", hr);
        return hr;
    }

    IMMDeviceCollection *pMMDeviceCollection;

    // get all the active render endpoints
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(
        eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
    );
    pMMDeviceEnumerator->Release();
    if (FAILED(hr)) {
        printf("IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x\n", hr);
        return hr;
    }

    UINT count;
    hr = pMMDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        pMMDeviceCollection->Release();
        printf("IMMDeviceCollection::GetCount failed: hr = 0x%08x\n", hr);
        return hr;
    }
    printf("Active render endpoints found: %u\n", count);

    for (UINT i = 0; i < count; i++) {
        IMMDevice *pMMDevice;

        // get the "n"th device
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            pMMDeviceCollection->Release();
            printf("IMMDeviceCollection::Item failed: hr = 0x%08x\n", hr);
            return hr;
        }

        // open the property store on that device
        IPropertyStore *pPropertyStore;
        hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        pMMDevice->Release();
        if (FAILED(hr)) {
            pMMDeviceCollection->Release();
            printf("IMMDevice::OpenPropertyStore failed: hr = 0x%08x\n", hr);
            return hr;
        }

        // get the long name property
        PROPVARIANT pv; PropVariantInit(&pv);
        hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
        pPropertyStore->Release();
        if (FAILED(hr)) {
            pMMDeviceCollection->Release();
            printf("IPropertyStore::GetValue failed: hr = 0x%08x\n", hr);
            return hr;
        }

        if (VT_LPWSTR != pv.vt) {
            printf("PKEY_Device_FriendlyName variant type is %u - expected VT_LPWSTR", pv.vt);

            PropVariantClear(&pv);
            pMMDeviceCollection->Release();
            return E_UNEXPECTED;
        }

        printf("    %ls\n", pv.pwszVal);
        
        PropVariantClear(&pv);
    }    
    pMMDeviceCollection->Release();
    
    return S_OK;
}

HRESULT get_specific_device(LPCWSTR szLongName, IMMDevice **ppMMDevice) {
    HRESULT hr = S_OK;

    *ppMMDevice = NULL;
    
    // get an enumerator
    IMMDeviceEnumerator *pMMDeviceEnumerator;

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        printf("CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x\n", hr);
        return hr;
    }

    IMMDeviceCollection *pMMDeviceCollection;

    // get all the active render endpoints
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(
        eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
    );
    pMMDeviceEnumerator->Release();
    if (FAILED(hr)) {
        printf("IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x\n", hr);
        return hr;
    }

    UINT count;
    hr = pMMDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        pMMDeviceCollection->Release();
        printf("IMMDeviceCollection::GetCount failed: hr = 0x%08x\n", hr);
        return hr;
    }

    for (UINT i = 0; i < count; i++) {
        IMMDevice *pMMDevice;

        // get the "n"th device
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            pMMDeviceCollection->Release();
            printf("IMMDeviceCollection::Item failed: hr = 0x%08x\n", hr);
            return hr;
        }

        // open the property store on that device
        IPropertyStore *pPropertyStore;
        hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        if (FAILED(hr)) {
            pMMDevice->Release();
            pMMDeviceCollection->Release();
            printf("IMMDevice::OpenPropertyStore failed: hr = 0x%08x\n", hr);
            return hr;
        }

        // get the long name property
        PROPVARIANT pv; PropVariantInit(&pv);
        hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
        pPropertyStore->Release();
        if (FAILED(hr)) {
            pMMDevice->Release();
            pMMDeviceCollection->Release();
            printf("IPropertyStore::GetValue failed: hr = 0x%08x\n", hr);
            return hr;
        }

        if (VT_LPWSTR != pv.vt) {
            printf("PKEY_Device_FriendlyName variant type is %u - expected VT_LPWSTR", pv.vt);

            PropVariantClear(&pv);
            pMMDevice->Release();
            pMMDeviceCollection->Release();
            return E_UNEXPECTED;
        }

        // is it a match?
        if (0 == _wcsicmp(pv.pwszVal, szLongName)) {
            // did we already find it?
            if (NULL == *ppMMDevice) {
                *ppMMDevice = pMMDevice;
                pMMDevice->AddRef();
            } else {
                printf("Found (at least) two devices named %ls\n", szLongName);
                PropVariantClear(&pv);
                pMMDevice->Release();
                pMMDeviceCollection->Release();
                return E_UNEXPECTED;
            }
        }
        
        pMMDevice->Release();
        PropVariantClear(&pv);
    }
    pMMDeviceCollection->Release();
    
    if (NULL == *ppMMDevice) {
        printf("Could not find a device named %ls\n", szLongName);
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    return S_OK;
}

void usage(LPCWSTR exe) {
    printf(
        "%ls\n"
        "%ls -?\n"
        "%ls --list-devices\n"
        "%ls --device \"Device long name\"\n"
        "\n"
        "    With no arguments, plays silence to the default audio device.\n"
        "    -? prints this message.\n"
        "    --list-devices displays the long names of all active playback devices.\n"
        "    --device plays silence to the specified device.",
        exe, exe, exe, exe
    );
}
