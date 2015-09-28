// log.h

#define LOG(sz, ...) wprintf(sz L"\n", __VA_ARGS__);
#define ERR(sz, ...) wprintf(L"ERROR: " sz L"\n", __VA_ARGS__);