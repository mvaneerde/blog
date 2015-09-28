// cleanup.cpp

#include <windows.h>
#include <mfapi.h>

#include "cleanup.h"

// CoTaskMemFreeOnExit class
CoTaskMemFreeOnExit::CoTaskMemFreeOnExit(PVOID p) : m_p(p) {}

CoTaskMemFreeOnExit::~CoTaskMemFreeOnExit() { CoTaskMemFree(m_p); }

// DisposeOfMFActivateArray class
DisposeOfMFActivateArray::DisposeOfMFActivateArray(
    IMFActivate **ppMFActivateObjects,
    UINT32 cMFActivateObjects
)
: m_ppMFActivateObjects(ppMFActivateObjects)
, m_cMFActivateObjects(cMFActivateObjects)
{}

DisposeOfMFActivateArray::~DisposeOfMFActivateArray() {
    for (UINT32 i = 0; i < m_cMFActivateObjects; i++) {
        m_ppMFActivateObjects[i]->Release();
    }
    
    CoTaskMemFree(m_ppMFActivateObjects);
}
