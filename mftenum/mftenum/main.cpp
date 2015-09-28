// main.cpp

#include <windows.h>
#include <objbase.h>
#include <stdio.h>

#include "log.h"
#include "mftenum.h"

int _cdecl wmain() {

    HRESULT hr;
    
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return hr;
    }
    
    hr = EnumerateMFTs();
    
    CoUninitialize();

    return (SUCCEEDED(hr) ? 0 : hr);
}