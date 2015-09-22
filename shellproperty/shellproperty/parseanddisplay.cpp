// parseanddisplay.cpp

#include "common.h"

HRESULT PropertyKeyFromName(LPCWSTR name, _Out_ PROPERTYKEY *key) {
    // first see if this is a registered name
    HRESULT hr = PSGetPropertyKeyFromName(name, key);
    if (SUCCEEDED(hr)) {
        return hr;
    }

    // next, see if this is a known key
    for (size_t i = 0; i < known_count; i++) {
        if (0 == _wcsicmp(name, known[i].name)) {
            *key = known[i].key;
            return S_OK;
        }
    }
    
    // finally, see if this is a "{fmtid} pid" string
    hr = PSPropertyKeyFromString(name, key);
    if (SUCCEEDED(hr)) {
        return S_OK;
    }
    
    ERR(L"Could not find a property named %s", name);
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT PropertyNameFromKey(const PROPERTYKEY &key, _Out_ LPWSTR *ppName) {
    // first see if this is a registered name
    HRESULT hr = PSGetNameFromPropertyKey(key, ppName);
    if (SUCCEEDED(hr)) {
        return hr;
    }

    // next, see if this is a known key
    for (size_t i = 0; i < known_count; i++) {
        if (key == known[i].key) {
            hr = SHStrDupW(known[i].name, ppName);
            if (FAILED(hr)) {
                ERR(L"ShStrDupW failed to copy %s: hr = 0x%08x", known[i].name, hr);
            }
            
            return hr;
        }
    }
    
    // finally, see if this is a "{fmtid} pid" string
    // this should not fail
    CComHeapPtr<WCHAR> string;
    if (!string.Allocate(PKEYSTR_MAX)) {
        ERR(L"Could not allocate %d WCHARs", PKEYSTR_MAX);
        return E_OUTOFMEMORY;
    }
    
    hr = PSStringFromPropertyKey(key, string, PKEYSTR_MAX);
    if (FAILED(hr)) {
        ERR(L"PSStringFromPropertyKey failed: hr = 0x%08x", hr);
        return hr;
    }
    
    *ppName = string.Detach();
    return S_OK;
}

HRESULT InitPropertyValueFromArgs(
    int cArgs,
    _In_reads_(cArgs) LPCWSTR szArgs[],
    _Out_ PROPVARIANT *value,
    _Out_ int *pArgsConsumed
) {
    HRESULT hr;

    if (cArgs < 1) {
        ERR(L"Expected at least one argument, not %d", cArgs);
        return E_INVALIDARG;
    }
    
    // VT_EMPTY
    if (0 == _wcsicmp(szArgs[0], L"VT_EMPTY")) {
        PropVariantInit(value);
        
        *pArgsConsumed = 1;
        return S_OK;
    }

    // VT_LPWSTR
    if (0 == _wcsicmp(szArgs[0], L"VT_LPWSTR")) {
        if (cArgs < 2) {
            ERR(L"VT_LPWSTR needs an additional argument");
            return E_INVALIDARG;
        }
        
        hr = InitPropVariantFromString(szArgs[1], value);
        if (FAILED(hr)) {
            ERR(L"InitPropVariantFromString failed: hr = 0x%08x", hr);
            return hr;
        }
        
        *pArgsConsumed = 2;
        return S_OK;
    }
    
    // VT_LPWSTR | VT_VECTOR
    if (
        0 == _wcsicmp(szArgs[0], L"VT_LPWSTR | VT_VECTOR") ||
        0 == _wcsicmp(szArgs[0], L"VT_VECTOR | VT_LPWSTR")
    ) {
        if (cArgs < 3) {
            ERR(L"VT_VECTOR | VT_LPWSTR needs at least two additional arguments");
            return E_INVALIDARG;
        }
        
        hr = InitPropVariantFromStringVector(&szArgs[1], cArgs - 1, value);
        if (FAILED(hr)) {
            ERR(L"InitPropVariantFromStringVector failed: hr = 0x%08x", hr);
            return hr;
        }
        
        *pArgsConsumed = cArgs;
        return S_OK;
    }

    // VT_UI4
    if (0 == _wcsicmp(szArgs[0], L"VT_UI4")) {
        if (cArgs < 2) {
            ERR(L"VT_UI4 needs an additional argument");
            return E_INVALIDARG;
        }
        
        WCHAR *end = NULL;
        UINT32 ulVal = wcstoul(szArgs[1], &end, 0);
        if (0 != errno || NULL == end || L'\0' != end[0]) {
            ERR(L"Could not parse %s as a 32-bit unsigned int", szArgs[1]);
            return E_INVALIDARG;
        }
        
        hr = InitPropVariantFromUInt32(ulVal, value);
        if (FAILED(hr)) {
            ERR(L"InitPropVariantFromUInt32 failed: hr = 0x%08x", hr);
            return hr;
        }
        
        *pArgsConsumed = 2;
        return S_OK;
    }
    
    ERR(L"Unexpected argument type %s", szArgs[0]);
    return E_INVALIDARG;
}

HRESULT PropertyStoreFromFilename(
    LPCWSTR name,
    GETPROPERTYSTOREFLAGS flags,
    _Out_ IPropertyStore **ppPropertyStore
) {
    HRESULT hr;
    CComHeapPtr<WCHAR> path;
    
    if (PathIsRelative(name)) {
        DWORD required = GetCurrentDirectory(0, NULL);
        CComHeapPtr<WCHAR> current;
        if (!current.Allocate(required)) {
            ERR(L"Could not allocate %d WCHARs", required);
            return E_OUTOFMEMORY;
        }
        
        DWORD written = GetCurrentDirectory(required, current);
        if (written != required - 1) {
            ERR(L"Expected GetCurrentDirectory to write %d characters; actually wrote %d", required - 1, written);
            return E_UNEXPECTED;
        }
        
        hr = PathAllocCombine(current, name, PATHCCH_ALLOW_LONG_PATHS, &path);
        if (FAILED(hr)) {
            ERR(L"PathAllocCombine failed to combine %s and %s: hr = 0x%08x", static_cast<LPCWSTR>(current), name, hr);
            return hr;
        }
    } else {
        hr = SHStrDupW(name, &path);
        if (FAILED(hr)) {
            ERR(L"ShStrDupW failed to copy %s: hr = 0x%08x", name, hr);
            return hr;
        }
   }

    // grab a shell item for the file
    CComPtr<IShellItem2> pShellItem2;
    hr = SHCreateItemFromParsingName(path, NULL, IID_PPV_ARGS(&pShellItem2));
    if (FAILED(hr)) {
        ERR(L"SHCreateItemFromParsingName failed: hr = 0x%08x", hr);
        return hr;
    }
    
    // open a property store
    CComPtr<IPropertyStore> pPropertyStore;
    hr = pShellItem2->GetPropertyStore(flags, IID_PPV_ARGS(ppPropertyStore));
    if (FAILED(hr)) {
        ERR(L"IShellItem2::GetPropertyStore failed: hr = 0x%08x", hr);
        return hr;
    }
    
    return S_OK;
}

HRESULT DisplayPropertyValue(const PROPERTYKEY &key, const PROPVARIANT &value) {
    // get the canonical name from the key
    CComHeapPtr<WCHAR> name;
    HRESULT hr = PropertyNameFromKey(key, &name);
    if (FAILED(hr)) {
        return hr;
    }

    // try the canonical way first
    CComPtr<IPropertyDescription> pPropertyDescription;
    hr = PSGetPropertyDescription(key, IID_PPV_ARGS(&pPropertyDescription));
    if (SUCCEEDED(hr)) {
        CComHeapPtr<WCHAR> strValue;
        hr = pPropertyDescription->FormatForDisplay(value, PDFF_DEFAULT, &strValue);
        if (FAILED(hr)) {
            ERR(L"IPropertyDescription::FormatForDisplay failed: hr = 0x%08x", hr);
            return hr;
        }
        
        LOG(L"%s: %s", name, static_cast<LPCWSTR>(strValue));
        
        return S_OK;
    }
    
    // the canonical way failed; log the raw value
    switch (value.vt) {
        case VT_EMPTY:
            LOG(L"%s: VT_EMPTY", name);
            return S_OK;
    
        case VT_LPWSTR:
            LOG(L"%s: VT_LPWSTR %s", name, value.pwszVal);
            return S_OK;
            
        case VT_VECTOR | VT_LPWSTR:
            LOG(L"%s: VT_VECTOR | VT_LPWSTR", name);
            for (UINT i = 0; i < value.calpwstr.cElems; i++) {
                LOG(L"    %u: %s", i + 1, value.calpwstr.pElems[i]);
            }
            return S_OK;
        
        case VT_BSTR:
            LOG(L"%s: VT_BSTR %s", name, value.bstrVal);
            return S_OK;
        
        case VT_BOOL:
            LOG(L"%s: VT_BOOL %s", name, (value.boolVal ? L"TRUE" : L"FALSE"));
            return S_OK;
        
        case VT_I2:
            LOG(L"%s: VT_I2 %d (0x%x)", name, value.iVal, value.iVal);
            return S_OK;

        case VT_UI2:
            LOG(L"%s: VT_UI2 %u (0x%x)", name, value.uiVal, value.uiVal);
            return S_OK;

        case VT_I4:
            LOG(L"%s: VT_I4 %d (0x%x)", name, value.lVal, value.lVal);
            return S_OK;

        case VT_UI4:
            LOG(L"%s: VT_UI4 %u (0x%x)", name, value.ulVal, value.ulVal);
            return S_OK;

        case VT_I8:
            LOG(L"%s: VT_I8 %I64d (0x%I64x)", name, value.hVal.QuadPart, value.hVal.QuadPart);
            return S_OK;

        case VT_UI8:
            LOG(L"%s: VT_UI8 %I64u (0x%I64x)", name, value.uhVal.QuadPart, value.uhVal.QuadPart);
            return S_OK;
        
        case VT_FILETIME: {
            FILETIME ft = value.filetime;
            SYSTEMTIME st;
            WCHAR szLocalDate[255] = {};
            WCHAR szLocalTime[255] = {};

            FileTimeToLocalFileTime( &ft, &ft );
            FileTimeToSystemTime( &ft, &st );
            GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szLocalDate, ARRAYSIZE(szLocalDate) );
            GetTimeFormat( LOCALE_USER_DEFAULT, 0, &st, NULL, szLocalTime, ARRAYSIZE(szLocalDate) );
            LOG(
                L"%s: VT_FILETIME 0x%08x%08x (%s %s local time)",
                name, value.filetime.dwHighDateTime, value.filetime.dwLowDateTime,
                szLocalDate, szLocalTime
            );
            return S_OK;
        }
        
        case VT_CLSID:
            LOG(L"%s: VT_CLSID " GUID_FORMAT, name, GUID_VALUES(*(value.puuid)));
            return S_OK;
        
        default:
            ERR(L"%s has unhandled variant type %u (0x%x)", name, value.vt, value.vt);
            return E_INVALIDARG;
    }
}
