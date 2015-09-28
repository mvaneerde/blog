// apoenum.cpp

#include <windows.h>
#include <audioenginebaseapo.h>
#include <stdio.h>

#include "apoenum.h"
#include "details.h"
#include "log.h"

FNAPONOTIFICATIONCALLBACK myEnumerateAPOsCallback;

HRESULT EnumerateAudioProcessingObjects() {
    UINT32 nCount = 0;

    HRESULT hr = EnumerateAPOs(myEnumerateAPOsCallback, &nCount);
    if (FAILED(hr)) {
        ERR(L"EnumerateAPOs failed: hr = 0x%08x", hr);
        return hr;
    }
    
    LOG(L"APOs found: %u", nCount);
   
    return S_OK;
}

// note that APO_REG_PROPERTIES is a variable-sized structure so beware of slicing
// also note that it's only valid for the life of the callback;
// if you want to store the info for use later, make a deep copy
HRESULT WINAPI myEnumerateAPOsCallback(
    APO_REG_PROPERTIES *pProperties,
    VOID *pvRefData
) {
    UINT32 *pnCount = reinterpret_cast<UINT32 *>(pvRefData);
    if (NULL == pnCount) {
        ERR(L"myEnumerateAPOsCallback got a NULL pvRefData");
        return E_POINTER;
    }
    
    (*pnCount)++;
    
    return LogDetails(pProperties); // LogDetails logs its own failures
}