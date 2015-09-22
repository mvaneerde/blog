// main.cpp

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <mmdeviceapi.h>

#include "log.h"
#include "cleanup.h"
#include "devices.h"
#include "endpoints.h"

void usage(LPCWSTR exe) {
    LOG(L"%s [ --endpoints | --devices ]", exe);
}

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    bool endpoints = false;
    bool devices = false;
    
    switch (argc) {
        case 1:
            usage(argv[0]);
            return 0;
            
        case 2:
            if (0 == _wcsicmp(argv[1], L"--endpoints")) {
                endpoints = true;
            } else if (0 == _wcsicmp(argv[1], L"--devices")) {
                devices = true;
            } else {
                ERR(L"Unexpected argument %s", argv[1]);
                return E_INVALIDARG;
            }
            break;
            
        default:
            ERR(L"Expected 1 argument, not %d", argc);
            return E_INVALIDARG;
    }
    
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return hr;
    }
  
    CoUninitializeOnExit coUninit;

    if (endpoints) {
        hr = AudioEndpoints();
    } else if (devices) {
        hr = AudioDevices();
    } else {
        ERR(L"Neither endpoints nor devices was set");
        return E_INVALIDARG;
    }
    
    return (SUCCEEDED(hr) ? 0 : hr);
}