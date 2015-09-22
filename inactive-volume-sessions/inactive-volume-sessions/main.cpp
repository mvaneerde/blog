// main.cpp

#include "common.h"

int _cdecl wmain() {

    HRESULT hr = ListVolumesForDevices();

    return SUCCEEDED(hr) ? 0 : 1;
}
