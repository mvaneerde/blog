// cleanup.h

// to allow early return
// create an object on the stack
// when the object is destroyed,
// it calls CoTaskMemFree on its argument
//
// e.g.
// BYTE *pByte = CoTaskMemAlloc(...);
// if (NULL == pByte) { ... }
//
// CoTaskMemFreeOnExit freeBytes(pByte);
// // from this point on code can return without worrying about leaking pByte
//
class CoTaskMemFreeOnExit {

public:
    CoTaskMemFreeOnExit(PVOID p);
    ~CoTaskMemFreeOnExit();

private:
    PVOID m_p;
    
};

class CoUninitializeOnExit {

public:
    CoUninitializeOnExit();
    ~CoUninitializeOnExit();

};

class ReleaseOnExit {

public:
    ReleaseOnExit(IUnknown *p);
    ~ReleaseOnExit();

private:
    IUnknown *m_p;
    
};