// main.cpp

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)

HRESULT HertzFromScientificPitchNotation(LPCWSTR string, ULONG *pUlong);
HRESULT UlongFromString(LPCWSTR string, ULONG *pUlong);
HRESULT LongFromString(LPCWSTR string, LONG *pLong);

int _cdecl wmain(int argc, _In_reads_(argc) LPCWSTR argv[]) {

    // we want a positive even number of arguments
    // so argc - 1 must be positive and even
    if ((argc - 1 == 0) || ((argc - 1) % 2 != 0)) {
        LOG(
            L"%s frequency1 duration1 frequency2 duration2 ... frequency_n duration_n\n"
            L"Calls Beep(frequency1, duration1), then Beep(frequency2, duration2), etc.\n"
            L"frequency can be either in Hz\n"
            L"or in Scientific Pitch Notation (e.g. C4 = middle C = 256 Hz)\n"
            L"duration is in milliseconds",
            argv[0]
        );
        return -__LINE__;
    }
    
    for (int i = 1; i + 1 < argc; i += 2) {
    
        ULONG frequency;
        HRESULT hr = HertzFromScientificPitchNotation(argv[i], &frequency);
        if (FAILED(hr)) { return -__LINE__; }
    
        ULONG duration;
        hr = UlongFromString(argv[i + 1], &duration);
        if (FAILED(hr)) { return -__LINE__; }

        if (!Beep(frequency, duration)) {
            LOG(L"Beep(%u, %u) failed: GetLastError() = %u", frequency, duration, GetLastError());
            return -__LINE__;
        }
    }
    
    return 0;
}

HRESULT HertzFromScientificPitchNotation(LPCWSTR string, ULONG *pUlong) {
    LONG semitonesAway = 0;
    
    // first character should be one of A through G
    // we will accept lowercase too
    switch (string[0]) {
        case L'c':
        case L'C':
            // zero point is C4
            break;

        case L'd':
        case L'D':
            semitonesAway += 2;
            break;

        case L'e':
        case L'E':
            semitonesAway += 4;
            break;

        case L'f':
        case L'F':
            semitonesAway += 5;
            break;

        case L'g':
        case L'G':
            semitonesAway += 7;
            break;

        case L'a':
        case L'A':
            semitonesAway += 9;
            break;

        case L'b':
        case L'B':
            semitonesAway += 11;
            break;

        default:
            // Not scientific pitch notation
            // maybe a raw Hz measurement
            return UlongFromString(string, pUlong);
    }

    string++;
    // second character may be an accidental - sharp or flat
    // we will accept both Unicode sharps and flats
    // and the synonyms # and b (but not B)
    switch (string[0]) {
        case L'#':
#pragma warning( suppress: 4428 )
        case L'\u266f': // U+266f MUSIC SHARP SIGN
            semitonesAway++;
            string++;
            break;
            
        case L'b':
#pragma warning( suppress: 4428 )
        case L'\u266d': // MUSIC FLAT SIGN
            semitonesAway--;
            string++;
            break;
            
        default: break; // no-op
    }
    
    LONG octave;
    HRESULT hr = LongFromString(string, &octave);
    if (FAILED(hr)) {
        return hr;
    }
    
    double octaveRatio = 2.0;
    double semitoneRatio = pow(2.0, 1.0 / 12.0);
    
    LONG octavesAway = octave - 4; // zero point is C4

    if (12 == semitonesAway) {
        semitonesAway = 0;
        octavesAway++;
    }
    
    // LOG(L"Octaves away: %d, semitones away, %d", octavesAway, semitonesAway);
    double freq = 256.0;
    if (octavesAway) { freq *= pow(octaveRatio, octavesAway); }
    if (semitonesAway) { freq *= pow(semitoneRatio, semitonesAway); }
    // LOG(L"%lg Hz", freq);
    *pUlong = (ULONG)(freq + 0.5);
    
    return S_OK;
}

HRESULT UlongFromString(LPCWSTR string, ULONG *pUlong) {
    LPWSTR end = nullptr;
    *pUlong = wcstoul(string, &end, 0);
    if (0 != errno) {
        LOG(L"wcstoul(%s) set errno = %u", string, errno);
        return E_INVALIDARG;
    }
    if (nullptr == end) {
        LOG(L"wcstoul(%s) set end = nullptr", string);
        return E_INVALIDARG;
    }
    if (L'\0' != end[0]) {
        LOG(L"wcstoul(%s) stopped at %lc", string, end[0]);
        return E_INVALIDARG;
    }
    
    return S_OK;
}

HRESULT LongFromString(LPCWSTR string, LONG *pLong) {
    LPWSTR end = nullptr;
    *pLong = wcstol(string, &end, 0);
    if (0 != errno) {
        LOG(L"wcstol(%s) set errno = %u", string, errno);
        return E_INVALIDARG;
    }
    if (nullptr == end) {
        LOG(L"wcstol(%s) set end = nullptr", string);
        return E_INVALIDARG;
    }
    if (L'\0' != end[0]) {
        LOG(L"wcstol(%s) stopped at %lc", string, end[0]);
        return E_INVALIDARG;
    }
    
    return S_OK;
}
