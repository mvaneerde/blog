// log.h

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) LOG(L"ERROR: " format, __VA_ARGS__)

#define GUID_FORMAT L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"
#define GUID_VALUES(g) \
    g.Data1, g.Data2, g.Data3, \
    g.Data4[0], g.Data4[1], g.Data4[2], g.Data4[3],  \
    g.Data4[4], g.Data4[5], g.Data4[6], g.Data4[7]

