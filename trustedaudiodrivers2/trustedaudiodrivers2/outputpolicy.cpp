// outputpolicy.cpp

#include <windows.h>
#include <tchar.h>
#include <atlstr.h>
#include <mfidl.h>
#include <mfapi.h>

#include "log.h"
#include "stringify.h"
#include "outputschema.h"
#include "outputpolicy.h"

class CTrustedAudioDriversOutputPolicy : public IMFOutputPolicy {

friend
    HRESULT CreateTrustedAudioDriversOutputPolicy(
        DWORD dwConfigData,
        IMFOutputPolicy **ppMFOutputPolicy
    );

private:
    ULONG m_cRefCount;
    DWORD m_dwConfigData;
    GUID m_guidOriginator;
    IMFOutputSchema *m_pOutputSchema;
    IMFAttributes *m_pMFAttributes;
    
    CTrustedAudioDriversOutputPolicy(DWORD dwConfigData, HRESULT &hr);
    ~CTrustedAudioDriversOutputPolicy();

public:
    // IUnknown methods
    HRESULT STDMETHODCALLTYPE QueryInterface(
       /* [in] */ REFIID riid,
       /* [out] */ LPVOID *ppvObject
    );
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    
    // IMFOutputPolicy methods
    HRESULT STDMETHODCALLTYPE
        GenerateRequiredSchemas( 
            /* [in] */ DWORD dwAttributes,
            /* [in] */ GUID guidOutputSubType,
            /* [in] */ GUID *rgGuidProtectionSchemasSupported,
            /* [in] */ DWORD cProtectionSchemasSupported,
            /* [annotation][out] */ 
            __out  IMFCollection **ppRequiredProtectionSchemas
        );

    HRESULT STDMETHODCALLTYPE
        GetOriginatorID( 
            /* [annotation][out] */ 
            __out  GUID *pguidOriginatorID
        );

    HRESULT STDMETHODCALLTYPE
        GetMinimumGRLVersion( 
            /* [annotation][out] */ 
            __out  DWORD *pdwMinimumGRLVersion
        );
        
    // IMFAttributes methods
    HRESULT STDMETHODCALLTYPE GetItem( 
        __RPC__in REFGUID guidKey,
        /* [full][out][in] */ __RPC__inout_opt PROPVARIANT *pValue);
        
    HRESULT STDMETHODCALLTYPE GetItemType( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out MF_ATTRIBUTE_TYPE *pType);
        
    HRESULT STDMETHODCALLTYPE CompareItem( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFPROPVARIANT Value,
        /* [out] */ __RPC__out BOOL *pbResult);
        
    HRESULT STDMETHODCALLTYPE Compare( 
        __RPC__in_opt IMFAttributes *pTheirs,
        MF_ATTRIBUTES_MATCH_TYPE MatchType,
        /* [out] */ __RPC__out BOOL *pbResult);
    
    HRESULT STDMETHODCALLTYPE GetUINT32( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *punValue);
    
    HRESULT STDMETHODCALLTYPE GetUINT64( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT64 *punValue);
    
    HRESULT STDMETHODCALLTYPE GetDouble( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out double *pfValue);
    
    HRESULT STDMETHODCALLTYPE GetGUID( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out GUID *pguidValue);
    
    HRESULT STDMETHODCALLTYPE GetStringLength( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *pcchLength);
    
    HRESULT STDMETHODCALLTYPE GetString( 
        __RPC__in REFGUID guidKey,
        /* [size_is][out] */ __RPC__out_ecount_full(cchBufSize) LPWSTR pwszValue,
        UINT32 cchBufSize,
        /* [full][out][in] */ __RPC__inout_opt UINT32 *pcchLength);
    
    HRESULT STDMETHODCALLTYPE GetAllocatedString( 
        __RPC__in REFGUID guidKey,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(( *pcchLength + 1 ) ) LPWSTR *ppwszValue,
        /* [out] */ __RPC__out UINT32 *pcchLength);
    
    HRESULT STDMETHODCALLTYPE GetBlobSize( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *pcbBlobSize);
    
    HRESULT STDMETHODCALLTYPE GetBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][out] */ __RPC__out_ecount_full(cbBufSize) UINT8 *pBuf,
        UINT32 cbBufSize,
        /* [full][out][in] */ __RPC__inout_opt UINT32 *pcbBlobSize);
    
    HRESULT STDMETHODCALLTYPE GetAllocatedBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pcbSize) UINT8 **ppBuf,
        /* [out] */ __RPC__out UINT32 *pcbSize);
    
    HRESULT STDMETHODCALLTYPE GetUnknown( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out_opt LPVOID *ppv);
    
    HRESULT STDMETHODCALLTYPE SetItem( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFPROPVARIANT Value);
    
    HRESULT STDMETHODCALLTYPE DeleteItem( 
        __RPC__in REFGUID guidKey);
    
    HRESULT STDMETHODCALLTYPE DeleteAllItems( void);
    
    HRESULT STDMETHODCALLTYPE SetUINT32( 
        __RPC__in REFGUID guidKey,
        UINT32 unValue);
    
    HRESULT STDMETHODCALLTYPE SetUINT64( 
        __RPC__in REFGUID guidKey,
        UINT64 unValue);
    
    HRESULT STDMETHODCALLTYPE SetDouble( 
        __RPC__in REFGUID guidKey,
        double fValue);
    
    HRESULT STDMETHODCALLTYPE SetGUID( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFGUID guidValue);
    
    HRESULT STDMETHODCALLTYPE SetString( 
        __RPC__in REFGUID guidKey,
        /* [string][in] */ __RPC__in_string LPCWSTR wszValue);
    
    HRESULT STDMETHODCALLTYPE SetBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][in] */ __RPC__in_ecount_full(cbBufSize) const UINT8 *pBuf,
        UINT32 cbBufSize);
    
    HRESULT STDMETHODCALLTYPE SetUnknown( 
        __RPC__in REFGUID guidKey,
        /* [in] */ __RPC__in_opt IUnknown *pUnknown);
    
    HRESULT STDMETHODCALLTYPE LockStore( void);
    
    HRESULT STDMETHODCALLTYPE UnlockStore( void);
    
    HRESULT STDMETHODCALLTYPE GetCount( 
        /* [out] */ __RPC__out UINT32 *pcItems);
    
    HRESULT STDMETHODCALLTYPE GetItemByIndex( 
        UINT32 unIndex,
        /* [out] */ __RPC__out GUID *pguidKey,
        /* [full][out][in] */ __RPC__inout_opt PROPVARIANT *pValue);
    
    HRESULT STDMETHODCALLTYPE CopyAllItems( 
        /* [in] */ __RPC__in_opt IMFAttributes *pDest);    
}; // CTrustedAudioDriversOutputPolicy

HRESULT CreateTrustedAudioDriversOutputPolicy(
    DWORD dwConfigData,
    IMFOutputPolicy **ppMFOutputPolicy
) {

    if (NULL == ppMFOutputPolicy) {
        ERR(_T("Received null pointer in CreateTrustedAudioDriversOutputPolicy"));
        return E_POINTER;
    }

    *ppMFOutputPolicy = NULL;

    HRESULT hr = S_OK;
    CTrustedAudioDriversOutputPolicy *pPolicy =
        new CTrustedAudioDriversOutputPolicy(dwConfigData, hr);
    if (NULL == pPolicy) {
        ERR(_T("new CTrustedAudioDriversOutputPolicy returned a NULL pointer"));
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        ERR(_T("Creating a CTrustedAudioDriversOutputPolicy failed: hr = 0x%08x"), hr);
        delete pPolicy;
        return hr;
    }

    *ppMFOutputPolicy = static_cast<IMFOutputPolicy *>(pPolicy);
    
    return S_OK;
}// CreateTrustedAudioDriversOutputPolicy

// constructor
CTrustedAudioDriversOutputPolicy::CTrustedAudioDriversOutputPolicy(
    DWORD dwConfigData,
    HRESULT &hr
)
: m_cRefCount(1)
, m_dwConfigData(dwConfigData)
, m_pOutputSchema(NULL)
, m_pMFAttributes(NULL)
{
    hr = CoCreateGuid(&m_guidOriginator);
    if (FAILED(hr)) {
        ERR(_T("CoCreateGUID failed: hr = 0x%08x"), hr);
        return;
    }

    hr = CreateTrustedAudioDriversOutputSchema(
        dwConfigData, m_guidOriginator, &m_pOutputSchema
    );
    if (FAILED(hr)) {
        ERR(_T("CreateTrustedAudioDriversOutputSchema failed: hr = 0x%08x"), hr);
        return;
    }
    
    hr = MFCreateAttributes(&m_pMFAttributes, 0);
    if (FAILED(hr)) {
        ERR(_T("MFCreateAttributes failed: hr = 0x%08x"), hr);
        return;
    }
}

// destructor
CTrustedAudioDriversOutputPolicy::~CTrustedAudioDriversOutputPolicy()
{
    if (NULL != m_pOutputSchema) {
        m_pOutputSchema->Release();
    }
    
    if (NULL != m_pMFAttributes) {
        m_pMFAttributes->Release();
    }
}

#define RETURN_INTERFACE(T, iid, ppOut) \
    if (IsEqualIID(__uuidof(T), (iid))) { \
        this->AddRef(); \
        *(ppOut) = static_cast<T *>(this); \
        return S_OK; \
    } else {} (void)0

// IUnknown::QueryInterface
HRESULT STDMETHODCALLTYPE
    CTrustedAudioDriversOutputPolicy::QueryInterface(
        /* [in] */ REFIID riid,
        /* [out] */ LPVOID *ppvObject
) {

    if (NULL == ppvObject) {
        return E_POINTER;
    }

    *ppvObject = NULL;

    RETURN_INTERFACE(IUnknown, riid, ppvObject);
    RETURN_INTERFACE(IMFAttributes, riid, ppvObject);
    RETURN_INTERFACE(IMFOutputPolicy, riid, ppvObject);    

    return E_NOINTERFACE;
}

// IUnknown::AddRef
ULONG STDMETHODCALLTYPE
    CTrustedAudioDriversOutputPolicy::AddRef() {

    ULONG uNewRefCount = InterlockedIncrement(&m_cRefCount);

    return uNewRefCount;
}

// IUnknown::Release
ULONG STDMETHODCALLTYPE
    CTrustedAudioDriversOutputPolicy::Release() {

    ULONG uNewRefCount = InterlockedDecrement(&m_cRefCount);

    if (0 == uNewRefCount) {
        delete this;
    }

    return uNewRefCount;
}

// IMFOutputPolicy::GenerateRequiredSchemas
HRESULT STDMETHODCALLTYPE
    CTrustedAudioDriversOutputPolicy::GenerateRequiredSchemas( 
        /* [in] */ DWORD dwAttributes,
        /* [in] */ GUID guidOutputSubType,
        /* [in] */ GUID *rgGuidProtectionSchemasSupported,
        /* [in] */ DWORD cProtectionSchemasSupported,
        /* [annotation][out] */ 
        __out  IMFCollection **ppRequiredProtectionSchemas
) {

    // sanity checks
    if (NULL == ppRequiredProtectionSchemas) {
        ERR(_T("GenerateRequiredSchemas got a NULL output pointer"));
        
        return E_POINTER;
    }
    *ppRequiredProtectionSchemas = NULL;

    if (
        NULL == rgGuidProtectionSchemasSupported &&
        0 != cProtectionSchemasSupported
    ) {
        ERR(_T("Inconsistent schema support reported"));
        return E_POINTER;
    }
    
    HRESULT hr = S_OK;
    bool bTrustedAudioDriversSupported = false;

    TCHAR szGuid[39];
    StringFromGUID2(guidOutputSubType, szGuid, ARRAYSIZE(szGuid));
    LOG(
        _T("GenerateRequiredSchemas() called\n")
        _T("dwAttributes: %s (0x%08x)\n")
        _T("guidOutputSubType: %s (%s)\n")
        _T("cProtectionSchemasSupported: %d"),
        static_cast<LPCTSTR>(Attributes2String(dwAttributes)), dwAttributes,
        OutputSubType2String(guidOutputSubType), szGuid,
        cProtectionSchemasSupported
    );

    // log all the supported protection schemas
    for (DWORD i = 0; i < cProtectionSchemasSupported; i++) {
        StringFromGUID2(rgGuidProtectionSchemasSupported[i], szGuid, ARRAYSIZE(szGuid));
        LOG(
            _T("%s (%s)"),
            ProtectionSchema2String(rgGuidProtectionSchemasSupported[i]),
            szGuid
        );

        if (
            IsEqualIID(
                MFPROTECTION_TRUSTEDAUDIODRIVERS,
                rgGuidProtectionSchemasSupported[i]
            )
        ) {
            LOG(_T("MFPROTECTION_TRUSTEDAUDIODRIVERS supported."));
            bTrustedAudioDriversSupported = true;
        }
    }

    if (!bTrustedAudioDriversSupported) {
        ERR(
            _T("The Output Trust Authority does not support ")
            _T("MFPROTECTION_TRUSTEDAUDIODRIVERS!")
        );
        
        return HRESULT_FROM_WIN32(ERROR_RANGE_NOT_FOUND);
    }

    // if we've made it this far then the Output Trust Authority supports Trusted Audio Drivers
    // create a collection and put our output policy in it
    // then give that collection to the caller
    CComPtr<IMFCollection> pMFCollection;

    // create the collection
    hr = MFCreateCollection(&pMFCollection);
    if (FAILED(hr)) {
        ERR(_T("MFCreateCollection() failed: hr = 0x%08x"), hr);
        return hr;
    }

    // add our output policy to the collection
    hr = pMFCollection->AddElement(m_pOutputSchema);
    if (FAILED(hr)) {
        ERR(_T("IMFCollection::AddElement failed: hr = 0x%08x"), hr);
        return hr;
    }

    // give the collection to the caller
    return pMFCollection.CopyTo(ppRequiredProtectionSchemas); // increments refcount
}// GenerateRequiredSchemas

HRESULT STDMETHODCALLTYPE
    CTrustedAudioDriversOutputPolicy::GetOriginatorID( 
        /* [annotation][out] */ 
        __out  GUID *pguidOriginatorID
) {
    LOG(_T("GetOriginatorID() called"));

    if (NULL == pguidOriginatorID) {
        ERR(_T("GetOriginatorID received a NULL pointer"));
        return E_POINTER;
    }

    *pguidOriginatorID = m_guidOriginator;
    
    return S_OK;
}

HRESULT STDMETHODCALLTYPE
    CTrustedAudioDriversOutputPolicy::GetMinimumGRLVersion( 
        /* [annotation][out] */ 
        __out  DWORD *pdwMinimumGRLVersion
) {
    LOG(_T("GenerateMinimumGRLVersion() called"));

    if (NULL == pdwMinimumGRLVersion) {
        ERR(_T("GetMinimumGRLVersion received a NULL pointer"));
        return E_POINTER;
    }

    WARN(_T("Warning - don't know what minimum GRL to use. Guessing 0."));
    *pdwMinimumGRLVersion = 0;

    return S_OK;
}

// IMFAttributes methods

HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetItem( 
        __RPC__in REFGUID guidKey,
        /* [full][out][in] */ __RPC__inout_opt PROPVARIANT *pValue) {
    return m_pMFAttributes->GetItem(guidKey, pValue);
}
        
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetItemType( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out MF_ATTRIBUTE_TYPE *pType) {
    return m_pMFAttributes->GetItemType(guidKey, pType);
}
        
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::CompareItem( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFPROPVARIANT Value,
        /* [out] */ __RPC__out BOOL *pbResult) {
    return m_pMFAttributes->CompareItem(guidKey, Value, pbResult);
}
        
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::Compare( 
        __RPC__in_opt IMFAttributes *pTheirs,
        MF_ATTRIBUTES_MATCH_TYPE MatchType,
        /* [out] */ __RPC__out BOOL *pbResult) {
    return m_pMFAttributes->Compare(pTheirs, MatchType, pbResult);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetUINT32( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *punValue) {
    return m_pMFAttributes->GetUINT32(guidKey, punValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetUINT64( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT64 *punValue) {
    return m_pMFAttributes->GetUINT64(guidKey, punValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetDouble( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out double *pfValue) {
    return m_pMFAttributes->GetDouble(guidKey, pfValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetGUID( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out GUID *pguidValue) {
    return m_pMFAttributes->GetGUID(guidKey, pguidValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetStringLength( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *pcchLength) {
    return m_pMFAttributes->GetStringLength(guidKey, pcchLength);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetString( 
        __RPC__in REFGUID guidKey,
        /* [size_is][out] */ __RPC__out_ecount_full(cchBufSize) LPWSTR pwszValue,
        UINT32 cchBufSize,
        /* [full][out][in] */ __RPC__inout_opt UINT32 *pcchLength) {
    return m_pMFAttributes->GetString(guidKey, pwszValue, cchBufSize, pcchLength);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetAllocatedString( 
        __RPC__in REFGUID guidKey,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(( *pcchLength + 1 ) ) LPWSTR *ppwszValue,
        /* [out] */ __RPC__out UINT32 *pcchLength) {
    return m_pMFAttributes->GetAllocatedString(guidKey, ppwszValue, pcchLength);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetBlobSize( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *pcbBlobSize) {
    return m_pMFAttributes->GetBlobSize(guidKey, pcbBlobSize);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][out] */ __RPC__out_ecount_full(cbBufSize) UINT8 *pBuf,
        UINT32 cbBufSize,
        /* [full][out][in] */ __RPC__inout_opt UINT32 *pcbBlobSize) {
    return m_pMFAttributes->GetBlob(guidKey, pBuf, cbBufSize, pcbBlobSize);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetAllocatedBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pcbSize) UINT8 **ppBuf,
        /* [out] */ __RPC__out UINT32 *pcbSize) {
    return m_pMFAttributes->GetAllocatedBlob(guidKey, ppBuf, pcbSize);
}

HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetUnknown( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out_opt LPVOID *ppv) {
    return m_pMFAttributes->GetUnknown(guidKey, riid, ppv);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::SetItem( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFPROPVARIANT Value) {
    return m_pMFAttributes->SetItem(guidKey, Value);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::DeleteItem( 
        __RPC__in REFGUID guidKey) {
    return m_pMFAttributes->DeleteItem(guidKey);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::DeleteAllItems( void) {
    return m_pMFAttributes->DeleteAllItems();
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::SetUINT32( 
        __RPC__in REFGUID guidKey,
        UINT32 unValue) {
    return m_pMFAttributes->SetUINT32(guidKey, unValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::SetUINT64( 
        __RPC__in REFGUID guidKey,
        UINT64 unValue) {
    return m_pMFAttributes->SetUINT64(guidKey, unValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::SetDouble( 
        __RPC__in REFGUID guidKey,
        double fValue) {
    return m_pMFAttributes->SetDouble(guidKey, fValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::SetGUID( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFGUID guidValue) {
    return m_pMFAttributes->SetGUID(guidKey, guidValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::SetString( 
        __RPC__in REFGUID guidKey,
        /* [string][in] */ __RPC__in_string LPCWSTR wszValue) {
    return m_pMFAttributes->SetString(guidKey, wszValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::SetBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][in] */ __RPC__in_ecount_full(cbBufSize) const UINT8 *pBuf,
        UINT32 cbBufSize) {
    return m_pMFAttributes->SetBlob(guidKey, pBuf, cbBufSize);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::SetUnknown( 
        __RPC__in REFGUID guidKey,
        /* [in] */ __RPC__in_opt IUnknown *pUnknown) {
    return m_pMFAttributes->SetUnknown(guidKey, pUnknown);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::LockStore( void) {
    return m_pMFAttributes->LockStore();
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::UnlockStore( void) {
    return m_pMFAttributes->UnlockStore();
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetCount( 
        /* [out] */ __RPC__out UINT32 *pcItems) {
    return m_pMFAttributes->GetCount(pcItems);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::GetItemByIndex( 
        UINT32 unIndex,
        /* [out] */ __RPC__out GUID *pguidKey,
        /* [full][out][in] */ __RPC__inout_opt PROPVARIANT *pValue) {
    return m_pMFAttributes->GetItemByIndex(unIndex, pguidKey, pValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputPolicy::CopyAllItems( 
        /* [in] */ __RPC__in_opt IMFAttributes *pDest) {
    return m_pMFAttributes->CopyAllItems(pDest);
}
