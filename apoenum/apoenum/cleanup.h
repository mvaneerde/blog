// cleanup.h

class CoTaskMemFreeOnExit {
public:
    CoTaskMemFreeOnExit(PVOID p);
    ~CoTaskMemFreeOnExit();

private:
    PVOID m_p;
};