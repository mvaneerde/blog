// primes.cpp

#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "log.h"
#include "primes.h"

BYTE getbit(const BYTE *array, UINT64 index);
void setbit(BYTE *array, UINT64 index);

class CPrimes : public IPrimes {
public:
    CPrimes(UINT64 max, HRESULT *pHr);
    ~CPrimes();
    virtual UINT64 Count();
    virtual UINT64 Next();

    virtual void Reset();
    
    virtual void Free();
private:
    HRESULT Initialize(UINT64 max);
    
    BYTE *m_pBits;
    UINT64 m_nBits;
    UINT64 m_nPrimes;

    UINT64 m_bitPosition;
    UINT64 m_primePosition;
};

HRESULT CreatePrimes(UINT64 max, IPrimes **ppPrimes) {
    HRESULT hr = S_OK;
    IPrimes *pPrimes = new CPrimes(max, &hr);
    if (SUCCEEDED(hr)) {
        *ppPrimes = pPrimes;
    } else {
        pPrimes->Free();
    }
    
    return hr;
}

CPrimes::CPrimes(UINT64 max, HRESULT *pHr)
: m_pBits(nullptr)
, m_nBits(0)
, m_nPrimes(0)
, m_bitPosition(0)
, m_primePosition(0)
{
    *pHr = Initialize(max);
}

CPrimes::~CPrimes() {
    delete [] m_pBits;
}

UINT64 CPrimes::Count() {
    return m_nPrimes;
}

void CPrimes::Reset() {
    m_bitPosition = 0;
    m_primePosition = 0;
}

UINT64 CPrimes::Next() {
    if (m_primePosition >= m_nPrimes) {
        LOG(L"ERROR: m_primePosition is too large");
        return 0;
    }

    UINT64 prime = (m_primePosition ? 2 * m_bitPosition + 3 : 2);
    
    m_primePosition++;
    if (1 < m_primePosition && m_primePosition < m_nPrimes) {
        // advance to the next prime
        m_bitPosition++;
        for (; m_bitPosition < m_nBits; m_bitPosition++) {
            if (!getbit(m_pBits, m_bitPosition)) {
                // LOG(L"Bit position %I64u is 0 (%I64u)", m_bitPosition, 2 * m_bitPosition + 3);
                break;
            }
            // LOG(L"Bit position %I64u is 1 (%I64u)", m_bitPosition, 2 * m_bitPosition + 3);
        }
        
        if (m_bitPosition >= m_nBits) {
            LOG(L"ERROR: m_bitPosition walked off the end");
        }
    }
    
    return prime;
}

void CPrimes::Free() {
    LOG(L"Freeing CPrimes object");
    delete this;
}

HRESULT CPrimes::Initialize(UINT64 max) {
    if (max < 3) {
        LOG(L"Specify an input of at least 3, not %I64u", max);
        return E_INVALIDARG;
    }
    LOG(L"Will enumerate primes <= %I64u = %lg", max, (double)max);

    // figure out how much memory we need to allocate for the sieve
    m_nBits = (max - 1) / 2;
    UINT64 nBytes = m_nBits / 8;
    if (m_nBits % 8) { nBytes++; }
    logsize(L"Memory for sieve", nBytes);
    
    if ((UINT64)(size_t)(nBytes) < nBytes) {
        LOG(L"%I64u unsigned chars is too big for a size_t", nBytes);
        return E_INVALIDARG;
    }
    
    m_pBits = new BYTE[ (size_t)nBytes ];
    if (nullptr == m_pBits) {
        LOG(L"Could not allocate %I64u bytes for sieve", nBytes);
        return E_OUTOFMEMORY;
    }
            
    // initializing to 0
    for (UINT64 i = 0; i < nBytes; i++) { m_pBits[i] = 0; }
    logtime(L"Initialization complete");
        
    m_nPrimes = 1; // 2
    // LOG(L"2");
    UINT64 nSqrt = static_cast<UINT64>(sqrt((double)max) + 0.5);
    
    LOG(L"Sieving to %I64u", nSqrt);
    UINT64 n;
    for (n = 3; n <= nSqrt; n += 2) {
        // skip known non-primes
        if (getbit(m_pBits, (n - 3)/2)) {
            continue;
        }
        
        // LOG(L"%I64u", n);
        m_nPrimes++;

        for (UINT64 m = n * n; m < max; m += 2 * n) {
            setbit(m_pBits, (m - 3)/2);
        }
    }
    logtime(L"Sieving complete");
    
    LOG(L"Picking up the rest to %I64u", max);
    for (; n <= max; n += 2) {
        if (getbit(m_pBits, (n - 3)/2)) {
            continue;
        }
        
        // LOG(L"%I64u", n);
        m_nPrimes++;
    }
    logtime(L"Pickup complete");

    LOG(L"Primes: %I64u", m_nPrimes);
    
    return S_OK;
}

BYTE getbit(const BYTE *array, UINT64 index) {
    // LOG(L"Retrieving bit %I64u", index);
    return ((array[index / 8]) >> (index % 8)) & (BYTE)1;
}

void setbit(BYTE *array, UINT64 index) {
    // LOG(L"Setting bit %I64u", index);
    array[index / 8] |= ((BYTE)1 << (index % 8));
    // for (UINT64 i = 0; i < 1; i++) { LOG(L"%02x", array[i]); }
}
