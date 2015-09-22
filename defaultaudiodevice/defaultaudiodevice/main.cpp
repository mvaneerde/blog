// main.cpp

#include <initguid.h>
#include <windows.h>
#include <stdio.h>
#include <mmdeviceapi.h>
#include <devicetopology.h>
#include <functiondiscoverykeys_devpkey.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) LOG(L"Error: " format, __VA_ARGS__)

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() { CoUninitialize(); }
};

class ReleaseOnExit {
public:
    ReleaseOnExit(IUnknown *p) : m_p(p) {}
    ~ReleaseOnExit() { if (nullptr != m_p) { m_p->Release(); } }
private:
    IUnknown *m_p;
};

class CoTaskMemFreeOnExit {
public:
    CoTaskMemFreeOnExit(void *p) : m_p(p) {}
    ~CoTaskMemFreeOnExit() { if (nullptr != m_p) { CoTaskMemFree(m_p); } }
private:
    void *m_p;
};

class PropVariantClearOnExit {
public:
    PropVariantClearOnExit(PROPVARIANT *p) : m_p(p) {}
    ~PropVariantClearOnExit() { if (nullptr != m_p) { PropVariantClear(m_p); } }
private:
    PROPVARIANT *m_p;
};

void devices(EDataFlow flow);
void device(IMMDevice *pMMDevice);
HRESULT connectorDance(IMMDevice *pMMDevice, IPart **ppPart);
void presenceDetect(IPart *pPart);
void location(IPart *pPart);
LPCWSTR StringFromEndpointFormFactor(EndpointFormFactor f);
LPCWSTR StringFromGenLocation(EPcxGenLocation e);
LPCWSTR StringFromGeoLocation(EPcxGeoLocation e);
LPCWSTR StringFromJackSubType(LPCWSTR type);

int _cdecl wmain() {

    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;

    devices(eRender);
    devices(eCapture);
    
    return 0;
}

void devices(EDataFlow flow) {
    IMMDeviceEnumerator *pMMDeviceEnumerator = nullptr;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&pMMDeviceEnumerator)
    );
    if (FAILED(hr)) {
        ERR(L"CoCreateInstance failed: hr = 0x%08x", hr);
        return;
    }
    ReleaseOnExit releaseMMDeviceEnumerator(pMMDeviceEnumerator);
    
    IMMDeviceCollection *pMMDeviceCollection = nullptr;
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pMMDeviceCollection);
    if (FAILED(hr)) {
        ERR(L"IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x", hr);
        return;
    }
    ReleaseOnExit releaseMMDeviceCollection(pMMDeviceCollection);
    
    UINT count = 0;
    hr = pMMDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        ERR(L"IMMDeviceCollection::GetCount failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"%s devices: %u", (eRender == flow ? L"Render" : L"Capture"), count);
    
    for (UINT i = 0; i < count; i++) {
        IMMDevice *pMMDevice = nullptr;
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            ERR(L"IMMDeviceCollection::Item failed: hr = 0x%08x", hr);
            continue;
        }
        ReleaseOnExit releaseDevice(pMMDevice);
        
        device(pMMDevice);
        
        // blank line
        LOG(L"");
    }
}

void device(IMMDevice *pMMDevice) {
    WCHAR *id = nullptr;
    HRESULT hr = pMMDevice->GetId(&id);
    if (FAILED(hr)) {
        ERR(L"IMMDevice::GetId failed: hr = 0x%08x", hr);
        return;
    }
    CoTaskMemFreeOnExit freeId(id);
    
    IPropertyStore *pPropertyStore = nullptr;
    hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
    if (FAILED(hr)) {
        ERR(L"IMMDevice::OpenPropertyStore failed: hr = 0x%08x", hr);
        return;
    }
    ReleaseOnExit releasePropertyStore(pPropertyStore);
    
    // PKEY_Device_FriendlyName
    PROPVARIANT v; PropVariantInit(&v);
    PropVariantClearOnExit clearPropVariant(&v);

    hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &v);
    if (FAILED(hr)) {
        ERR(L"IPropertyStore::GetValue(PKEY_Device_FriendlyName) failed: hr = 0x%08x", hr);
        return;
    }
    if (VT_LPWSTR != v.vt) {
        ERR(L"PKEY_Device_FriendlyName has unexpected vartype 0x%x instead of VT_LPWSTR", v.vt);
        return;
    }
    LOG(L"    %s", v.pwszVal);
    LOG(L"    Id: %s", id);
    PropVariantClear(&v);
    
    IPart *pHardwarePinFactory = nullptr;
    hr = connectorDance(pMMDevice, &pHardwarePinFactory);
    if (FAILED(hr)) {
        return;
    }
    ReleaseOnExit releaseHardwarePinFactory(pHardwarePinFactory);

    presenceDetect(pHardwarePinFactory);
    
    // PKEY_AudioEndpoint_FormFactor
    hr = pPropertyStore->GetValue(PKEY_AudioEndpoint_FormFactor, &v);
    if (FAILED(hr)) {
        ERR(L"IPropertyStore::GetValue(PKEY_AudioEndpoint_FormFactor) failed: hr = 0x%08x", hr);
        return;
    }
    if (VT_UI4 != v.vt) {
        ERR(L"PKEY_AudioEndpoint_FormFactor has unexpected vartype 0x%x instead of VT_UI4", v.vt);
        return;
    }
    LOG(
        L"    PKEY_AudioEndpoint_FormFactor: %u (%s)",
        v.ulVal,
        StringFromEndpointFormFactor(static_cast<EndpointFormFactor>(v.ulVal))
    );
    PropVariantClear(&v);
    
    // PKEY_AudioEndpoint_JackSubType
    hr = pPropertyStore->GetValue(PKEY_AudioEndpoint_JackSubType, &v);
    if (FAILED(hr)) {
        ERR(L"IPropertyStore::GetValue(PKEY_AudioEndpoint_JackSubType) failed: hr = 0x%08x", hr);
        return;
    }
    if (VT_LPWSTR != v.vt) {
        ERR(L"PKEY_AudioEndpoint_JackSubType has unexpected vartype 0x%x instead of VT_LPWSTR", v.vt);
        return;
    }
    LOG(
        L"    PKEY_AudioEndpoint_JackSubType: %s (%s)",
        v.pwszVal,
        StringFromJackSubType(v.pwszVal)
    );
    PropVariantClear(&v);
    
    // PKEY_Device_EnumeratorName
    hr = pPropertyStore->GetValue(PKEY_Device_EnumeratorName, &v);
    if (FAILED(hr)) {
        ERR(L"IPropertyStore::GetValue(PKEY_Device_EnumeratorName) failed: hr = 0x%08x", hr);
        return;
    }
    if (VT_LPWSTR != v.vt) {
        ERR(L"PKEY_Device_EnumeratorName has unexpected vartype 0x%x instead of VT_LPWSTR", v.vt);
        return;
    }
    LOG(L"    PKEY_Device_EnumeratorName: %s", v.pwszVal);
    PropVariantClear(&v);

    location(pHardwarePinFactory);
}

HRESULT connectorDance(IMMDevice *pMMDevice, IPart **ppPart) {
    IDeviceTopology *pDeviceTopology = nullptr;
    HRESULT hr = pMMDevice->Activate(
        __uuidof(IDeviceTopology),
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void**>(&pDeviceTopology)
    );
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IDeviceTopology) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseDeviceTopology(pDeviceTopology);
    
    IConnector *pConnector = nullptr;
    hr = pDeviceTopology->GetConnector(0, &pConnector);
    if (FAILED(hr)) {
        ERR(L"IDeviceTopology::GetConnector(0) failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseConnector(pConnector);
    
    IConnector *pConnector2 = nullptr;
    hr = pConnector->GetConnectedTo(&pConnector2);
    if (FAILED(hr)) {
        ERR(L"IConnector::GetConnectedTo failed: hr = 0x%08x", hr);
        return hr;
    }
    ReleaseOnExit releaseConnector2(pConnector2);
    
    hr = pConnector2->QueryInterface(IID_PPV_ARGS(ppPart));
    if (FAILED(hr)) {
        ERR(L"IConnector::QueryInterface failed: hr = 0x%08x", hr);
        return hr;
    }
    
    return S_OK;
}

void presenceDetect(IPart *pPart) {
    // KSJACK_DESCRIPTION2.JackCapabilities & JACKDESC2_PRESENCE_DETECT_CAPABILITY
    IKsJackDescription2 *pKsJackDescription2 = nullptr;
    HRESULT hr = pPart->Activate(CLSCTX_ALL, IID_PPV_ARGS(&pKsJackDescription2));
    if (E_NOINTERFACE == hr) {
        LOG(L"    IKsJackDescription2 not supported");
        return;
    } else if (FAILED(hr)) {
        ERR(L"IPart::Activate(IKsJackDescription2) failed: hr = 0x%08x", hr);
        return;
    }
    ReleaseOnExit releaseKsJackDescription2(pKsJackDescription2);

    UINT jacks = 0;
    hr = pKsJackDescription2->GetJackCount(&jacks);
    if (FAILED(hr)) {
        ERR(L"IKsJackDescription2::GetJackCount failed: hr = 0x%08x", hr);
        return;
    }
    
    if (0 == jacks) {
        LOG(L"    IKsJackDescription2 says there are no jacks");
        return;
    }

    KSJACK_DESCRIPTION2 description = {};
    hr = pKsJackDescription2->GetJackDescription2(0, &description);
    if (FAILED(hr)) {
        ERR(L"IKsJackDescription2::GetJackDescription2 failed: hr = 0x%08x", hr);
        return;
    }
    
    LOG(
        L"    KSJACK_DESCRIPTION2.JackCapabilities: presence detection %s",
        (description.JackCapabilities & JACKDESC2_PRESENCE_DETECT_CAPABILITY) ?
            L"supported" :
            L"not supported"
    );
}

void location(IPart *pPart) {
    // KSJACK_DESCRIPTION.GenLocation and .GeoLocation
    IKsJackDescription *pKsJackDescription = nullptr;
    HRESULT hr = pPart->Activate(CLSCTX_ALL, IID_PPV_ARGS(&pKsJackDescription));
    if (E_NOINTERFACE == hr) {
        LOG(L"    IKsJackDescription not supported");
        return;
    } else if (FAILED(hr)) {
        ERR(L"IPart::Activate(IKsJackDescription) failed: hr = 0x%08x", hr);
        return;
    }
    ReleaseOnExit releaseKsJackDescription(pKsJackDescription);

    UINT jacks = 0;
    hr = pKsJackDescription->GetJackCount(&jacks);
    if (FAILED(hr)) {
        ERR(L"IKsJackDescription::GetJackCount failed: hr = 0x%08x", hr);
        return;
    }
        
    if (0 == jacks) {
        LOG(L"    IKsJackDescription says there are no jacks");
        return;
    }

    KSJACK_DESCRIPTION description = {};
    hr = pKsJackDescription->GetJackDescription(0, &description);
    if (FAILED(hr)) {
        ERR(L"IKsJackDescription::GetJackDescription failed: hr = 0x%08x", hr);
        return;
    }
    
    LOG(
        L"    KSJACK_DESCRIPTION.GenLocation: %u (%s)",
        description.GenLocation,
        StringFromGenLocation(description.GenLocation)
    );

    LOG(
        L"    KSJACK_DESCRIPTION.GeoLocation: %u (%s)",
        description.GeoLocation,
        StringFromGeoLocation(description.GeoLocation)
    );
}

#define CASE(x) case x: return L ## #x
LPCWSTR StringFromEndpointFormFactor(EndpointFormFactor f) {
    switch (f) {
        CASE(RemoteNetworkDevice);
        CASE(Speakers);
        CASE(LineLevel);
        CASE(Headphones);
        CASE(Microphone);
        CASE(Headset);
        CASE(Handset);
        CASE(UnknownDigitalPassthrough);
        CASE(SPDIF);
        CASE(DigitalAudioDisplayDevice);
        CASE(UnknownFormFactor);
        default: return L"Unrecognized";
    }
}

LPCWSTR StringFromGenLocation(EPcxGenLocation e) {
    switch (e) {
        CASE(eGenLocPrimaryBox);
        CASE(eGenLocInternal); 
        CASE(eGenLocSeparate);
        CASE(eGenLocOther);
        default: return L"Unrecognized";
    }
}

LPCWSTR StringFromGeoLocation(EPcxGeoLocation e) {
    switch (e) {
        CASE(eGeoLocRear);
        CASE(eGeoLocFront);  
        CASE(eGeoLocLeft); 
        CASE(eGeoLocRight); 
        CASE(eGeoLocTop);
        CASE(eGeoLocBottom);
        CASE(eGeoLocRearPanel);
        CASE(eGeoLocRiser);
        CASE(eGeoLocInsideMobileLid);
        CASE(eGeoLocDrivebay);
        CASE(eGeoLocHDMI);
        CASE(eGeoLocOutsideMobileLid);
        CASE(eGeoLocATAPI);
        CASE(eGeoLocNotApplicable);
        CASE(eGeoLocReserved6);
        default: return L"Unrecognized";
    }
}

#define CASE_GUID(x) if (g == x) { return L ## #x; } (void)0
LPCWSTR StringFromJackSubType(LPCWSTR type) {
    GUID g = {};
    HRESULT hr = IIDFromString(type, &g);
    if (FAILED(hr)) {
        return L"Invalid GUID";
    }
    
    CASE_GUID(KSNODETYPE_1394_DA_STREAM);
    CASE_GUID(KSNODETYPE_1394_DV_STREAM_SOUNDTRACK);
    CASE_GUID(KSNODETYPE_3D_EFFECTS);
    CASE_GUID(KSNODETYPE_ACOUSTIC_ECHO_CANCEL);
    CASE_GUID(KSNODETYPE_ADC);
    CASE_GUID(KSNODETYPE_AGC);
    CASE_GUID(KSNODETYPE_ANALOG_CONNECTOR);
    CASE_GUID(KSNODETYPE_ANALOG_TAPE);
    CASE_GUID(KSNODETYPE_AUDIO_ENGINE);
    CASE_GUID(KSNODETYPE_AUDIO_LOOPBACK);
    CASE_GUID(KSNODETYPE_BIDIRECTIONAL_UNDEFINED);
    CASE_GUID(KSNODETYPE_CABLE_TUNER_AUDIO);
    CASE_GUID(KSNODETYPE_CD_PLAYER);
    CASE_GUID(KSNODETYPE_CHORUS);
    CASE_GUID(KSNODETYPE_COMMUNICATION_SPEAKER);
    CASE_GUID(KSNODETYPE_DAC);
    CASE_GUID(KSNODETYPE_DAT_IO_DIGITAL_AUDIO_TAPE);
    CASE_GUID(KSNODETYPE_DCC_IO_DIGITAL_COMPACT_CASSETTE);
    CASE_GUID(KSNODETYPE_DELAY);
    CASE_GUID(KSNODETYPE_DEMUX);
    CASE_GUID(KSNODETYPE_DESKTOP_MICROPHONE);
    CASE_GUID(KSNODETYPE_DESKTOP_SPEAKER);
    CASE_GUID(KSNODETYPE_DEV_SPECIFIC);
    CASE_GUID(KSNODETYPE_DIGITAL_AUDIO_INTERFACE);
    CASE_GUID(KSNODETYPE_DISPLAYPORT_INTERFACE);
    CASE_GUID(KSNODETYPE_DOWN_LINE_PHONE);
    CASE_GUID(KSNODETYPE_DRM_DESCRAMBLE);
    CASE_GUID(KSNODETYPE_DSS_AUDIO);
    CASE_GUID(KSNODETYPE_DVD_AUDIO);
    CASE_GUID(KSNODETYPE_DYN_RANGE_COMPRESSOR);
    CASE_GUID(KSNODETYPE_ECHO_CANCELING_SPEAKERPHONE);
    CASE_GUID(KSNODETYPE_ECHO_SUPPRESSING_SPEAKERPHONE);
    CASE_GUID(KSNODETYPE_EMBEDDED_UNDEFINED);
    CASE_GUID(KSNODETYPE_EQUALIZATION_NOISE);
    CASE_GUID(KSNODETYPE_EQUALIZER);
    CASE_GUID(KSNODETYPE_EXTERNAL_UNDEFINED);
    CASE_GUID(KSNODETYPE_HANDSET);
    CASE_GUID(KSNODETYPE_HDMI_INTERFACE);
    CASE_GUID(KSNODETYPE_HEADPHONES);
    CASE_GUID(KSNODETYPE_HEADSET);
    CASE_GUID(KSNODETYPE_HEADSET_MICROPHONE);
    CASE_GUID(KSNODETYPE_HEADSET_SPEAKERS);
    CASE_GUID(KSNODETYPE_HEAD_MOUNTED_DISPLAY_AUDIO);
    CASE_GUID(KSNODETYPE_INPUT_UNDEFINED);
    CASE_GUID(KSNODETYPE_LEGACY_AUDIO_CONNECTOR);
    CASE_GUID(KSNODETYPE_LEVEL_CALIBRATION_NOISE_SOURCE);
    CASE_GUID(KSNODETYPE_LINE_CONNECTOR);
    CASE_GUID(KSNODETYPE_LOUDNESS);
    CASE_GUID(KSNODETYPE_LOW_FREQUENCY_EFFECTS_SPEAKER);
    CASE_GUID(KSNODETYPE_MICROPHONE);
    CASE_GUID(KSNODETYPE_MICROPHONE_ARRAY);
    CASE_GUID(KSNODETYPE_MICROPHONE_ARRAY_PROCESSOR);
    CASE_GUID(KSNODETYPE_MIDI_ELEMENT);
    CASE_GUID(KSNODETYPE_MIDI_JACK);
    CASE_GUID(KSNODETYPE_MINIDISK);
    CASE_GUID(KSNODETYPE_MULTITRACK_RECORDER);
    CASE_GUID(KSNODETYPE_MUTE);
    CASE_GUID(KSNODETYPE_MUX);
    CASE_GUID(KSNODETYPE_NOISE_SUPPRESS);
    CASE_GUID(KSNODETYPE_OMNI_DIRECTIONAL_MICROPHONE);
    CASE_GUID(KSNODETYPE_OUTPUT_UNDEFINED);
    CASE_GUID(KSNODETYPE_PARAMETRIC_EQUALIZER);
    CASE_GUID(KSNODETYPE_PEAKMETER);
    CASE_GUID(KSNODETYPE_PERSONAL_MICROPHONE);
    CASE_GUID(KSNODETYPE_PHONE_LINE);
    CASE_GUID(KSNODETYPE_PHONOGRAPH);
    CASE_GUID(KSNODETYPE_PROCESSING_MICROPHONE_ARRAY);
    CASE_GUID(KSNODETYPE_PROLOGIC_DECODER);
    CASE_GUID(KSNODETYPE_PROLOGIC_ENCODER);
    CASE_GUID(KSNODETYPE_RADIO_RECEIVER);
    CASE_GUID(KSNODETYPE_RADIO_TRANSMITTER);
    CASE_GUID(KSNODETYPE_REVERB);
    CASE_GUID(KSNODETYPE_ROOM_SPEAKER);
    CASE_GUID(KSNODETYPE_SATELLITE_RECEIVER_AUDIO);
    CASE_GUID(KSNODETYPE_SPDIF_INTERFACE);
    CASE_GUID(KSNODETYPE_SPEAKER);
    CASE_GUID(KSNODETYPE_SPEAKERPHONE_NO_ECHO_REDUCTION);
    CASE_GUID(KSNODETYPE_SPEAKERS_STATIC_JACK);
    CASE_GUID(KSNODETYPE_SRC);
    CASE_GUID(KSNODETYPE_STEREO_WIDE);
    CASE_GUID(KSNODETYPE_SUM);
    CASE_GUID(KSNODETYPE_SUPERMIX);
    CASE_GUID(KSNODETYPE_SURROUND_ENCODER);
    CASE_GUID(KSNODETYPE_SYNTHESIZER);
    CASE_GUID(KSNODETYPE_TELEPHONE);
    CASE_GUID(KSNODETYPE_TELEPHONY_UNDEFINED);
    CASE_GUID(KSNODETYPE_TONE);
    CASE_GUID(KSNODETYPE_TV_TUNER_AUDIO);
    CASE_GUID(KSNODETYPE_UPDOWN_MIX);
    CASE_GUID(KSNODETYPE_VCR_AUDIO);
    CASE_GUID(KSNODETYPE_VIDEO_CAMERA_TERMINAL);
    CASE_GUID(KSNODETYPE_VIDEO_DISC_AUDIO);
    CASE_GUID(KSNODETYPE_VIDEO_INPUT_MTT);
    CASE_GUID(KSNODETYPE_VIDEO_INPUT_TERMINAL);
    CASE_GUID(KSNODETYPE_VIDEO_OUTPUT_MTT);
    CASE_GUID(KSNODETYPE_VIDEO_OUTPUT_TERMINAL);
    CASE_GUID(KSNODETYPE_VIDEO_PROCESSING);
    CASE_GUID(KSNODETYPE_VIDEO_SELECTOR);
    CASE_GUID(KSNODETYPE_VIDEO_STREAMING);
    CASE_GUID(KSNODETYPE_VOLUME);

    return L"Unrecognized";
}