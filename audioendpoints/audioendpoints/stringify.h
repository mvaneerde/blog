// stringify.h

LPCWSTR StringFromState(DWORD state);
LPCWSTR StringFromKey(PROPERTYKEY key, LPCWSTR fallback);
LPCWSTR StringFromWaveFormatTag(WORD wFormatTag);
LPCWSTR StringFromSubFormat(GUID SubFormat);