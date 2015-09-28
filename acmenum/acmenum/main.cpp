// main.cpp

#include <windows.h>
#include <mmreg.h>
#include <mmiscapi.h>
#include <msacm.h>
#include <stdio.h>

#include "log.h"
#include "acmenum.h"

int _cdecl wmain() {

    MMRESULT mmr = EnumerateACMDrivers();
    
    return mmr;
}