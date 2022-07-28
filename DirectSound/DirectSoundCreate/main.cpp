// main.cpp

#include <windows.h>
#include <objbase.h>
#include <stdio.h>

#include "log.h"
#include "prefs.h"
#include "cleanup.h"
#include "create.h"

int _cdecl wmain(int argc, LPCWSTR argv[]) {

    HRESULT hr;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return hr;
    }

    CoUninitializeOnExit coUninit;

    Prefs p;

    hr = p.Set(argc, argv);

    if (hr == S_FALSE) {
        return 0;
    }

    hr = Create(p.V8(), p.Render(), p.DeviceId());

    return (SUCCEEDED(hr) ? 0 : hr);
}
