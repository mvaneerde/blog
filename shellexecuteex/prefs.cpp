#include "common.h"

Prefs::Prefs() {
    cbSize = sizeof(SHELLEXECUTEINFOW);
    fMask = SEE_MASK_DEFAULT; // TODO
    hwnd = GetConsoleWindow();
    lpVerb = nullptr;
    lpFile = nullptr; // required parameter
    lpParameters = nullptr;
    lpDirectory = nullptr;
    nShow = -1; // required parameter
    hInstApp = 0; // output
    lpIDList = nullptr; // TODO
    lpClass = nullptr; // TODO
    hkeyClass = nullptr; // TODO
    dwHotKey = 0; // TODO
    hIcon = nullptr; // TODO
    hMonitor = nullptr; // TODO
    hProcess = nullptr; // TODO
}

bool Prefs::Parse(int argc, LPCWSTR argv[], bool &run) {
    if (IsUsage(argc, argv)) {
        ShowUsage();
        run = false;
        return true;
    }

    // mask options
    // SEE_MASK_CLASSKEY TODO
    // SEE_MASK_IDLIST TODO
    // SEE_MASK_INVOKEIDLIST TODO
    // SEE_MASK_ICON TODO
    // SEE_MASK_HOTKEY TODO
    // SEE_MASK_HMONITOR TODO
    // SEE_MASK_FLAG_HINST_IS_SITE TODO

    bool seenClass = false;
    bool seenNoCloseProcess = false;
    bool seenConnectNetworkDrive = false;
    bool seenNoAsync = false;
    bool seenDoEnvironmentSubstitution = false;
    bool seenNoUi = false;
    bool seenUnicode = false;
    bool seenNoConsole = false;
    bool seenAsyncOk = false;
    bool seenNoQueryClassStore = false;
    bool seenNoZoneChecks = false;
    bool seenWaitForInputIdle = false;
    bool seenLogUsage = false;
    bool seenVerb = false;
    bool seenFile = false;
    bool seenParameters = false;
    bool seenDirectory = false;
    bool seenShow = false;

    for (int i = 1; i < argc; i++) {
        // --class-name
        if (0 == _wcsicmp(argv[i], L"--class-name")) {
            if (seenClass) {
                LOG(L"%s", L"Multiple class arguments passed");
                return false;
            }
            seenClass = true;

            i++;
            if (i == argc) {
                LOG(L"%s", L"--class requires a value");
                return false;
            }

            fMask |= SEE_MASK_CLASSNAME;
            lpClass = argv[i];
            continue;
        }

        // --no-close-process
        if (0 == _wcsicmp(argv[i], L"--no-close-process")) {
            if (seenNoCloseProcess) {
                LOG(L"%s", L"Multiple --no-close-process arguments passed");
                return false;
            }
            seenNoCloseProcess = true;

            fMask |= SEE_MASK_NOCLOSEPROCESS;
            continue;
        }

        // --connect-network-drive
        if (0 == _wcsicmp(argv[i], L"--connect-network-drive")) {
            if (seenConnectNetworkDrive) {
                LOG(L"%s", L"Multiple --connect-network-drive arguments passed");
                return false;
            }
            seenConnectNetworkDrive = true;

            fMask |= SEE_MASK_CONNECTNETDRV;
            continue;
        }

        // --no-async
        if (0 == _wcsicmp(argv[i], L"--no-async")) {
            if (seenNoAsync) {
                LOG(L"%s", L"Multiple --no-async arguments passed");
                return false;
            }
            seenNoAsync = true;

            fMask |= SEE_MASK_NOASYNC;
            continue;
        }

        // --do-environment-substitution
        if (0 == _wcsicmp(argv[i], L"--do-environment-substitution")) {
            if (seenDoEnvironmentSubstitution) {
                LOG(L"%s", L"Multiple --do-environment-substitution arguments passed");
                return false;
            }
            seenDoEnvironmentSubstitution = true;

            fMask |= SEE_MASK_DOENVSUBST;
            continue;
        }

        // --no-ui
        if (0 == _wcsicmp(argv[i], L"--no-ui")) {
            if (seenNoUi) {
                LOG(L"%s", L"Multiple --no-ui arguments passed");
                return false;
            }
            seenNoUi = true;

            fMask |= SEE_MASK_FLAG_NO_UI;
            continue;
        }

        // --unicode
        if (0 == _wcsicmp(argv[i], L"--unicode")) {
            if (seenUnicode) {
                LOG(L"%s", L"Multiple --unicode arguments passed");
                return false;
            }
            seenUnicode = true;

            fMask |= SEE_MASK_UNICODE;
            continue;
        }

        // --no-console
        if (0 == _wcsicmp(argv[i], L"--no-console")) {
            if (seenNoConsole) {
                LOG(L"%s", L"Multiple --no-console arguments passed");
                return false;
            }
            seenNoConsole = true;

            fMask |= SEE_MASK_NO_CONSOLE;
            continue;
        }

        // --async-ok
        if (0 == _wcsicmp(argv[i], L"--async-ok")) {
            if (seenAsyncOk) {
                LOG(L"%s", L"Multiple --async-ok arguments passed");
                return false;
            }
            seenAsyncOk = true;

            fMask |= SEE_MASK_ASYNCOK;
            continue;
        }

        // --no-zone-checks
        if (0 == _wcsicmp(argv[i], L"--no-zone-checks")) {
            if (seenNoZoneChecks) {
                LOG(L"%s", L"Multiple --no-zone-checks arguments passed");
                return false;
            }
            seenNoZoneChecks = true;

            fMask |= SEE_MASK_NOZONECHECKS;
            continue;
        }

        // --wait-for-input-idle
        if (0 == _wcsicmp(argv[i], L"--wait-for-input-idle")) {
            if (seenWaitForInputIdle) {
                LOG(L"%s", L"Multiple --wait-for-input-idle arguments passed");
                return false;
            }
            seenWaitForInputIdle = true;

            fMask |= SEE_MASK_WAITFORINPUTIDLE;
            continue;
        }

        // --log-usage
        if (0 == _wcsicmp(argv[i], L"--log-usage")) {
            if (seenLogUsage) {
                LOG(L"%s", L"Multiple --log-usage arguments passed");
                return false;
            }
            seenLogUsage = true;

            fMask |= SEE_MASK_FLAG_LOG_USAGE;
            continue;
        }

        // --no-query-class-store
        if (0 == _wcsicmp(argv[i], L"--no-query-class-store")) {
            if (seenNoQueryClassStore) {
                LOG(L"%s", L"Multiple --no-query-class-store arguments passed");
                return false;
            }
            seenNoQueryClassStore = true;

            fMask |= SEE_MASK_NOQUERYCLASSSTORE;
            continue;
        }

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

        // --directory
        if (0 == _wcsicmp(argv[i], L"--directory")) {
            if (seenDirectory) {
                LOG(L"%s", L"Multiple --directory arguments passed");
                return false;
            }
            seenDirectory = true;

            i++;
            if (i == argc) {
                LOG(L"%s", L"--directory requires a value");
                return false;
            }

            lpDirectory = argv[i];
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
    // no parameters
    if (argc == 1) {
        return true;
    }

    // one parameter, -? or equivalent
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
    LOG(L"%s", L"    [--verb <verb>]");
    LOG(L"%s", L"    --file <file>");
    LOG(L"%s", L"    [--parameters <parameters>]");
    LOG(L"%s", L"    [--directory <directory>]");
    LOG(L"%s", L"    [--show <show-options>]");
    LOG(L"%s", L"    [--class-name <class-name>]");
    LOG(L"%s", L"    [--no-close-process]");
    LOG(L"%s", L"    [--connect-network-drive]");
    LOG(L"%s", L"    [--no-async]");
    LOG(L"%s", L"    [--do-environment-substitution]");
    LOG(L"%s", L"    [--no-ui]");
    LOG(L"%s", L"    [--unicode]");
    LOG(L"%s", L"    [--no-console]");
    LOG(L"%s", L"    [--async-ok]");
    LOG(L"%s", L"    [--no-query-class-store]");
    LOG(L"%s", L"    [--no-zone-checks]");
    LOG(L"%s", L"    [--wait-for-input-idle]");
    LOG(L"%s", L"    [--log-usage]");

    LOG(L"%s", L"");
    LOG(L"%s", L"show-options:");
    for (int i = 0; i < _countof(showInts); i++) {
        LOG(L"    %s%s",
            showInts[i].name,
            (i == _countof(showInts) - 1 ? L"" : L" |"));
    }
}

void Prefs::LogResult(BOOL result) {
    LOG(L"ShellExecute %s", (result ? L"succeeded" : L"failed"));
    if (result) {
        LOG(L"hProcess: 0x%p", hProcess);
        if (hProcess != nullptr) {
            CloseHandle(hProcess);
        }
    } else {
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