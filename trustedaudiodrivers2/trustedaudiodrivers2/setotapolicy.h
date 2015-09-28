// setotapolicy.h

HRESULT SetOTAPolicy(
    IMMDevice *pMMDevice,
    bool bCopyOK,
    bool bDigitalOutputDisable,
    bool bTestCertificateEnable,
    DWORD dwDrmLevel
);