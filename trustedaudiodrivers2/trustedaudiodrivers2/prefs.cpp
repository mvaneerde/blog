// prefs.cpp
#include <windows.h>
#include <tchar.h>
#include <mmdeviceapi.h>
#include <atlstr.h>
#include <functiondiscoverykeys_devpkey.h>

#include "log.h"
#include "prefs.h"

// show usage statement
void usage(LPCTSTR exe) {
    LOG(
        _T("%s --list-devices\n")
        _T("\n")
        _T("%s \n")
        _T("     --device [ communications | console | multimedia | <device-name> ]\n")
        _T("     --copy-ok [ 0 | 1 ]\n")
        _T("     --digital-output-disable [ 0 | 1 ]\n")
        _T("     --test-certificate-enable [ 0 | 1 ]\n")
        _T("     --drm-level <drm-level>\n")
        _T("\n")
        _T("--list-devices: displays a list of output devices.\n")
        _T("\n")
        _T("Sets a Trusted Audio Drivers 2 output policy on a given audio endpoint.\n")
        _T("\n")
        _T("--device\n")
        _T("    communications: set policy on default communications render endpoint.\n")
        _T("    console: set policy on default console render endpoint.\n")
        _T("    multimedia: set policy on default multimedia render endpoint.\n")
        _T("    <device-name>: set policy on the endpoint with this long name.\n")
        _T("\n")
        _T("--copy-ok: 1 or 0. Set to 0 to turn on \"copy protection\" (SCMS or HDCP)\n")
        _T("--digital-output-disable: 1 or 0. Set to 1 to disable output (or turn on HDCP.)\n")
        _T("--test-certificate-enable: 1 or 0. Set to 1 to allow the test certificate.\n")
        _T("--drm-level: set to a number. 1100 is a good default.\n")
        ,
        exe,
        exe
    );
}

HRESULT list_devices();
HRESULT get_specific_device(LPCWSTR szLongName, IMMDevice **ppMMDevice);

// parse command line
CPrefs::CPrefs(int argc, LPCTSTR argv[], HRESULT &hr)
: pMMDevice(NULL)
, bCopyOK(true)
, bDigitalOutputDisable(false)
, bTestCertificateEnable(false)
, dwDrmLevel(0)
{
    // no arguments
    if (1 == argc) {
        usage(argv[0]);
        hr = S_FALSE;
        return;
    }

    // one argument
    if (2 == argc) {
        // single /? or -? argument
        if (0 == _tcscmp(_T("-?"), argv[1]) || 0 == _tcscmp(_T("/?"), argv[1])) {
            usage(argv[0]);
            hr = S_FALSE;
        } else if (0 == _tcscmp(_T("--list-devices"), argv[1])){
            hr = list_devices();
            if (SUCCEEDED(hr)) {
                hr = S_FALSE;
            }
        } else {
            ERR(_T("Unexpected argument: %s"), argv[1]);
            hr = E_INVALIDARG;
        }

        return;
    }

    // otherwise loop through the arguments
    bool bEndpointSet = false;
    bool bCopyOKSet = false;
    bool bDigitalOutputDisableSet = false;
    bool bTestCertificateEnableSet = false;
    bool bDrmLevelSet = false;
    LPCTSTR tszEndpointID = NULL;
    bool bUseDefault = false;
    ERole eRole = eConsole;
    
    for (int i = 1; i < argc; i++) {
        LPCTSTR tszOption = argv[i];
        LPCTSTR tszTemp = NULL;

        // --device [ console | communications | multimedia | <device-name> ]
        if (0 == _tcsicmp(_T("--device"), tszOption)) {
            if (bEndpointSet) {
                ERR(_T("Multiple --device arguments"));
                hr = E_INVALIDARG;
                return;
            }

            if (i == argc - 1) {
                ERR(_T("--device needs an argument"));
                hr = E_INVALIDARG;
                return;
            }

            tszEndpointID = argv[++i];
            if (0 == _tcsicmp(_T("console"), tszEndpointID)) {
                bUseDefault = true;
                eRole = eConsole;
            } else if (0 == _tcsicmp(_T("communications"), tszEndpointID)) {
                bUseDefault = true;
                eRole = eCommunications;
            } else if (0 == _tcsicmp(_T("multimedia"), tszEndpointID)) {
                bUseDefault = true;
                eRole = eMultimedia;
            }
            
            // any other string will be interpreted as a device name
            bEndpointSet = true;
            continue;
        } // --device

        // --copy-ok [0 | 1]
        if (0 == _tcsicmp(_T("--copy-ok"), tszOption)) {
            if (bCopyOKSet) {
                ERR(_T("Multiple --copy-ok arguments"));
                hr = E_INVALIDARG;
                return;
            }

            if (i == argc - 1) {
                ERR(_T("--copy-ok argument has no associated value"));
                hr = E_INVALIDARG;
                return;
            }

            tszTemp = argv[++i];
            if (0 == _tcsicmp(_T("0"), tszTemp)) {
                bCopyOK = false;
            } else if (0 == _tcsicmp(_T("1"), tszTemp)) {
                bCopyOK = true;
            } else {
                ERR(_T("Invalid --copy-ok value.  Expected 1 or 0, got %s"), tszTemp);
                hr = E_INVALIDARG;
                return;
            }

            bCopyOKSet = true;
            continue;
        } // --copy-ok

        // --digital-output-disable [0 | 1]
        if (0 == _tcsicmp(_T("--digital-output-disable"), tszOption)) {
            if (bDigitalOutputDisableSet) {
                ERR(_T("Multiple --digital-output-disable arguments"));
                hr = E_INVALIDARG;
                return;
            }

            if (i == argc - 1) {
                ERR(_T("--digital-output-disable argument has no associated value"));
                hr = E_INVALIDARG;
                return;
            }

            tszTemp = argv[++i];
            if (0 == _tcsicmp(_T("0"), tszTemp)) {
                bDigitalOutputDisable = false;
            } else if (0 == _tcsicmp(_T("1"), tszTemp)) {
                bDigitalOutputDisable = true;
            } else {
                ERR(
                    _T("Invalid --digital-output-disable value.  Expected 1 or 0, got %s"),
                    tszTemp
                );
                hr = E_INVALIDARG;
                return;
            }

            bDigitalOutputDisableSet = true;
            continue;
        } // --digital-output-disable

        // --test-certificate-enable [0 | 1]
        if (0 == _tcsicmp(_T("--test-certificate-enable"), tszOption)) {
            if (bTestCertificateEnableSet) {
                ERR(_T("Multiple --test-certificate-enable arguments"));
                hr = E_INVALIDARG;
                return;
            }

            if (i == argc - 1) {
                ERR(_T("--test-certificate-enable argument has no associated value"));
                hr = E_INVALIDARG;
                return;
            }

            tszTemp = argv[++i];
            if (0 == _tcsicmp(_T("0"), tszTemp)) {
                bTestCertificateEnable= false;
            } else if (0 == _tcsicmp(_T("1"), tszTemp)) {
                bTestCertificateEnable = true;
            } else {
                ERR(
                    _T("Invalid --test-certificate-enable value.  Expected 1 or 0, got %s"),
                    tszTemp
                );
                hr = E_INVALIDARG;
                return;
            }

            bTestCertificateEnableSet = true;
            continue;
        } // --test-certificate-enable
        
        // --drm-level <drm-level>
        if (0 == _tcsicmp(_T("--drm-level"), tszOption)) {
            if (bDrmLevelSet) {
                ERR(_T("Multiple --drm-level arguments"));
                hr = E_INVALIDARG;
                return;
            }

            if (i == argc - 1) {
                ERR(_T("--drm-level argument has no associated value"));
                hr = E_INVALIDARG;
                return;
            }

            tszTemp = argv[++i];
            TCHAR *pEnd = NULL;
            dwDrmLevel = (DWORD)_tcstol(tszTemp, &pEnd, 0);

            // various ways for things to go wrong
            if (0 != errno || NULL == pEnd || _T('\0') != *pEnd) {
                ERR(
                    _T("Invalid --drm-level value.  Expected a number, got %s"),
                    tszTemp
                );
                hr = E_INVALIDARG;
                return;
            }

            bDrmLevelSet = true;
            continue;
        } // --drm-level

        // unrecognized option
        ERR(_T("Unexpected argument: %s"), tszOption);
        hr = E_INVALIDARG;
        return;
    } // for each argument

    // make sure all the arguments were set
    if (!bEndpointSet) {
        ERR(_T("--endpoint not set"));
        hr = E_INVALIDARG;
        return;
    }
  
    if (!bCopyOKSet) {
        ERR(_T("--copy-ok not set"));
        hr = E_INVALIDARG;
        return;
    }

    if (!bDigitalOutputDisableSet) {
        ERR(_T("--digital-output-disable not set"));
        hr = E_INVALIDARG;
        return;
    }

    if (!bTestCertificateEnableSet) {
        ERR(_T("--test-certificate-enable not set"));
        hr = E_INVALIDARG;
        return;
    }

    // get the actual IMMDevice corresponding to the requested endpoint
    CComPtr <IMMDeviceEnumerator> pMMDeviceEnumerator;
    hr = pMMDeviceEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
    if (FAILED(hr)) {
        ERR(_T("Could not CoCreate IMMDeviceEnumerator: hr = 0x%08x"), hr);
        return;
    }
    
    if (bUseDefault) {
        hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eRole, &pMMDevice);
        if (E_NOTFOUND == hr) {
            ERR(
                _T("There is no default audio endpoint on this system for that role.")
            );
            return;
        } else if (FAILED(hr)) {
            ERR(
                _T("IMMDeviceEnumerator::GetDefaultAudioEndpoint(eRender, %u) failed: hr = 0x%08x"),
                eRole, hr
            );
            return;
        }
    } else {
        hr = get_specific_device(CStringW(tszEndpointID), &pMMDevice);
        // get_specific_devices will log any appropriate failure
        return;
    }

    hr = S_OK;
}

CPrefs::~CPrefs() {
    if (NULL != pMMDevice) {
        pMMDevice->Release();
    }
}

HRESULT list_devices() {
    HRESULT hr = S_OK;

    // get an enumerator
    CComPtr<IMMDeviceEnumerator> pMMDeviceEnumerator;

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        ERR(_T("CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x"), hr);
        return hr;
    }

    CComPtr<IMMDeviceCollection> pMMDeviceCollection;

    // get all the active render endpoints
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(
        eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
    );
    if (FAILED(hr)) {
        ERR(_T("IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x"), hr);
        return hr;
    }

    UINT count;
    hr = pMMDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        ERR(_T("IMMDeviceCollection::GetCount failed: hr = 0x%08x"), hr);
        return hr;
    }
    LOG(_T("Active render endpoints found: %u"), count);

    for (UINT i = 0; i < count; i++) {
        CComPtr<IMMDevice> pMMDevice;

        // get the "n"th device
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            ERR(_T("IMMDeviceCollection::Item failed: hr = 0x%08x"), hr);
            return hr;
        }

        // open the property store on that device
        CComPtr<IPropertyStore> pPropertyStore;
        hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        if (FAILED(hr)) {
            ERR(_T("IMMDevice::OpenPropertyStore failed: hr = 0x%08x"), hr);
            return hr;
        }

        // get the long name property
        PROPVARIANT pv; PropVariantInit(&pv);
        hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
        if (FAILED(hr)) {
            ERR(_T("IPropertyStore::GetValue failed: hr = 0x%08x"), hr);
            return hr;
        }

        if (VT_LPWSTR != pv.vt) {
            ERR(_T("PKEY_Device_FriendlyName variant type is %u - expected VT_LPWSTR"), pv.vt);

            PropVariantClear(&pv);
            return E_UNEXPECTED;
        }

        LOG(_T("%ls"), pv.pwszVal);
        PropVariantClear(&pv);
    }    
    
    return S_OK;    
}

HRESULT get_specific_device(LPCWSTR szLongName, IMMDevice **ppMMDevice) {
    HRESULT hr = S_OK;

    *ppMMDevice = NULL;
    
    // get an enumerator
    CComPtr<IMMDeviceEnumerator> pMMDeviceEnumerator;

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        ERR(_T("CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x"), hr);
        return hr;
    }

    CComPtr<IMMDeviceCollection> pMMDeviceCollection;

    // get all the active render endpoints
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(
        eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
    );
    if (FAILED(hr)) {
        ERR(_T("IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x"), hr);
        return hr;
    }

    UINT count;
    hr = pMMDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        ERR(_T("IMMDeviceCollection::GetCount failed: hr = 0x%08x"), hr);
        return hr;
    }

    for (UINT i = 0; i < count; i++) {
        CComPtr<IMMDevice> pMMDevice;

        // get the "n"th device
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            ERR(_T("IMMDeviceCollection::Item failed: hr = 0x%08x"), hr);
            return hr;
        }

        // open the property store on that device
        CComPtr<IPropertyStore> pPropertyStore;
        hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        if (FAILED(hr)) {
            ERR(_T("IMMDevice::OpenPropertyStore failed: hr = 0x%08x"), hr);
            return hr;
        }

        // get the long name property
        PROPVARIANT pv; PropVariantInit(&pv);
        hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
        if (FAILED(hr)) {
            ERR(_T("IPropertyStore::GetValue failed: hr = 0x%08x"), hr);
            return hr;
        }

        if (VT_LPWSTR != pv.vt) {
            ERR(_T("PKEY_Device_FriendlyName variant type is %u - expected VT_LPWSTR"), pv.vt);

            PropVariantClear(&pv);
            return E_UNEXPECTED;
        }

        // is it a match?
        if (0 == _wcsicmp(pv.pwszVal, szLongName)) {
            // did we already find it?
            if (NULL == *ppMMDevice) {
                *ppMMDevice = pMMDevice;
                (*ppMMDevice)->AddRef();
            } else {
                ERR(_T("Found (at least) two devices named %ls"), szLongName);
                (*ppMMDevice)->Release();
                *ppMMDevice = NULL;
                PropVariantClear(&pv);
                return E_UNEXPECTED;
            }
        }
        
        PropVariantClear(&pv);
    }
    
    if (NULL == *ppMMDevice) {
        ERR(_T("Could not find a device named %ls"), szLongName);
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    return S_OK;
}
