// details.cpp

#include <windows.h>
#include <audioenginebaseapo.h>
#include <stdio.h>

#include "details.h"
#include "cleanup.h"
#include "log.h"

#define DISPLAY_FLAG(flag, x) \
    (((x) & (flag)) == (flag) ? L"        " L ## #flag L"\n" : L"")

#define DISPLAY_INVALID_FLAGS(mask, x) \
    (((x) & ~(mask)) == 0 ? L"" : L"        Invalid flags!\n")

HRESULT LogDetails(PAPO_REG_PROPERTIES pApo) {
    
    if (NULL == pApo) {
        ERR(L"LogDetails got a NULL pointer");
        return E_POINTER;
    }
    
    LPWSTR szClsid = NULL;
    HRESULT hr = StringFromIID(pApo->clsid, &szClsid);
    if (FAILED(hr)) {
        ERR(L"StringFromIID failed: hr = 0x%08x", hr);
        return hr;
    }
    CoTaskMemFreeOnExit freeClsid(szClsid);
    
    LOG(
        L"-- APO properties for %s --\n"
        L"    clsid: %s\n"
        L"    Flags: 0x%08x\n"
        L"%s%s%s%s%s"
        L"    szFriendlyName: \"%s\"\n"
        L"    szCopyrightInfo: \"%s\"\n"
        L"    u32MajorVersion: %u\n"
        L"    u32MinorVersion: %u\n"
        L"    u32MinInputConnections: %u\n"
        L"    u32MaxInputConnections: %u\n"
        L"    u32MinOutputConnections: %u\n"
        L"    u32MaxOutputConnections: %u\n"
        L"    u32MaxInstances: %u\n"
        L"    u32NumAPOInterfaces: %u",
        pApo->szFriendlyName,
        szClsid,
        pApo->Flags,
            DISPLAY_FLAG(APO_FLAG_INPLACE, pApo->Flags),
            DISPLAY_FLAG(APO_FLAG_SAMPLESPERFRAME_MUST_MATCH, pApo->Flags),
            DISPLAY_FLAG(APO_FLAG_FRAMESPERSECOND_MUST_MATCH, pApo->Flags),
            DISPLAY_FLAG(APO_FLAG_BITSPERSAMPLE_MUST_MATCH, pApo->Flags),
            DISPLAY_INVALID_FLAGS(
                APO_FLAG_INPLACE |
                APO_FLAG_SAMPLESPERFRAME_MUST_MATCH |
                APO_FLAG_FRAMESPERSECOND_MUST_MATCH |
                APO_FLAG_BITSPERSAMPLE_MUST_MATCH,
                pApo->Flags
            ),
        pApo->szFriendlyName,
        pApo->szCopyrightInfo,
        pApo->u32MajorVersion,
        pApo->u32MinorVersion,
        pApo->u32MinInputConnections,
        pApo->u32MaxInputConnections,
        pApo->u32MinOutputConnections,
        pApo->u32MaxOutputConnections,
        pApo->u32MaxInstances,
        pApo->u32NumAPOInterfaces
    );
    
    for (UINT32 i = 0; i < pApo->u32NumAPOInterfaces; i++) {
        LPWSTR szIid = NULL;
        hr = StringFromIID(pApo->iidAPOInterfaceList[i], &szIid);
        if (FAILED(hr)) {
            ERR(L"StringFromIID failed: hr = 0x%08x", hr);
            return hr;
        }
        CoTaskMemFreeOnExit freeIid(szIid);
        
        LOG(L"    #%u: %s", i + 1, szIid);
    }
    
    LOG(L""); // blank line

    return S_OK;
}