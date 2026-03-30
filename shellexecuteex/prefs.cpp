#include "common.h"

Prefs::Prefs() {
    cbSize = sizeof(SHELLEXECUTEINFOW);
    fMask = SEE_MASK_DEFAULT;
    hwnd = GetConsoleWindow();
    lpVerb = nullptr;
    lpFile = nullptr; // required later
    lpParameters = nullptr;
    lpDirectory = nullptr;
    nShow = -1; // required later
    hInstApp = 0; // output
    lpIDList = nullptr;
    lpClass = nullptr;
    hkeyClass = nullptr;
    dwHotKey = 0;
    hIcon = nullptr;
    hMonitor = nullptr;
    hProcess = nullptr;
}

bool Prefs::Parse(int argc, LPCWSTR argv[], bool &run) {
    // TODO
    // fMask = SEE_MASK_DEFAULT;
    // lpDirectory = nullptr;
    // hInstApp = 0;
    // lpIDList = nullptr;
    // lpClass = nullptr;
    // hkeyClass = nullptr;
    // dwHotKey = 0;
    // hIcon = nullptr;
    // hMonitor = nullptr;
    // hProcess = nullptr;

    if (IsUsage(argc, argv)) {
        ShowUsage();
        run = false;
        return true;
    }

    bool seenFile = false;
    bool seenShow = false;
    bool seenVerb = false;
    bool seenParameters = false;

    for (int i = 1; i < argc; i++) {
        // --verb
        if (0 == _wcsicmp(argv[i], L"--verb")) {
            if (seenVerb) {
                LOG(L"%s", L"Multiple --verb arguments passed");
                return false;
            }
            seenVerb = true;

            i++;
            if (i == argc) {
                LOG(L"%s", L"--verb requires a value");
                return false;
            }

            lpVerb = argv[i];
            continue;
        }

        // --file
        if (0 == _wcsicmp(argv[i], L"--file")) {
            if (seenFile) {
                LOG(L"%s", L"Multiple --file arguments passed");
                return false;
            }
            seenFile = true;

            i++;
            if (i == argc) {
                LOG(L"%s", L"--file requires a value");
                return false;
            }

            lpFile = argv[i];
            continue;
        }

        // --parameters
        if (0 == _wcsicmp(argv[i], L"--parameters")) {
            if (seenParameters) {
                LOG(L"%s", L"Multiple --parameters arguments passed");
                return false;
            }
            seenParameters = true;

            i++;
            if (i == argc) {
                LOG(L"%s", L"--parameters requires a value");
                return false;
            }

            lpParameters = argv[i];
            continue;
        }

        // --show
        if (0 == _wcsicmp(argv[i], L"--show")) {
            if (seenShow) {
                LOG(L"%s", L"Multiple --show arguments passed");
                return false;
            }
            seenShow = true;

            i++;
            if (i == argc) {
                LOG(L"%s", L"--show requires a value");
                return false;
            }

            bool found = false;
            nShow = ShowInt_From_String(argv[i], found);
            if (!found) {
                LOG(L"%s", L"Unrecognized value for --show");
            }
            continue;
        }

        // any other argument
        LOG(L"Unrecognized argument %s", argv[i]);
        return false;
    }

    // file is required
    if (!seenFile) {
        LOG(L"%s", L"--file is required");
        return false;
    }

    // show is required
    if (!seenShow) {
        LOG(L"%s", L"--show is required");
        return false;
    }

    run = true;
    return true;
}

bool Prefs::IsUsage(int argc, LPCWSTR argv[]) {
    if (argc == 1) {
        return true;
    }

    if (argc == 2) {
        LPCWSTR help[] = {
            L"-?",
            L"/?",
            L"--help"
        };

        for (int i = 0; i < _countof(help); i++) {
            if (0 == _wcsicmp(argv[1], help[i])) {
                return true;
            }
        }
    }

    return false;
}

void Prefs::ShowUsage() {
    LOG(L"%s", L"shellexecute.exe [-? | /? | --help]");
    LOG(L"%s", L"    show usage");
    LOG(L"%s", L"");
    LOG(L"%s", L"shellexecute.exe");
    LOG(L"%s", L"    --mask [mask-options]");
    LOG(L"%s", L"    [--verb <verb>]");
    LOG(L"%s", L"    --file <file>");
    LOG(L"%s", L"    [--parameters <parameters>]");
    LOG(L"%s", L"    [--directory <directory>]");
    LOG(L"%s", L"    [--show <show-options>]");
    LOG(L"%s", L"");
    LOG(L"%s", L"mask-options: TODO");
    LOG(L"%s", L"show-options:");
    for (int i = 0; i < _countof(showInts); i++) {
        LOG(L"    %s%s",
            showInts[i].name,
            (i == _countof(showInts) - 1 ? L"" : L" |"));
    }
}

void Prefs::LogResult(BOOL result) {
    LOG(L"ShellExecute %s", (result ? L"succeeded" : L"failed"));
    if (!result) {
        DWORD error = GetLastError();
        LOG(L"Last error: %d", error);
    }
    LOG(L"hInstApp: 0x%p", hInstApp);
}

int Prefs::ShowInt_From_String(LPCWSTR s, bool &found) {
    for (int i = 0; i < _countof(showInts); i++) {
        if (0 == _wcsicmp(s, showInts[i].name)) {
            found = true;
            return showInts[i].value;
        }
    }

    found = false;
    return -1;
}

Prefs::ShowInt_Mapping::ShowInt_Mapping(LPCWSTR n, int v)
    : name(n), value(v) {
    }


#define SHOWINT_MAPPING_ENTRY(i) Prefs::ShowInt_Mapping(L ## #i, i)

Prefs::ShowInt_Mapping Prefs::showInts[14] = {
    SHOWINT_MAPPING_ENTRY(SW_HIDE),

    // these are equal
    SHOWINT_MAPPING_ENTRY(SW_SHOWNORMAL),
    SHOWINT_MAPPING_ENTRY(SW_NORMAL),

    SHOWINT_MAPPING_ENTRY(SW_SHOWMINIMIZED),

    // these are equal
    SHOWINT_MAPPING_ENTRY(SW_SHOWMAXIMIZED),
    SHOWINT_MAPPING_ENTRY(SW_MAXIMIZE),

    SHOWINT_MAPPING_ENTRY(SW_SHOWNOACTIVATE),
    SHOWINT_MAPPING_ENTRY(SW_SHOW),
    SHOWINT_MAPPING_ENTRY(SW_MINIMIZE),
    SHOWINT_MAPPING_ENTRY(SW_SHOWMINNOACTIVE),
    SHOWINT_MAPPING_ENTRY(SW_SHOWNA),
    SHOWINT_MAPPING_ENTRY(SW_RESTORE),
    SHOWINT_MAPPING_ENTRY(SW_SHOWDEFAULT),
    SHOWINT_MAPPING_ENTRY(SW_FORCEMINIMIZE),
};