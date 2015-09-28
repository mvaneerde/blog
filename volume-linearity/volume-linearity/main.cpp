// main.cpp

#include <windows.h>
#include <endpointvolume.h>
#include <stdio.h>
#include "cleanup.h"
#include "log.h"
#include "meter.h"
#include "vary.h"
#include "render.h"
#include "endpoint.h"
#include "capture.h"

int _cdecl wmain(int argc, LPCWSTR argv[]) {

    if (1 == argc) {
        LOG(
            L"%s --signal | --stream | --session | --channel |\n"
            L"    --endpoint-db | --capture\n"
            L"--signal varies the amplitude of the generated signal from 0 to 1\n"
            L"--stream varies the IAudioStreamVolume from 0 to 1\n"
            L"--session varies the ISimpleAudioVolume from 0 to 1\n"
            L"--channel varies the IChannelAudioVolume from 0 to 1\n"
            L"--endpoint-db varies the IAudioEndpointVolume from X dB to Y dB\n"
            L"    where X and Y are the min and max values supported\n"
            L"--capture varies session, channel, and endpoint-db volumes\n"
            L"    on the default capture device\n"
            ,
            argv[0]
        );
        return 0;
    }
    
    if (2 != argc) {
        ERR(L"Expected one argument, not %d", argc - 1);
        return -__LINE__;
    }

    EVary which = eSignal;
    if (0 == _wcsicmp(argv[1], L"--signal")) {
        which = eSignal;
    } else if (0 == _wcsicmp(argv[1], L"--stream")) {
        which = eAudioStreamVolume;
    } else if (0 == _wcsicmp(argv[1], L"--session")) {
        which = eSimpleAudioVolume;
    } else if (0 == _wcsicmp(argv[1], L"--channel")) {
        which = eChannelAudioVolume;
    } else if (0 == _wcsicmp(argv[1], L"--endpoint-db")) {
        which = eAudioEndpointVolume;
    } else if (0 == _wcsicmp(argv[1], L"--capture")) {
        which = eCaptureVolume;
    } else {
        ERR(L"Unrecognized argument %s", argv[1]);
        return E_INVALIDARG;
    }
    
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;
    
    // activate an IAudioMeterInformation on the default console render device
    IAudioMeterInformation *pAudioMeterInformation = NULL;
    hr = GetAudioMeterInformation(&pAudioMeterInformation);
    if (FAILED(hr)) {
        return -__LINE__;
    }
    ReleaseOnExit releaseAudioMeterInformation(pAudioMeterInformation);
    
    // dump the hardware-powered-ness
    DWORD dwHardwareSupport;
    hr = pAudioMeterInformation->QueryHardwareSupport(&dwHardwareSupport);
    if (FAILED(hr)) {
        ERR(L"IAudioMeterInformation::QueryHardwareSupport failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    if (dwHardwareSupport == 0) {
        LOG(L"The audio meter is entirely software-based.");
    }
    
    if (dwHardwareSupport & ENDPOINT_HARDWARE_SUPPORT_VOLUME) {
        LOG(L"There is a hardware volume control");
        dwHardwareSupport &= ~ENDPOINT_HARDWARE_SUPPORT_VOLUME;
    }
    
    if (dwHardwareSupport & ENDPOINT_HARDWARE_SUPPORT_MUTE) {
        LOG(L"There is a hardware mute");
        dwHardwareSupport &= ~ENDPOINT_HARDWARE_SUPPORT_MUTE;
    }

    if (dwHardwareSupport & ENDPOINT_HARDWARE_SUPPORT_METER) {
        LOG(L"There is a hardware peak meter");
        dwHardwareSupport &= ~ENDPOINT_HARDWARE_SUPPORT_METER;
    }

    if (dwHardwareSupport != 0) {
        ERR(L"Unrecognized flags in hardware support: 0x%08x", dwHardwareSupport);
        return -__LINE__;
    }
 
    switch (which) {
        case eSignal:
        case eAudioStreamVolume:
        case eSimpleAudioVolume:
        case eChannelAudioVolume:
            hr = Render(pAudioMeterInformation, which);
            if (FAILED(hr)) {
                return hr;
            }
            break;
        
        case eAudioEndpointVolume:
            hr = Endpoint();
            if (FAILED(hr)) {
                return hr;
            }
            break;

        case eCaptureVolume:
            hr = Capture();
            if (FAILED(hr)) {
                return hr;
            }
            break;
            
        default:
            ERR(L"EVary type %u is not recognized", which);
            return E_INVALIDARG;
   }
    
    return 0;
}