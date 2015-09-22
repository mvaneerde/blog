// main.cpp

#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <search.h>

int __cdecl shellsort(const void *a, const void *b);

struct line {
    line() : text(nullptr), next(nullptr) {}

    LPWSTR text;
    line *next;
};
void DeleteLines(line *p);

struct chunk {
    chunk() : next(nullptr) {}

    WCHAR text[1024];
    chunk *next;
};
void DeleteChunks(chunk *p);

line *ReadLines();

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) wprintf(L"Error: " format L"\n", __VA_ARGS__)

bool g_reverse = false;

int _cdecl wmain(int argc, LPCWSTR argv[]) {
    g_reverse = (argc == 2 && 0 == _wcsicmp(argv[1], L"-reverse"));

    line *first = ReadLines();
    LPWSTR *lines = nullptr;

    int count = 0;
    for (auto current = first; current != nullptr; current = current->next) {
        count++;
    }

    if (count == 0) {
        goto Cleanup;
    }

    lines = new LPWSTR[count];
    if (lines == nullptr) {
        ERR(L"Out of memory");
        goto Cleanup;
    }

    auto current = first;
    for (int i = 0; i < count; i++) {
        lines[i] = current->text;
        current = current->next;
    }

    qsort(lines, count, sizeof(lines[0]), shellsort);

    for (int i = 0; i < count; i++) {
        LOG(L"%s", lines[i]);
    }

Cleanup:
    DeleteLines(first);
    delete [] lines;

    return 0;
}

line *ReadLines() {
    line *firstLine = nullptr;
    line *currentLine = nullptr;
    chunk *firstChunk = nullptr;
    chunk *currentChunk = nullptr;
    size_t currentLen = 0;
    bool isEof = false;

    while (!isEof) {
        bool isEndOfLine = false;

        // allocate a chunk
        if (firstChunk == nullptr) {
            firstChunk = new chunk;
            currentChunk = firstChunk;
        } else {
            currentChunk->next = new chunk;
            currentChunk = currentChunk->next;
        }
        
        if (currentChunk == nullptr) {
            ERR(L"Out of memory");
            goto Error;
        }

        if (fgetws(currentChunk->text, ARRAYSIZE(currentChunk->text), stdin)) {
            // is this the end of a line?
            size_t len = wcslen(currentChunk->text);
            currentLen += len;
            if (len > 0 && currentChunk->text[len - 1] == L'\n') {
                // trim the newline
                currentChunk->text[len - 1] = L'\0';
                currentLen--;

                isEndOfLine = true;
            }
        } else {
            int error = ferror(stdin);
            if (error != 0) {
                ERR(L"STDIN error: %d", error);
                goto Error;
            } else if (feof(stdin)) {
                isEof = true;
            } else {
                ERR(L"fgetws returned null but STDIN is neither EOF nor in error");
                goto Error;
            }
        }

        if (currentLen > 0 && (isEndOfLine || isEof)) {
            // allocate a line
            if (firstLine == nullptr) {
                firstLine = new line;
                currentLine = firstLine;
            } else {
                currentLine->next = new line;
                currentLine = currentLine->next;
            }

            if (currentLine == nullptr) {
                ERR(L"Out of memory");
                goto Error;
            }

            currentLen++; // add terminating null
            currentLine->text = new WCHAR[currentLen];
            if (currentLine->text == nullptr) {
                ERR(L"Out of memory");
                goto Error;
            }

            auto s = &currentLine->text[0];

            for (auto p = firstChunk; p != nullptr; p = p->next) {
                auto len = wcslen(p->text);
                auto err = wcscpy_s(s, currentLen, p->text);
                if (err) {
                    ERR(L"wcscpy_s failed: %u", err);
                    goto Error;
                }

                s += len;
                currentLen -= len;
            }

            currentLen--; // should be 0 now

            // free all the chunks we used to create this line
            DeleteChunks(firstChunk);
            firstChunk = nullptr;
        }
    } // while

    return firstLine;

Error:
    DeleteChunks(firstChunk);
    DeleteLines(firstLine);
    return nullptr;
}

void DeleteLines(line *p) {
    while (p != nullptr) {
        delete [] p->text;
        auto next = p->next;
        delete p;
        p = next;
    }
}

void DeleteChunks(chunk *p) {
    while (p != nullptr) {
        auto next = p->next;
        delete p;
        p = next;
    }
}

int __cdecl shellsort(const void *a, const void *b) {
    auto x = *reinterpret_cast<const LPCWSTR *>(g_reverse ? b : a);
    auto y = *reinterpret_cast<const LPCWSTR *>(g_reverse ? a : b);

    return StrCmpLogicalW(x, y);
}