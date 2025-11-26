#pragma once

#include <windows.h>
#include <vector>

class PSDParser
{
public:
    static HBITMAP ExtractThumbnail(IStream* pStream, UINT cx);

private:
    struct PSDHeader {
        char signature[4];  // "8BPS"
        WORD version;       // Always 1
        BYTE reserved[6];   // Must be zero
        WORD channels;      // Number of channels (1-56)
        DWORD height;       // Height in pixels (1-30000)
        DWORD width;        // Width in pixels (1-30000)
        WORD depth;         // Bits per channel (1, 8, 16, or 32)
        WORD colorMode;     // Color mode
    };

    static DWORD ReadDWord(IStream* pStream);
    static WORD ReadWord(IStream* pStream);
    static bool ReadHeader(IStream* pStream, PSDHeader& header);
    static HBITMAP CreateBitmapFromRGBA(const BYTE* data, UINT width, UINT height, UINT targetSize);
    static HBITMAP CreateBitmapFromRGB(const BYTE* data, UINT width, UINT height, UINT targetSize);
};
