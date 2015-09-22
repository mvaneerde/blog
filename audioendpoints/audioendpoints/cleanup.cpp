// cleanup.cpp

#include <windows.h>
#include <objbase.h>

#include "cleanup.h"

// CoTaskMemFreeOnExit class
CoTaskMemFreeOnExit::CoTaskMemFreeOnExit(PVOID p) : m_p(p) {}

void CoTaskMemFreeOnExit::NewTarget(PVOID p) {
    m_p = p;
}

CoTaskMemFreeOnExit::~CoTaskMemFreeOnExit() {
    if (NULL != m_p) {
        CoTaskMemFree(m_p);
    }
}

// PropVariantClearOnExit class
PropVariantClearOnExit::PropVariantClearOnExit(PROPVARIANT *p) : m_p(p) {}

PropVariantClearOnExit::~PropVariantClearOnExit() {
    if (NULL != m_p) {
        PropVariantClear(m_p);
    }
}

// CoUninitializeOnExit class
CoUninitializeOnExit::CoUninitializeOnExit() {}
CoUninitializeOnExit::~CoUninitializeOnExit() { CoUninitialize(); }

// ReleaseOnExit class
ReleaseOnExit::ReleaseOnExit(IUnknown *p) : m_p(p) {}
ReleaseOnExit::~ReleaseOnExit() {
    if (NULL != m_p) {
        m_p->Release();
    }
}