#include "PSDThumbnailProvider.h"
#include "PSDParser.h"
#include <shlwapi.h>

extern long g_cDllRef;

PSDThumbnailProvider::PSDThumbnailProvider() : m_cRef(1), m_pStream(nullptr)
{
    InterlockedIncrement(&g_cDllRef);
}

PSDThumbnailProvider::~PSDThumbnailProvider()
{
    if (m_pStream)
    {
        m_pStream->Release();
        m_pStream = nullptr;
    }
    InterlockedDecrement(&g_cDllRef);
}

#pragma region IUnknown

IFACEMETHODIMP PSDThumbnailProvider::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(PSDThumbnailProvider, IThumbnailProvider),
        QITABENT(PSDThumbnailProvider, IInitializeWithStream),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) PSDThumbnailProvider::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) PSDThumbnailProvider::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

#pragma endregion

#pragma region IInitializeWithStream

IFACEMETHODIMP PSDThumbnailProvider::Initialize(IStream* pStream, DWORD grfMode)
{
    HRESULT hr = E_UNEXPECTED;
    if (m_pStream == nullptr)
    {
        m_pStream = pStream;
        m_pStream->AddRef();
        hr = S_OK;
    }
    return hr;
}

#pragma endregion

#pragma region IThumbnailProvider

IFACEMETHODIMP PSDThumbnailProvider::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
    HRESULT hr = E_FAIL;

    if (m_pStream == nullptr)
    {
        return E_UNEXPECTED;
    }

    // Reset stream to beginning
    LARGE_INTEGER li = {};
    hr = m_pStream->Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr))
    {
        return hr;
    }

    // Extract thumbnail from PSD
    HBITMAP hbmp = PSDParser::ExtractThumbnail(m_pStream, cx);
    if (hbmp != nullptr)
    {
        *phbmp = hbmp;
        *pdwAlpha = WTSAT_ARGB;
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

#pragma endregion
