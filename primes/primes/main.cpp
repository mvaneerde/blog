// main.cpp

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "primes.h"

DWORD start = GetTickCount();

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    if (0 == argc - 1) {
        LOG(L"Enumerates primes <= the given maximum");
        return -__LINE__;
    }

    LPWSTR end = NULL;
    UINT64 max = _wcstoui64(argv[1], &end, 0);

    if (0 != errno || NULL == end || L'\0' != end[0]) {
        LOG(L"Could not parse %s as a string", argv[1]);
        return -__LINE__;
    }

    IPrimes *pPrimes = nullptr;
    HRESULT hr = CreatePrimes(max, &pPrimes);
    if (FAILED(hr)) {
        return -__LINE__;
    }

    UINT64 nPrimes = pPrimes->Count();
    if (nPrimes < 50) {
        // will log all the primes
        for (UINT64 i = 0; i < nPrimes; i++) {
            LOG(L"%I64u: %I64u", i + 1, pPrimes->Next());
        }
    } else {
        // will only log 10 primes
        for (UINT64 i = 0; i < nPrimes; i++) {
            UINT64 nPrime = pPrimes->Next();
            if (0 == i % (nPrimes / 10)) {
                LOG(L"%I64u: %I64u", i + 1, nPrime);
            }
        }        
    }
    logtime(L"Enumerating complete");
    
    pPrimes->Free();

    return 0;
}

