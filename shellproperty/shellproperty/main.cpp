// main.cpp

#include <initguid.h>

#include "common.h"

void usage() {
    LOG(
        L"shellproperty read [ <key> | all ] from <filename>\n"
        L"shellproperty set <key> on <filename> to <vartype> <vartype-specific-arguments>\n"
        L"\n"
        L"<vartype>: VT_EMPTY | VT_LPWSTR | \"VT_VECTOR | VT_LPWSTR\" | VT_UI4"
    );
}

int _cdecl wmain(int argc, _In_reads_(argc) LPCWSTR argv[]) {

    if (0 == argc - 1) {
        usage();
        return 0;
    }

    if (2 > argc - 1) {
        ERR(L"Expected at least two arguments, not %d", argc - 1);
        return -__LINE__;
    }

    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;
    
    if (0 == _wcsicmp(L"read", argv[1])) {
        if (3 != argc - 2) {
            ERR(L"Expected three arguments to \"read\" not %d", argc - 2);
            return -__LINE__;
        }
    
        if (0 == _wcsicmp(argv[2], L"all")) {
            if (0 != _wcsicmp(L"from", argv[3])) {
                ERR(L"Third argument must be \"from\", not %s", argv[3]);
                return -__LINE__;
            }

            CComPtr<IPropertyStore> pPropertyStore;
            hr = PropertyStoreFromFilename(argv[4], GPS_DEFAULT, &pPropertyStore);
            if (FAILED(hr)) {
                return -__LINE__;
            }
            
            DWORD count;
            hr = pPropertyStore->GetCount(&count);
            if (FAILED(hr)) {
                ERR(L"IPropertyStore::GetCount failed: hr = 0x%08x", hr);
                return -__LINE__;
            }
            
            for (DWORD i = 0; i < count; i++) {
                PROPERTYKEY key = {};
                hr = pPropertyStore->GetAt(i, &key);
                if (FAILED(hr)) {
                    ERR(L"IPropertyStore::GetAt failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                PROPVARIANT value; PropVariantInit(&value);
                hr = pPropertyStore->GetValue(key, &value);
                if (FAILED(hr)) {
                    ERR(L"IPropertyStore::GetValue failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                PropVariantClearOnExit pvcoe(&value);
                
                // display it
                hr = DisplayPropertyValue(key, value);
                if (FAILED(hr)) {
                    return -__LINE__;
                }
            }
        } else {
            // get the key from the name
            PROPERTYKEY key = {0};
            hr = PropertyKeyFromName(argv[2], &key);
            if (FAILED(hr)) {
                return -__LINE__;
            }
            
            if (0 != _wcsicmp(L"from", argv[3])) {
                ERR(L"Third argument must be \"from\", not %s", argv[3]);
                return -__LINE__;
            }

            CComPtr<IPropertyStore> pPropertyStore;
            hr = PropertyStoreFromFilename(argv[4], GPS_DEFAULT, &pPropertyStore);
            if (FAILED(hr)) {
                return -__LINE__;
            }
            
            // read the property
            PROPVARIANT value; PropVariantInit(&value);
            hr = pPropertyStore->GetValue(key, &value);
            if (FAILED(hr)) {
                ERR(L"IPropertyStore::GetValue failed: hr = 0x%08x", hr);
                return -__LINE__;
            }
            PropVariantClearOnExit pvcoe(&value);

            // display it
            hr = DisplayPropertyValue(key, value);
            if (FAILED(hr)) {
                return -__LINE__;
            }
        }
    } else if (0 == _wcsicmp(L"set", argv[1])) {
        if (5 > argc - 2) {
            ERR(L"Expected at least five arguments to \"set\" not %d", argc - 2);
            return -__LINE__;
        }
    
        // see if this is a known key
        PROPERTYKEY key = {0};
        hr = PropertyKeyFromName(argv[2], &key);
        if (FAILED(hr)) {
            return -__LINE__;
        }

        if (0 != _wcsicmp(L"on", argv[3])) {
            ERR(L"Third argument must be \"on\", not %s", argv[3]);
            return -__LINE__;
        }

        CComPtr<IPropertyStore> pPropertyStore;
        hr = PropertyStoreFromFilename(argv[4], GPS_READWRITE, &pPropertyStore);
        if (FAILED(hr)) {
            return -__LINE__;
        }

        if (0 != _wcsicmp(L"to", argv[5])) {
            ERR(L"Fifth argument must be \"to\", not %s", argv[5]);
            return -__LINE__;
        }
        
        int argsConsumed = 0;
        PROPVARIANT value;
        hr = InitPropertyValueFromArgs(argc - 6, &argv[6], &value, &argsConsumed);
        if (FAILED(hr)) {
            return -__LINE__;
        }
        PropVariantClearOnExit pvcoe(&value);
        
        if (argsConsumed != argc - 6) {
            ERR(L"Args consumed: %d (expected %d)", argsConsumed, argc - 6);
            return -__LINE__;
        }
        
        // write the property
        hr = pPropertyStore->SetValue(key, value);
        if (FAILED(hr)) {
            ERR(L"IPropertyStore::SetValue failed: hr = 0x%08x", hr);
            return -__LINE__;
        }

        // commit
        hr = pPropertyStore->Commit();
        if (FAILED(hr)) {
            ERR(L"IPropertyStore::Commit failed: hr = 0x%08x", hr);
            return -__LINE__;
        }
    }
        
    return 0;
}
