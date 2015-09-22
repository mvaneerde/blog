// main.cpp

#include <initguid.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <devicetopology.h>
#include <stdio.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

HRESULT WalkTreeFromPart(
    IPart *pPart,
    EDataFlow eDirection,
    bool bFollowConnector,
    int iTabLevel = 0
);
HRESULT DisplayMute(IAudioMute *pMute, int iTabLevel);
HRESULT DisplayKsJackDescription(IKsJackDescription *pKsJackDescription, int iTabLevel);
HRESULT DisplayVolume(IAudioVolumeLevel *pVolume, int iTabLevel);
void Tab(int iTabLevel);

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() {
        CoUninitialize();
    }
};

class ReleaseOnExit {
public:
    ReleaseOnExit(IUnknown *p) : m_p(p) {}
    ~ReleaseOnExit() {
        if (m_p) {
            m_p->Release();
        }
    }
private:
    IUnknown *m_p;
};

class CoTaskMemFreeOnExit {
public:
    CoTaskMemFreeOnExit(void *p) : m_p(p) {}
    ~CoTaskMemFreeOnExit() {
        CoTaskMemFree(m_p);
    }
private:
    void *m_p;
};

class PropVariantClearOnExit {
public:
    PropVariantClearOnExit(PROPVARIANT *p) : m_p(p) {}
    ~PropVariantClearOnExit() {
        PropVariantClear(m_p);
    }
private:
    PROPVARIANT *m_p;
};

#define CASE_RETURN(x) case x: return L ## #x

LPCWSTR StringFromConnectionType(EPcxConnectionType type) {
    switch (type) {
        CASE_RETURN(eConnTypeUnknown);
        CASE_RETURN(eConnType3Point5mm);
        CASE_RETURN(eConnTypeQuarter);
        CASE_RETURN(eConnTypeAtapiInternal);
        CASE_RETURN(eConnTypeRCA);
        CASE_RETURN(eConnTypeOptical);
        CASE_RETURN(eConnTypeOtherDigital);
        CASE_RETURN(eConnTypeOtherAnalog);
        CASE_RETURN(eConnTypeMultichannelAnalogDIN);
        CASE_RETURN(eConnTypeXlrProfessional);
        CASE_RETURN(eConnTypeRJ11Modem);
        CASE_RETURN(eConnTypeCombination);
        default: return L"Unknown";
    }
};

LPCWSTR StringFromGeoLocation(EPcxGeoLocation loc) {
    switch (loc) {
        CASE_RETURN(eGeoLocRear);
        CASE_RETURN(eGeoLocFront);
        CASE_RETURN(eGeoLocLeft);
        CASE_RETURN(eGeoLocRight);
        CASE_RETURN(eGeoLocTop);
        CASE_RETURN(eGeoLocBottom);
        CASE_RETURN(eGeoLocRearPanel);
        CASE_RETURN(eGeoLocRiser);
        CASE_RETURN(eGeoLocInsideMobileLid);
        CASE_RETURN(eGeoLocDrivebay);
        CASE_RETURN(eGeoLocHDMI);
        CASE_RETURN(eGeoLocOutsideMobileLid);
        CASE_RETURN(eGeoLocATAPI);
        CASE_RETURN(eGeoLocNotApplicable);
        CASE_RETURN(eGeoLocReserved6);
        default: return L"Unknown";
    }
}

LPCWSTR StringFromGenLocation(EPcxGenLocation loc) {
    switch (loc) {
        CASE_RETURN(eGenLocPrimaryBox);
        CASE_RETURN(eGenLocInternal);
        CASE_RETURN(eGenLocSeparate);
        CASE_RETURN(eGenLocOther);
        default: return L"Unknown";
    }
}

LPCWSTR StringFromPortConnection(EPxcPortConnection con) {
    switch (con) {
        CASE_RETURN(ePortConnJack);
        CASE_RETURN(ePortConnIntegratedDevice);
        CASE_RETURN(ePortConnBothIntegratedAndJack);
        CASE_RETURN(ePortConnUnknown);
        default: return L"Unknown";
    }
}

LPCWSTR StringFromDataFlow(EDataFlow d) {
    switch (d) {
        CASE_RETURN(eRender);
        CASE_RETURN(eCapture);
    default: return L"Unknown";
    }
}

int __cdecl wmain(void) {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        LOG(L"Failed CoInitializeEx: hr = 0x%08x", hr);
        return __LINE__;
    }
    CoUninitializeOnExit cuoe;

    // get active devices
    IMMDeviceEnumerator *pEnum = NULL;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
        (void**)&pEnum
    );
    if (FAILED(hr)) {
        LOG(L"Couldn't get device enumerator: hr = 0x%08x", hr);
        return __LINE__;
    }
    ReleaseOnExit releaseEnum(pEnum);
    
    IMMDeviceCollection *pDeviceCollection = NULL;
    hr = pEnum->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE | DEVICE_STATE_UNPLUGGED, &pDeviceCollection);
    if (FAILED(hr)) {
        LOG(L"Couldn't get device collection: hr = 0x%08x", hr);
        return __LINE__;
    }
    ReleaseOnExit releaseDeviceCollection(pDeviceCollection);
    
    UINT nDevices = 0;
    hr = pDeviceCollection->GetCount(&nDevices);
    if (FAILED(hr)) {
        LOG(L"Couldn't get device collection count: hr = 0x%08x", hr);
        return __LINE__;
    }
    
    for (UINT i = 0; i < nDevices; i++) {
        IMMDevice *pDevice = NULL;
        hr = pDeviceCollection->Item(i, &pDevice);
        if (FAILED(hr)) {
            LOG(L"Couldn't get device: hr = 0x%08x", hr);
            return __LINE__;
        }
        ReleaseOnExit releaseDevice(pDevice);

        IMMEndpoint *pEndpoint = NULL;
        hr = pDevice->QueryInterface(IID_PPV_ARGS(&pEndpoint));
        if (FAILED(hr)) {
            LOG(L"Couldn't get endpoint: hr = 0x%08x", hr);
            return __LINE__;
        }
        ReleaseOnExit releaseEndpoint(pEndpoint);

        EDataFlow eDirection = eAll;
        hr = pEndpoint->GetDataFlow(&eDirection);
        if (FAILED(hr)) {
            LOG(L"Couldn't get data flow: hr = 0x%08x", hr);
            return __LINE__;
        }

        LOG(L"%s endpoint", StringFromDataFlow(eDirection));

        IPropertyStore *pPropertyStore = NULL;
        hr = pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        if (FAILED(hr)) {
            LOG(L"Couldn't get property store: hr = 0x%08x", hr);
            return __LINE__;
        }
        ReleaseOnExit releasePropertyStore(pPropertyStore);

        // get the long name property
        PROPVARIANT pv; PropVariantInit(&pv);
        hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
        if (FAILED(hr)) {
            LOG(L"Couldn't get friendly name: hr = 0x%08x", hr);
            return hr;
        }
        PropVariantClearOnExit clearPropVariant(&pv);

        LOG(L"Name: %s", pv.pwszVal);

        // get the ID
        WCHAR *wszId = NULL;
        hr = pDevice->GetId(&wszId);
        if (FAILED(hr)) {
            LOG(L"Couldn't get device ID: hr = 0x%08x", hr);
            return __LINE__;
        }
        CoTaskMemFreeOnExit releaseId(wszId);

        LOG(L"Endpoint ID: %s", wszId);

        // get device topology object for that endpoint
        IDeviceTopology *pDT = NULL;
        hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDT);
        if (FAILED(hr)) {
            LOG(L"Couldn't get device topology object: hr = 0x%08x", hr);
            return __LINE__;
        }
        ReleaseOnExit releaseDT(pDT);

        // get the single connector for that endpoint
        IConnector *pConnector = NULL;
        hr = pDT->GetConnector(0, &pConnector);
        if (FAILED(hr)) {
            LOG(L"Couldn't get the connector on the endpoint: hr = 0x%08x", hr);
            return __LINE__;
        }
        ReleaseOnExit releaseConnector(pConnector);

        // QI on the device's connector for IPart
        IPart *pPart = NULL;
        hr = pConnector->QueryInterface(IID_PPV_ARGS(&pPart));
        if (FAILED(hr)) {
            LOG(L"Couldn't get the part: hr = 0x%08x", hr);
            return __LINE__;
        }
        ReleaseOnExit releasePart(pPart);

        // all the real work is done in this function
        // follow the connector from this trivial endpoint topology
        // over to the rest of the topologies
        hr = WalkTreeFromPart(pPart, eDirection, true, 1);
        if (FAILED(hr)) {
            LOG(L"Couldn't walk the tree: hr = 0x%08x", hr);
            return __LINE__;
        }

        LOG(L"");
    }

    return 0;
}

HRESULT WalkTreeFromPart(
    IPart *pPart,
    EDataFlow eDirection,
    bool bFollowConnector,
    int iTabLevel /* = 0 */
) {
    HRESULT hr = S_OK;

    UINT nId = 0;
    hr = pPart->GetLocalId(&nId);
    if (FAILED(hr)) {
        LOG(L"Could not get part id: hr = 0x%08x", hr);
        return hr;
    }

    LPWSTR pwszPartName = NULL;
    hr = pPart->GetName(&pwszPartName);
    if (FAILED(hr)) {
        LOG(L"Could not get part name: hr = 0x%08x", hr);
        return hr;
    }
    CoTaskMemFreeOnExit freeName(pwszPartName);
    
    Tab(iTabLevel);
    LOG(L"0x%x: %s", nId, pwszPartName);

    // see if this is a volume node part
    IAudioVolumeLevel *pVolume = NULL;
    hr = pPart->Activate(CLSCTX_ALL, __uuidof(IAudioVolumeLevel), (void**)&pVolume);
    if (E_NOINTERFACE == hr) {
        // not a volume node
    } else if (FAILED(hr)) {
        LOG(L"Unexpected failure trying to activate IAudioVolumeLevel: hr = 0x%08x", hr);
        return hr;
    } else {
        // it's a volume node...
        ReleaseOnExit releaseVolume(pVolume);
        hr = DisplayVolume(pVolume, iTabLevel);
        if (FAILED(hr)) {
            LOG(L"DisplayVolume failed: hr = 0x%08x", hr);
            return hr;
        }
    }

    // see if this is a mute node part
    IAudioMute *pMute = NULL;
    hr = pPart->Activate(CLSCTX_ALL, __uuidof(IAudioMute), (void**)&pMute);
    if (E_NOINTERFACE == hr) {
        // not a mute node
    } else if (FAILED(hr)) {
        LOG(L"Unexpected failure trying to activate IAudioMute: hr = 0x%08x", hr);
        return hr;
    } else {
        // it's a mute node...
        ReleaseOnExit releaseMute(pMute);
        hr = DisplayMute(pMute, iTabLevel);
        if (FAILED(hr)) {
            LOG(L"DisplayMute failed: hr = 0x%08x", hr);
            return hr;
        }
    }

    // see if this is a jack description part
    IKsJackDescription *pKsJackDescription = NULL;
    hr = pPart->Activate(CLSCTX_ALL, __uuidof(IKsJackDescription), (void**)&pKsJackDescription);
    if (E_NOINTERFACE == hr) {
        // not a jack description part
    } else if (FAILED(hr)) {
        LOG(L"Unexpected failure trying to activate IKsJackDescription: hr = 0x%08x", hr);
        return hr;
    } else {
        // it's a mute node...
        ReleaseOnExit releaseJackDescription(pKsJackDescription);
        hr = DisplayKsJackDescription(pKsJackDescription, iTabLevel);
        if (FAILED(hr)) {
            LOG(L"DisplayKsJackDescription failed: hr = 0x%08x", hr);
            return hr;
        }
    }

    // get the list of incoming or outgoing parts, according to the direction
    IPartsList *pParts = NULL;
    hr = (
        eRender == eDirection ?
            pPart->EnumPartsIncoming(&pParts) :
            pPart->EnumPartsOutgoing(&pParts)
    );
    if (E_NOTFOUND == hr) {
        // not an error... we've just reached the end of the path
    } else if (FAILED(hr)) {
        LOG(L"Couldn't enum next parts: hr = 0x%08x", hr);
        return hr;
    } else {
        ReleaseOnExit releaseOnExit(pParts);
        
        UINT nParts = 0;
        hr = pParts->GetCount(&nParts);
        if (FAILED(hr)) {
            LOG(L"Couldn't get count of incoming parts: hr = 0x%08x", hr);
            return hr;
        }

        // walk the tree on each incoming/outgoing part recursively
        for (UINT n = 0; n < nParts; n++) {
            IPart *pNextPart = NULL;
            hr = pParts->GetPart(n, &pNextPart);
            if (FAILED(hr)) {
                LOG(L"Couldn't get part #%u (0-based) of %u (1-based:) hr = 0x%08x", n, nParts, hr);
                return hr;
            }
            ReleaseOnExit releaseNextPart(pNextPart);

            // if the incoming/outgoing part is a connector, follow it
            hr = WalkTreeFromPart(pNextPart, eDirection, true, iTabLevel + 1);
            if (FAILED(hr)) {
                LOG(L"Couldn't walk tree on part #%u (0-based) of %u (1-based:) hr = 0x%08x", n, nParts, hr);
                return hr;
            }
        }
    }
    
    if (bFollowConnector) {
        // if this part is a connector, do the connector dance
        IConnector *pConnHere = NULL;
        hr = pPart->QueryInterface(IID_PPV_ARGS(&pConnHere));
        if (E_NOINTERFACE == hr) {
            // that's fine
        } else if (FAILED(hr)) {
            LOG(L"Unexpected failure getting the connector on the endpoint: hr = 0x%08x", hr);
            return hr;
        } else {
            ReleaseOnExit releaseConnHere(pConnHere);

            // get the connector on the other side
            IConnector *pConnOtherSide = NULL;
            hr = pConnHere->GetConnectedTo(&pConnOtherSide);
            if (E_NOTFOUND == hr) {
                // that's fine, it just isn't connected to anything
            } else if (FAILED(hr)) {
                LOG(L"Couldn't get the connector on the upstream device: hr = 0x%08x", hr);
                return __LINE__;
            } else {
                ReleaseOnExit releaseConnOtherSide(pConnOtherSide);

                // QI on the device's connector for IPart
                IPart *pPartOtherSide = NULL;
                hr = pConnOtherSide->QueryInterface(IID_PPV_ARGS(&pPartOtherSide));
                if (FAILED(hr)) {
                    LOG(L"Couldn't get the part on the other side: hr = 0x%08x", hr);
                    return __LINE__;
                }
                ReleaseOnExit releasePartOtherSide(pPartOtherSide);

                // get the device topology interface for the other side
                IDeviceTopology *pDTOtherSide = NULL;
                hr = pPartOtherSide->GetTopologyObject(&pDTOtherSide);
                if (FAILED(hr)) {
                    LOG(L"Couldn't get the other side's device topology: hr = 0x%08x", hr);
                    return __LINE__;
                }
                ReleaseOnExit releaseDTOtherSide(pDTOtherSide);
                
                // get the device name for the other side
                LPWSTR pwszDeviceName = NULL;
                hr = pDTOtherSide->GetDeviceId(&pwszDeviceName);
                if (FAILED(hr)) {
                    LOG(L"Could not get device name: hr = 0x%08x", hr);
                    return hr;
                }
                CoTaskMemFreeOnExit freeDeviceName(pwszDeviceName);
                Tab(iTabLevel);
                LOG(L"%s", pwszDeviceName);

                // walk the other side's tree
                // but don't follow the connector back to here
                hr = WalkTreeFromPart(pPartOtherSide, eDirection, false, iTabLevel + 1);
                if (FAILED(hr)) {
                    LOG(L"Couldn't walk the tree: hr = 0x%08x", hr);
                    return __LINE__;
                }
            }
        }
    }
        
    return S_OK;
}

HRESULT DisplayVolume(IAudioVolumeLevel *pVolume, int iTabLevel) {
    HRESULT hr = S_OK;
    UINT nChannels = 0;

    hr = pVolume->GetChannelCount(&nChannels);
    if (FAILED(hr)) {
        LOG(L"GetChannelCount failed: hr = %08x", hr);
        return hr;
    }

    for (UINT n = 0; n < nChannels; n++) {
        float fMinLevelDB, fMaxLevelDB, fStepping, fLevelDB;

        hr = pVolume->GetLevelRange(n, &fMinLevelDB, &fMaxLevelDB, &fStepping);
        if (FAILED(hr)) {
            LOG(L"GetLevelRange failed: hr = 0x%08x", hr);
            return hr;
        }

        hr = pVolume->GetLevel(n, &fLevelDB);
        if (FAILED(hr)) {
            LOG(L"GetLevel failed: hr = 0x%08x", hr);
            return hr;
        }

        Tab(iTabLevel);
        LOG(
            L"Channel %u volume, %g dB to %g dB in steps of %g dB: %g dB",
            n, fMinLevelDB, fMaxLevelDB, fStepping, fLevelDB
        );
    }

    return S_OK;
}

HRESULT DisplayMute(IAudioMute *pMute, int iTabLevel) {
    HRESULT hr = S_OK;
    BOOL bMuted = FALSE;

    hr = pMute->GetMute(&bMuted);
    if (FAILED(hr)) {
        LOG(L"GetMute failed: hr = 0x%08x", hr);
        return hr;
    }

    Tab(iTabLevel);
    LOG(L"Mute node: %s", bMuted ? L"MUTED" : L"NOT MUTED");

    return S_OK;
}

HRESULT DisplayKsJackDescription(IKsJackDescription *pKsJackDescription, int iTabLevel) {
    HRESULT hr = S_OK;

    UINT count = 0;
    hr = pKsJackDescription->GetJackCount(&count);
    if (FAILED(hr)) {
        LOG(L"GetJackCount failed: hr = 0x%08x", hr);
        return hr;
    }

    Tab(iTabLevel);
    LOG(L"Jacks: %u", count);
    
    for (UINT i = 0; i < count; i++) {
        KSJACK_DESCRIPTION desc = {};
        hr = pKsJackDescription->GetJackDescription(0, &desc);
        if (FAILED(hr)) {
            LOG(L"GetJackDescription failed: hr = 0x%08x", hr);
            return hr;
        }
        
        Tab(iTabLevel + 1);
        LOG(L"-- Jack %u --", i + 1);
        Tab(iTabLevel + 2);
        LOG(L"ChannelMapping: 0x%x", desc.ChannelMapping);
        Tab(iTabLevel + 2);
        LOG(
            L"Color: 0x%06x (red = %d, green = %d, blue = %d)",
            desc.Color,
                (desc.Color >> 16) & 0xff,
                (desc.Color >> 8) & 0xff,
                (desc.Color >> 0) & 0xff
        );
        Tab(iTabLevel + 2);
        LOG(L"Connection Type: %d (%s)", desc.ConnectionType, StringFromConnectionType(desc.ConnectionType));
        Tab(iTabLevel + 2);
        LOG(L"Geometric Location: %d (%s)", desc.GeoLocation, StringFromGeoLocation(desc.GeoLocation));
        Tab(iTabLevel + 2);
        LOG(L"General Location: %d (%s)", desc.GenLocation, StringFromGenLocation(desc.GenLocation));
        Tab(iTabLevel + 2);
        LOG(L"Port Connection: %d (%s)", desc.PortConnection, StringFromPortConnection(desc.PortConnection));
        Tab(iTabLevel + 2);
        LOG(L"IsConnected: %s", desc.IsConnected ? L"Yes" : L"No");
    }

    return S_OK;
}

void Tab(int iTabLevel) {
    for (int i = 0; i < iTabLevel; i++) {
        wprintf(L"  ");
    }
}
