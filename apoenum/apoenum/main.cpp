// main.cpp

#include <windows.h>
#include <stdio.h>

#include "log.h"
#include "apoenum.h"

int _cdecl wmain() {

    HRESULT hr = EnumerateAudioProcessingObjects();
    
    return hr;
}