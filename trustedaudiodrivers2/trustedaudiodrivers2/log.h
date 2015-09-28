// log.h

#define LOG(fmt, ...)  _tprintf(fmt _T("\n"), __VA_ARGS__)
#define WARN(fmt, ...) _tprintf(_T("-- WARNING -- ") fmt _T("\n"), __VA_ARGS__)
#define ERR(fmt, ...)  _tprintf(_T("-- ERROR -- ") fmt _T("\n"), __VA_ARGS__)

