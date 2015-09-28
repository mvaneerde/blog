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

// specifically to clean up the array handed back by MFTEnumEx
// this is a list of pointers which must be CoTaskMemFree'd
// but before that, each pointer needs to be IUnknown::Release'd
//
// note that an array of IMFActivate *'s cannot be cast to an array of IUnknown *'s
//
class DisposeOfMFActivateArray {

public:
    DisposeOfMFActivateArray(
        IMFActivate **ppMFActivateObjects,
        UINT32 cMFActivateObjects
    );
    
    ~DisposeOfMFActivateArray();
    
private:
    IMFActivate **m_ppMFActivateObjects;
    UINT32 m_cMFActivateObjects;

};
