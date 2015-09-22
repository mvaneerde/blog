// main.cpp

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

class DeleteArrayOnExit {
public:
    DeleteArrayOnExit(void *p) : m_p(p) {}
    ~DeleteArrayOnExit() { delete [] m_p; }
    
private:
    void *m_p;
};

#define CASE_RETURN(x) case x: return L ## #x

LPCWSTR StringFromStatus(DWORD status) {
    switch (status) {
        CASE_RETURN(MIXERLINE_LINEF_ACTIVE);
        CASE_RETURN(MIXERLINE_LINEF_DISCONNECTED);
        CASE_RETURN(MIXERLINE_LINEF_SOURCE);
        
        default:
            return L"Unrecognized";
    }
}

LPCWSTR StringFromComponentType(DWORD type) {
    switch (type) {
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_UNDEFINED);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_DIGITAL);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_LINE);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_MONITOR);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_HEADPHONES);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_TELEPHONE);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_WAVEIN);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_DST_VOICEIN);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_DIGITAL);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_LINE);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY);
        CASE_RETURN(MIXERLINE_COMPONENTTYPE_SRC_ANALOG);

        default:
            return L"Unrecognized";
    }
}

LPCWSTR StringFromTargetType(DWORD type) {
    switch (type) {
        CASE_RETURN(MIXERLINE_TARGETTYPE_UNDEFINED);
        CASE_RETURN(MIXERLINE_TARGETTYPE_WAVEOUT);
        CASE_RETURN(MIXERLINE_TARGETTYPE_WAVEIN);
        CASE_RETURN(MIXERLINE_TARGETTYPE_MIDIOUT);
        CASE_RETURN(MIXERLINE_TARGETTYPE_MIDIIN);
        CASE_RETURN(MIXERLINE_TARGETTYPE_AUX);
        
        default:
            return L"Unrecognized";
    }
}

LPCWSTR StringFromControlType(DWORD type) {
    switch (type) {
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_CUSTOM);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_BOOLEANMETER);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_SIGNEDMETER);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_PEAKMETER);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_BOOLEAN);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_ONOFF);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_MUTE);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_MONO);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_LOUDNESS);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_STEREOENH);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_BASS_BOOST);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_BUTTON);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_DECIBELS);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_SIGNED);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_UNSIGNED);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_PERCENT);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_SLIDER);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_PAN);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_QSOUNDPAN);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_FADER);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_VOLUME);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_BASS);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_TREBLE);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_EQUALIZER);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_SINGLESELECT);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_MUX);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_MIXER);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_MICROTIME);
        CASE_RETURN(MIXERCONTROL_CONTROLTYPE_MILLITIME);
        default:
            return L"Unrecognized";
    }
}

LPCWSTR StringFromControlStatus(DWORD status) {
    switch (status) {
        CASE_RETURN(MIXERCONTROL_CONTROLF_DISABLED);
        CASE_RETURN(MIXERCONTROL_CONTROLF_MULTIPLE);
        CASE_RETURN(MIXERCONTROL_CONTROLF_UNIFORM);
        default:
            return L"Unrecognized";
    }
}

void LogLineInfo(UINT dev, MIXERLINE line);

int _cdecl wmain() {

    UINT devs = mixerGetNumDevs();
    LOG(L"Mixer devices: %u", devs);
    
    for (UINT dev = 0; dev < devs; dev++) {
        MIXERCAPS caps = {};
        MMRESULT mmr = mixerGetDevCaps(dev, &caps, sizeof(caps));
        
        if (MMSYSERR_NOERROR != mmr) {
            LOG(L"mixerGetDevCaps failed: MMRESULT = 0x%08x", mmr);
            continue;
        }
        
        // log capabilities
        LOG(
            L"-- %u: %s --\n"
            L"    Device ID: %u\n"
            L"    Manufacturer identifier: %u\n"
            L"    Product identifier: %u\n"
            L"    Driver version: %u.%u\n"
            L"    Product name: %s\n"
            L"    Support: 0x%x\n"
            L"    Destinations: %u"
            ,
            dev,
            caps.szPname ? caps.szPname : L"(no name)",
            dev,
            caps.wMid,
            caps.wPid,
            caps.vDriverVersion / 256, caps.vDriverVersion % 256,
            caps.szPname ? caps.szPname : L"(no name)",
            caps.fdwSupport,
            caps.cDestinations
        );

        // enumerate destination lines
        for (UINT dest = 0; dest < caps.cDestinations; dest++) {
            MIXERLINE line = {};
            line.cbStruct = sizeof(line);
            line.dwDestination = dest;
            
            mmr = mixerGetLineInfo(
                reinterpret_cast<HMIXEROBJ>(dev),
                &line,
                MIXER_OBJECTF_MIXER | MIXER_GETLINEINFOF_DESTINATION
            );
            if (MMSYSERR_NOERROR != mmr) {
                LOG(L"mixerGetLineInfo failed: MMRESULT = 0x%08x", mmr);
                continue;
            }
            
            LogLineInfo(dev, line);
        } // for each line
    } // for each device
    
    return 0;
} // main

void LogLineInfo(UINT dev, MIXERLINE line) {
    // log the line information
    LOG(
        L"        -- Destination %u: %s --\n"
        L"            Destination: %d\n"
        L"            Source: %d\n"
        L"            Line ID: 0x%08x\n"
        L"            Status: %s (%u)\n"
        L"            User: 0x%p\n"
        L"            Component Type: %s (%u)\n"
        L"            Channels: %u\n"
        L"            Connections: %u\n"
        L"            Controls: %u\n"
        L"            Short name: %s\n"
        L"            Long name: %s\n"
        L"            -- Target: %s --\n"
        L"                Type: %s (%u)\n"
        L"                Device ID: %u\n"
        L"                Manufacturer identifier: %u\n"
        L"                Product identifier: %u\n"
        L"                Driver version: %u.%u\n"
        L"                Product name: %s"
        ,
        line.dwDestination, line.szName,
        line.dwDestination,
        line.dwSource,
        line.dwLineID,
        StringFromStatus(line.fdwLine), line.fdwLine,
        (void*)line.dwUser,
        StringFromComponentType(line.dwComponentType), line.dwComponentType,
        line.cChannels,
        line.cConnections,
        line.cControls,
        line.szShortName,
        line.szName,
        line.Target.szPname,
        StringFromTargetType(line.Target.dwType), line.Target.dwType,
        line.Target.dwDeviceID,
        line.Target.wMid,
        line.Target.wPid,
        line.Target.vDriverVersion / 256, line.Target.vDriverVersion % 256,
        line.Target.szPname
    );
    
    if (0 == line.cControls) {
        return;
    }
    
    // enumerate the controls on this line
    MIXERCONTROL *controls = new MIXERCONTROL[line.cControls];
    if (nullptr == controls) {
        LOG(L"Could not allocate enough memory to get line controls");
        return;
    }
    
    MIXERLINECONTROLS line_controls = {};
    line_controls.cbStruct = sizeof(line_controls);
    line_controls.dwLineID = line.dwLineID;
    line_controls.cControls = line.cControls;
    line_controls.cbmxctrl = sizeof(controls[0]);
    line_controls.pamxctrl = controls;
    
    MMRESULT mmr = mixerGetLineControls(
        reinterpret_cast<HMIXEROBJ>(dev),
        &line_controls,
        MIXER_GETLINECONTROLSF_ALL | MIXER_OBJECTF_MIXER
    );
    if (MMSYSERR_NOERROR != mmr) {
        LOG(L"mixerGetLineControls failed: MMRESULT = 0x%08x", mmr);
        return;
    }
    
    for (DWORD i = 0; i < line.cControls; i++) {
        // log information about the control
        LOG(
            L"            -- Control %u: %s --\n"
            L"                Type: %s (0x%x)\n"
            L"                Status: %s (0x%x)\n"
            L"                Item count: %u\n"
            L"                Short name: %s\n"
            L"                Long name: %s"
            ,
            controls[i].dwControlID, controls[i].szName,
            StringFromControlType(controls[i].dwControlType), controls[i].dwControlType,
            StringFromControlStatus(controls[i].fdwControl), controls[i].fdwControl,
            controls[i].cMultipleItems,
            controls[i].szShortName,
            controls[i].szName
        );
        
        // log bounds/metrics and the current value of the control
        MIXERCONTROLDETAILS details = {};
        details.cbStruct = sizeof(details);
        details.dwControlID = controls[i].dwControlID;
        details.cChannels = (
            (MIXERCONTROL_CONTROLTYPE_CUSTOM == controls[i].dwControlType) ?
            0 :
            (controls[i].fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) ?
            1 :
            line.cChannels
        );
        details.cMultipleItems = (
            (controls[i].fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE) ?
            controls[i].cMultipleItems :
            0
        );
        
        int cValues = (
            (controls[i].fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE) ?
            details.cMultipleItems * details.cChannels :
            details.cChannels
        );
        
        // query for list names
        switch (controls[i].dwControlType) {
            case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
            case MIXERCONTROL_CONTROLTYPE_MIXER:
            case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
            case MIXERCONTROL_CONTROLTYPE_MUX:
            case MIXERCONTROL_CONTROLTYPE_SINGLESELECT: {
                // one label for each "multiple" item
                // so force the channel count to 1
                DWORD channels = details.cChannels;
                details.cChannels = 1;
                if (0 == (controls[i].fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE) || 0 == details.cMultipleItems) {
                    LOG(L"Can't query LISTTEXT on a non-multiple item");
                    continue;
                }
                
                MIXERCONTROLDETAILS_LISTTEXT *values = new MIXERCONTROLDETAILS_LISTTEXT[details.cMultipleItems];
                if (nullptr == values) {
                    LOG(L"Could not allocate enough memory to query values");
                    continue;
                }
                
                details.cbDetails = cValues * sizeof(values[0]);
                details.paDetails = values;
                
                mmr = mixerGetControlDetails(
                    reinterpret_cast<HMIXEROBJ>(dev),
                    &details,
                    MIXER_GETCONTROLDETAILSF_LISTTEXT | MIXER_OBJECTF_MIXER
                );
                if (MMSYSERR_NOERROR != mmr) {
                    LOG(L"mixerGetControlDetails failed: MMRESULT = 0x%08x", mmr);
                    continue;
                }
                details.cChannels = channels; // put it back
                
                LOG(L"                -- List Text --");
                for (UINT j = 0; j < details.cMultipleItems; j++) {
                    LOG(L"                    %u; %u; %s", values[j].dwParam1, values[j].dwParam2, values[j].szName);
                }
                break;
            }
            
            default:
                // no list names
                break;
        }
        
        // query for values
        switch (controls[i].dwControlType) {
            // custom data
            case MIXERCONTROL_CONTROLTYPE_CUSTOM: {
                details.hwndOwner = nullptr;
                BYTE *data = new BYTE[controls[i].Metrics.cbCustomData];
                if (nullptr == data) {
                    LOG(L"Could not allocate enough data to read custom control");
                    continue;
                }
                DeleteArrayOnExit daoe(data);
                
                details.cbDetails = controls[i].Metrics.cbCustomData;
                details.paDetails = data;
                
                mmr = mixerGetControlDetails(
                    reinterpret_cast<HMIXEROBJ>(dev),
                    &details,
                    MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_MIXER
                );
                if (MMSYSERR_NOERROR != mmr) {
                    LOG(L"mixerGetControlDetails failed: MMRESULT = 0x%08x", mmr);
                    continue;
                }
                
                LOG(L"                -- Value --");
                for (UINT i = 0; i < controls[i].Metrics.cbCustomData; i++) {
                    LOG(L"                    %02x", data[i]);
                }
                break;
            }

            // MIXERCONTROLDETAILS_BOOLEAN
            case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
            case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
            case MIXERCONTROL_CONTROLTYPE_BASS_BOOST:
            case MIXERCONTROL_CONTROLTYPE_BUTTON:
            case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
            case MIXERCONTROL_CONTROLTYPE_MONO:
            case MIXERCONTROL_CONTROLTYPE_MUTE:
            case MIXERCONTROL_CONTROLTYPE_ONOFF:
            case MIXERCONTROL_CONTROLTYPE_STEREOENH:
            case MIXERCONTROL_CONTROLTYPE_MIXER:
            case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
            case MIXERCONTROL_CONTROLTYPE_MUX:
            case MIXERCONTROL_CONTROLTYPE_SINGLESELECT: {
                MIXERCONTROLDETAILS_BOOLEAN *values = new MIXERCONTROLDETAILS_BOOLEAN[cValues];
                if (nullptr == values) {
                    LOG(L"Could not allocate enough memory to query values");
                    continue;
                }
                
                details.cbDetails = cValues * sizeof(values[0]);
                details.paDetails = values;
                
                mmr = mixerGetControlDetails(
                    reinterpret_cast<HMIXEROBJ>(dev),
                    &details,
                    MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_MIXER
                );
                if (MMSYSERR_NOERROR != mmr) {
                    LOG(L"mixerGetControlDetails failed: MMRESULT = 0x%08x", mmr);
                    continue;
                }
                
                LOG(L"                -- Values --");
                for (int j = 0; j < cValues; j++) {
                    LOG(L"                    %s", (values[j].fValue ? L"TRUE" : L"FALSE"));
                }
                break;
            }

            // MIXERCONTROLDETAILS_SIGNED
            case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
            case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
            case MIXERCONTROL_CONTROLTYPE_SIGNED:
            case MIXERCONTROL_CONTROLTYPE_DECIBELS:
            case MIXERCONTROL_CONTROLTYPE_PAN:
            case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
            case MIXERCONTROL_CONTROLTYPE_SLIDER: {
                MIXERCONTROLDETAILS_SIGNED *values = new MIXERCONTROLDETAILS_SIGNED[cValues];
                if (nullptr == values) {
                    LOG(L"Could not allocate enough memory to query values");
                    continue;
                }
                DeleteArrayOnExit daoe(values);
                
                details.cbDetails = cValues * sizeof(values[0]);
                details.paDetails = values;
                
                mmr = mixerGetControlDetails(
                    reinterpret_cast<HMIXEROBJ>(dev),
                    &details,
                    MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_MIXER
                );
                if (MMSYSERR_NOERROR != mmr) {
                    LOG(L"mixerGetControlDetails failed: MMRESULT = 0x%08x", mmr);
                    continue;
                }
                
                LOG(L"                -- Values --");
                for (int j = 0; j < cValues; j++) {
                    LOG(L"                    %d on a scale of %d to %d", values[j].lValue, controls[i].Bounds.lMinimum, controls[i].Bounds.lMaximum);
                }

                break;
            }
            
            // MIXERCONTROLDETAILS_UNSIGNED
            case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
            case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
            case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
            case MIXERCONTROL_CONTROLTYPE_BASS:
            case MIXERCONTROL_CONTROLTYPE_TREBLE:
            case MIXERCONTROL_CONTROLTYPE_FADER:
            case MIXERCONTROL_CONTROLTYPE_VOLUME:
            case MIXERCONTROL_CONTROLTYPE_MICROTIME:
            case MIXERCONTROL_CONTROLTYPE_MILLITIME:
            case MIXERCONTROL_CONTROLTYPE_PERCENT: {
                MIXERCONTROLDETAILS_UNSIGNED *values = new MIXERCONTROLDETAILS_UNSIGNED[cValues];
                if (nullptr == values) {
                    LOG(L"Could not allocate enough memory to query values");
                    continue;
                }
                DeleteArrayOnExit daoe(values);
                
                details.cbDetails = cValues * sizeof(values[0]);
                details.paDetails = values;
                
                mmr = mixerGetControlDetails(
                    reinterpret_cast<HMIXEROBJ>(dev),
                    &details,
                    MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_MIXER
                );
                if (MMSYSERR_NOERROR != mmr) {
                    LOG(L"mixerGetControlDetails failed: MMRESULT = 0x%08x", mmr);
                    continue;
                }
                
                LOG(L"                -- Values --");
                for (int j = 0; j < cValues; j++) {
                    LOG(L"                    0x%x on a scale of 0x%x to 0x%x", values[j].dwValue, controls[i].Bounds.dwMinimum, controls[i].Bounds.dwMaximum);
                }

                break;
            }
                
            default:
                LOG(L"Unrecognized control type 0x%x", controls[i].dwControlType);
                break;
        } // switch on control type
    } // for each control
}