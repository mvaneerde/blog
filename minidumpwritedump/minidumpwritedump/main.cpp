// main.cpp

#include "common.h"

int _cdecl wmain(int argc, LPCWSTR argv[])
{
    // parse arguments
    HRESULT hr;
    Prefs prefs(argc, argv, hr);

    switch (hr) {
    case S_OK:
        return dump(prefs.processId, prefs.dumpType, prefs.fileName);

    case S_FALSE:
        // nothing to do
        return 0;

    default:
        return hr;
    }
}
