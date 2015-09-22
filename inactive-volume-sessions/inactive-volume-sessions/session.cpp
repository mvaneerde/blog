// session.cpp

#include "common.h"

void LogSession(IAudioSessionControl *pAudioSessionControl);
void LogSessionState(IAudioSessionControl *pAudioSessionControl);
void LogSessionIconPath(IAudioSessionControl *pAudioSessionControl);
void LogSessionDisplayName(IAudioSessionControl *pAudioSessionControl);
void LogSessionGroupingParam(IAudioSessionControl *pAudioSessionControl);
void LogSession2(IAudioSessionControl *pAudioSessionControl);
void LogSession2Identifier(IAudioSessionControl2 *pAudioSessionControl2);
void LogSession2InstanceIdentifier(IAudioSessionControl2 *pAudioSessionControl2);
void LogSession2ProcessId(IAudioSessionControl2 *pAudioSessionControl2);
void LogSession2SystemSoundsSession(IAudioSessionControl2 *pAudioSessionControl2);
void LogSessionMeters(IAudioSessionControl *pAudioSessionControl);
void LogSessionMeterPeak(IAudioMeterInformation *pAudioMeterInformation);
void LogSessionMeterChannels(IAudioMeterInformation *pAudioMeterInformation);
void LogSessionSimpleVolume(IAudioSessionControl *pAudioSessionControl);
void LogSessionSimpleVolumeVolume(ISimpleAudioVolume *pSimpleAudioVolume);
void LogSessionSimpleVolumeMute(ISimpleAudioVolume *pSimpleAudioVolume);
void LogSessionChannelVolume(IAudioSessionControl *pAudioSessionControl);

void LogSessions(IMMDevice *pMMDevice) {
    // get a session enumerator
    CComPtr<IAudioSessionManager2> pAudioSessionManager2;
    HRESULT hr = pMMDevice->Activate(
        __uuidof(IAudioSessionManager2),
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void **>(&pAudioSessionManager2)
        );
    if (FAILED(hr)) {
        ERR(L"IMMDevice::Activate(IAudioSessionManager2) failed: hr = 0x%08x", hr);
        return;
    }

    CComPtr<IAudioSessionEnumerator> pAudioSessionEnumerator;
    hr = pAudioSessionManager2->GetSessionEnumerator(&pAudioSessionEnumerator);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionManager2::GetSessionEnumerator() failed: hr = 0x%08x", hr);
        return;
    }

    // iterate over all the sessions
    int count = 0;
    hr = pAudioSessionEnumerator->GetCount(&count);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionEnumerator::GetCount() failed: hr = 0x%08x", hr);
        return;
    }

    if (count == 0) {
        LOG(L"No sessions");
    }

    for (int session = 0; session < count; session++) {
        if (session > 0) {
            LOG(L"");
        }

        LOG(L"Session #%d of %d", session + 1, count);

        // get the session identifier
        CComPtr<IAudioSessionControl> pAudioSessionControl;
        hr = pAudioSessionEnumerator->GetSession(session, &pAudioSessionControl);
        if (FAILED(hr)) {
            ERR(L"IAudioSessionEnumerator::GetSession() failed: hr = 0x%08x", hr);
            continue;
        }

        LogSession(pAudioSessionControl);
    }
}

void LogSession(IAudioSessionControl *pAudioSessionControl) {
    LogSessionState(pAudioSessionControl);
    LogSessionIconPath(pAudioSessionControl);
    LogSessionDisplayName(pAudioSessionControl);
    LogSessionGroupingParam(pAudioSessionControl);
    LogSession2(pAudioSessionControl);
    LogSessionMeters(pAudioSessionControl);
    LogSessionSimpleVolume(pAudioSessionControl);
    LogSessionChannelVolume(pAudioSessionControl);
}

void LogSessionState(IAudioSessionControl *pAudioSessionControl) {
    AudioSessionState state;
    HRESULT hr = pAudioSessionControl->GetState(&state);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl::GetState() failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    State: %s (%u)", StringFromAudioSessionState(state), state);
}

void LogSessionIconPath(IAudioSessionControl *pAudioSessionControl) {
    CComHeapPtr<WCHAR> szIconPath;
    HRESULT hr = pAudioSessionControl->GetIconPath(&szIconPath);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl::GetIconPath() failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    Icon path: %s", static_cast<LPCWSTR>(szIconPath));
}

void LogSessionDisplayName(IAudioSessionControl *pAudioSessionControl) {
    CComHeapPtr<WCHAR> szDisplayName;
    HRESULT hr = pAudioSessionControl->GetDisplayName(&szDisplayName);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl::GetDisplayName() failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    Display name: %s", static_cast<LPCWSTR>(szDisplayName));
}

void LogSessionGroupingParam(IAudioSessionControl *pAudioSessionControl) {
    GUID guidGroupingParam;
    HRESULT hr = pAudioSessionControl->GetGroupingParam(&guidGroupingParam);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl::GetGroupingParam() failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    Grouping parameter: " GUID_FORMAT, GUID_VALUES(guidGroupingParam));
}

void LogSession2(IAudioSessionControl *pAudioSessionControl) {
    CComPtr<IAudioSessionControl2> pAudioSessionControl2;
    HRESULT hr = pAudioSessionControl->QueryInterface(IID_PPV_ARGS(&pAudioSessionControl2));
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl::QueryInterface(IAudioSessionControl2) failed: hr = 0x%08x", hr);
        return;
    }

    LogSession2Identifier(pAudioSessionControl2);
    LogSession2InstanceIdentifier(pAudioSessionControl2);
    LogSession2ProcessId(pAudioSessionControl2);
    LogSession2SystemSoundsSession(pAudioSessionControl2);
}

void LogSession2Identifier(IAudioSessionControl2 *pAudioSessionControl2) {
    CComHeapPtr<WCHAR> szSessionIdentifier;
    HRESULT hr = pAudioSessionControl2->GetSessionIdentifier(&szSessionIdentifier);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl2::GetSessionIdentifier() failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    Session identifier: %s", static_cast<LPCWSTR>(szSessionIdentifier));
}

void LogSession2InstanceIdentifier(IAudioSessionControl2 *pAudioSessionControl2) {
    CComHeapPtr<WCHAR> szSessionInstanceIdentifier;
    HRESULT hr = pAudioSessionControl2->GetSessionInstanceIdentifier(&szSessionInstanceIdentifier);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl2::GetSessionInstanceIdentifier() failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    Session instance identifier: %s", static_cast<LPCWSTR>(szSessionInstanceIdentifier));
}

void LogSession2ProcessId(IAudioSessionControl2 *pAudioSessionControl2) {
    DWORD pid = 0;
    HRESULT hr = pAudioSessionControl2->GetProcessId(&pid);
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl2::GetProcessId() failed: hr = 0x%08x", hr);
        return;
    }

    bool bMultiProcess = (AUDCLNT_S_NO_SINGLE_PROCESS == hr);
    LOG(L"    Process ID: %u%s", pid, (bMultiProcess ? L" (multi-process)" : L" (single-process)"));
}

void LogSession2SystemSoundsSession(IAudioSessionControl2 *pAudioSessionControl2) {
    HRESULT hr = pAudioSessionControl2->IsSystemSoundsSession();
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl2::IsSystemSoundsSession() failed: hr = 0x%08x", hr);
        return;
    }

    bool bIsSystemSoundsSession = (S_OK == hr);
    LOG(L"    System sounds session: %s", (bIsSystemSoundsSession ? L"yes" : L"no"));
}

void LogSessionMeters(IAudioSessionControl *pAudioSessionControl) {
    // get the current audio peak meter level for this session
    CComPtr<IAudioMeterInformation> pAudioMeterInformation;
    HRESULT hr = pAudioSessionControl->QueryInterface(IID_PPV_ARGS(&pAudioMeterInformation));
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl::QueryInterface(IAudioMeterInformation) failed: hr = 0x%08x", hr);
        return;
    }

    LogSessionMeterPeak(pAudioMeterInformation);
    LogSessionMeterChannels(pAudioMeterInformation);
}

void LogSessionMeterPeak(IAudioMeterInformation *pAudioMeterInformation) {
    float peak = 0.0f;
    HRESULT hr = pAudioMeterInformation->GetPeakValue(&peak);
    if (FAILED(hr)) {
        ERR(L"IAudioMeterInformation::GetPeakValue() failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    Peak value: %g", peak);
}

void LogSessionMeterChannels(IAudioMeterInformation *pAudioMeterInformation) {
    UINT nMeterChannelCount;
    HRESULT hr = pAudioMeterInformation->GetMeteringChannelCount(&nMeterChannelCount);
    if (FAILED(hr)) {
        ERR(L"IAudioMeterInformation::GetMeteringChannelCount() failed: hr = 0x%08x", hr);
        return;
    }

    if (nMeterChannelCount == 0) {
        ERR(L"IAudioMeterInformation::GetMeteringChannelCount() reports zero channels");
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
        LOG(L"    Peak, channel %u of %u: %g", c + 1, nMeterChannelCount, pfPeaks[c]);
    }
}

void LogSessionSimpleVolume(IAudioSessionControl *pAudioSessionControl) {
    CComPtr<ISimpleAudioVolume> pSimpleAudioVolume;
    HRESULT hr = pAudioSessionControl->QueryInterface(IID_PPV_ARGS(&pSimpleAudioVolume));
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl::QueryInterface(ISimpleAudioVolume) failed: hr = 0x%08x", hr);
        return;
    }

    LogSessionSimpleVolumeVolume(pSimpleAudioVolume);
    LogSessionSimpleVolumeMute(pSimpleAudioVolume);
}

void LogSessionSimpleVolumeVolume(ISimpleAudioVolume *pSimpleAudioVolume) {
    float fMasterVolume;
    HRESULT hr = pSimpleAudioVolume->GetMasterVolume(&fMasterVolume);
    if (FAILED(hr)) {
        ERR(L"ISimpleAudioVolume::GetMasterVolume failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    Master volume: %g (%g dB FS)", fMasterVolume, DbFromAmp(fMasterVolume));
}

void LogSessionSimpleVolumeMute(ISimpleAudioVolume *pSimpleAudioVolume) {
    BOOL bMute;
    HRESULT hr = pSimpleAudioVolume->GetMute(&bMute);
    if (FAILED(hr)) {
        ERR(L"ISimpleAudioVolume::GetMute failed: hr = 0x%08x", hr);
        return;
    }

    LOG(L"    %s", (bMute ? L"Muted" : L"Not muted"));
}

void LogSessionChannelVolume(IAudioSessionControl *pAudioSessionControl) {
    CComPtr<IChannelAudioVolume> pChannelAudioVolume;
    HRESULT hr = pAudioSessionControl->QueryInterface(IID_PPV_ARGS(&pChannelAudioVolume));
    if (FAILED(hr)) {
        ERR(L"IAudioSessionControl::QueryInterface(IChannelAudioVolume) failed: hr = 0x%08x", hr);
        return;
    }

    UINT32 nVolumeChannelCount;
    hr = pChannelAudioVolume->GetChannelCount(&nVolumeChannelCount);
    if (FAILED(hr)) {
        ERR(L"IChannelAudioVolume::GetChannelCount failed: hr = 0x%08x", hr);
        return;
    }

    if (nVolumeChannelCount == 0) {
        ERR(L"IChannelAudioVolume::GetChannelCount returned zero channels");
        return;
    }

    CComHeapPtr<float> fVolumes;
    if (!fVolumes.Allocate(nVolumeChannelCount)) {
        ERR(L"Could not allocate %u floats to store channel volumes", nVolumeChannelCount);
        return;
    }

    hr = pChannelAudioVolume->GetAllVolumes(nVolumeChannelCount, fVolumes);
    if (FAILED(hr)) {
        ERR(L"IChannelAudioVolume::GetAllVolumes failed: hr = 0x%08x", hr);
        return;
    }

    for (UINT32 channel = 0; channel < nVolumeChannelCount; channel++) {
        LOG(
            L"    Volume, channel #%u of %u: %g%% (%g dB FS)",
            channel + 1,
            nVolumeChannelCount,
            fVolumes[channel] * 100.0f,
            DbFromAmp(fVolumes[channel])
        );
    }
}
