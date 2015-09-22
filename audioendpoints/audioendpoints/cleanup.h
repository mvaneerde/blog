// cleanup.h

class CoTaskMemFreeOnExit {

public:
    CoTaskMemFreeOnExit(PVOID p);
    void NewTarget(PVOID p);
    ~CoTaskMemFreeOnExit();

private:
    PVOID m_p;
    
};

class PropVariantClearOnExit {

public:
    PropVariantClearOnExit(PROPVARIANT *p);
    ~PropVariantClearOnExit();

private:
    PROPVARIANT *m_p;
    
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