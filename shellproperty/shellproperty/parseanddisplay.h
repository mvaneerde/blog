// parseanddisplay.h

HRESULT PropertyKeyFromName(LPCWSTR name, _Out_ PROPERTYKEY *key);
HRESULT PropertyNameFromKey(const PROPERTYKEY &key, _Out_ LPWSTR *ppName);
HRESULT InitPropertyValueFromArgs(
    int cArgs,
    _In_reads_(cArgs) LPCWSTR szArgs[],
    _Out_ PROPVARIANT *value,
    _Out_ int *pArgsConsumed
);
HRESULT PropertyStoreFromFilename(
    LPCWSTR name,
    GETPROPERTYSTOREFLAGS flags,
    _Out_ IPropertyStore **ppPropertyStore
);
HRESULT DisplayPropertyValue(const PROPERTYKEY &key, const PROPVARIANT &value);
