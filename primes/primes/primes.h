// primes.h

struct IPrimes {
    virtual UINT64 Count() = 0;
    virtual UINT64 Next() = 0;

    virtual void Reset() = 0;
    
    virtual void Free() = 0;
};

HRESULT CreatePrimes(UINT64 max, IPrimes **ppPrimes);