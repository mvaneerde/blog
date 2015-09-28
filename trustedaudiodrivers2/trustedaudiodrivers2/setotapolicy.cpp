// setotapolicy.cpp

#include <windows.h>
#include <tchar.h>
#include <mmdeviceapi.h>
#include <atlstr.h>
#include <mfidl.h>

#include "log.h"
#include "stringify.h"
#include "outputpolicy.h"

#include "setotapolicy.h"

HRESULT SetOTAPolicy(
    IMMDevice *pMMDevice,
    bool bCopyOK,
    bool bDigitalOutputDisable,
    bool bTestCertificateEnable,
    DWORD dwDrmLevel
) {
    HRESULT hr = S_OK;

    LOG(
        _T("Will set the following audio policy:\n")
        _T("Test Certificate Enable: %s\n")
        _T("Copy OK: %s\n")
        _T("Digital Output Disable: %s\n")
        _T("DRM Level: %u\n"),
        (bTestCertificateEnable ? _T("True") : _T("False") ),
        (bCopyOK ? _T("True") : _T("False") ),
        (bDigitalOutputDisable? _T("True") : _T("False") ),
        dwDrmLevel
    );

    DWORD dwConfigData =
        MAKE_MFPROTECTIONDATA_TRUSTEDAUDIODRIVERS2(
            bTestCertificateEnable, bDigitalOutputDisable, bCopyOK, dwDrmLevel
        );

    CComPtr<IMFOutputPolicy> pMFOutputPolicy;
    hr = CreateTrustedAudioDriversOutputPolicy(dwConfigData, &pMFOutputPolicy);
    if (FAILED(hr)) {
        ERR(_T("CreateTrustedAudioDriversOutputPolicy failed: hr = 0x%08x"), hr);
        return hr;
    }

    CComPtr<IMFTrustedOutput> pMFTrustedOutput;
    // activate IMFTrustedOutput
    hr = pMMDevice->Activate(
        __uuidof(IMFTrustedOutput), CLSCTX_ALL, NULL,
        (void**)&pMFTrustedOutput
    );

    if (FAILED(hr)) {
        ERR(_T("IMMDevice::Activate(IMFTrustedOutput) failed: hr = 0x%08x"), hr);
        return hr;
    }

    // get count of Output Trust Authorities on this trusted output
    DWORD dwCountOfOTAs;
    hr = pMFTrustedOutput->GetOutputTrustAuthorityCount(&dwCountOfOTAs);
    if (FAILED(hr)) {
        ERR(
            _T("IMFTrustedOutput::GetOutputTrustAuthorityCount failed: hr = 0x%08x"),
            hr
        );
        return hr;
    }

    // sanity check - fail on endpoints with no output trust authorities
    if (0 == dwCountOfOTAs) {
        hr = E_NOTFOUND;
        ERR(_T("There are no Output Trust Authorities associated with this endpoint."));
        return hr;
    }

    LOG(_T("Output Trust Authorities on this endpoint: %d"), dwCountOfOTAs);
    
    // loop over each output trust authority on the endpoint
    bool bAnySuccesses = false;
    for (DWORD i = 0; i < dwCountOfOTAs; i++) {
        LOG(_T("Processing Output Trust Authority #%d of %d"), i + 1, dwCountOfOTAs);

        // get the output trust authority
        CComPtr<IMFOutputTrustAuthority> pMFOutputTrustAuthority;
        hr = pMFTrustedOutput->GetOutputTrustAuthorityByIndex(i, &pMFOutputTrustAuthority);
        if (FAILED(hr)) {
            ERR(
                _T("IMFTrustedOutput::GetOutputTrustAuthority(%d) failed: hr = 0x%08x"),
                i, hr
            );
            return hr;
        }

        // log the purpose of the output trust authority
        MFPOLICYMANAGER_ACTION action;
        hr = pMFOutputTrustAuthority->GetAction(&action);
        if (FAILED(hr)) {
            ERR(_T("IMFOutputTrustAuthority::GetAction failed: hr = 0x%08x"), hr);
            return hr;
        }

        LOG(_T("OTA action is %s (%d)"), MFPolicyManagerAction2String(action), action);

        // only PEACTION_PLAY Output Trust Authorities are relevant
        if (PEACTION_PLAY != action) {
            LOG(_T("Skipping as the OTA action is not PEACTION_PLAY"));
            continue;
        }

        BYTE *pbTicket = NULL;
        DWORD cbTicket = 0;
        
        // we're only setting a single policy but the API allows setting multiple policies
        // (like WaitForMultipleObjects)
        IMFOutputPolicy *rMFOutputPolicies[1] = { pMFOutputPolicy };
        hr = pMFOutputTrustAuthority->SetPolicy(
            rMFOutputPolicies,
            ARRAYSIZE(rMFOutputPolicies),
            &pbTicket,
            &cbTicket
        );
        
        // we don't need no stinkin' tickets
        if (NULL != pbTicket) { CoTaskMemFree(pbTicket); }
        
        if (FAILED(hr)) {
            ERR(_T("IMFOutputTrustAuthority::SetPolicy failed: hr = 0x%08x"), hr);
            return hr;
        }
        
        // want to distinguish between S_OK and MF_S_WAIT_FOR_POLICY_SET
        LOG(_T("IMFOutputTrustAuthority::SetPolicy returned 0x%08x"), hr);
        bAnySuccesses = true;
    }// for each output trust authority

    if (!bAnySuccesses) {
        ERR(_T("Did not successfully apply the policy to any output trust authorities."));
        return E_NOTFOUND;
    }

    // hold the IMFTrustedOutput to preserve the policy
    LOG(
        _T("Policy successfully applied.  Press any key to release IMFTrustedOutput...")
    );
#pragma prefast(suppress: __WARNING_RETVAL_IGNORED_FUNC_COULD_FAIL, "Not using the character anyway");
    getchar(); // intentionally ignoring return value
    
    return S_OK;
}
