// prefs.h

class CPrefs {
public:
    IMMDevice *m_pMMDevice;

    // set hr to S_FALSE to abort but return success
    CPrefs(int argc, LPCWSTR argv[], HRESULT &hr);
    ~CPrefs();

};
