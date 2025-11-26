#include "PSDParser.h"
#include <vector>
#include <algorithm>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Helper to swap bytes for big-endian to little-endian conversion
inline DWORD SwapDWord(DWORD val)
{
    return ((val & 0xFF000000) >> 24) |
           ((val & 0x00FF0000) >> 8) |
           ((val & 0x0000FF00) << 8) |
           ((val & 0x000000FF) << 24);
}

inline WORD SwapWord(WORD val)
{
    return ((val & 0xFF00) >> 8) |
           ((val & 0x00FF) << 8);
}

DWORD PSDParser::ReadDWord(IStream* pStream)
{
    DWORD value = 0;
    ULONG bytesRead = 0;
    pStream->Read(&value, sizeof(DWORD), &bytesRead);
    return SwapDWord(value);
}

WORD PSDParser::ReadWord(IStream* pStream)
{
    WORD value = 0;
    ULONG bytesRead = 0;
    pStream->Read(&value, sizeof(WORD), &bytesRead);
    return SwapWord(value);
}

bool PSDParser::ReadHeader(IStream* pStream, PSDHeader& header)
{
    ULONG bytesRead = 0;
    
    // Read signature
    pStream->Read(header.signature, 4, &bytesRead);
    if (bytesRead != 4 || memcmp(header.signature, "8BPS", 4) != 0)
        return false;

    header.version = ReadWord(pStream);
    pStream->Read(header.reserved, 6, &bytesRead);
    header.channels = ReadWord(pStream);
    header.height = ReadDWord(pStream);
    header.width = ReadDWord(pStream);
    header.depth = ReadWord(pStream);
    header.colorMode = ReadWord(pStream);

    return header.version == 1;
}

HBITMAP PSDParser::CreateBitmapFromRGBA(const BYTE* data, UINT width, UINT height, UINT targetSize)
{
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Create bitmap from raw RGBA data
    Bitmap* srcBitmap = new Bitmap(width, height, width * 4, PixelFormat32bppARGB, (BYTE*)data);
    
    // Calculate thumbnail dimensions maintaining aspect ratio
    UINT thumbWidth, thumbHeight;
    if (width > height)
    {
        thumbWidth = targetSize;
        thumbHeight = (UINT)((float)height / width * targetSize);
    }
    else
    {
        thumbHeight = targetSize;
        thumbWidth = (UINT)((float)width / height * targetSize);
    }

    // Create thumbnail bitmap
    Bitmap* thumbBitmap = new Bitmap(thumbWidth, thumbHeight, PixelFormat32bppARGB);
    Graphics* graphics = new Graphics(thumbBitmap);
    graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics->DrawImage(srcBitmap, 0, 0, thumbWidth, thumbHeight);

    HBITMAP hBitmap;
    thumbBitmap->GetHBITMAP(Color(0, 0, 0, 0), &hBitmap);

    delete graphics;
    delete thumbBitmap;
    delete srcBitmap;
    
    GdiplusShutdown(gdiplusToken);

    return hBitmap;
}

HBITMAP PSDParser::CreateBitmapFromRGB(const BYTE* data, UINT width, UINT height, UINT targetSize)
{
    // Convert RGB to RGBA
    std::vector<BYTE> rgbaData(width * height * 4);
    for (UINT i = 0; i < width * height; i++)
    {
        rgbaData[i * 4 + 0] = data[i * 3 + 2]; // B
        rgbaData[i * 4 + 1] = data[i * 3 + 1]; // G
        rgbaData[i * 4 + 2] = data[i * 3 + 0]; // R
        rgbaData[i * 4 + 3] = 255;              // A
    }
    
    return CreateBitmapFromRGBA(rgbaData.data(), width, height, targetSize);
}

HBITMAP PSDParser::ExtractThumbnail(IStream* pStream, UINT cx)
{
    PSDHeader header;
    if (!ReadHeader(pStream, header))
        return nullptr;

    // Skip Color Mode Data section
    DWORD colorModeLength = ReadDWord(pStream);
    LARGE_INTEGER li;
    li.QuadPart = colorModeLength;
    pStream->Seek(li, STREAM_SEEK_CUR, nullptr);

    // Read Image Resources section
    DWORD imageResourcesLength = ReadDWord(pStream);
    LARGE_INTEGER startPos;
    pStream->Seek({}, STREAM_SEEK_CUR, (ULARGE_INTEGER*)&startPos);

    // Look for thumbnail resource (resource ID 1033 or 1036)
    DWORD bytesRead = 0;
    while (bytesRead < imageResourcesLength)
    {
        char signature[4];
        ULONG read;
        pStream->Read(signature, 4, &read);
        if (read != 4 || memcmp(signature, "8BIM", 4) != 0)
            break;

        WORD resourceId = ReadWord(pStream);
        
        // Read Pascal string (name)
        BYTE nameLength;
        pStream->Read(&nameLength, 1, &read);
        if (nameLength > 0)
        {
            li.QuadPart = nameLength;
            pStream->Seek(li, STREAM_SEEK_CUR, nullptr);
        }
        
        // Align to even byte boundary
        if ((nameLength + 1) % 2 != 0)
        {
            li.QuadPart = 1;
            pStream->Seek(li, STREAM_SEEK_CUR, nullptr);
        }

        DWORD resourceSize = ReadDWord(pStream);
        
        // Check if this is a thumbnail resource
        if (resourceId == 1033 || resourceId == 1036) // JPEG or raw RGB thumbnail
        {
            if (resourceId == 1036) // Raw RGB thumbnail
            {
                DWORD format = ReadDWord(pStream);
                DWORD width = ReadDWord(pStream);
                DWORD height = ReadDWord(pStream);
                DWORD widthBytes = ReadDWord(pStream);
                DWORD totalSize = ReadDWord(pStream);
                DWORD sizeAfterCompression = ReadDWord(pStream);
                WORD bitsPerPixel = ReadWord(pStream);
                WORD planes = ReadWord(pStream);

                // Read RGB data
                DWORD dataSize = resourceSize - 28; // Subtract header size
                std::vector<BYTE> rgbData(dataSize);
                pStream->Read(rgbData.data(), dataSize, &read);

                if (format == 1 && read == dataSize) // Raw RGB format
                {
                    return CreateBitmapFromRGB(rgbData.data(), width, height, cx);
                }
            }
            else if (resourceId == 1033) // JPEG thumbnail
            {
                // Skip JPEG thumbnail for now, use resource 1036 if available
                li.QuadPart = resourceSize;
                pStream->Seek(li, STREAM_SEEK_CUR, nullptr);
            }
        }
        else
        {
            // Skip this resource
            li.QuadPart = resourceSize;
            pStream->Seek(li, STREAM_SEEK_CUR, nullptr);
        }

        // Align to even byte boundary
        if (resourceSize % 2 != 0)
        {
            li.QuadPart = 1;
            pStream->Seek(li, STREAM_SEEK_CUR, nullptr);
        }

        LARGE_INTEGER currentPos;
        pStream->Seek({}, STREAM_SEEK_CUR, (ULARGE_INTEGER*)&currentPos);
        bytesRead = (DWORD)(currentPos.QuadPart - startPos.QuadPart);
    }

    // If no thumbnail found in resources, try to render from composite image data
    // Skip Layer and Mask Information section
    LARGE_INTEGER currentPos;
    pStream->Seek({}, STREAM_SEEK_CUR, (ULARGE_INTEGER*)&currentPos);
    
    DWORD layerMaskLength = ReadDWord(pStream);
    li.QuadPart = layerMaskLength;
    pStream->Seek(li, STREAM_SEEK_CUR, nullptr);
    
    // Now we're at the Image Data section
    // Read compression method
    WORD compression = ReadWord(pStream);
    
    // For now, only support uncompressed (0) or RLE (1)
    if (compression > 1)
        return nullptr;
    
    // Calculate target dimensions
    UINT thumbWidth, thumbHeight;
    if (header.width > header.height)
    {
        thumbWidth = cx;
        thumbHeight = (UINT)((float)header.height / header.width * cx);
    }
    else
    {
        thumbHeight = cx;
        thumbWidth = (UINT)((float)header.width / header.height * cx);
    }
    
    if (thumbWidth == 0) thumbWidth = 1;
    if (thumbHeight == 0) thumbHeight = 1;
    
    // For large images, this would be too slow. Limit to reasonable sizes.
    if (header.width > 4096 || header.height > 4096)
        return nullptr;
    
    // Read the image data based on compression
    std::vector<BYTE> imageData;
    
    if (compression == 0) // Uncompressed
    {
        // Read raw pixel data (planar format: RRR...GGG...BBB...AAA...)
        DWORD channelSize = header.width * header.height;
        DWORD totalSize = channelSize * header.channels;
        imageData.resize(totalSize);
        
        ULONG read;
        pStream->Read(imageData.data(), totalSize, &read);
        if (read != totalSize)
            return nullptr;
    }
    else if (compression == 1) // RLE compression (PackBits)
    {
        // Read byte counts for each scanline (2 bytes per scanline per channel)
        DWORD scanlineCount = header.height * header.channels;
        std::vector<WORD> byteCounts(scanlineCount);
        
        for (DWORD i = 0; i < scanlineCount; i++)
        {
            byteCounts[i] = ReadWord(pStream);
        }
        
        // Read and decompress the RLE data
        imageData.resize(header.width * header.height * header.channels);
        DWORD destOffset = 0;
        
        for (DWORD i = 0; i < scanlineCount; i++)
        {
            std::vector<BYTE> compressedLine(byteCounts[i]);
            ULONG read;
            pStream->Read(compressedLine.data(), byteCounts[i], &read);
            
            if (read != byteCounts[i])
                return nullptr;
            
            // Decode PackBits RLE
            DWORD srcPos = 0;
            DWORD linePixels = 0;
            
            while (srcPos < byteCounts[i] && linePixels < header.width)
            {
                char n = (char)compressedLine[srcPos++];
                
                if (n >= 0)
                {
                    // Copy next n+1 bytes literally
                    DWORD count = n + 1;
                    for (DWORD j = 0; j < count && srcPos < byteCounts[i] && linePixels < header.width; j++)
                    {
                        imageData[destOffset++] = compressedLine[srcPos++];
                        linePixels++;
                    }
                }
                else if (n != -128)
                {
                    // Repeat next byte -n+1 times
                    DWORD count = -n + 1;
                    if (srcPos < byteCounts[i])
                    {
                        BYTE value = compressedLine[srcPos++];
                        for (DWORD j = 0; j < count && linePixels < header.width; j++)
                        {
                            imageData[destOffset++] = value;
                            linePixels++;
                        }
                    }
                }
                // n == -128 is a no-op
            }
        }
    }
    else
    {
        // Unsupported compression
        return nullptr;
    }
    
    // Convert planar to interleaved RGBA
    std::vector<BYTE> rgbaData(header.width * header.height * 4);
    DWORD channelSize = header.width * header.height;
    
    for (DWORD i = 0; i < header.width * header.height; i++)
    {
        // Color mode 3 = RGB
        if (header.colorMode == 3)
        {
            BYTE r = (header.channels >= 1) ? imageData[i] : 0;
            BYTE g = (header.channels >= 2) ? imageData[channelSize + i] : 0;
            BYTE b = (header.channels >= 3) ? imageData[channelSize * 2 + i] : 0;
            BYTE a = (header.channels >= 4) ? imageData[channelSize * 3 + i] : 255;
            
            // Convert to BGRA for Windows
            rgbaData[i * 4 + 0] = b;
            rgbaData[i * 4 + 1] = g;
            rgbaData[i * 4 + 2] = r;
            rgbaData[i * 4 + 3] = a;
        }
        else
        {
            // Other color modes not supported yet
            return nullptr;
        }
    }
    
    return CreateBitmapFromRGBA(rgbaData.data(), header.width, header.height, cx);
}
