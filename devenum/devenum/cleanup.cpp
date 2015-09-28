// cleanup.cpp

#include <windows.h>
#include <objbase.h>

#include "cleanup.h"

// CoTaskMemFreeOnExit class
CoTaskMemFreeOnExit::CoTaskMemFreeOnExit(PVOID p) : m_p(p) {}

CoTaskMemFreeOnExit::~CoTaskMemFreeOnExit() { CoTaskMemFree(m_p); }

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

// VariantInitAndClearOnExit class
VariantInitAndClearOnExit::VariantInitAndClearOnExit(VARIANT *pVar)
: m_pVar(pVar) {
    VariantInit(m_pVar);
}
VariantInitAndClearOnExit::~VariantInitAndClearOnExit() {
    VariantClear(m_pVar);
}