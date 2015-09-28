// log.cpp
#include <windows.h>
#include <stdio.h>

#include "log.h"

LPCWSTR sizeUnit(double *pBytes);
LPCWSTR timeUnit(double *pMilliseconds);

void logsize(LPCWSTR prefix, UINT64 nBytes) {
    double s = (double)nBytes;
    
    LPCWSTR szUnit = sizeUnit(&s);
    
    LOG(L"%s: %lg %s", prefix, s, szUnit);
}

void logtime(LPCWSTR prefix) {
    double t = (double)(GetTickCount() - start);
    LPCWSTR unit = timeUnit(&t);
    LOG(L"%s: %lg %s since start", prefix, t, unit);
}

LPCWSTR sizeUnit(double *pBytes) {
    LPCWSTR unit = L"bytes";
    
    if (*pBytes < 1024.0) { return unit; }
    *pBytes /= 1024.0;
    unit = L"KB";

    if (*pBytes < 1024.0) { return unit; }
    *pBytes /= 1024.0;
    unit = L"MB";
    
    if (*pBytes < 1024.0) { return unit; }
    *pBytes /= 1024.0;
    return L"GB";
}

LPCWSTR timeUnit(double *pMilliseconds) {
    LPCWSTR unit = L"milliseconds";

    if (*pMilliseconds < 1000.0) { return unit; }
    *pMilliseconds /= 1000.0;
    unit = L"seconds";

    if (*pMilliseconds < 60.0) { return unit; }
    *pMilliseconds /= 60.0;
    unit = L"minutes";

    if (*pMilliseconds < 60.0) { return unit; }
    *pMilliseconds /= 60.0;
    unit = L"hours";

    if (*pMilliseconds < 24.0) { return unit; }
    *pMilliseconds /= 24.0;
    return L"days";
}
