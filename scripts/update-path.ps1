Param(
    [string]$Directory
);

# NOTE: this script updates the persistent user PATH (HKCU\Environment\Path) in
# the registry and broadcasts WM_SETTINGCHANGE so newly launched shells pick up
# the change. It does not alter an already-running shell's live environment; open
# a new shell after running it. Invoke it with PowerShell (the tool must preserve
# a %VAR% argument literally, which a cmd/batch wrapper cannot do because cmd
# expands environment variables in arguments before any script runs).
#
# We manipulate HKCU\Environment\Path directly, as a single REG_EXPAND_SZ value
# holding every entry (installer-managed defaults plus the directories we add).
# Keeping it REG_EXPAND_SZ is essential so any %VAR% in an entry (e.g.
# %LOCALAPPDATA%\Microsoft\WindowsApps) re-expands at process launch. All entries
# live in one value expanded in a single pass, so there is no cross-variable
# reference and no registry-ordering fragility. setx / .NET SetEnvironmentVariable
# both write REG_SZ, so we write with RegistryValueKind ExpandString and broadcast
# WM_SETTINGCHANGE ourselves so new shells pick the change up.
#
# The directory argument may itself contain environment variables (e.g.
# %USERPROFILE%\path). We validate the EXPANDED path exists but store the
# argument UNEXPANDED so the %VAR% survives in the registry. A directory already
# present (after expansion) in either the user (HKCU) or machine (HKLM) PATH is a
# no-op.

$ErrorActionPreference = "Stop";

$target = "";
If ($Directory) {
    $target = $Directory.Trim();
}

If ([string]::IsNullOrWhiteSpace($target)) {
    [Console]::Error.WriteLine("update-path requires a path");
    Exit 1;
}

# validate existence against the EXPANDED path (so an env-var'd argument like
# %USERPROFILE%\foo is checked correctly); we still store it unexpanded below
$expandedTarget = [Environment]::ExpandEnvironmentVariables($target);
If (-not (Test-Path -LiteralPath $expandedTarget)) {
    [Console]::Error.WriteLine("Non-existent path: $target");
    Exit 1;
}

$environmentKeyPath = "HKEY_CURRENT_USER\Environment";
$key = [Microsoft.Win32.Registry]::CurrentUser.OpenSubKey("Environment", $false);
$doNotExpand = [Microsoft.Win32.RegistryValueOptions]::DoNotExpandEnvironmentNames;

$existingHkcuPath = $null;
If ($key) {
    $existingHkcuPath = $key.GetValue("Path", $null, $doNotExpand);
    $key.Close();
}

# also read the machine (HKLM) Path so we can treat a directory already on the
# system PATH as a no-op; we never write here (would need elevation)
$existingHklmPath = $null;
$hklmKey = [Microsoft.Win32.Registry]::LocalMachine.OpenSubKey(
    "SYSTEM\CurrentControlSet\Control\Session Manager\Environment", $false);
If ($hklmKey) {
    $existingHklmPath = $hklmKey.GetValue("Path", $null, $doNotExpand);
    $hklmKey.Close();
}

# treat a ";"-delimited value as a set of entries, ignoring empty segments and
# leading/trailing whitespace; returns $true if $needle is present as a whole
# entry (case-insensitively), NOT merely as a substring of some other entry.
# Entries and the needle are compared AFTER environment-variable expansion so an
# env-var'd entry like %USERPROFILE%\path matches an absolute C:\Users\...\path.
Function Test-PathEntry([string]$list, [string]$needle) {
    If ([string]::IsNullOrEmpty($list)) {
        Return $false;
    }
    $needleExpanded = [Environment]::ExpandEnvironmentVariables($needle).Trim();
    ForEach ($entry in $list.Split(";")) {
        $entryExpanded = [Environment]::ExpandEnvironmentVariables($entry).Trim();
        If ($entryExpanded -ieq $needleExpanded) {
            Return $true;
        }
    }
    Return $false;
}

# a directory already present (after expansion) in EITHER the user or machine
# PATH is a no-op; exit non-zero so callers can tell nothing was added
If ((Test-PathEntry $existingHkcuPath $target) -or (Test-PathEntry $existingHklmPath $target)) {
    Exit 1;
}

# store $target UNEXPANDED so any %VAR% in it is preserved in the registry and
# re-expands at process launch
If ([string]::IsNullOrWhiteSpace($existingHkcuPath)) {
    $newHkcuPath = $target;
} Else {
    $newHkcuPath = "$existingHkcuPath;$target";
}

# persist Path (REG_EXPAND_SZ)
[Microsoft.Win32.Registry]::SetValue(
    $environmentKeyPath,
    "Path",
    $newHkcuPath,
    [Microsoft.Win32.RegistryValueKind]::ExpandString
);

# broadcast WM_SETTINGCHANGE so already-running shells that ask for it (and any
# newly spawned ones) pick up the change; SetValue alone does not notify anyone
If (-not ([System.Management.Automation.PSTypeName]"Win32.NativeMethods").Type) {
    Add-Type -Namespace Win32 -Name NativeMethods -MemberDefinition @"
[System.Runtime.InteropServices.DllImport("user32.dll", SetLastError = true, CharSet = System.Runtime.InteropServices.CharSet.Auto)]
public static extern System.IntPtr SendMessageTimeout(System.IntPtr hWnd, uint Msg, System.IntPtr wParam, string lParam, uint fuFlags, uint uTimeout, out System.UIntPtr lpdwResult);
"@;
}
$HWND_BROADCAST = [System.IntPtr]0xffff;
$WM_SETTINGCHANGE = 0x1a;
$SMTO_ABORTIFHUNG = 0x2;
$result = [System.UIntPtr]::Zero;
[void][Win32.NativeMethods]::SendMessageTimeout(
    $HWND_BROADCAST, $WM_SETTINGCHANGE, [System.IntPtr]::Zero, "Environment",
    $SMTO_ABORTIFHUNG, 5000, [ref]$result
);

# exit 0 = we added $target; non-zero = no-op (already present) or error
Exit 0;
