// prefs.h

class Prefs {
public:
    HRESULT Set(int argc, LPCWSTR argv[]);
    bool V8();
    bool Render();
    LPCGUID DeviceId();

private:
    bool _capture = false;
    bool _v8 = false;
    bool _specificDevice = false;
    GUID _deviceId;
};