// main.cpp

#include <initguid.h>
#include <windows.h>
#include <math.h>
#include <appmodel.h>
#include <cguid.h>
#include <atlbase.h>
#include <stdio.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>

#define AUDCLNT_S_NO_SINGLE_PROCESS AUDCLNT_SUCCESS (0x00d)

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

#define GUID_FORMAT L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"
#define GUID_VALUES(g) \
    g.Data1, g.Data2, g.Data3, \
    g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3],  \
    g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() {
        CoUninitialize();
    }
};

class CloseHandleOnExit {
public:
    CloseHandleOnExit(HANDLE h) : m_h(h) {}
    ~CloseHandleOnExit() {
        CloseHandle(m_h);
    }
private:
    HANDLE m_h;
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

BOOL CALLBACK LogWindowTitles(HWND h, LPARAM pid);

float DbFromAmp(float a);

int _cdecl wmain() {
    HRESULT hr = S_OK;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        LOG(L"CoInitialize failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    CoUninitializeOnExit cuoe;
    
    // get default device
    CComPtr<IMMDeviceEnumerator> pMMDeviceEnumerator;
    hr = pMMDeviceEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
    if (FAILED(hr)) {
        LOG(L"CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x", hr);
        return -__LINE__;
    }
    
    EDataFlow flows[] = { eRender, eCapture };
    for (UINT f = 0; f < ARRAYSIZE(flows); f++) {
        LOG(L"-- %s devices --", (flows[f] == eRender ? L"Playback" : L"Recording"));
    
        CComPtr<IMMDeviceCollection> pMMDeviceCollection;
        hr = pMMDeviceEnumerator->EnumAudioEndpoints(flows[f], DEVICE_STATE_ACTIVE, &pMMDeviceCollection);
        if (FAILED(hr)) {
            LOG(L"IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x", hr);
            continue;
        }
        
        UINT32 nDevices;
        hr = pMMDeviceCollection->GetCount(&nDevices);
        if (FAILED(hr)) {
            LOG(L"IMMDeviceCollection::GetCount failed: hr = 0x%08x", hr);
            continue;
        }
        
        for (UINT32 d = 0; d < nDevices; d++) {
            CComPtr<IMMDevice> pMMDevice;
            hr = pMMDeviceCollection->Item(d, &pMMDevice);
            if (FAILED(hr)) {
                LOG(L"IMMDeviceCollection::Item failed: hr = 0x%08x", hr);
                continue;
            }

            // get the name of the endpoint
            CComPtr<IPropertyStore> pPropertyStore;
            hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
            if (FAILED(hr)) {
                LOG(L"IMMDevice::OpenPropertyStore failed: hr = 0x%08x", hr);
                continue;
            }

            PROPVARIANT v; PropVariantInit(&v);
            PropVariantClearOnExit pvcoe(&v);
            hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &v);
            if (FAILED(hr)) {
                LOG(L"IPropertyStore::GetValue(PKEY_Device_FriendlyName) failed: hr = 0x%08x", hr);
                continue;
            }
            
            if (VT_LPWSTR != v.vt) {
                LOG(L"PKEY_Device_FriendlyName has unexpected vartype %u", v.vt);
                continue;
            }
            
            // get the current audio peak meter level for this endpoint
            CComPtr<IAudioMeterInformation> pAudioMeterInformation;
            hr = pMMDevice->Activate(
                __uuidof(IAudioMeterInformation),
                CLSCTX_ALL,
                NULL,
                reinterpret_cast<void**>(&pAudioMeterInformation)
            );
            if (FAILED(hr)) {
                LOG(L"IMMDevice::Activate(IAudioMeterInformation) failed: hr = 0x%08x", hr);
                continue;
            }
            
            float peak = 0.0f;
            hr = pAudioMeterInformation->GetPeakValue(&peak);
            if (FAILED(hr)) {
                LOG(L"IAudioMeterInformation::GetPeakValue() failed: hr = 0x%08x", hr);
                continue;
            }

            // get an endpoint volume interface
            CComPtr<IAudioEndpointVolume> pAudioEndpointVolume;
            hr = pMMDevice->Activate(
                __uuidof(IAudioEndpointVolume),
                CLSCTX_ALL,
                nullptr,
                reinterpret_cast<void **>(&pAudioEndpointVolume)
            );
            if (FAILED(hr)) {
                LOG(L"IMMDevice::Activate(IAudioEndpointVolume) failed: hr = 0x%08x", hr);
                continue;
            }

            BOOL mute;
            hr = pAudioEndpointVolume->GetMute(&mute);
            if (FAILED(hr)) {
                LOG(L"IAudioEndpointVolume::GetMute failed: hr = 0x%08x", hr);
                continue;
            }
            
            float dbMin;
            float dbMax;
            float dbStep;
            hr = pAudioEndpointVolume->GetVolumeRange(&dbMin, &dbMax, &dbStep);
            if (FAILED(hr)) {
                LOG(L"IAudioEndpointVolume::GetVolumeRange failed: hr = 0x%08x", hr);
                continue;
            }

            float pctMaster;
            hr = pAudioEndpointVolume->GetMasterVolumeLevelScalar(&pctMaster);
            if (FAILED(hr)) {
                LOG(L"IAudioEndpointVolume::GetMasterVolumeLevelScalar failed: hr = 0x%08x", hr);
                continue;
            }

            float dbMaster;
            hr = pAudioEndpointVolume->GetMasterVolumeLevel(&dbMaster);
            if (FAILED(hr)) {
                LOG(L"IAudioEndpointVolume::GetMasterVolumeLevel failed: hr = 0x%08x", hr);
                continue;
            }
            
            LOG(
                L"%s\n"
                L"    Peak: %g\n"
                L"    Mute: %d\n"
                L"    Volume range: 0%% to 100%% (%g dB to %g dB in steps of %g dB)\n"
                L"    Master: %g%% (%g dB)",
                v.pwszVal,
                peak,
                mute,
                dbMin, dbMax, dbStep,
                pctMaster * 100.0f, dbMaster
            );

            UINT nChannelCount;
            hr = pAudioEndpointVolume->GetChannelCount(&nChannelCount);
            if (FAILED(hr)) {
                LOG(L"IAudioEndpointVolume::GetChannelCount failed: hr = 0x%08x", hr);
                continue;
            }
            
            for (UINT c = 0; c < nChannelCount; c++) {
                float pctChannel;
                hr = pAudioEndpointVolume->GetChannelVolumeLevelScalar(c, &pctChannel);
                if (FAILED(hr)) {
                    LOG(L"IAudioEndpointVolume::GetChannelVolumeLevelScalar(%u) failed: hr = 0x%08x", c, hr);
                    continue;
                }
                
                float dbChannel;
                hr = pAudioEndpointVolume->GetChannelVolumeLevel(c, &dbChannel);
                if (FAILED(hr)) {
                    LOG(L"IAudioEndpointVolume::GetChannelVolumeLevel(%u) failed: hr = 0x%08x", c, hr);
                    continue;
                }

                LOG(L"    Channel %u of %u: %g%% (%g dB)", c + 1, nChannelCount, pctChannel * 100.0f, dbChannel);
            }
            
            LOG(L"");

            // get a session enumerator
            CComPtr<IAudioSessionManager2> pAudioSessionManager2;
            hr = pMMDevice->Activate(
                __uuidof(IAudioSessionManager2),
                CLSCTX_ALL,
                nullptr,
                reinterpret_cast<void **>(&pAudioSessionManager2)
            );
            if (FAILED(hr)) {
                LOG(L"IMMDevice::Activate(IAudioSessionManager2) failed: hr = 0x%08x", hr);
                return -__LINE__;
            }
            
            CComPtr<IAudioSessionEnumerator> pAudioSessionEnumerator;
            hr = pAudioSessionManager2->GetSessionEnumerator(&pAudioSessionEnumerator);
            if (FAILED(hr)) {
                LOG(L"IAudioSessionManager2::GetSessionEnumerator() failed: hr = 0x%08x", hr);
                return -__LINE__;
            }
            
            // iterate over all the sessions
            int count = 0;
            hr = pAudioSessionEnumerator->GetCount(&count);
            if (FAILED(hr)) {
                LOG(L"IAudioSessionEnumerator::GetCount() failed: hr = 0x%08x", hr);
                return -__LINE__;
            }

            int active = 0;
            for (int session = 0; session < count; session++) {
                // get the session identifier
                CComPtr<IAudioSessionControl> pAudioSessionControl;
                hr = pAudioSessionEnumerator->GetSession(session, &pAudioSessionControl);
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionEnumerator::GetSession() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                AudioSessionState state;
                hr = pAudioSessionControl->GetState(&state);
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl::GetState() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                if (AudioSessionStateActive != state) {
                    // skip this session
                    continue;
                }
                active++;

                CComHeapPtr<WCHAR> szIconPath;
                hr = pAudioSessionControl->GetIconPath(&szIconPath);
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl::GetIconPath() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }

                CComHeapPtr<WCHAR> szDisplayName;
                hr = pAudioSessionControl->GetDisplayName(&szDisplayName);
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl::GetDisplayName() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }

                GUID guidGroupingParam;
                hr = pAudioSessionControl->GetGroupingParam(&guidGroupingParam);
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl::GetGroupingParam() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }

                CComPtr<IAudioSessionControl2> pAudioSessionControl2;
                hr = pAudioSessionControl->QueryInterface(IID_PPV_ARGS(&pAudioSessionControl2));
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl::QueryInterface(IAudioSessionControl2) failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }

                CComHeapPtr<WCHAR> szSessionIdentifier;
                hr = pAudioSessionControl2->GetSessionIdentifier(&szSessionIdentifier);
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl2::GetSessionIdentifier() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                CComHeapPtr<WCHAR> szSessionInstanceIdentifier;
                hr = pAudioSessionControl2->GetSessionInstanceIdentifier(&szSessionInstanceIdentifier);
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl2::GetSessionInstanceIdentifier() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }

                DWORD pid = 0;
                hr = pAudioSessionControl2->GetProcessId(&pid);
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl2::GetProcessId() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                bool bMultiProcess = (AUDCLNT_S_NO_SINGLE_PROCESS == hr);
                
                hr = pAudioSessionControl2->IsSystemSoundsSession();
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl2::IsSystemSoundsSession() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                bool bIsSystemSoundsSession = (S_OK == hr);
                
                // get the current audio peak meter level for this session
                CComPtr<IAudioMeterInformation> pAudioMeterInformation;
                hr = pAudioSessionControl->QueryInterface(IID_PPV_ARGS(&pAudioMeterInformation));
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl::QueryInterface(IAudioMeterInformation) failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                float peak = 0.0f;
                hr = pAudioMeterInformation->GetPeakValue(&peak);
                if (FAILED(hr)) {
                    LOG(L"IAudioMeterInformation::GetPeakValue() failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                        
                LOG(
                    L"    Active session #%d\n"
                    L"        Peak value: %g\n"
                    L"        Icon path: %s\n"
                    L"        Display name: %s\n"
                    L"        Grouping parameter: " GUID_FORMAT L"\n"
                    L"        Process ID: %u%s\n"
                    L"        Session identifier: %s\n"
                    L"        Session instance identifier: %s\n"
                    L"        System sounds session: %s"
                    ,
                    active,
                    peak,
                    static_cast<LPCWSTR>(szIconPath),
                    static_cast<LPCWSTR>(szDisplayName),
                    GUID_VALUES(guidGroupingParam),
                    pid, (bMultiProcess ? L" (multi-process)" : L" (single-process)"),
                    static_cast<LPCWSTR>(szSessionIdentifier),
                    static_cast<LPCWSTR>(szSessionInstanceIdentifier),
                    (bIsSystemSoundsSession ? L"yes" : L"no")
                );
                
                // get the package full name (if this is a Windows Store app)
                // or the window text of all the top-level Windows (if this is a desktop app)
                HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
                if (NULL == h) {
                    LOG(L"OpenProcess failed: error = %u", GetLastError());
                    return -__LINE__;
                }
                CloseHandleOnExit closeProcess(h);
                
                UINT32 chars = 0;
                LONG result = GetPackageFullName(h, &chars, NULL);
                switch (result) {
                    case ERROR_INSUFFICIENT_BUFFER: {
                        CComHeapPtr<WCHAR> packageFullName;
                        if (!packageFullName.Allocate(chars)) {
                            LOG(L"Allocate failed");
                            return -__LINE__;
                        }
                        
                        result = GetPackageFullName(h, &chars, packageFullName);
                        if (ERROR_SUCCESS != result) {
                            LOG(L"Unexpected return code from GetPackageFullName: %u", result);
                            return -__LINE__;
                        }
                        
                        LOG(L"        Package full name: %s", static_cast<LPCWSTR>(packageFullName));
                        break;
                    }

                    case APPMODEL_ERROR_NO_PACKAGE: {
                        // find top-level windows in that process
                        if (!EnumWindows(LogWindowTitles, pid)) {
                            LOG(L"EnumWindows failed: error = %u", GetLastError());
                            return -__LINE__;
                        }
                        break;
                    }

                    default: {
                        LOG(L"Unexpected return code from GetPackageFullName: %u", result);
                        return -__LINE__;
                    }
                }

                // query the volumes
                CComPtr<ISimpleAudioVolume> pSimpleAudioVolume;
                hr = pAudioSessionControl->QueryInterface(IID_PPV_ARGS(&pSimpleAudioVolume));
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl::QueryInterface(ISimpleAudioVolume) failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }

                float fMasterVolume;
                hr = pSimpleAudioVolume->GetMasterVolume(&fMasterVolume);
                if (FAILED(hr)) {
                    LOG(L"ISimpleAudioVolume::GetMasterVolume failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                LOG(L"        Master volume: %g (%g dB FS)", fMasterVolume, DbFromAmp(fMasterVolume));
                
                BOOL bMute;
                hr = pSimpleAudioVolume->GetMute(&bMute);
                if (FAILED(hr)) {
                    LOG(L"ISimpleAudioVolume::GetMute failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                LOG(L"        %s", (bMute ? L"Muted" : L"Not muted"));
                
                CComPtr<IChannelAudioVolume> pChannelAudioVolume;
                hr = pAudioSessionControl->QueryInterface(IID_PPV_ARGS(&pChannelAudioVolume));
                if (FAILED(hr)) {
                    LOG(L"IAudioSessionControl::QueryInterface(IChannelAudioVolume) failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                UINT32 nChannelCount;
                hr = pChannelAudioVolume->GetChannelCount(&nChannelCount);
                if (FAILED(hr)) {
                    LOG(L"IChannelAudioVolume::GetChannelCount failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }
                
                CComHeapPtr<float> fVolumes;
                if (!fVolumes.Allocate(nChannelCount)) {
                    LOG(L"Could not allocate %u floats to store channel volumes", nChannelCount);
                    return -__LINE__;
                }
                
                hr = pChannelAudioVolume->GetAllVolumes(nChannelCount, fVolumes);
                if (FAILED(hr)) {
                    LOG(L"IChannelAudioVolume::GetAllVolumes failed: hr = 0x%08x", hr);
                    return -__LINE__;
                }

                for (UINT32 channel = 0; channel < nChannelCount; channel++) {
                    LOG(
                        L"        Channel #%u volume: %g (%g dB FS)",
                        channel + 1,
                        fVolumes[channel],
                        DbFromAmp(fVolumes[channel])
                    );
                }
                
                LOG(L"");
            }
        } // device
    } // flow
    
    return 0;
}

BOOL CALLBACK LogWindowTitles(HWND h, LPARAM pid) {
    DWORD pid2;
    GetWindowThreadProcessId(h, &pid2);
    
    if ((DWORD)pid == pid2 && IsWindowVisible(h)) {
        CComHeapPtr<WCHAR> text;

        LRESULT length = SendMessage(h, WM_GETTEXTLENGTH, 0, 0);
        if (!text.Allocate(length + 1)) {
            LOG(L"Allocate failed");
            return FALSE;
        }
        
        LRESULT copied = SendMessage(h, WM_GETTEXT, length + 1, (LPARAM)(WCHAR*)text);
        if (copied != length) {
            LOG(L"WM_GETTEXT failed; copied = %Iu, length = %Iu", copied, length);
            return FALSE;
        }
        
        LOG(L"        HWND: 0x%p %s", h, static_cast<LPCWSTR>(text));
    }
    
    return TRUE;
}

float DbFromAmp(float a) {
    return 20.0f * log10f(a);
}