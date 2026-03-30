#include "common.h"

int wmain(int argc, LPCWSTR argv[]) {
    bool run = false;
    Prefs p; // inherits from SHELLEXECUTEOPTIONSW
    if (p.Parse(argc, argv, run)) {
        if (run) {
            BOOL result = ShellExecuteExW(&p);
            DWORD error = (result ? ERROR_SUCCESS : GetLastError());
            p.LogResult(result);
            LOG(L"Result of ShellExecutExW: %d", result);
            if (!result) {
                return error;
            }
        } else {
            return ERROR_INVALID_PARAMETER;
        }
    }

    return 0;
}