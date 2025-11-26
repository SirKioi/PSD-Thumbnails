#pragma once

#include <windows.h>
#include <thumbcache.h>
#include <shlwapi.h>
#include <new>

// {C7B8E8A1-5F2D-4E3C-9A1B-2D3E4F5A6B7C}
static const GUID CLSID_PSDThumbnailProvider = 
{ 0xC7B8E8A1, 0x5F2D, 0x4E3C, { 0x9A, 0x1B, 0x2D, 0x3E, 0x4F, 0x5A, 0x6B, 0x7C } };

class PSDThumbnailProvider : 
    public IThumbnailProvider,
    public IInitializeWithStream
{
public:
    PSDThumbnailProvider();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IThumbnailProvider
    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream* pStream, DWORD grfMode);

private:
    ~PSDThumbnailProvider();

    long m_cRef;
    IStream* m_pStream;
};
