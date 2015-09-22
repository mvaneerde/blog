// stringify.cpp

#include "common.h"

#define CASE_RETURN(x) case x: return L ## #x

LPCWSTR StringFromAudioSessionState(AudioSessionState state) {
    switch (state) {
        CASE_RETURN(AudioSessionStateInactive);
        CASE_RETURN(AudioSessionStateActive);
        CASE_RETURN(AudioSessionStateExpired);
        default: return L"Unrecognized";
    }
}

LPCWSTR StringFromDataFlow(EDataFlow flow) {
    switch (flow) {
        CASE_RETURN(eRender);
        CASE_RETURN(eCapture);
        CASE_RETURN(eAll);
        default: return L"Unrecognized";
    }
}