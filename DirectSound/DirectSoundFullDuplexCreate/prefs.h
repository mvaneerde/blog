// prefs.h

class Prefs {
public:
    HRESULT Set(int argc, LPCWSTR argv[]);
    LPCGUID RenderDeviceId();
    LPCGUID CaptureDeviceId();

private:
    bool _useRenderDevice = false;
    GUID _renderDeviceId;

    bool _useCaptureDevice = false;
    GUID _captureDeviceId;
};