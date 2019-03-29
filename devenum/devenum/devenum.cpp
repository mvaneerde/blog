// dmoenum.cpp

#include <windows.h>
#include <objbase.h>
#include <stdio.h>

// hack to avoid needing to install the DirectX SDK
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__

#include <dshow.h>
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>

// to get this file, download and install the Windows Vista SDK
// http://www.microsoft.com/en-us/download/confirmation.aspx?id=23719
// the file will be at "C:\Program Files\Microsoft SDKs\Windows\v6.0\Include\qedit.h"
//
// this in turn depends on dxtrans.h from the August 2007 DirectX SDK, so download that too
// http://www.microsoft.com/en-us/download/details.aspx?id=13287
// the file will be at "C:\Program Files (x86)\Microsoft DirectX SDK (August 2007)\Include\dxtrans.h"
//
#include <qedit.h>

#include "log.h"
#include "devenum.h"
#include "cleanup.h"

HRESULT EnumerateDirectShowFilterCategory(
    ICreateDevEnum *pCreateDevEnum,
    GUID category
);

HRESULT DisplayDirectShowFilter(IMoniker *pMoniker);

struct Filter_Category {
    LPCWSTR name;
    GUID guid;
};

HRESULT EnumerateDirectShowFilters() {
    Filter_Category categories[] = {
        { L"Audio Capture Sources", CLSID_AudioInputDeviceCategory },
        { L"Audio Compressors", CLSID_AudioCompressorCategory },
        { L"Audio Renderers", CLSID_AudioRendererCategory },
        { L"Device Control Filters", CLSID_DeviceControlCategory },
        { L"DirectShow Filters", CLSID_LegacyAmFilterCategory },
        { L"External Renderers", CLSID_TransmitCategory },
        { L"Midi Renderers", CLSID_MidiRendererCategory },
        { L"Video Capture Sources", CLSID_VideoInputDeviceCategory },
        { L"Video Compressors", CLSID_VideoCompressorCategory },
        { L"WDM Stream Decompression Devices", CLSID_DVDHWDecodersCategory },
        { L"WDM Streaming Capture Devices", AM_KSCATEGORY_CAPTURE },
        { L"WDM Streaming Crossbar Devices", AM_KSCATEGORY_CROSSBAR },
        { L"WDM Streaming Rendering Devices", AM_KSCATEGORY_RENDER },
        { L"WDM Streaming Tee/Splitter Devices", AM_KSCATEGORY_SPLITTER },
        { L"WDM Streaming TV Audio Devices", AM_KSCATEGORY_TVAUDIO },
        { L"WDM Streaming TV Tuner Devices", AM_KSCATEGORY_TVTUNER },
        { L"WDM Streaming VBI Codecs", AM_KSCATEGORY_VBICODEC },
        { L"WDM Streaming Communication Transforms", KSCATEGORY_COMMUNICATIONSTRANSFORM },
        { L"WDM Streaming Data Transforms", KSCATEGORY_DATATRANSFORM },
        { L"WDM Streaming Interface Transforms", KSCATEGORY_INTERFACETRANSFORM },
        { L"WDM Streaming Mixer Devices", KSCATEGORY_MIXER },
        { L"BDA Network Providers", KSCATEGORY_BDA_NETWORK_PROVIDER },
        { L"BDA Receiver Components", KSCATEGORY_BDA_RECEIVER_COMPONENT },
        { L"BDA Rendering Filters", KSCATEGORY_IP_SINK },
        { L"BDA Source Filters", KSCATEGORY_BDA_NETWORK_TUNER },
        { L"BDA Transport Information Renderers", KSCATEGORY_BDA_TRANSPORT_INFORMATION },
        { L"Video Effects (1 input)", CLSID_VideoEffects1Category },
        { L"Video Effects (2 inputs)", CLSID_VideoEffects2Category },
        { L"EncAPI Encoders", CLSID_MediaEncoderCategory },
        { L"EncAPI Multiplexers", CLSID_MediaMultiplexerCategory },
    };
    
    ICreateDevEnum *pCreateDevEnum = NULL;

    HRESULT hrRet = CoCreateInstance(
        CLSID_SystemDeviceEnum,
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(ICreateDevEnum),
        reinterpret_cast<void **>(&pCreateDevEnum)
    );
    if (FAILED(hrRet)) {
        ERR(L"CoCreateInstance(CLSID_SystemDeviceEnum, ...) failed: hr = 0x%08x", hrRet);
        return hrRet;
    }
    
    ReleaseOnExit releaseCreateDevEnum(pCreateDevEnum);
    
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
        
        hr = EnumerateDirectShowFilterCategory(pCreateDevEnum, categories[i].guid);
        if (FAILED(hr)) { hrRet = hr; }
        
        LOG(L""); // blank line
    }
    
    return hrRet;
}

HRESULT EnumerateDirectShowFilterCategory(
    ICreateDevEnum *pCreateDevEnum,
    GUID category
) {
    IEnumMoniker *pEnumMoniker = NULL;
    
    HRESULT hr = pCreateDevEnum->CreateClassEnumerator(
        category,
        &pEnumMoniker,
        0 // enumerate all kinds of filters
    );
    
    if (FAILED(hr)) {
        ERR(L"ICreateDevEnum::CreateClassEnumerator failed: hr = 0x%08x", hr);
        return hr;
    } else if (S_FALSE == hr) {
        // no filters in this category
        return S_OK;
    }
    
    ReleaseOnExit releaseEnumMoniker(pEnumMoniker);
    
    for (;;) {
        IMoniker *pMoniker = NULL;
        ULONG nRetrieved = 0;
        
        // get them one at a time
        hr = pEnumMoniker->Next(
            1,
            &pMoniker,
            &nRetrieved
        );
        
        if (FAILED(hr)) {
            ERR(L"IEnumMoniker::Next failed: hr = 0x%08x", hr);
            return hr;
        }
        
        if (0 == nRetrieved) {
            // done
            break;
        }
        
        ReleaseOnExit releaseMoniker(pMoniker);
        
        hr = DisplayDirectShowFilter(pMoniker);
        if (FAILED(hr)) {
            return hr;
        }
    }
    
    return S_OK;
}

HRESULT DisplayDirectShowFilter(IMoniker *pMoniker) {
    // need a binding context
    IBindCtx *pBindCtx = NULL;
    
    HRESULT hr = CreateBindCtx(
        0, // reserved
        &pBindCtx
    );
    
    if (FAILED(hr)) {
        ERR(L"CreateBindCtx() failed: hr = 0x%08x", hr);
        return hr;
    }
    
    ReleaseOnExit releaseBindCtx(pBindCtx);

    // get the property bag for this moniker
    IPropertyBag *pPropertyBag = NULL;
    
    hr = pMoniker->BindToStorage(
        pBindCtx,
        NULL,
        __uuidof(IPropertyBag),
        reinterpret_cast<void **>(&pPropertyBag)
    );
    
    if (FAILED(hr)) {
        ERR(L"IMoniker::BindToStorage(IPropertyBag) failed: hr = 0x%08x", hr);
        return hr;
    }
    
    ReleaseOnExit releasePropertyBag(pPropertyBag);
    
    // grab the friendly name out of the property bag
    VARIANT varFriendlyName;
    VariantInitAndClearOnExit clearFriendlyName(&varFriendlyName);
    hr = pPropertyBag->Read(
        L"FriendlyName",
        &varFriendlyName,
        NULL // to get fancy, pass an IErrorlog interface pointer
    );
    if (FAILED(hr)) {
        ERR(L"IPropertyBag::Read(FriendlyName) failed: hr = 0x%08x", hr);
        return hr;
    }
    if (VT_BSTR != varFriendlyName.vt) {
        ERR(L"Unexpected variant type %u for FriendlyName property (expected VT_BSTR", varFriendlyName.vt);
        return E_UNEXPECTED;
    }
    LOG(L"    %s", varFriendlyName.bstrVal);
    
    return hr;
}