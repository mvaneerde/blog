// list.h

struct AcmDriverInfo {
    HACMDRIVERID id;
    DWORD fdwSupport;
    
    AcmDriverInfo();
    AcmDriverInfo(HACMDRIVERID i, DWORD d);
};

struct AcmDriverList {
    UINT32 nCount;
    AcmDriverInfo *pDrivers;
    
    AcmDriverList();
    ~AcmDriverList();
    
    MMRESULT Add(AcmDriverInfo newDriver);
};
