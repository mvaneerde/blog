// stringify.cpp

#include <windows.h>
#include <tchar.h>
#include <mfidl.h>
#include <atlstr.h>

#include "stringify.h"

#define CASE_RETURN(x) case x: return _T(#x)
#define GUID_RETURN(var, val) \
    if (IsEqualIID((var), (val))) { \
        return _T(#val); \
    } else {} (void)0

LPCTSTR MFPolicyManagerAction2String(MFPOLICYMANAGER_ACTION a) {
    switch(a) {
        CASE_RETURN(PEACTION_NO);
        CASE_RETURN(PEACTION_PLAY);
        CASE_RETURN(PEACTION_COPY);
        CASE_RETURN(PEACTION_EXPORT);
        CASE_RETURN(PEACTION_EXTRACT);
        CASE_RETURN(PEACTION_RESERVED1);
        CASE_RETURN(PEACTION_RESERVED2);
        CASE_RETURN(PEACTION_RESERVED3);
        // for some reason, PEACTION_RESERVED3 == PEACTION_LAST
        //CASE_RETURN(PEACTION_LAST);

        default: return _T("Unknown");
    }    
}

LPCTSTR OutputSubType2String(GUID guidOutputSubType) {
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_AGP);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_COMPONENT);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_COMPOSITE);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_D_JPN);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_DISPLAYPORT_EMBEDDED);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_DISPLAYPORT_EXTERNAL);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_DVI);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_HDMI);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_LVDS);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_PCI);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_PCI_Express);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_PCIX);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_SPDIF);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_SVIDEO);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_UDI_EMBEDDED);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_UDI_EXTERNAL);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_UNKNOWN);
    GUID_RETURN(guidOutputSubType, MFCONNECTOR_VGA);
    
    return _T("Unknown");
}

LPCTSTR ProtectionSchema2String(GUID guidProtectionSchema) {
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_DISABLE);
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_CONSTRICTVIDEO);
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_CONSTRICTAUDIO);
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_TRUSTEDAUDIODRIVERS);
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_HDCP);
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_CGMSA);
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_ACP);
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_WMDRMOTA);
    GUID_RETURN(guidProtectionSchema, MFPROTECTION_FFT);

    return _T("Unknown");
}

#define EXTRACT_FLAG(flag, bitvar, strvar) \
    if ((bitvar) & (flag)) { \
        /* add to string */ \
        strvar += CString(((strvar) == _T("")) ? _T("") : _T(" | ")) + _T(#flag); \
        \
        /* clear flag */ \
        (bitvar) &= ~(flag); \
    } else {} (void)0

CString Attributes2String(DWORD dwAttributes) {

    if (0 == dwAttributes) {
        return _T("None");
    }

    CString szFlags;

    EXTRACT_FLAG(MFOUTPUTATTRIBUTE_BUS, dwAttributes, szFlags);
    EXTRACT_FLAG(MFOUTPUTATTRIBUTE_COMPRESSED, dwAttributes, szFlags);
    EXTRACT_FLAG(MFOUTPUTATTRIBUTE_BUSIMPLEMENTATION, dwAttributes, szFlags);
    EXTRACT_FLAG(MFOUTPUTATTRIBUTE_DIGITAL, dwAttributes, szFlags);
    EXTRACT_FLAG(
        MFOUTPUTATTRIBUTE_NONSTANDARDIMPLEMENTATION,
        dwAttributes, szFlags
    );
    EXTRACT_FLAG(MFOUTPUTATTRIBUTE_SOFTWARE, dwAttributes, szFlags);
    EXTRACT_FLAG(MFOUTPUTATTRIBUTE_VIDEO, dwAttributes, szFlags);
    
    // any leftover attributes?
    if (0 != dwAttributes) {
        szFlags += CString((szFlags == _T("")) ? _T("") : _T(" | ")) + _T("???");
    }
    
    return szFlags;
}
