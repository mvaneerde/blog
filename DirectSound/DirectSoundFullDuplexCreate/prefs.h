// prefs.h

class Prefs {
public:
    HRESULT Set(int argc, LPCWSTR argv[]);
    LPCGUID RenderDeviceId();
    LPCGUID CaptureDeviceId();

private:
    bool _specificRenderDevice = false;
    GUID _renderDeviceId;

    bool _specificCaptureDevice = false;
    GUID _captureDeviceId;
};