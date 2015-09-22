// volume.cpp

#include "common.h"

HRESULT ListVolumesForFlow(EDataFlow flow);
HRESULT ListVolumesForCollection(IMMDeviceCollection *pMMDeviceCollection);
void ListVolumesForDevice(IMMDevice *pMMDevice);
void LogDeviceName(IMMDevice *pMMDevice);
void LogMeters(IMMDevice *pMMDevice);
void LogEndpointVolume(IMMDevice *pMMDevice);
void LogEndpointVolumeMute(IAudioEndpointVolume *pAudioEndpointVolume);
void LogEndpointVolumeRange(IAudioEndpointVolume *pAudioEndpointVolume);
void LogEndpointVolumeMasterScalar(IAudioEndpointVolume *pAudioEndpointVolume);
void LogEndpointVolumeMaster(IAudioEndpointVolume *pAudioEndpointVolume);
void LogEndpointVolumeChannelsScalar(IAudioEndpointVolume *pAudioEndpointVolume);
void LogEndpointVolumeChannels(IAudioEndpointVolume *pAudioEndpointVolume);

HRESULT ListVolumesForDevices() {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        ERR(L"CoInitialize failed: hr = 0x%08x", hr);
        return hr;
    }
    CoUninitializeOnExit cuoe;

    // get all devices

    EDataFlow flows[] = { eRender, eCapture };
    for (UINT f = 0; f < ARRAYSIZE(flows); f++) {
        HRESULT hrTemp = ListVolumesForFlow(flows[f]);
        if (FAILED(hrTemp)) {
            hr = hrTemp;
        }
    }

    return hr;
}

HRESULT ListVolumesForFlow(EDataFlow flow) {
    CComPtr<IMMDeviceEnumerator> pMMDeviceEnumerator;
    HRESULT hr = pMMDeviceEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
    if (FAILED(hr)) {
        ERR(L"CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x", hr);
        return hr;
    }

    LOG(L"-- Devices of EDataFlow %s (%u) --", StringFromDataFlow(flow), flow);

    CComPtr<IMMDeviceCollection> pMMDeviceCollection;
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &pMMDeviceCollection);
    if (FAILED(hr)) {
        ERR(L"IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x", hr);
        return hr;
    }

    return ListVolumesForCollection(pMMDeviceCollection);
}

HRESULT ListVolumesForCollection(IMMDeviceCollection *pMMDeviceCollection) {
    UINT32 nDevices;
    HRESULT hr = pMMDeviceCollection->GetCount(&nDevices);
    if (FAILED(hr)) {
        ERR(L"IMMDeviceCollection::GetCount failed: hr = 0x%08x", hr);
        return hr;
    }

    if (nDevices == 0) {
        LOG(L"No such devices.");
        return S_OK;
    }

    for (UINT32 d = 0; d < nDevices; d++) {
        CComPtr<IMMDevice> pMMDevice;
        HRESULT hrTemp = pMMDeviceCollection->Item(d, &pMMDevice);
        if (FAILED(hrTemp)) {
            ERR(L"IMMDeviceCollection::Item failed: hr = 0x%08x", hrTemp);
            hr = hrTemp;
            continue;
        }

        ListVolumesForDevice(pMMDevice);
    }

    return hr;
}

void ListVolumesForDevice(IMMDevice *pMMDevice) {
    LogDeviceName(pMMDevice);
    LogMeters(pMMDevice);
    LogEndpointVolume(pMMDevice);
    LogSessions(pMMDevice);
}

void LogDeviceName(IMMDevice *pMMDevice) {
    // get the name of the endpoint
    CComPtr<IPropertyStore> pPropertyStore;
    HRESULT hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
    if (FAILED(hr)) {
        ERR(L"IMMDevice::OpenPropertyStore failed: hr = 0x%08x", hr);
        return;
    }

    PROPVARIANT v; PropVariantInit(&v);
    PropVariantClearOnExit pvcoe(&v);
    hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &v);
    if (FAILED(hr)) {
        ERR(L"IPropertyStore::GetValue(PKEY_Device_FriendlyName) failed: hr = 0x%08x", hr);
        return;
    }

    if (VT_LPWSTR != v.vt) {
        ERR(L"PKEY_Device_FriendlyName has unexpected vartype %u", v.vt);
        return;
    }

    LOG(L"%s", v.pwszVal);
}

void LogMeters(IMMDevice *pMMDevice) {
    // get the current audio peak meter level for this endpoint
    CComPtr<IAudioMeterInformation> pAudioMeterInformation;
    HRESULT hr = pMMDevice->Activate(
        __uuidof(IAudioMeterInformation),
        CLSCTX_ALL,
        NULL,
        reinterpret_cast<void**>(&pAudioMeterInformation)
        );
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IAudioMeterInformation) failed: hr = 0x%08x", hr);
        return;
    }

    float peak = 0.0f;
    hr = pAudioMeterInformation->GetPeakValue(&peak);
    if (FAILED(hr)) {
        ERR(L"IAudioMeterInformation::GetPeakValue() failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"Peak: %g", peak);

    UINT nMeterChannelCount;
    hr = pAudioMeterInformation->GetMeteringChannelCount(&nMeterChannelCount);
    if (FAILED(hr)) {
        ERR(L"IAudioMeterInformation::GetMeteringChannelCount() failed: hr = 0x%08x", hr);
        return;
    }

    if (nMeterChannelCount == 0) {
        ERR(L"IAudioMeterInformation::GetMeteringChannelCount returned zero channels");
        return;
    }

    CComHeapPtr<float> pfPeaks;
    if (!pfPeaks.Allocate(nMeterChannelCount)) {
        ERR(L"Could not allocate %u floats", nMeterChannelCount);
        return;
    }

    hr = pAudioMeterInformation->GetChannelsPeakValues(nMeterChannelCount, pfPeaks);
    if (FAILED(hr)) {
        ERR(L"IAudioMeterInformation::GetChannelsPeakValues() failed: hr = 0x%08x", hr);
        return;
    }

    for (UINT c = 0; c < nMeterChannelCount; c++) {
        LOG(L"Peak, channel %u of %u: %g", c + 1, nMeterChannelCount, pfPeaks[c]);
    }
}

void LogEndpointVolume(IMMDevice *pMMDevice) {
    // get an endpoint volume interface
    CComPtr<IAudioEndpointVolume> pAudioEndpointVolume;
    HRESULT hr = pMMDevice->Activate(
        __uuidof(IAudioEndpointVolume),
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void **>(&pAudioEndpointVolume)
        );
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IAudioEndpointVolume) failed: hr = 0x%08x", hr);
        return;
    }

    LogEndpointVolumeMute(pAudioEndpointVolume);
    LogEndpointVolumeRange(pAudioEndpointVolume);
    LogEndpointVolumeMasterScalar(pAudioEndpointVolume);
    LogEndpointVolumeMaster(pAudioEndpointVolume);
    LogEndpointVolumeChannelsScalar(pAudioEndpointVolume);
    LogEndpointVolumeChannels(pAudioEndpointVolume);

    LOG(L"");
}

void LogEndpointVolumeMute(IAudioEndpointVolume *pAudioEndpointVolume) {
    BOOL mute;
    HRESULT hr = pAudioEndpointVolume->GetMute(&mute);
    if (FAILED(hr)) {
        ERR(L"IAudioEndpointVolume::GetMute failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"%s", (mute ? L"Muted" : L"Not muted"));
}

void LogEndpointVolumeRange(IAudioEndpointVolume *pAudioEndpointVolume) {
    float dbMin;
    float dbMax;
    float dbStep;
    HRESULT hr = pAudioEndpointVolume->GetVolumeRange(&dbMin, &dbMax, &dbStep);
    if (FAILED(hr)) {
        ERR(L"IAudioEndpointVolume::GetVolumeRange failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"Volume range: 0%% to 100%% (%g dB to %g dB in steps of %g dB)", dbMin, dbMax, dbStep);
}

void LogEndpointVolumeMasterScalar(IAudioEndpointVolume *pAudioEndpointVolume) {
    float pctMaster;
    HRESULT hr = pAudioEndpointVolume->GetMasterVolumeLevelScalar(&pctMaster);
    if (FAILED(hr)) {
        ERR(L"IAudioEndpointVolume::GetMasterVolumeLevelScalar failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"Master (%%): %g%%", pctMaster * 100.0f);
}

void LogEndpointVolumeMaster(IAudioEndpointVolume *pAudioEndpointVolume) {
    float dbMaster;
    HRESULT hr = pAudioEndpointVolume->GetMasterVolumeLevel(&dbMaster);
    if (FAILED(hr)) {
        ERR(L"IAudioEndpointVolume::GetMasterVolumeLevel failed: hr = 0x%08x", hr);
    }

    LOG(L"Master (dB): %g dB", dbMaster);
}

void LogEndpointVolumeChannelsScalar(IAudioEndpointVolume *pAudioEndpointVolume) {
    UINT nVolumeChannelCount;
    HRESULT hr = pAudioEndpointVolume->GetChannelCount(&nVolumeChannelCount);
    if (FAILED(hr)) {
        ERR(L"IAudioEndpointVolume::GetChannelCount failed: hr = 0x%08x", hr);
        return;
    }

    if (nVolumeChannelCount == 0) {
        ERR(L"IAudioEndpointVolume::GetChannelCount returned zero channels");
        return;
    }

    for (UINT c = 0; c < nVolumeChannelCount; c++) {
        float pctChannel;
        hr = pAudioEndpointVolume->GetChannelVolumeLevelScalar(c, &pctChannel);
        if (FAILED(hr)) {
            ERR(L"IAudioEndpointVolume::GetChannelVolumeLevelScalar(%u) failed: hr = 0x%08x", c, hr);
            continue;
        }

        LOG(L"Volume, channel %u of %u: %g%%", c + 1, nVolumeChannelCount, pctChannel * 100.0f);
    }
}

void LogEndpointVolumeChannels(IAudioEndpointVolume *pAudioEndpointVolume) {
    UINT nVolumeChannelCount;
    HRESULT hr = pAudioEndpointVolume->GetChannelCount(&nVolumeChannelCount);
    if (FAILED(hr)) {
        ERR(L"IAudioEndpointVolume::GetChannelCount failed: hr = 0x%08x", hr);
        return;
    }

    if (nVolumeChannelCount == 0) {
        ERR(L"IAudioEndpointVolume::GetChannelCount returned zero channels");
        return;
    }

    for (UINT c = 0; c < nVolumeChannelCount; c++) {
        float dbChannel;
        hr = pAudioEndpointVolume->GetChannelVolumeLevel(c, &dbChannel);
        if (FAILED(hr)) {
            ERR(L"IAudioEndpointVolume::GetChannelVolumeLevel(%u) failed: hr = 0x%08x", c, hr);
            continue;
        }

        LOG(L"Volume, channel %u of %u: %g dB", c + 1, nVolumeChannelCount, dbChannel);
    }
}
