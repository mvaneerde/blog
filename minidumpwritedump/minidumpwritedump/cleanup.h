// cleanup.h

class HandleCloser {
public:
    HandleCloser(HANDLE h) : m_h(h) {}
    ~HandleCloser() { CloseHandle(m_h); }
private:
    HANDLE m_h;
};
