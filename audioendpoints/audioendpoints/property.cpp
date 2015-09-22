// property.cpp
#include <windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <mmdeviceapi.h>
#include <devpkey.h>
#include <propkey.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>

#include "log.h"
#include "cleanup.h"
#include "stringify.h"
#include "property.h"

HRESULT LogValue(PROPERTYKEY key, LPCWSTR szKey, const PROPVARIANT &val);
HRESULT LogWaveFormat(LPCWAVEFORMATEX pWfx, UINT nBytes);
HRESULT LogBytes(const BYTE *pbBytes, UINT nBytes);

HRESULT LogProperty(PROPERTYKEY key, const PROPVARIANT &val) {
    WCHAR rKey[255] = {0};
    
    HRESULT hr = StringCchPrintf(
        rKey,
        ARRAYSIZE(rKey),
        L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x},%u",
        key.fmtid.Data1,
            key.fmtid.Data2,
            key.fmtid.Data3,
            key.fmtid.Data4[0], key.fmtid.Data4[1],
            key.fmtid.Data4[2], key.fmtid.Data4[3], key.fmtid.Data4[4], key.fmtid.Data4[5], key.fmtid.Data4[6], key.fmtid.Data4[7],
        key.pid
    );
    if (FAILED(hr)) {
        ERR(L"StringCchPrintf failed: hr = 0x%08x", hr);
        return hr;
    }    
    
    LPCWSTR szKey = StringFromKey(key, rKey);
    
    return LogValue(key, szKey, val);
}

HRESULT LogValue(PROPERTYKEY key, LPCWSTR szKey, const PROPVARIANT &val) {
    switch (val.vt) {
        case VT_BOOL:
            LOG(L"%s: VT_BOOL 0x%x", szKey, val.boolVal);
            return S_OK;
    
        case VT_CLSID:
            LOG(
                L"%s: VT_CLSID {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                szKey,
                    val.puuid->Data1,
                    val.puuid->Data2,
                    val.puuid->Data3,
                    val.puuid->Data4[0], val.puuid->Data4[1],
                    val.puuid->Data4[2], val.puuid->Data4[3], val.puuid->Data4[4], val.puuid->Data4[5], val.puuid->Data4[6], val.puuid->Data4[7]
            );
            return S_OK;
    
        case VT_UI4:
            LOG(L"%s: VT_UI4 %u", szKey, val.ulVal);
            return S_OK;
    
        case VT_UI8:
            LOG(L"%s: VT_UI8 %I64u", szKey, val.uhVal.QuadPart);
            return S_OK;

        case VT_LPWSTR:
            LOG(L"%s: VT_LPWSTR %s", szKey, val.pwszVal);
            return S_OK;
            
        case VT_BLOB:
            LOG(L"%s VT_BLOB of size %u", szKey, val.blob.cbSize);
            if (
                PKEY_AudioEngine_DeviceFormat == key ||
                PKEY_AudioEngine_OEMFormat == key
            ) {
                LPCWAVEFORMATEX pWfx = reinterpret_cast<LPCWAVEFORMATEX>(val.blob.pBlobData);
                return LogWaveFormat(pWfx, val.blob.cbSize);
            } else {
                return LogBytes(val.blob.pBlobData, val.blob.cbSize);
            }
            
        default:
            ERR(L"%s: Unrecognized vartype %u", szKey, val.vt);
            return E_NOTIMPL;
    }
}

HRESULT LogWaveFormat(LPCWAVEFORMATEX pWfx, UINT nBytes) {
    if (NULL == pWfx) {
        ERR(L"LogWaveFormat called with a NULL pointer");
        return E_POINTER;
    }
    
    if (nBytes < sizeof(PCMWAVEFORMAT)) {
        ERR(L"Wave format is only %u bytes, smaller even than a PCMWAVEFORMAT (%Iu bytes)", nBytes, sizeof(PCMWAVEFORMAT));
        return E_INVALIDARG;
    } else if (nBytes == sizeof(PCMWAVEFORMAT)) {
        const PCMWAVEFORMAT *pPcm = reinterpret_cast<const PCMWAVEFORMAT *>(pWfx);
        
        // PCMWAVEFORMAT must have a wFormatTag of WAVE_FORMAT_PCM
        if (WAVE_FORMAT_PCM != pPcm->wf.wFormatTag) {
            ERR(L"PCMWAVEFORMAT has invalid format tag %u (expected %u)", pPcm->wf.wFormatTag, WAVE_FORMAT_PCM);
            return E_INVALIDARG;
        }
        
        LOG(
            L"    PCMWAVEFORMAT\n"
            L"        wf.wFormatTag: %u (%s)\n"
            L"        wf.nChannels: %u\n"
            L"        wf.nSamplesPerSec: %u\n"
            L"        wf.nAvgBytesPerSec: %u\n"
            L"        wf.nBlockAlign: %u\n"
            L"        wBitsPerSample: %u",
            pPcm->wf.wFormatTag, StringFromWaveFormatTag(pPcm->wf.wFormatTag),
            pPcm->wf.nChannels,
            pPcm->wf.nSamplesPerSec,
            pPcm->wf.nAvgBytesPerSec,
            pPcm->wf.nBlockAlign,
            pPcm->wBitsPerSample
        );
        return S_OK;
    } else if (nBytes < sizeof(WAVEFORMATEX)) {
        ERR(
            L"Wave format is %u bytes, "
            L"bigger than a PCMWAVEFORMAT (%Iu bytes) "
            L"but smaller than a WAVEFORMATEX (%Iu bytes)",
            nBytes,
            sizeof(PCMWAVEFORMAT),
            sizeof(WAVEFORMATEX)
        );
        return E_INVALIDARG;
    } else if (nBytes != sizeof(WAVEFORMATEX) + pWfx->cbSize) {
        ERR(
            L"Wave format takes up %u bytes "
            L"but sizeof(WAVEFORMATEX) + pWfx->cbSize is %Iu bytes",
            nBytes,
            sizeof(WAVEFORMATEX) + pWfx->cbSize
        );
        return E_INVALIDARG;
    } else {
        // nBytes matches cbSize
        switch (pWfx->wFormatTag) {
           
            case WAVE_FORMAT_EXTENSIBLE:
                {
                    if (pWfx->cbSize < sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)) {
                        ERR(
                            L"cbSize for a WAVE_FORMAT_EXTENSIBLE format must be at least "
                            L"sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX) (%Iu), "
                            L"not %u",
                            sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX),
                            pWfx->cbSize
                        );
                        return E_INVALIDARG;
                    }
                
                    const WAVEFORMATEXTENSIBLE *pWfxExt = reinterpret_cast<const WAVEFORMATEXTENSIBLE *>(pWfx);
                    
                    LOG(
                        L"    WAVEFORMATEXTENSIBLE\n"
                        L"    Format (\n"
                        L"        wFormatTag: %u (%s)\n"
                        L"        nChannels: %u\n"
                        L"        nSamplesPerSec: %u\n"
                        L"        nAvgBytesPerSec: %u\n"
                        L"        nBlockAlign: %u\n"
                        L"        wBitsPerSample: %u\n"
                        L"        cbSize: %u\n"
                        L"    )\n"
                        L"    Samples (\n"
                        L"        wValidBitsPerSample / wSamplesPerBlock / wReserved: %u\n"
                        L"    )\n"
                        L"    dwChannelMask: 0x%x\n"
                        L"    SubFormat: {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X} (%s)",
                        pWfxExt->Format.wFormatTag, StringFromWaveFormatTag(pWfxExt->Format.wFormatTag),
                        pWfxExt->Format.nChannels,
                        pWfxExt->Format.nSamplesPerSec,
                        pWfxExt->Format.nAvgBytesPerSec,
                        pWfxExt->Format.nBlockAlign,
                        pWfxExt->Format.wBitsPerSample,
                        pWfxExt->Format.cbSize,
                        pWfxExt->Samples.wValidBitsPerSample,
                        pWfxExt->dwChannelMask,
                        pWfxExt->SubFormat.Data1,
                        pWfxExt->SubFormat.Data2,
                        pWfxExt->SubFormat.Data3,
                        pWfxExt->SubFormat.Data4[0],
                        pWfxExt->SubFormat.Data4[1],
                        pWfxExt->SubFormat.Data4[2],
                        pWfxExt->SubFormat.Data4[3],
                        pWfxExt->SubFormat.Data4[4],
                        pWfxExt->SubFormat.Data4[5],
                        pWfxExt->SubFormat.Data4[6],
                        pWfxExt->SubFormat.Data4[7],
                        StringFromSubFormat(pWfxExt->SubFormat)
                    );

                    if (pWfx->cbSize > sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)) {
                        LOG(L"    Trailing bytes:\n");
                        return LogBytes(
                            reinterpret_cast<const BYTE *>(pWfx) + sizeof(WAVEFORMATEXTENSIBLE),
                            pWfx->cbSize - (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
                        );
                    }
                    
                    return S_OK;
                }
                
            case WAVE_FORMAT_PCM:
                if (pWfx->cbSize > 0) {
                    ERR(L"cbSize for a WAVE_FORMAT_PCM format must be 0, not %u", pWfx->cbSize);
                    return E_INVALIDARG;
                }
                // intentionally fall through
            default:
                LOG(
                    L"    WAVEFORMATEX\n"
                    L"        wFormatTag: %u (%s)\n"
                    L"        nChannels: %u\n"
                    L"        nSamplesPerSec: %u\n"
                    L"        nAvgBytesPerSec: %u\n"
                    L"        nBlockAlign: %u\n"
                    L"        wBitsPerSample: %u\n"
                    L"        cbSize: %u",
                    pWfx->wFormatTag, StringFromWaveFormatTag(pWfx->wFormatTag),
                    pWfx->nChannels,
                    pWfx->nSamplesPerSec,
                    pWfx->nAvgBytesPerSec,
                    pWfx->nBlockAlign,
                    pWfx->wBitsPerSample,
                    pWfx->cbSize
                );
                
                if (pWfx->cbSize > 0) {
                    LOG(L"    Trailing bytes:");
                    return LogBytes(reinterpret_cast<const BYTE *>(pWfx) + sizeof(WAVEFORMATEX), pWfx->cbSize);
                }

                return S_OK;
        }
    }
}

HRESULT LogBytes(const BYTE *pbBytes, UINT nBytes) {
    UINT bytesLogged = 0;
    while (bytesLogged < nBytes) {
        switch (nBytes - bytesLogged) {
            case 1:
                LOG(
                    L"    %02x",
                    pbBytes[bytesLogged]
                );
                bytesLogged += 1;
                break;
            case 2:
                LOG(
                    L"    %02x %02x",
                    pbBytes[bytesLogged],
                    pbBytes[bytesLogged + 1]
                );
                bytesLogged += 2;
                break;
            case 3:
                LOG(
                    L"    %02x %02x %02x",
                    pbBytes[bytesLogged],
                    pbBytes[bytesLogged + 1],
                    pbBytes[bytesLogged + 2]
                );
                bytesLogged += 3;
                break;
            case 4:
                LOG(
                    L"    %02x %02x %02x %02x",
                    pbBytes[bytesLogged],
                    pbBytes[bytesLogged + 1],
                    pbBytes[bytesLogged + 2],
                    pbBytes[bytesLogged + 3]
                );
                bytesLogged += 4;
                break;
            case 5:
                LOG(
                    L"    %02x %02x %02x %02x  %02x",
                    pbBytes[bytesLogged],
                    pbBytes[bytesLogged + 1],
                    pbBytes[bytesLogged + 2],
                    pbBytes[bytesLogged + 3],
                    pbBytes[bytesLogged + 4]
                );
                bytesLogged += 5;
                break;
            case 6:
                LOG(
                    L"    %02x %02x %02x %02x  %02x %02x",
                    pbBytes[bytesLogged],
                    pbBytes[bytesLogged + 1],
                    pbBytes[bytesLogged + 2],
                    pbBytes[bytesLogged + 3],
                    pbBytes[bytesLogged + 4],
                    pbBytes[bytesLogged + 5]
                );
                bytesLogged += 6;
                break;
            case 7:
                LOG(
                    L"    %02x %02x %02x %02x  %02x %02x %02x",
                    pbBytes[bytesLogged],
                    pbBytes[bytesLogged + 1],
                    pbBytes[bytesLogged + 2],
                    pbBytes[bytesLogged + 3],
                    pbBytes[bytesLogged + 4],
                    pbBytes[bytesLogged + 5],
                    pbBytes[bytesLogged + 6]
                );
                bytesLogged += 7;
                break;
            default:
                LOG(
                    L"    %02x %02x %02x %02x  %02x %02x %02x %02x",
                    pbBytes[bytesLogged],
                    pbBytes[bytesLogged + 1],
                    pbBytes[bytesLogged + 2],
                    pbBytes[bytesLogged + 3],
                    pbBytes[bytesLogged + 4],
                    pbBytes[bytesLogged + 5],
                    pbBytes[bytesLogged + 6],
                    pbBytes[bytesLogged + 7]
                );
                bytesLogged += 8;
                break;
        }
    }
    
    return S_OK;
}
