// log.h

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

void logsize(LPCWSTR prefix, UINT64 nBytes);
void logtime(LPCWSTR prefix);

extern DWORD start;