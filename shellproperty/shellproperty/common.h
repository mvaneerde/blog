// common.h

#include <windows.h>
#include <cguid.h>
#include <atlbase.h>
#include <pathcch.h>
#include <propvarutil.h>
#include <propkey.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <stdio.h>

#include "parseanddisplay.h"
#include "properties.h"

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) LOG(L"ERROR: " format, __VA_ARGS__)

#define GUID_FORMAT L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"
#define GUID_VALUES(g) \
    (g).Data1, (g).Data2, (g).Data3, \
    (g).Data4[0], (g).Data4[1], (g).Data4[2], (g).Data4[3], \
    (g).Data4[4], (g).Data4[5], (g).Data4[6], (g).Data4[7]

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() {
        CoUninitialize();
    }
};

class PropVariantClearOnExit {
public:
    PropVariantClearOnExit(PROPVARIANT *pv) : m_pv(pv) {}
    ~PropVariantClearOnExit() {
        HRESULT hr = PropVariantClear(m_pv);
        if (FAILED(hr)) {
            LOG(L"PropVariantClear failed: hr = 0x%08x", hr);
        }
    }
private:
    PROPVARIANT *m_pv;
};
