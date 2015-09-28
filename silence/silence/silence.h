// silence.h

// call CreateThread on this function
// feed it the address of a PlaySilenceThreadFunctionArguments
// it will render silence to the IMMDevice
// until the stop event is set
// any failures will be propagated back via hr

struct PlaySilenceThreadFunctionArguments {
    IMMDevice *pMMDevice;
    HANDLE hStartedEvent;
    HANDLE hStopEvent;
    HRESULT hr;
};

DWORD WINAPI PlaySilenceThreadFunction(LPVOID pContext);
