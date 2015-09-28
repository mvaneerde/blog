// outputschema.cpp

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <atlstr.h>

#include "log.h"
#include "outputschema.h"

class CTrustedAudioDriversOutputSchema : public IMFOutputSchema {

friend
    HRESULT CreateTrustedAudioDriversOutputSchema(
        DWORD dwConfigData,
        GUID guidOriginatorID,
        IMFOutputSchema **ppMFOutputSchema
    );

private:
    CTrustedAudioDriversOutputSchema(DWORD dwConfigData, GUID guidOriginatorID, _Out_ HRESULT &hr);
    ~CTrustedAudioDriversOutputSchema();

    ULONG m_cRefCount;
    DWORD m_dwConfigData;
    GUID m_guidOriginatorID;
    IMFAttributes *m_pMFAttributes;
    
public:
    // IUnknown methods
    HRESULT STDMETHODCALLTYPE QueryInterface(
       /* [in] */ REFIID riid,
       /* [out] */ LPVOID *ppvObject
    );
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    // IMFOutputSchema methods
    HRESULT STDMETHODCALLTYPE GetConfigurationData(__out DWORD *pdwVal);
    HRESULT STDMETHODCALLTYPE GetOriginatorID(__out GUID *pguidOriginatorID);
    HRESULT STDMETHODCALLTYPE GetSchemaType(__out GUID *pguidSchemaType);

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
}; // CTrustedAudioDriversOutputSchema

HRESULT CreateTrustedAudioDriversOutputSchema(
    DWORD dwConfigData,
    GUID guidOriginatorID,
    IMFOutputSchema **ppMFOutputSchema
) {
    if (NULL == ppMFOutputSchema) {
        return E_POINTER;
    }

    *ppMFOutputSchema = NULL;

    HRESULT hr = E_FAIL;
    CTrustedAudioDriversOutputSchema *pSchema =
        new CTrustedAudioDriversOutputSchema(dwConfigData, guidOriginatorID, hr);
        
    if (NULL == pSchema) {
        ERR(_T("new CTrustedAudioDriversOutputSchema returned a NULL pointer"));
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        return hr;
    }

    *ppMFOutputSchema = static_cast<IMFOutputSchema *>(pSchema);

    return S_OK;
}// CreateTrustedAudioDriversOutputSchema

// constructor
CTrustedAudioDriversOutputSchema::CTrustedAudioDriversOutputSchema(
    DWORD dwConfigData, GUID guidOriginatorID, _Out_ HRESULT &hr
)
: m_cRefCount(1)
, m_dwConfigData(dwConfigData)
, m_guidOriginatorID(guidOriginatorID)
, m_pMFAttributes(NULL)
{
    hr = MFCreateAttributes(&m_pMFAttributes, 0);
    if (FAILED(hr)) {
        ERR(_T("MFCreateAttributes failed: hr = 0x%08x"), hr);
        return;
    }
}

// destructor
CTrustedAudioDriversOutputSchema::~CTrustedAudioDriversOutputSchema() {
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
    CTrustedAudioDriversOutputSchema::QueryInterface(
        /* [in] */ REFIID riid,
        /* [out] */ LPVOID *ppvObject
) {

    if (NULL == ppvObject) {
        return E_POINTER;
    }

    *ppvObject = NULL;

    RETURN_INTERFACE(IUnknown, riid, ppvObject);
    RETURN_INTERFACE(IMFAttributes, riid, ppvObject);
    RETURN_INTERFACE(IMFOutputSchema, riid, ppvObject);

    return E_NOINTERFACE;
}

// IUnknown::AddRef
ULONG STDMETHODCALLTYPE
    CTrustedAudioDriversOutputSchema::AddRef() {

    ULONG uNewRefCount = InterlockedIncrement(&m_cRefCount);

    return uNewRefCount;
}

// IUnknown::Release
ULONG STDMETHODCALLTYPE
    CTrustedAudioDriversOutputSchema::Release() {

    ULONG uNewRefCount = InterlockedDecrement(&m_cRefCount);

    if (0 == uNewRefCount) {
        delete this;
    }

    return uNewRefCount;
}

// IMFOutputSchema::GetConfigurationData
HRESULT STDMETHODCALLTYPE
    CTrustedAudioDriversOutputSchema::GetConfigurationData(__out DWORD *pdwVal) {

    LOG(_T("IMFOutputSchema::GetConfigurationData called"));

    if (NULL == pdwVal) { return E_POINTER; }

    *pdwVal = m_dwConfigData;
    
    return S_OK;
}

// IMFOutputSchema::GetOriginatorID
HRESULT STDMETHODCALLTYPE
    CTrustedAudioDriversOutputSchema::GetOriginatorID(__out GUID *pguidOriginatorID) {

    LOG(_T("IMFOutputSchema::GetOriginatorID called"));

    if (NULL == pguidOriginatorID) { return E_POINTER; }

    *pguidOriginatorID = m_guidOriginatorID;
    
    return S_OK;
}

// IMFOutputSchema::GetSchemaType
HRESULT STDMETHODCALLTYPE
    CTrustedAudioDriversOutputSchema::GetSchemaType(__out GUID *pguidSchemaType) {

    LOG(_T("IMFOutputSchema::GetSchemaType called"));

    if (NULL == pguidSchemaType) { return E_POINTER; }

    *pguidSchemaType = MFPROTECTION_TRUSTEDAUDIODRIVERS;
    
    return S_OK;
}

// IMFAttributes methods

HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetItem( 
        __RPC__in REFGUID guidKey,
        /* [full][out][in] */ __RPC__inout_opt PROPVARIANT *pValue) {
    return m_pMFAttributes->GetItem(guidKey, pValue);
}
        
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetItemType( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out MF_ATTRIBUTE_TYPE *pType) {
    return m_pMFAttributes->GetItemType(guidKey, pType);
}
        
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::CompareItem( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFPROPVARIANT Value,
        /* [out] */ __RPC__out BOOL *pbResult) {
    return m_pMFAttributes->CompareItem(guidKey, Value, pbResult);
}
        
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::Compare( 
        __RPC__in_opt IMFAttributes *pTheirs,
        MF_ATTRIBUTES_MATCH_TYPE MatchType,
        /* [out] */ __RPC__out BOOL *pbResult) {
    return m_pMFAttributes->Compare(pTheirs, MatchType, pbResult);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetUINT32( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *punValue) {
    return m_pMFAttributes->GetUINT32(guidKey, punValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetUINT64( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT64 *punValue) {
    return m_pMFAttributes->GetUINT64(guidKey, punValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetDouble( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out double *pfValue) {
    return m_pMFAttributes->GetDouble(guidKey, pfValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetGUID( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out GUID *pguidValue) {
    return m_pMFAttributes->GetGUID(guidKey, pguidValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetStringLength( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *pcchLength) {
    return m_pMFAttributes->GetStringLength(guidKey, pcchLength);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetString( 
        __RPC__in REFGUID guidKey,
        /* [size_is][out] */ __RPC__out_ecount_full(cchBufSize) LPWSTR pwszValue,
        UINT32 cchBufSize,
        /* [full][out][in] */ __RPC__inout_opt UINT32 *pcchLength) {
    return m_pMFAttributes->GetString(guidKey, pwszValue, cchBufSize, pcchLength);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetAllocatedString( 
        __RPC__in REFGUID guidKey,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(( *pcchLength + 1 ) ) LPWSTR *ppwszValue,
        /* [out] */ __RPC__out UINT32 *pcchLength) {
    return m_pMFAttributes->GetAllocatedString(guidKey, ppwszValue, pcchLength);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetBlobSize( 
        __RPC__in REFGUID guidKey,
        /* [out] */ __RPC__out UINT32 *pcbBlobSize) {
    return m_pMFAttributes->GetBlobSize(guidKey, pcbBlobSize);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][out] */ __RPC__out_ecount_full(cbBufSize) UINT8 *pBuf,
        UINT32 cbBufSize,
        /* [full][out][in] */ __RPC__inout_opt UINT32 *pcbBlobSize) {
    return m_pMFAttributes->GetBlob(guidKey, pBuf, cbBufSize, pcbBlobSize);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetAllocatedBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pcbSize) UINT8 **ppBuf,
        /* [out] */ __RPC__out UINT32 *pcbSize) {
    return m_pMFAttributes->GetAllocatedBlob(guidKey, ppBuf, pcbSize);
}

HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetUnknown( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out_opt LPVOID *ppv) {
    return m_pMFAttributes->GetUnknown(guidKey, riid, ppv);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::SetItem( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFPROPVARIANT Value) {
    return m_pMFAttributes->SetItem(guidKey, Value);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::DeleteItem( 
        __RPC__in REFGUID guidKey) {
    return m_pMFAttributes->DeleteItem(guidKey);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::DeleteAllItems( void) {
    return m_pMFAttributes->DeleteAllItems();
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::SetUINT32( 
        __RPC__in REFGUID guidKey,
        UINT32 unValue) {
    return m_pMFAttributes->SetUINT32(guidKey, unValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::SetUINT64( 
        __RPC__in REFGUID guidKey,
        UINT64 unValue) {
    return m_pMFAttributes->SetUINT64(guidKey, unValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::SetDouble( 
        __RPC__in REFGUID guidKey,
        double fValue) {
    return m_pMFAttributes->SetDouble(guidKey, fValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::SetGUID( 
        __RPC__in REFGUID guidKey,
        __RPC__in REFGUID guidValue) {
    return m_pMFAttributes->SetGUID(guidKey, guidValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::SetString( 
        __RPC__in REFGUID guidKey,
        /* [string][in] */ __RPC__in_string LPCWSTR wszValue) {
    return m_pMFAttributes->SetString(guidKey, wszValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::SetBlob( 
        __RPC__in REFGUID guidKey,
        /* [size_is][in] */ __RPC__in_ecount_full(cbBufSize) const UINT8 *pBuf,
        UINT32 cbBufSize) {
    return m_pMFAttributes->SetBlob(guidKey, pBuf, cbBufSize);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::SetUnknown( 
        __RPC__in REFGUID guidKey,
        /* [in] */ __RPC__in_opt IUnknown *pUnknown) {
    return m_pMFAttributes->SetUnknown(guidKey, pUnknown);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::LockStore( void) {
    return m_pMFAttributes->LockStore();
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::UnlockStore( void) {
    return m_pMFAttributes->UnlockStore();
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetCount( 
        /* [out] */ __RPC__out UINT32 *pcItems) {
    return m_pMFAttributes->GetCount(pcItems);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::GetItemByIndex( 
        UINT32 unIndex,
        /* [out] */ __RPC__out GUID *pguidKey,
        /* [full][out][in] */ __RPC__inout_opt PROPVARIANT *pValue) {
    return m_pMFAttributes->GetItemByIndex(unIndex, pguidKey, pValue);
}
    
HRESULT STDMETHODCALLTYPE CTrustedAudioDriversOutputSchema::CopyAllItems( 
        /* [in] */ __RPC__in_opt IMFAttributes *pDest) {
    return m_pMFAttributes->CopyAllItems(pDest);
}
