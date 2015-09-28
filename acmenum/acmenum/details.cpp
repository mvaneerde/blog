// details.cpp

#include <windows.h>
#include <mmreg.h>
#include <mmiscapi.h>
#include <msacm.h>
#include <stdio.h>

#include "details.h"
#include "log.h"

#define CHARS_FROM_FOURCC(x) \
    (CHAR)((x) & 0xff), \
    (CHAR)(((x) >> 8) & 0xff), \
    (CHAR)(((x) >> 16) & 0xff), \
    (CHAR)(((x) >> 24) & 0xff) \
    
#define COMPONENTS_FROM_VERSION_DWORD(x) \
    ((x) >> 24), \
    (((x) >> 16) & 0xff), \
    (((x) >> 8) & 0xffff)

#define DISPLAY_FLAG(flag, x) \
    (((x) & (flag)) == (flag) ? L"        " L ## #flag L"\n" : L"")

#define DISPLAY_INVALID_FLAGS(mask, x) \
    (((x) & ~(mask)) == 0 ? L"" : L"        Invalid flags!\n")

    
void LogDetails(ACMDRIVERDETAILS details) {
        
    LOG(
        L"-- ACM Driver Details: %s --\n"
        L"    cbStruct: %u\n"
        L"    fccType: 0x%08x (%hc%hc%hc%hc)\n"
        L"    fccComp: 0x%08x (%hc%hc%hc%hc)\n"
        L"    wMid: %u\n"
        L"    wPid: %u\n"
        L"    vdwACM: 0x%08x (%u.%u.%u)\n"
        L"    vdwDriver: 0x%08x (%u.%u.%u)\n"
        L"    fdwSupport: 0x%08x\n"
        L"%s%s%s%s%s%s%s%s"
        L"    cFormatTags: %d\n"
        L"    cFilterTags: %d\n"
        L"    hicon: 0x%p\n"
        L"    szShortName: \"%s\"\n"
        L"    szLongName: \"%s\"\n"
        L"    szCopyright: \"%s\"\n"
        L"    szLicensing: \"%s\"\n"
        L"    szFeatures: \"%s\"",
        
        details.szShortName,
        details.cbStruct,
        details.fccType, CHARS_FROM_FOURCC(details.fccType ? details.fccType : 0x20202020), // print spaces rather than \0s
        details.fccComp, CHARS_FROM_FOURCC(details.fccComp ? details.fccComp : 0x20202020), // to allow piping
        details.wMid,
        details.wPid,
        details.vdwACM, COMPONENTS_FROM_VERSION_DWORD(details.vdwACM),
        details.vdwDriver, COMPONENTS_FROM_VERSION_DWORD(details.vdwDriver),
        details.fdwSupport,
            DISPLAY_FLAG( ACMDRIVERDETAILS_SUPPORTF_ASYNC, details.fdwSupport ),
            DISPLAY_FLAG( ACMDRIVERDETAILS_SUPPORTF_CODEC, details.fdwSupport ),
            DISPLAY_FLAG( ACMDRIVERDETAILS_SUPPORTF_CONVERTER, details.fdwSupport ),
            DISPLAY_FLAG( ACMDRIVERDETAILS_SUPPORTF_DISABLED, details.fdwSupport ),
            DISPLAY_FLAG( ACMDRIVERDETAILS_SUPPORTF_FILTER, details.fdwSupport ),
            DISPLAY_FLAG( ACMDRIVERDETAILS_SUPPORTF_HARDWARE, details.fdwSupport ),
            DISPLAY_FLAG( ACMDRIVERDETAILS_SUPPORTF_LOCAL, details.fdwSupport ),
            DISPLAY_INVALID_FLAGS(
                ACMDRIVERDETAILS_SUPPORTF_ASYNC |
                ACMDRIVERDETAILS_SUPPORTF_CODEC |
                ACMDRIVERDETAILS_SUPPORTF_CONVERTER |
                ACMDRIVERDETAILS_SUPPORTF_DISABLED |
                ACMDRIVERDETAILS_SUPPORTF_FILTER |
                ACMDRIVERDETAILS_SUPPORTF_HARDWARE |
                ACMDRIVERDETAILS_SUPPORTF_LOCAL,
                details.fdwSupport
            ),
        details.cFormatTags,
        details.cFilterTags,
        details.hicon, // this is a shared icon - do not call DestroyIcon(..) on this HICON
        details.szShortName,
        details.szLongName,
        details.szCopyright,
        details.szLicensing,
        details.szFeatures
    );
    
}