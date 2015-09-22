// main.cpp

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

void usage();

_Success_(return != false)
bool ReadRect(
    _Inout_ int *pArgc,
    _Deref_pre_count_(*pArgc)
    _Deref_pre_readonly_
    _Deref_post_count_(*pArgc) LPCWSTR **pArgv,
    _Out_ RECT *pRect
);

#define RECT_FORMAT L"(left = %d; top = %d; right = %d; bottom = %d)"
#define RECT_VALUES(r) r.left, r.top, r.right, r.bottom

int _cdecl wmain(int argc, _In_reads_(argc) LPCWSTR argv[]) {

    // consume program name
    argc--; argv++;

    if (argc <= 0) {
        usage();
        return 0;
    }
    
    if (0 == _wcsicmp(argv[0], L"union")) {
        argc--; argv++;
        
        RECT A = {0};
#pragma prefast(suppress: __WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED, "argc and argv are locked")
        if (!ReadRect(&argc, &argv, &A)) {
            return -__LINE__;
        }

        RECT B = {0};
#pragma prefast(suppress: __WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED, "argc and argv are locked")
        if (!ReadRect(&argc, &argv, &B)) {
            return -__LINE__;
        }
        
        if (0 != argc) {
            LOG(L"Unrecognized arguments starting with %s", argv[0]);
            return -__LINE__;
        }
        
        RECT C = {0};
        UnionRect(&C, &A, &B);
        
        LOG(
            L"      " RECT_FORMAT L"\n"
            L"union " RECT_FORMAT L"\n"
            L"    = " RECT_FORMAT,
            RECT_VALUES(A),
            RECT_VALUES(B),
            RECT_VALUES(C)
        );
    } else if (0 == _wcsicmp(argv[0], L"intersect")) {
        argc--; argv++;
        
        RECT A = {0};
#pragma prefast(suppress: __WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED, "argc and argv are locked")
        if (!ReadRect(&argc, &argv, &A)) {
            return -__LINE__;
        }

        RECT B = {0};
#pragma prefast(suppress: __WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED, "argc and argv are locked")
        if (!ReadRect(&argc, &argv, &B)) {
            return -__LINE__;
        }
        
        if (0 != argc) {
            LOG(L"Unrecognized arguments starting with %s", argv[0]);
            return -__LINE__;
        }

        RECT C = {0};
        IntersectRect(&C, &A, &B);
        
        LOG(
            L"          " RECT_FORMAT L"\n"
            L"intersect " RECT_FORMAT L"\n"
            L"        = " RECT_FORMAT,
            RECT_VALUES(A),
            RECT_VALUES(B),
            RECT_VALUES(C)
        );
    } else if (0 == _wcsicmp(argv[0], L"subtract")) {
        argc--; argv++;
        
        RECT A = {0};
#pragma prefast(suppress: __WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED, "argc and argv are locked")
        if (!ReadRect(&argc, &argv, &A)) {
            return -__LINE__;
        }

        RECT B = {0};
#pragma prefast(suppress: __WARNING_POTENTIAL_BUFFER_OVERFLOW_NULLTERMINATED, "argc and argv are locked")
        if (!ReadRect(&argc, &argv, &B)) {
            return -__LINE__;
        }
        
        if (0 != argc) {
            LOG(L"Unrecognized arguments starting with %s", argv[0]);
            return -__LINE__;
        }

        RECT C = {0};
        SubtractRect(&C, &A, &B);
        
        LOG(
            L"         " RECT_FORMAT L"\n"
            L"subtract " RECT_FORMAT L"\n"
            L"       = " RECT_FORMAT,
            RECT_VALUES(A),
            RECT_VALUES(B),
            RECT_VALUES(C)
        );
    } else {
        LOG(L"Unrecognized argument %s", argv[0]);
        return -__LINE__;
    }

    return 0;
}

void usage() {
    LOG(
        L"%s",
        L"rects.exe\n"
        L"    union     (left1 top1 right1 bottom1) (left2 top2 right2 bottom2) |\n"
        L"    intersect (left1 top1 right1 bottom1) (left2 top2 right2 bottom2) |\n"
        L"    subtract  (left1 top1 right1 bottom1) (left2 top2 right2 bottom2)"
    );
}

_Success_(return != false)
bool ReadRect(
    _Inout_ int *pArgc,
    _Deref_pre_readonly_
    _Deref_pre_count_(*pArgc)
    _Deref_post_count_(*pArgc) LPCWSTR **pArgv,
    _Out_ RECT *pRect
) {
    if (4 > *pArgc) {
        LOG(L"Need four coordinates for a rectangle; we only have %u", *pArgc);
        return false;
    }
    
    LPCWSTR szLeft = (*pArgv)[0];
    LPCWSTR szTop = (*pArgv)[1];
    LPCWSTR szRight = (*pArgv)[2];
    LPCWSTR szBottom = (*pArgv)[3];
    
    RECT rect = {};
    
    if (L'(' == szLeft[0]) {
        szLeft++;
    } else {
        LOG(L"Missing ( before left coordinate");
        return false;
    }

    WCHAR *end = nullptr;
    
    rect.left = wcstol(szLeft, &end, 0);
    if (0 != errno || nullptr == end || L'\0' != end[0]) {
        LOG(L"Could not parse %s as a number", szLeft);
        return false;
    }

    rect.top = wcstol(szTop, &end, 0);
    if (0 != errno || nullptr == end || L'\0' != end[0]) {
        LOG(L"Could not parse %s as a number", szTop);
        return false;
    }

    rect.right = wcstol(szRight, &end, 0);
    if (0 != errno || nullptr == end || L'\0' != end[0]) {
        LOG(L"Could not parse %s as a number", szRight);
        return false;
    }

    rect.bottom = wcstol(szBottom, &end, 0);
    if (0 != errno || nullptr == end || 0 != _wcsicmp(end, L")")) {
        LOG(L"%s should be a number followed by a )", szBottom);
        return false;
    }

    if (rect.right < rect.left) {
        LOG(L"Malformed rectangle: right (%d) < left (%d)", rect.right, rect.left);
        return false;
    }
    
    if (rect.bottom < rect.top) {
        LOG(L"Malformed rectangle: bottom (%d) < top (%d)", rect.bottom, rect.top);
        return false;
    }
    
    *pRect = rect;
    *pArgc -= 4;
    *pArgv += 4;
    
    return true;
}
