// main.cpp

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <stdio.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

class DeleteArrayOnExit {
public:
    DeleteArrayOnExit(PVOID p) : m_p(p) {}
    ~DeleteArrayOnExit() { delete [] m_p; }
private:
    PVOID m_p;
};

void LogMidiInCaps(UINT i, MIDIINCAPS caps);
void GetMidiInDeviceInterface(UINT i);
void LogMidiOutCaps(UINT i, MIDIOUTCAPS caps);
void GetMidiOutDeviceInterface(UINT i);

int _cdecl wmain() {

    UINT devs = midiInGetNumDevs();
    LOG(L"midiIn devices: %u", devs);
    for (UINT dev = 0; dev < devs; dev++) {
        MIDIINCAPS caps = {};
        MMRESULT mmr = midiInGetDevCaps(dev, &caps, sizeof(caps));
        
        if (MMSYSERR_NOERROR != mmr) {
             LOG(L"midiInGetDevCaps failed: mmr = 0x%08x", mmr);
             return mmr;
        }
        
        LogMidiInCaps(dev, caps);
        GetMidiInDeviceInterface(dev);
    }

    devs = midiOutGetNumDevs();
    LOG(L"midiOut devices: %u", devs);
    for (UINT dev = 0; dev < devs; dev++) {
        MIDIOUTCAPS caps = {};
        MMRESULT mmr = midiOutGetDevCaps(dev, &caps, sizeof(caps));
        
        if (MMSYSERR_NOERROR != mmr) {
             LOG(L"midiOutGetDevCaps failed: mmr = 0x%08x", mmr);
             return mmr;
        }
        
        LogMidiOutCaps(dev, caps);
        GetMidiOutDeviceInterface(dev);
    }

    return 0;
}

void GetMidiInDeviceInterface(UINT i) {
    // query the size of the device interface string
    HMIDIIN h = reinterpret_cast<HMIDIIN>(i);
    ULONG size = 0;
    MMRESULT mmr = midiInMessage(
        h,
        DRV_QUERYDEVICEINTERFACESIZE,
        reinterpret_cast<DWORD_PTR>(&size),
        0
    );
    if (MMSYSERR_NOERROR != mmr) {
        LOG(L"midiInMessage(DRV_QUERYDEVICEINTERFACESIZE) failed: mmr = 0x%08x", mmr);
        return;
    }
    
    if (0 == size) {
        LOG(L"No device interface");
        return;
    }
    if (size % sizeof(WCHAR)) {
        LOG(L"Device interface length in bytes (%u) should be a multiple of the size of a WCHAR!", size);
        return;
    }
    
    WCHAR *buffer = new WCHAR[size / sizeof(WCHAR)];
    if (nullptr == buffer) {
        LOG(L"Could not allocate %Iu WCHARs", size / sizeof(WCHAR));
    }
    DeleteArrayOnExit daoe(buffer);
    
    mmr = midiInMessage(
        h,
        DRV_QUERYDEVICEINTERFACE,
        reinterpret_cast<DWORD_PTR>(buffer),
        size
    );
    if (MMSYSERR_NOERROR != mmr) {
        LOG(L"midiInMessage(DRV_QUERYDEVICEINTERFACE) failed: mmr = 0x%08x", mmr);
        return;
    }

    LOG(L"    Device interface: \"%s\"", buffer);
}

void GetMidiOutDeviceInterface(UINT i) {
    // query the size of the device interface string
    HMIDIOUT h = reinterpret_cast<HMIDIOUT>(i);
    ULONG size = 0;
    MMRESULT mmr = midiOutMessage(
        h,
        DRV_QUERYDEVICEINTERFACESIZE,
        reinterpret_cast<DWORD_PTR>(&size),
        0
    );
    if (MMSYSERR_NOERROR != mmr) {
        LOG(L"midiOutMessage(DRV_QUERYDEVICEINTERFACESIZE) failed: mmr = 0x%08x", mmr);
        return;
    }
    
    if (0 == size) {
        LOG(L"No device interface");
        return;
    }
    if (size % sizeof(WCHAR)) {
        LOG(L"Device interface length in bytes (%u) should be a multiple of the size of a WCHAR!", size);
        return;
    }
    
    WCHAR *buffer = new WCHAR[size / sizeof(WCHAR)];
    if (nullptr == buffer) {
        LOG(L"Could not allocate %Iu WCHARs", size / sizeof(WCHAR));
    }
    DeleteArrayOnExit daoe(buffer);
    
    mmr = midiOutMessage(
        h,
        DRV_QUERYDEVICEINTERFACE,
        reinterpret_cast<DWORD_PTR>(buffer),
        size
    );
    if (MMSYSERR_NOERROR != mmr) {
        LOG(L"midiOutMessage(DRV_QUERYDEVICEINTERFACE) failed: mmr = 0x%08x", mmr);
        return;
    }

    LOG(L"    Device interface: \"%s\"", buffer);
}

void LogMidiInCaps(UINT i, MIDIINCAPS caps) {
    LOG(
        L"-- %u: %s --\n"
        L"    Device ID: %u\n"
        L"    Manufacturer identifier: %u\n"
        L"    Product identifier: %u\n"
        L"    Driver version: %u.%u\n"
        L"    Product name: %s\n"
        L"    Support: 0x%x"
        ,
        i,
        caps.szPname ? caps.szPname : L"(no name)",
        i,
        caps.wMid,
        caps.wPid,
        caps.vDriverVersion / 256, caps.vDriverVersion % 256,
        caps.szPname ? caps.szPname : L"(no name)",
        caps.dwSupport
    );
}

void LogMidiOutCaps(UINT i, MIDIOUTCAPS caps) {
    LOG(
        L"-- %u: %s --\n"
        L"    Device ID: %u\n"
        L"    Manufacturer identifier: %u\n"
        L"    Product identifier: %u\n"
        L"    Driver version: %u.%u\n"
        L"    Product name: %s\n"
        L"    Technology: %u (%s)\n"
        L"    Voices: %u\n"
        L"    Notes: %u\n"
        L"    Channel mask: 0x%x\n"
        L"    Support: 0x%x"
        L"%s%s%s%s"
        ,
        i,
        caps.szPname ? caps.szPname : L"(no name)",
        i,
        caps.wMid,
        caps.wPid,
        caps.vDriverVersion / 256, caps.vDriverVersion % 256,
        caps.szPname ? caps.szPname : L"(no name)",
        caps.wTechnology,
        caps.wTechnology     == MOD_MIDIPORT  ? L"MOD_MIDIPORT" :
            caps.wTechnology == MOD_SYNTH     ? L"MOD_SYNTH" :
            caps.wTechnology == MOD_SQSYNTH   ? L"MOD_SQSYNTH" :
            caps.wTechnology == MOD_FMSYNTH   ? L"MOD_FMSYNTH" :
            caps.wTechnology == MOD_MAPPER    ? L"MOD_MAPPER" :
            caps.wTechnology == MOD_WAVETABLE ? L"MOD_WAVETABLE" :
            caps.wTechnology == MOD_SWSYNTH   ? L"MOD_SWSYNTH" :
            L"Unrecognized",
        caps.wVoices,
        caps.wNotes,
        caps.wChannelMask,
        caps.dwSupport,
            (caps.dwSupport & MIDICAPS_CACHE)    ? L"\n        MIDICAPS_CACHE"    : L"",
            (caps.dwSupport & MIDICAPS_LRVOLUME) ? L"\n        MIDICAPS_LRVOLUME" : L"",
            (caps.dwSupport & MIDICAPS_STREAM)   ? L"\n        MIDICAPS_STREAM"   : L"",
            (caps.dwSupport & MIDICAPS_VOLUME)   ? L"\n        MIDICAPS_VOLUME"   : L""
    );
}
