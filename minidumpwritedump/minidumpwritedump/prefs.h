// prefs.h

class Prefs {
public:
    DWORD processId = 0;
    MINIDUMP_TYPE dumpType = MiniDumpNormal;
    LPCWSTR fileName = nullptr;

    // set hr to S_FALSE to abort but return success
    Prefs(int argc, LPCWSTR argv[], HRESULT& hr);

private:
    HRESULT Initialize(int argc, LPCWSTR argv[]);
};
