// prefs.h

class Prefs {
public:
    HRESULT Set(int argc, LPCWSTR argv[]);
    bool V8();
    LPCGUID RenderDeviceId();
    LPCGUID CaptureDeviceId();

private:
    bool _v8 = false;

    bool _specificRenderDevice = false;
    GUID _renderDeviceId;

    bool _specificCaptureDevice = false;
    GUID _captureDeviceId;
};