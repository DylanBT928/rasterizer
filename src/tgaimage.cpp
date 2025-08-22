#include "tgaimage.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>

TGAColor::TGAColor() : val(0), bytespp(1) {}

TGAColor::TGAColor(unsigned char R, unsigned char G, unsigned char B,
                   unsigned char A)
    : b(B), g(G), r(R), a(A), bytespp(4)
{
}

TGAColor::TGAColor(int v, int bpp) : val(v), bytespp(bpp) {}

TGAColor::TGAColor(const TGAColor& c) : val(c.val), bytespp(c.bytespp) {}

TGAColor::TGAColor(const unsigned char* p, int bpp) : val(0), bytespp(bpp)
{
    for (int i = 0; i < bpp; ++i) raw[i] = p[i];
}

TGAColor& TGAColor::operator=(const TGAColor& c)
{
    if (this != &c)
    {
        bytespp = c.bytespp;
        val = c.val;
    }

    return *this;
}

TGAImage::TGAImage() : data(nullptr), width(0), height(0), bytespp(0) {}

TGAImage::TGAImage(int w, int h, int bpp)
    : data(nullptr), width(w), height(h), bytespp(bpp)
{
    unsigned long nBytes = width * height * bytespp;
    data = new unsigned char[nBytes];
    memset(data, 0, nBytes);
}

TGAImage::TGAImage(const TGAImage& img)
    : width(img.width), height(img.height), bytespp(img.bytespp)
{
    unsigned long nBytes = width * height * bytespp;
    data = new unsigned char[nBytes];
    memcpy(data, img.data, nBytes);
}

TGAImage::~TGAImage()
{
    if (data)
        delete[] data;
}

bool TGAImage::readTGAFile(const char* file)
{
    if (data)
        delete[] data;

    data = nullptr;
    std::ifstream in;
    in.open(file, std::ios::binary);

    if (!in.is_open())
    {
        std::cerr << "Cannot open file " << file << ".\n";
        in.close();
        return false;
    }

    TGAHeader header;
    in.read((char*)&header, sizeof(header));

    if (!in.good())
    {
        std::cerr << "An error occured while reading the header.\n";
        in.close();
        return false;
    }

    width = header.width;
    height = header.height;
    bytespp = header.bitsPerPixel >> 3;

    if (width <= 0 || height <= 0 ||
        (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA))
    {
        std::cerr << "Bad bpp, width, or height value.\n";
        in.close();
        return false;
    }

    unsigned long nBytes = width * height * bytespp;
    data = new unsigned char[nBytes];

    if (header.dataTypeCode == 2 || header.dataTypeCode == 3)
    {
        in.read((char*)data, nBytes);

        if (!in.good())
        {
            std::cerr << "An error occured while reading the data.\n";
            in.close();
            return false;
        }
    }
    else if (header.dataTypeCode == 10 || header.dataTypeCode == 11)
    {
        if (!loadRLEData(in))
        {
            std::cerr << "An error occurred while reading the data.\n";
            in.close();
            return false;
        }
    }
    else
    {
        std::cerr << "Unknown file format " << (int)header.dataTypeCode
                  << ".\n";
        in.close();
        return false;
    }

    if (!(header.imageDescriptor & 0x20))
        flipVertically();

    if (header.imageDescriptor & 0x10)
        flipHorizontally();

    std::cerr << width << "x" << height << "/" << bytespp * 8 << std::endl;
    in.close();
    return true;
}

bool TGAImage::writeTGAFile(const char* file, bool rle)
{
    unsigned char developerAreaRef[4] = {0, 0, 0, 0};
    unsigned char extensionAreaRef[4] = {0, 0, 0, 0};
    unsigned char footer[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O',
                                'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};
    std::ofstream out;
    out.open(file, std::ios::binary);

    if (!out.is_open())
    {
        std::cerr << "Cannot open file " << file << ".\n";
        out.close();
        return false;
    }

    TGAHeader header;
    memset((void*)&header, 0, sizeof(header));
    header.bitsPerPixel = bytespp << 3;
    header.width = width;
    header.height = height;
    header.dataTypeCode =
        (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
    header.imageDescriptor = 0x20;  // Top-left origin
    out.write((char*)&header, sizeof(header));

    if (!out.good())
    {
        std::cerr << "Cannot dump the TGA file.\n";
        out.close();
        return false;
    }

    if (!rle)
    {
        out.write((char*)data, width * height * bytespp);

        if (!out.good())
        {
            std::cerr << "Cannot unload raw data.\n";
            out.close();
            return false;
        }
    }
    else
    {
        if (!unloadRLEData(out))
        {
            std::cerr << "Cannot unload RLE data.\n";
            out.close();
            return false;
        }
    }

    out.write((char*)developerAreaRef, sizeof(developerAreaRef));

    if (!out.good())
    {
        std::cerr << "Cannot dump the TGA file.\n";
        out.close();
        return false;
    }

    out.write((char*)extensionAreaRef, sizeof(extensionAreaRef));

    if (!out.good())
    {
        std::cerr << "Cannot dump the TGA file.\n";
        out.close();
        return false;
    }

    out.write((char*)footer, sizeof(footer));

    if (!out.good())
    {
        std::cerr << "Cannot dump the TGA file.\n";
        out.close();
        return false;
    }

    out.close();
    return true;
}

bool TGAImage::flipHorizontally()
{
    if (!data)
        return false;

    int half = width >> 1;

    for (int i = 0; i < half; ++i)
    {
        for (int j = 0; j < height; ++j)
        {
            TGAColor c1 = get(i, j);
            TGAColor c2 = get(width - i - 1, j);
            set(i, j, c2);
            set(width - i - 1, j, c1);
        }
    }

    return true;
}

bool TGAImage::flipVertically()
{
    if (!data)
        return false;

    unsigned long bytesPerLine = width * bytespp;
    unsigned char* line = new unsigned char[bytesPerLine];
    int half = height >> 1;

    for (int i = 0; i < half; ++i)
    {
        unsigned long l1 = bytesPerLine * i;
        unsigned long l2 = (height - i - 1) * bytesPerLine;
        memmove((void*)line, (void*)(data + l1), bytesPerLine);
        memmove((void*)(data + l1), (void*)(data + l2), bytesPerLine);
        memmove((void*)(data + l2), (void*)line, bytesPerLine);
    }

    delete[] line;
    return true;
}

bool TGAImage::scale(int w, int h)
{
    if (w <= 0 || h <= 0 || !data)
        return false;

    unsigned char* tData = new unsigned char[w * h * bytespp];
    unsigned long nLineBytes = w * bytespp;
    unsigned long oLineBytes = width * bytespp;
    int nScanLine = 0;
    int oScanLine = 0;
    int errY = 0;

    for (int i = 0; i < height; ++i)
    {
        int errX = width - w;
        int nx = -bytespp;
        int ox = -bytespp;

        for (int j = 0; j < width; ++j)
        {
            ox += bytespp;
            errX += w;

            while ((int)width <= errX)
            {
                errX -= width;
                nx += bytespp;
                memcpy(tData + nScanLine + nx, data + oScanLine + ox, bytespp);
            }
        }

        errY += h;
        oScanLine += oLineBytes;

        while ((int)height <= errY)
        {
            if ((int)height << 1 <= errY)
                memcpy(tData + nScanLine + nLineBytes, tData + nScanLine,
                       nLineBytes);

            errY -= height;
            nScanLine += nLineBytes;
        }
    }

    delete[] data;
    data = tData;
    width = w;
    height = h;
    return true;
}

TGAColor TGAImage::get(int x, int y)
{
    if (!data || x < 0 || y < 0 || width <= x || height <= y)
        return TGAColor();

    return TGAColor(data + (x + y * width) * bytespp, bytespp);
}

bool TGAImage::set(int x, int y, TGAColor c)
{
    if (!data || x < 0 || y < 0 || width <= x || height <= y)
        return false;

    memcpy(data + (x + y * width) * bytespp, c.raw, bytespp);
    return true;
}

int TGAImage::getWidth() { return width; }

int TGAImage::getHeight() { return height; }

int TGAImage::getBytesPP() { return bytespp; }

unsigned char* TGAImage::buffer() { return data; }

void TGAImage::clear() { memset((void*)data, 0, width * height * bytespp); }

TGAImage& TGAImage::operator=(const TGAImage& img)
{
    if (this != &img)
    {
        if (data)
            delete[] data;

        width = img.width;
        height = img.height;
        bytespp = img.bytespp;
        unsigned long nBytes = width * height * bytespp;
        data = new unsigned char[nBytes];
        memcpy(data, img.data, nBytes);
    }

    return *this;
}

bool TGAImage::loadRLEData(std::ifstream& in)
{
    unsigned long pixelCount = width * height;
    unsigned long currentPixel = 0;
    unsigned long currentByte = 0;
    TGAColor colorBuffer;

    while (currentPixel < pixelCount)
    {
        unsigned char chunkHeader = in.get();

        if (!in.good())
        {
            std::cerr << "An error occurred while reading the data.\n";
            return false;
        }

        if (chunkHeader < 128)
        {
            ++chunkHeader;

            for (int i = 0; i < chunkHeader; ++i)
            {
                in.read((char*)colorBuffer.raw, bytespp);

                if (!in.good())
                {
                    std::cerr << "An error occured while reading the header.\n";
                    return false;
                }

                for (int j = 0; j < bytespp; ++j)
                    data[currentByte++] = colorBuffer.raw[j];

                ++currentPixel;

                if (pixelCount < currentPixel)
                {
                    std::cerr << "Too many pixels read.\n";
                    return false;
                }
            }
        }
        else
        {
            chunkHeader -= 127;
            in.read((char*)colorBuffer.raw, bytespp);

            if (!in.good())
            {
                std::cerr << "An error occured while reading the header.\n";
                return false;
            }

            for (int i = 0; i < chunkHeader; ++i)
            {
                for (int j = 0; j < bytespp; ++j)
                    data[currentByte++] = colorBuffer.raw[j];

                ++currentPixel;

                if (pixelCount < currentPixel)
                {
                    std::cerr << "Too many pixels read.\n";
                    return false;
                }
            }
        }
    }

    return true;
}

bool TGAImage::unloadRLEData(std::ofstream& out)
{
    const unsigned char maxChunkLength = 128;
    unsigned long nPixels = width * height;
    unsigned long currentPixel = 0;

    while (currentPixel < nPixels)
    {
        unsigned long chunkStart = currentPixel * bytespp;
        unsigned long currentByte = currentPixel * bytespp;
        unsigned char runLength = 1;
        bool raw = true;

        while (currentPixel + runLength < nPixels && runLength < maxChunkLength)
        {
            bool succEq = true;

            for (int i = 0; succEq && i < bytespp; ++i)
                succEq =
                    (data[currentByte + i] == data[currentByte + bytespp + i]);

            currentByte += bytespp;

            if (runLength == 1)
                raw = !succEq;

            if (raw && succEq)
            {
                --runLength;
                break;
            }

            if (!raw && !succEq)
                break;

            ++runLength;
        }

        currentPixel += runLength;
        out.put(raw ? runLength - 1 : runLength + 127);

        if (!out.good())
        {
            std::cerr << "Cannot dump the TGA file.\n";
            return false;
        }

        out.write((char*)(data + chunkStart),
                  (raw ? runLength * bytespp : bytespp));

        if (!out.good())
        {
            std::cerr << "Cannot dump the TGA file.\n";
            return false;
        }
    }
    return true;
}
