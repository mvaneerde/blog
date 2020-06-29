// prefs.cpp

#include "common.h"

void usage(LPCWSTR exe) {
    LOG(
        L"%s [--help | -? | /?]\n"
        L"%s --processId <processId> --dumpType <dumpType> --fileName <fileName>\n"
        L"\n"
        L"See https://docs.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump",
        exe,
        exe
    );
}

Prefs::Prefs(int argc, LPCWSTR argv[], HRESULT& hr)
{
    hr = Initialize(argc, argv);
}

HRESULT Prefs::Initialize(int argc, LPCWSTR argv[])
{
    // check for usage statement
    if (
        (argc == 1) ||
        (
            (argc == 2) &&
            (
                _wcsicmp(argv[1], L"--help") == 0 ||
                _wcsicmp(argv[1], L"-?") == 0 ||
                _wcsicmp(argv[1], L"/?") == 0
            )
        )
    ) {
        usage(argv[0]);
        return S_FALSE;
    }

    bool seenProcessId = false;
    bool seenDumpType = false;
    bool seenFileName = false;
    for (int i = 1; i < argc; i++) {
        // --processId
        if (_wcsicmp(argv[i], L"--processId") == 0) {
            if (seenProcessId)
            {
                ERR(L"Multiple --processId arguments");
                return E_INVALIDARG;
            }

            if (++i == argc)
            {
                ERR(L"--processId is missing the process ID");
                return E_INVALIDARG;
            }

            PWCHAR end = nullptr;
            processId = wcstoul(argv[i], &end, 0);
            if (end == nullptr || *end != L'\0' || errno != 0) {
                ERR(L"Could not parse process ID %s", argv[i]);
                return E_INVALIDARG;
            }

            seenProcessId = true;
            continue;
        }

        // --dumpType
        if (_wcsicmp(argv[i], L"--dumpType") == 0) {
            if (seenDumpType)
            {
                ERR(L"Multiple --dumpType arguments");
                return E_INVALIDARG;
            }

            if (++i == argc)
            {
                ERR(L"--dumpType is missing the dump type");
                return E_INVALIDARG;
            }

            PWCHAR end = nullptr;
            dumpType = static_cast<MINIDUMP_TYPE>(wcstoul(argv[i], &end, 0));
            if (end == nullptr || *end != L'\0' || errno != 0) {
                ERR(L"Could not parse dump type %s", argv[i]);
                return E_INVALIDARG;
            }

            seenDumpType = true;
            continue;
        }

        // --fileName
        if (_wcsicmp(argv[i], L"--fileName") == 0) {
            if (seenFileName)
            {
                ERR(L"Multiple --fileName arguments");
                return E_INVALIDARG;
            }

            if (++i == argc)
            {
                ERR(L"--fileName is missing the file name");
                return E_INVALIDARG;
            }

            fileName = argv[i];

            seenFileName = true;
            continue;
        }

        ERR(L"Unexpected argument: %s", argv[i]);
        return E_INVALIDARG;
    }

    if (!seenProcessId)
    {
        ERR(L"--processId <processId> is required");
        return E_INVALIDARG;
    }

    if (!seenDumpType)
    {
        ERR(L"--dumpType <dumpType> is required");
        return E_INVALIDARG;
    }

    if (!seenFileName)
    {
        ERR(L"--fileName <fileName> is required");
        return E_INVALIDARG;
    }

    return S_OK;
}
