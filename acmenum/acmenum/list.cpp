// list.cpp

#include <windows.h>
#include <mmreg.h>
#include <mmiscapi.h>
#include <msacm.h>
#include <stdio.h>

#include "list.h"
#include "log.h"

// AcmDriverInfo
AcmDriverInfo::AcmDriverInfo()
: id(NULL), fdwSupport(0)
{}

AcmDriverInfo::AcmDriverInfo(HACMDRIVERID i, DWORD d)
: id(i), fdwSupport(d)
{}

// AcmDriverList
AcmDriverList::AcmDriverList()
: nCount(0), pDrivers(NULL)
{}


AcmDriverList::~AcmDriverList() {
    if (NULL != pDrivers) {
        delete [] pDrivers;
    }
}
    
MMRESULT AcmDriverList::Add(AcmDriverInfo newDriver) {
    // allocate the new list
    AcmDriverInfo *pNewDrivers = new AcmDriverInfo[nCount + 1];
    
    if (NULL == pNewDrivers) {
        ERR(L"Could not allocate %u-element AcmDriverInfo array", nCount + 1);
        return MMSYSERR_NOMEM;
    }
    
    // copy all the old drivers over
    for (UINT32 i = 0; i < nCount; i++) {
        pNewDrivers[i] = pDrivers[i];
    }
    
    // don't forget the new driver
    pNewDrivers[nCount] = newDriver;
    
    // free the old list
    if (NULL != pDrivers) {
        delete [] pDrivers;
    }
    
    // swap in the new list
    pDrivers = pNewDrivers;
    nCount++;
    
    return MMSYSERR_NOERROR;
}