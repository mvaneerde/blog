// mftenum.cpp

#include <windows.h>
#include <stdio.h>
#include <mfapi.h>
#include <mftransform.h>
#include <mferror.h>
#include <ks.h>

#include "cleanup.h"
#include "log.h"
#include "mftenum.h"


HRESULT EnumerateMFTCategory(GUID category);
HRESULT DisplayMFT(IMFActivate *pMFActivate);

struct MFT_Category {
    GUID guid;
    LPCWSTR name;
};

HRESULT EnumerateMFTs() {

    MFT_Category categories[] = {
        { MFT_CATEGORY_AUDIO_DECODER, L"Audio decoders" },
        { MFT_CATEGORY_AUDIO_EFFECT, L"Audio effects" },
        { MFT_CATEGORY_AUDIO_ENCODER, L"Audio encoders" },
        { MFT_CATEGORY_DEMULTIPLEXER, L"Demultiplexers" },
        { MFT_CATEGORY_MULTIPLEXER, L"Multiplexers" },
        { MFT_CATEGORY_OTHER, L"Miscellaneous" },
        { MFT_CATEGORY_VIDEO_DECODER, L"Video decoders" },
        { MFT_CATEGORY_VIDEO_EFFECT, L"Video effects" },
        { MFT_CATEGORY_VIDEO_ENCODER, L"Video encoders" },
        { MFT_CATEGORY_VIDEO_PROCESSOR, L"Video processors" },
        
        { KSCATEGORY_DATACOMPRESSOR, L"KS data compressors" },
        { KSCATEGORY_DATADECOMPRESSOR, L"KS data decompressors" },
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
        
        hr = EnumerateMFTCategory(categories[i].guid);
        if (FAILED(hr)) { hrRet = hr; }
        
        LOG(L""); // blank line
    }
    
    return hrRet;
}

HRESULT EnumerateMFTCategory(GUID category) {
    HRESULT hrRet = S_OK;
    
    DWORD dwFlags = 0
        // enumerate all three kinds of data flow
        | MFT_ENUM_FLAG_SYNCMFT
        | MFT_ENUM_FLAG_ASYNCMFT
        | MFT_ENUM_FLAG_HARDWARE
        
        // include not-usually-included kinds of MFTs
        | MFT_ENUM_FLAG_FIELDOFUSE
        | MFT_ENUM_FLAG_LOCALMFT
        | MFT_ENUM_FLAG_TRANSCODE_ONLY
        
        // DO NOT filter or sort the results
        // we want to include even blocked MFTs in this list
        // EXPLICITLY NOT SETTING MFT_ENUM_FLAG_SORTANDFILTER
    ;
    IMFActivate **ppMFActivateObjects = NULL; // array of pointers
    UINT32 cMFActivateObjects = 0;
    
    hrRet = MFTEnumEx(
        category,
        dwFlags,
        NULL, // any input type
        NULL, // any output type
        &ppMFActivateObjects,
        &cMFActivateObjects
    );
    
    if (FAILED(hrRet)) {
        ERR(L"MFTEnumEx failed: hr = 0x%08x", hrRet);
        return hrRet;
    }
    
    if (0 == cMFActivateObjects) {
        LOG(L"No IMFTransforms found.");
        return S_FALSE;
    }

    DisposeOfMFActivateArray dispose(ppMFActivateObjects, cMFActivateObjects);
    
    for (UINT32 i = 0; i < cMFActivateObjects; i++) {
        HRESULT hr = DisplayMFT(ppMFActivateObjects[i]);
        if (FAILED(hr)) {
            hrRet = hr;
            continue;
        }
    }
    
    return hrRet;    
}

HRESULT DisplayMFT(IMFActivate *pMFActivate) {
    HRESULT hr;

    // get the CLSID GUID from the IMFAttributes of the activation object
    GUID guidMFT = { 0 };
    hr = pMFActivate->GetGUID(MFT_TRANSFORM_CLSID_Attribute, &guidMFT);
    if (MF_E_ATTRIBUTENOTFOUND == hr) {
        LOG(L"IMFTransform has no CLSID.");
        return hr;
    } else if (FAILED(hr)) {
        ERR(L"IMFAttributes::GetGUID(MFT_TRANSFORM_CLSID_Attribute) failed: hr = 0x%08x", hr);
        return hr;
    }

    LPWSTR szGuid = NULL;
    hr = StringFromIID(guidMFT, &szGuid);
    if (FAILED(hr)) {
        ERR(L"StringFromIID failed: hr = 0x%08x", hr);
        return hr;
    }
    CoTaskMemFreeOnExit freeGuid(szGuid);
    
    // get the friendly name string from the IMFAttributes of the activation object
    LPWSTR szFriendlyName = NULL;
#pragma prefast(suppress: __WARNING_PASSING_FUNCTION_UNEXPECTED_NULL, "IMFAttributes::GetAllocatedString third argument is optional");
    hr = pMFActivate->GetAllocatedString(
        MFT_FRIENDLY_NAME_Attribute,
        &szFriendlyName,
        NULL // don't care about the length of the string, and MSDN agrees, but SAL annotation is missing "opt"
    );
  
    if (MF_E_ATTRIBUTENOTFOUND == hr) {
        LOG(L"IMFTransform has no friendly name.");
        return hr;
    } else if (FAILED(hr)) {
        ERR(L"IMFAttributes::GetAllocatedString(MFT_FRIENDLY_NAME_Attribute) failed: hr = 0x%08x", hr);
        return hr;
    }
    CoTaskMemFreeOnExit freeFriendlyName(szFriendlyName);
    
    LOG(L"%s (%s)", szFriendlyName, szGuid);
    
    return S_OK;
}