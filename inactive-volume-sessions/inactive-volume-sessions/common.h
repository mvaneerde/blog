// common.h

#include <windows.h>
#include <math.h>
#include <cguid.h>
#include <atlbase.h>
#include <stdio.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>

#include "cleanup.h"
#include "db.h"
#include "log.h"
#include "session.h"
#include "stringify.h"
#include "volume.h"

#define AUDCLNT_S_NO_SINGLE_PROCESS AUDCLNT_SUCCESS (0x00d)
