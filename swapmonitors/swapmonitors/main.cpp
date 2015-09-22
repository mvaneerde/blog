// main.cpp

#include <windows.h>
#include <stdio.h>

#define LOG(format, ...) wprintf( format L"\n", __VA_ARGS__)
#define ERR(format, ...) LOG( L"ERROR: " format, __VA_ARGS__)

// assumptions:
// there are precisely two monitors
// the monitors are laid out horizontally
//
// this will put the left monitor on the right, and the right monitor on the left
//
// since we can only move the secondary monitor,
// this means the secondary monitor will go from:
// a wholly negative x position to a wholly positive
// or from a wholly positive x position to a wholly negative
//
// we will also align the tops of the monitors
// (that is, we will set the secondary to (..., 0)
//
int _cdecl wmain() {
    DISPLAY_DEVICE device = {};
    device.cb = sizeof(device);
    
    // primary monitor is always positioned at (0, 0)
    int count = 0;
    DWORD dxWidthPrimary = 0;
    DWORD dxWidthSecondary = 0;
    POINTL posSecondary = {0};
    WCHAR nameSecondary[32] = {0};
    
    bool seenPrimary = false;
    for (
        DWORD i = 0;
        EnumDisplayDevices(nullptr, i, &device, 0);
        i++
    ) {
        // LOG(L"Device %u: %s", i, device.DeviceName);
        
        if ((device.StateFlags & DISPLAY_DEVICE_ACTIVE) != DISPLAY_DEVICE_ACTIVE) {
            LOG(L"Skipping inactive display");
            continue;
        }
        
        count++;
        
        DEVMODE mode = {};
        mode.dmSize = sizeof(mode);
        
        if (!EnumDisplaySettings(
            device.DeviceName,
            ENUM_CURRENT_SETTINGS,
            &mode
        )) {
            LOG(L"EnumDisplaySettings failed with error %u", GetLastError());
            return -__LINE__;
        }
        
        // LOG(L"Position: (%d, %d)", mode.dmPosition.x, mode.dmPosition.y);
        // LOG(L"Dimensions: %u x %u", mode.dmPelsWidth, mode.dmPelsHeight);
        
        if (0 == mode.dmPosition.x && 0 == mode.dmPosition.y) {
            if (seenPrimary) {
                ERR(L"There appear to be at least two monitors positioned at (0, 0)");
                return -__LINE__;
            }
            
            seenPrimary = true;
            dxWidthPrimary = mode.dmPelsWidth;
        } else {
            posSecondary = mode.dmPosition;
            dxWidthSecondary = mode.dmPelsWidth;
            wcscpy_s(nameSecondary, ARRAYSIZE(nameSecondary), device.DeviceName);
        }
    }
    
    DWORD err = GetLastError();
    if (ERROR_SUCCESS != err) {
        LOG(L"EnumDisplayDevices failed with error %u", err);
        return -__LINE__;
    }
    
    if (2 != count) {
        LOG(L"Count of active monitors is %u; we will only swap 2, nothing to do", count);
        return 0;
    }
    
    if (!seenPrimary) {
        LOG(L"No primary monitor");
        return -__LINE__;
    }
    
    if (nameSecondary[0] == L'\0') {
        LOG(L"No secondary monitor");
        return -__LINE__;
    }
    
    // LOG(
    //    L"Primary is %u wide; secondary (%s) is %u wide and is at (%d, %d)",
    //    dxWidthPrimary, nameSecondary, dxWidthSecondary, posSecondary.x, posSecondary.y
    // );
    
    posSecondary.y = 0;
    if (posSecondary.x < 0) {
        // currently on the left, move to the right;
        posSecondary.x = dxWidthPrimary;
    } else {
        // currently on the right, move to the left
        posSecondary.x = -(LONG)dxWidthSecondary;
    }
    
    DEVMODE mode = {0};
    mode.dmSize = sizeof(mode);

    mode.dmFields |= DM_POSITION;
    mode.dmPosition = posSecondary;
    
    LONG status = ChangeDisplaySettingsEx(
        nameSecondary,
        &mode,
        nullptr, // reserved
        CDS_GLOBAL | CDS_UPDATEREGISTRY,
        nullptr // no video parameter
    );
    
    if (DISP_CHANGE_SUCCESSFUL != status) {
        LOG(L"ChangeDisplaySettingsEx returned %d", status);
        return -__LINE__;
    }
 
    LOG(L"Moved secondary monitor to (%d, %d)", posSecondary.x, posSecondary.y);
    
    return 0;
}