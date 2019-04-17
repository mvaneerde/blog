// dmoenum.cpp

#include <windows.h>
#include <objbase.h>
#include <dmo.h>
#include <dmoreg.h>
#include <stdio.h>

#include "log.h"
#include "dmoenum.h"
#include "cleanup.h"

HRESULT EnumerateDMOCategory(GUID category);

struct DMO_Category {
    GUID guid;
    LPCWSTR name;
};

HRESULT EnumerateDMOs() {
    DMO_Category categories[] = {
        { DMOCATEGORY_AUDIO_DECODER, L"Audio decoders" },
        { DMOCATEGORY_AUDIO_EFFECT, L"Audio effects" },
        { DMOCATEGORY_AUDIO_ENCODER, L"Audio encoders" },
        { DMOCATEGORY_VIDEO_DECODER, L"Video decoders" },
        { DMOCATEGORY_VIDEO_EFFECT, L"Video effects" },
        { DMOCATEGORY_VIDEO_ENCODER, L"Video encoders" },
        { DMOCATEGORY_AUDIO_CAPTURE_EFFECT, L"Audio capture effects" },
    };

    HRESULT hrRet = S_OK;
    
    for (int i = 0; i < ARRAYSIZE(categories); i++) {
        LPWSTR szGuid = NULL;
        HRESULT hr = StringFromIID(categories[i].guid, &szGuid);
        if (FAILED(hr)) {
            ERR(L"StringFromIID failed: hr = 0x%08x", hr);
            hrRet = hr;
            continue;
        }
    
        CoTaskMemFreeOnExit freeGuid(szGuid);
    
        LOG(L"-- %s (%s) --", categories[i].name, szGuid);
        
        hr = EnumerateDMOCategory(categories[i].guid);
        if (FAILED(hr)) { hrRet = hr; }
        
        LOG(L""); // blank line
    }
    
    return hrRet;
}

HRESULT EnumerateDMOCategory(GUID category) {
    IEnumDMO *pEnumDMO = NULL;

    HRESULT hr = DMOEnum(
        category,
        DMO_ENUMF_INCLUDE_KEYED, // include even DMOs that need to be unlocked
        0, NULL, // all input types
        0, NULL, // all output types
        &pEnumDMO
    );
    if (FAILED(hr)) {
        ERR(L"DMOEnum(...) failed: hr = 0x%08x", hr);
        return hr;
    }
    
    ReleaseOnExit releaseEnum(pEnumDMO);
    
    // let's get them one at a time
    for (;;) {
        CLSID iid = {0};
        LPWSTR szName = NULL;
        DWORD dwItemsFetched = 0;
        
        hr = pEnumDMO->Next(1, &iid, &szName, &dwItemsFetched);
        if (FAILED(hr)) {
            ERR(L"IEnumDMO::Next failed: hr = 0x%08x", hr);
            return hr;
        }
        if (0 == dwItemsFetched) {
            // that's all there is... there isn't any more
            break;
        }
        CoTaskMemFreeOnExit freeName(szName);
       
        LPWSTR szGuid = NULL;
        hr = StringFromIID(iid, &szGuid);
        if (FAILED(hr)) {
            ERR(L"StringFromIID failed: hr = 0x%08x", hr);
            return hr;
        }
        CoTaskMemFreeOnExit freeGuid(szGuid);
        
        LOG(L"%s (%s)", szName, szGuid);
    }
    
    return S_OK;
}