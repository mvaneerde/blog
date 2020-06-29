// dump.cpp

#include "common.h"

HRESULT dump(DWORD processId, MINIDUMP_TYPE dumpType, LPCWSTR fileName) {
    // open the output file
    HANDLE file = CreateFile(
        fileName,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (file == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        ERR(L"Creating file %s failed with error %d", fileName, err);
        return HRESULT_FROM_WIN32(err);
    }
    HandleCloser closeFile(file);

    // open the target process
    DWORD accessRights = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
    if ((dumpType & MiniDumpWithHandleData) == MiniDumpWithHandleData) {
        accessRights |= PROCESS_DUP_HANDLE;
    }
    HANDLE process = OpenProcess(
        accessRights,
        FALSE,
        processId
    );
    if (process == nullptr)
    {
        DWORD err = GetLastError();
        ERR(L"Opening process %d failed with error %d", processId, err);
        return HRESULT_FROM_WIN32(err);
    }
    HandleCloser closeProcess(process);
    // write the dump file
    if (!MiniDumpWriteDump(
        process,
        processId,
        file,
        dumpType,
        nullptr,
        nullptr,
        nullptr
    )) {
        DWORD err = GetLastError();
        ERR(L"MiniDumpWriteDump failed with error %d", err);
        return HRESULT_FROM_WIN32(err);
    }

    return S_OK;
}

