#include "common.h"

Prefs::Prefs() {
    cbSize = sizeof(SHELLEXECUTEINFOW);
    fMask = SEE_MASK_DEFAULT;
    hwnd = GetConsoleWindow();
    lpVerb = nullptr;
    lpFile = nullptr;
    lpParameters = nullptr;
    lpDirectory = nullptr;
    nShow = -1;
    hInstApp = 0;
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
    // lpVerb = nullptr;
    // lpParameters = nullptr;
    // lpDirectory = nullptr;
    // nShow = -1;
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

    for (int i = 1; i < argc; i++) {
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
    LOG(L"%s", L"show-options: TODO");
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
    for (int i = 0; i < _countof(Prefs::showInts); i++) {
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


Prefs::ShowInt_Mapping Prefs::showInts[4] = {
    Prefs::ShowInt_Mapping(L"SW_HIDE", SW_HIDE),

    // these are equal
    Prefs::ShowInt_Mapping(L"SW_SHOWNORMAL", SW_SHOWNORMAL),
    Prefs::ShowInt_Mapping(L"SW_NORMAL", SW_NORMAL),

    Prefs::ShowInt_Mapping(L"SW_SHOWMINIMIZED", SW_SHOWMINIMIZED),
};