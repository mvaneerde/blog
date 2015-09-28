// cleanup.h

class ReleaseOnExit {
public:
    ReleaseOnExit(IUnknown *p) : m_p(p) {}
    ~ReleaseOnExit() {
        if (NULL != m_p) {
            m_p->Release();
        }
    }

private:
    IUnknown *m_p;
};

class CoUninitializeOnExit {
public:
    CoUninitializeOnExit() {}
    ~CoUninitializeOnExit() {
        CoUninitialize();
    }
};

class CoTaskMemFreeOnExit {
public:
    CoTaskMemFreeOnExit(PVOID p) : m_p(p) {}
    ~CoTaskMemFreeOnExit() {
        CoTaskMemFree(m_p);
    }
private:
    PVOID m_p;
};

class CloseHandleOnExit {
public:
    CloseHandleOnExit(HANDLE h) : m_h(h) {}
    ~CloseHandleOnExit() {
        if (NULL != m_h && INVALID_HANDLE_VALUE != m_h) {
            CloseHandle(m_h);
        }
    }
private:
    HANDLE m_h;
};