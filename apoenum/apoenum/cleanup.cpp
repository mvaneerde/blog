// cleanup.cpp
#include <windows.h>
#include <objbase.h>
#include <stdio.h>

#include "log.h"
#include "cleanup.h"

// CoTaskMemFreeOnExit
CoTaskMemFreeOnExit::CoTaskMemFreeOnExit(PVOID p)
: m_p(p)
{}

CoTaskMemFreeOnExit::~CoTaskMemFreeOnExit() {
    CoTaskMemFree(m_p);
}
