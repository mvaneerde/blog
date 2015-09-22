// main.cpp

#include <windows.h>
#include <stdio.h>

#pragma warning ( push )
#pragma warning ( disable : 4996 ) // sphelper.h uses deprecated GetVersionEx
#include <sphelper.h>
#pragma warning ( pop )

#include <sapi.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) wprintf(L"ERROR:" format L"\n", __VA_ARGS__)

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() { CoUninitialize(); }
};

class ReleaseOnExit {
public:
    ReleaseOnExit(IUnknown *p) : m_p(p) {}
    ~ReleaseOnExit() { m_p->Release(); }
private:
    IUnknown *m_p;
};

class CoTaskMemFreeOnExit {
public:
    CoTaskMemFreeOnExit(PVOID p) : m_p(p) {}
    ~CoTaskMemFreeOnExit() { CoTaskMemFree(m_p); }
private:
    PVOID m_p;
};

void DumpCategory(LPCWSTR category);
void Tab(int nTabLevel);
void EnumDataKey(int nTabLevel, ISpDataKey *pSpDataKey);

int _cdecl wmain() {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;

    struct Category {
        LPCWSTR name;
        LPCWSTR category;
        
        Category(LPCWSTR n, LPCWSTR c) : name(n), category(c) {}
    };

#define CATEGORY_ENTRY(x) Category(L ## #x, x)
    
    Category categories[] = {
        CATEGORY_ENTRY(SPCAT_AUDIOOUT),
        CATEGORY_ENTRY(SPCAT_AUDIOIN),
        CATEGORY_ENTRY(SPCAT_VOICES),
        CATEGORY_ENTRY(SPCAT_RECOGNIZERS),
        CATEGORY_ENTRY(SPCAT_APPLEXICONS),
        CATEGORY_ENTRY(SPCAT_PHONECONVERTERS),
        CATEGORY_ENTRY(SPCAT_RECOPROFILES)
    };
    
    for (int i = 0; i < ARRAYSIZE(categories); i++) {
        LOG(L"-- %s --", categories[i].name);
        DumpCategory(categories[i].category);
        LOG(L"");
    }
    
}

void DumpCategory(LPCWSTR category) {
    // enumerate tokens in each category
    IEnumSpObjectTokens *pEnumSpObjectTokens = nullptr;
    HRESULT hr = SpEnumTokens(category, nullptr, nullptr, &pEnumSpObjectTokens);
    if (SPERR_NOT_FOUND == hr) {
        LOG(L"  None found.");
        return;
    } else if (FAILED(hr)) {
        ERR(L"SpEnumTokens failed: hr = 0x%08x", hr);
        return;
    }
    ReleaseOnExit rEnumSpObjectTokens(pEnumSpObjectTokens);
    
    ULONG nTokens = 0;
    hr = pEnumSpObjectTokens->GetCount(&nTokens);
    if (FAILED(hr)) {
        ERR(L"IEnumSpObjectTokens::GetCount failed: hr = 0x%08x", hr);
        return;
    }
    
    for (ULONG token = 0; token < nTokens; token++) {
        ISpObjectToken *pSpObjectToken = nullptr;
        hr = pEnumSpObjectTokens->Next(1, &pSpObjectToken, nullptr);
        if (FAILED(hr)) {
            ERR(L"IEnumSpObjectTokens::Next failed: hr = 0x%08x", hr);
            return;
        }
        ReleaseOnExit rSpObjectToken(pSpObjectToken);
        
        LPWSTR description = nullptr;
        hr = SpGetDescription(pSpObjectToken, &description);
        if (FAILED(hr)) {
            ERR(L"SpGetDescription failed: hr = 0x%08x", hr);
            continue;
        }
        CoTaskMemFreeOnExit fDescription(description);
        
        LOG(L"  #%u: %s", token + 1, description);
 
        EnumDataKey(2, pSpObjectToken);
    }
}

void Tab(int nTabLevel) {
    for (int i = 0; i < nTabLevel; i++) { wprintf(L"  "); }
}

void EnumDataKey(int nTabLevel, ISpDataKey *pSpDataKey) {
    HRESULT hr = S_OK;
    
    // enumerate subkeys recursively
    for (ULONG k = 0; ; k++) {
        LPWSTR key = nullptr;
        hr = pSpDataKey->EnumKeys(k, &key);
        if (SPERR_NO_MORE_ITEMS == hr) {
            // done
            break;
        } else if (FAILED(hr)) {
            ERR(L"ISpDataKey::EnumKeys failed: hr = 0x%08x", hr);
            continue;
        }
        CoTaskMemFreeOnExit fKey(key);
        
        Tab(nTabLevel);
        LOG(L"%s", key);
        
        ISpDataKey *pSubKey = nullptr;
        hr = pSpDataKey->OpenKey(key, &pSubKey);
        if (FAILED(hr)) {
            ERR(L"ISpDataKey::OpenKeys failed: hr = 0x%08x", hr);
            continue;
        }
        ReleaseOnExit rSubKey(pSubKey);
        
        EnumDataKey(nTabLevel + 1, pSubKey);
    }
    
    // enumerate values
    for (ULONG v = 0; ; v++) {
        LPWSTR val = nullptr;
        hr = pSpDataKey->EnumValues(v, &val);
        if (SPERR_NO_MORE_ITEMS == hr) {
            // done
            break;
        } else if (FAILED(hr)) {
            ERR(L"ISpDataKey::EnumKeys failed: hr = 0x%08x", hr);
            continue;
        }
        CoTaskMemFreeOnExit fVal(val);
        
        // how do we know whether it's a string or a DWORD?
        LPWSTR data = nullptr;
        hr = pSpDataKey->GetStringValue(val, &data);
        if (FAILED(hr)) {
            ERR(L"ISpDataKey::GetStringValue failed: hr = 0x%08x", hr);
            continue;
        }
        CoTaskMemFreeOnExit fData(data);
        
        Tab(nTabLevel);
        if (0 == wcscmp(val, L"")) {
            LOG(L"(default) = %s", data);
        } else {
            LOG(L"%s = %s", val, data);
        }
    }
}