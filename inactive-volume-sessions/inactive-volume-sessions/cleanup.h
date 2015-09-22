// cleanup.h

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() {
        CoUninitialize();
    }
};

class CloseHandleOnExit {
public:
    CloseHandleOnExit(HANDLE h) : m_h(h) {}
    ~CloseHandleOnExit() {
        CloseHandle(m_h);
    }
private:
    HANDLE m_h;
};

class PropVariantClearOnExit {
public:
    PropVariantClearOnExit(PROPVARIANT *p) : m_p(p) {}
    ~PropVariantClearOnExit() {
        PropVariantClear(m_p);
    }
private:
    PROPVARIANT *m_p;
};

