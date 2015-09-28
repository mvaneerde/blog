// main.cpp
#include <windows.h>
#include <initguid.h>
#include <cguid.h>
#include <tchar.h>
#include <objbase.h>
#include <atlstr.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#include "log.h"
#include "prefs.h"
#include "setotapolicy.h"

class CCoUninitializeOnExit {
public:
    ~CCoUninitializeOnExit() { CoUninitialize(); }
};

int _cdecl _tmain(int argc, LPCTSTR argv[]) {
    HRESULT hr = S_OK;

    // coinitialize
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        ERR(_T("CoInitializeEx(COINIT_APARTMENTTHREADED) failed: hr = 0x%08x"), hr);
        return hr;
    }

    CCoUninitializeOnExit c;
    
    // parse command line
    CPrefs prefs(argc, argv, hr);

    if (FAILED(hr)) {
        // CPrefs::CPrefs will log the appropriate error
        return hr;
    }

    if (S_FALSE == hr) {
        // usage statement... skip execution
        return 0;
    }

    hr = SetOTAPolicy(
        prefs.pMMDevice,
        prefs.bCopyOK,
        prefs.bDigitalOutputDisable,
        prefs.bTestCertificateEnable,
        prefs.dwDrmLevel
    );

    if (FAILED(hr)) {
        // SetOTAPolicy will log the appropriate error
        return hr;
    }

    return 0;
}
