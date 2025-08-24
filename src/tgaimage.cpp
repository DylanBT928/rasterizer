#include "tgaimage.hpp"

#include <cstring>
#include <iostream>

TGAImage::TGAImage(const int w, const int h, const int bpp) noexcept
    : w(w), h(h), bpp(bpp), data(w * h * bpp, 0)
{
}

TGAColor TGAImage::get(const int x, const int y) const
{
    if (data.empty() || x < 0 || y < 0 || x >= w || y >= h)
        return {};

    const std::size_t idx{
        (static_cast<std::size_t>(y) * static_cast<std::size_t>(w) +
         static_cast<std::size_t>(x)) *
        static_cast<std::size_t>(bpp)};
    const std::uint8_t* p{data.data() + idx};

    TGAColor pixel{};
    pixel.bytesPerPixel = bpp;
    std::memcpy(pixel.rgba.data(), p, static_cast<std::size_t>(bpp));

    return pixel;
}

void TGAImage::set(const int x, const int y, const TGAColor& c)
{
    if (data.empty() || x < 0 || y < 0 || x >= w || y >= h)
        return;

    const std::size_t idx{
        (static_cast<std::size_t>(y) * static_cast<std::size_t>(w) +
         static_cast<std::size_t>(x)) *
        static_cast<std::size_t>(bpp)};

    std::memcpy(&data[idx], c.rgba.data(), static_cast<std::size_t>(bpp));
}

bool TGAImage::readTGAFile(const std::filesystem::path& filename)
{
    std::ifstream in(filename, std::ios::binary);

    if (!in)
    {
        std::cerr << "Cannot open file " << filename << '\n';
        return false;
    }

    TGAHeader header{};

    in.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (!in)
    {
        std::cerr << "Error reading TGA header\n";
        return false;
    }

    if (header.idLength)
    {
        in.seekg(header.idLength, std::ios::cur);

        if (!in)
        {
            std::cerr << "Error skipping TGA Image ID field\n";
            return false;
        }
    }

    if (header.colorMapType != 0)
    {
        std::cerr << "Color-mapped TGA not supported\n";
        return false;
    }

    w = header.width;
    h = header.height;
    bpp = static_cast<std::uint8_t>(header.bitsPerPixel >> 3);

    if (w <= 0 || h <= 0 || (bpp != GRAYSCALE && bpp != RGB && bpp != RGBA))
    {
        std::cerr << "Bad size or unsupported bpp (" << int(bpp) << ")\n";
        return false;
    }

    const std::size_t nbytes = static_cast<std::size_t>(w) *
                               static_cast<std::size_t>(h) *
                               static_cast<std::size_t>(bpp);
    data.assign(nbytes, 0);

    if (header.dataTypeCode == 2 || header.dataTypeCode == 3)
    {
        in.read(reinterpret_cast<char*>(data.data()),
                static_cast<std::streamsize>(nbytes));

        if (!in)
        {
            std::cerr << "Error reading pixel data\n";
            return false;
        }
    }
    else if (header.dataTypeCode == 10 || header.dataTypeCode == 11)
    {
        if (!loadRLEData(in))
        {
            std::cerr << "Error reading RLE data\n";
            return false;
        }
    }
    else
    {
        std::cerr << "Unknown file format " << int(header.dataTypeCode) << '\n';
        return false;
    }

    if (bpp >= 3)
        for (std::size_t i{0}; i + (bpp - 1) < data.size(); i += bpp)
            std::swap(data[i], data[i + 2]);

    if (!(header.imageDescriptor & 0x20))
        flipVertically();

    if (header.imageDescriptor & 0x10)
        flipHorizontally();

    std::cerr << w << 'x' << h << '/' << (bpp * 8) << '\n';
    return true;
}

bool TGAImage::writeTGAFile(const std::filesystem::path& filename,
                            const bool vflip, const bool rle) const
{
    std::ofstream out(filename, std::ios::binary);

    if (!out)
    {
        std::cerr << "Cannot open file " << filename << '\n';
        return false;
    }

    TGAHeader header{};
    header.bitsPerPixel = static_cast<std::uint8_t>(bpp << 3);
    header.width = static_cast<std::uint16_t>(w);
    header.height = static_cast<std::uint16_t>(h);
    header.dataTypeCode = (bpp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
    header.imageDescriptor = vflip ? 0x00 : 0x20;

    out.write(reinterpret_cast<const char*>(&header), sizeof(header));

    if (!out)
    {
        std::cerr << "Error writing TGA header\n";
        return false;
    }

    if (!rle)
    {
        const std::size_t rowBytes{static_cast<std::size_t>(w) * bpp};

        if (bpp == GRAYSCALE)
        {
            out.write(reinterpret_cast<const char*>(data.data()),
                      static_cast<std::streamsize>(data.size()));

            if (!out)
            {
                std::cerr << "Error writing pixels\n";
                return false;
            }
        }
        else
        {
            std::vector<std::uint8_t> row(rowBytes);

            if (bpp == 3)
            {
                for (int y{0}; y < h; ++y)
                {
                    const std::size_t base =
                        static_cast<std::size_t>(y) * rowBytes;

                    for (int x{0}; x < w; ++x)
                    {
                        const std::size_t s =
                            base + static_cast<std::size_t>(x) * 3;
                        const std::size_t d = static_cast<std::size_t>(x) * 3;
                        row[d] = data[s + 2];
                        row[d + 1] = data[s + 1];
                        row[d + 2] = data[s];
                    }

                    out.write(reinterpret_cast<const char*>(row.data()),
                              static_cast<std::streamsize>(rowBytes));

                    if (!out)
                    {
                        std::cerr << "Error writing pixels\n";
                        return false;
                    }
                }
            }
            else
            {
                for (int y{0}; y < h; ++y)
                {
                    const std::size_t base{static_cast<std::size_t>(y) *
                                           rowBytes};

                    for (int x{0}; x < w; ++x)
                    {
                        const std::size_t s{base +
                                            static_cast<std::size_t>(x) * 4};
                        const std::size_t d{static_cast<std::size_t>(x) * 4};
                        row[d] = data[s + 2];
                        row[d + 1] = data[s + 1];
                        row[d + 2] = data[s + 0];
                        row[d + 3] = data[s + 3];
                    }

                    out.write(reinterpret_cast<const char*>(row.data()),
                              static_cast<std::streamsize>(rowBytes));

                    if (!out)
                    {
                        std::cerr << "Error writing pixels\n";
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        if (!unloadRLEData(out))
        {
            std::cerr << "Error writing RLE data\n";
            return false;
        }
    }

    static constexpr std::uint8_t developerAreaRef[4]{0, 0, 0, 0};
    static constexpr std::uint8_t extensionAreaRef[4]{0, 0, 0, 0};
    static constexpr std::uint8_t footer[18]{'T', 'R', 'U', 'E', 'V', 'I',
                                             'S', 'I', 'O', 'N', '-', 'X',
                                             'F', 'I', 'L', 'E', '.', '\0'};

    out.write(reinterpret_cast<const char*>(developerAreaRef),
              sizeof(developerAreaRef));
    out.write(reinterpret_cast<const char*>(extensionAreaRef),
              sizeof(extensionAreaRef));
    out.write(reinterpret_cast<const char*>(footer), sizeof(footer));

    if (!out)
    {
        std::cerr << "Error writing TGA footer\n";
        return false;
    }

    return true;
}

void TGAImage::flipHorizontally()
{
    if (data.empty() || w <= 1)
        return;

    const std::size_t BPP{static_cast<std::size_t>(bpp)};
    const std::size_t bytesPerRow{static_cast<std::size_t>(w) * BPP};

    for (int y{0}; y < h; ++y)
    {
        std::uint8_t* row{data.data() +
                          static_cast<std::size_t>(y) * bytesPerRow};

        for (int xL{0}, xR{w - 1}; xL < xR; ++xL, --xR)
        {
            std::uint8_t* L{row + static_cast<std::size_t>(xL) * BPP};
            std::uint8_t* R{row + static_cast<std::size_t>(xR) * BPP};

            for (std::size_t t{0}; t < BPP; ++t) std::swap(L[t], R[t]);
        }
    }
}

void TGAImage::flipVertically()
{
    if (data.empty() || h <= 1)
        return;

    const std::size_t BPP{static_cast<std::size_t>(bpp)};
    const std::size_t bytesPerRow{static_cast<std::size_t>(w) * BPP};
    std::vector<std::uint8_t> rowBuf(bytesPerRow);

    for (int yTop{0}, yBot{h - 1}; yTop < yBot; ++yTop, --yBot)
    {
        std::uint8_t* top{data.data() +
                          static_cast<std::size_t>(yTop) * bytesPerRow};
        std::uint8_t* bot{data.data() +
                          static_cast<std::size_t>(yBot) * bytesPerRow};

        std::memcpy(rowBuf.data(), top, bytesPerRow);
        std::memcpy(top, bot, bytesPerRow);
        std::memcpy(bot, rowBuf.data(), bytesPerRow);
    }
}

bool TGAImage::loadRLEData(std::ifstream& in)
{
    const std::size_t BPP{static_cast<std::size_t>(bpp)};
    const std::size_t pixelCount{static_cast<std::size_t>(w) *
                                 static_cast<std::size_t>(h)};
    std::size_t curPix{0};
    std::size_t curByte{0};

    while (curPix < pixelCount)
    {
        std::uint8_t chunkHeader{0};

        if (!in.read(reinterpret_cast<char*>(&chunkHeader), 1))
        {
            std::cerr << "Error reading RLE chunk header\n";
            return false;
        }

        if (chunkHeader < 128)
        {
            const std::size_t count{static_cast<std::size_t>(chunkHeader) + 1};

            for (std::size_t i{0}; i < count; ++i)
            {
                if (!in.read(reinterpret_cast<char*>(data.data() + curByte),
                             static_cast<std::streamsize>(BPP)))
                {
                    std::cerr << "Error reading raw RLE pixel\n";
                    return false;
                }

                curByte += static_cast<std::size_t>(bpp);

                if (++curPix > pixelCount)
                {
                    std::cerr << "Too many pixels (raw)\n";
                    return false;
                }
            }
        }
        else
        {
            std::size_t count{static_cast<std::size_t>(chunkHeader) - 127};
            std::uint8_t px[4];

            if (!in.read(reinterpret_cast<char*>(px),
                         static_cast<std::streamsize>(BPP)))
            {
                std::cerr << "Error reading RLE seed pixel\n";
                return false;
            }

            for (std::size_t i{0}; i < count; ++i)
            {
                for (std::size_t t{0}; t < BPP; ++t) data[curByte + t] = px[t];

                curByte += BPP;

                if (++curPix > pixelCount)
                {
                    std::cerr << "Too many pixels (rle)\n";
                    return false;
                }
            }
        }
    }

    return true;
}

bool TGAImage::unloadRLEData(std::ofstream& out) const
{
    constexpr std::uint8_t kMaxChunkLen{128};
    const std::size_t BPP{static_cast<std::size_t>(bpp)};
    const std::size_t nPixels{static_cast<std::size_t>(w) *
                              static_cast<std::size_t>(h)};
    std::array<std::uint8_t, kMaxChunkLen * 4> buf{};
    std::size_t curPix{0};

    while (curPix < nPixels)
    {
        const std::size_t chunkStartPix{curPix};
        const std::size_t chunkStartByte{chunkStartPix * BPP};
        std::size_t probeByte{chunkStartByte};
        std::size_t runLen{1};
        bool raw{true};

        while (curPix + runLen < nPixels && runLen < kMaxChunkLen)
        {
            const bool equal{std::memcmp(&data[probeByte],
                                         &data[probeByte + BPP], BPP) == 0};
            probeByte += BPP;

            if (runLen == 1)
                raw = !equal;

            if (raw && equal)
            {
                --runLen;
                break;
            }

            if (!raw && !equal)
                break;

            ++runLen;
        }

        out.put(raw ? static_cast<char>(runLen - 1)
                    : static_cast<char>(runLen + 127));

        if (!out)
            return false;

        if (raw)
        {
            const std::size_t start{chunkStartByte};

            if (BPP == 1)
            {
                const std::size_t bytes{runLen * BPP};

                if (!out.write(reinterpret_cast<const char*>(&data[start]),
                               static_cast<std::streamsize>(bytes)))
                    return false;
            }
            else if (BPP == 3)
            {
                std::size_t s{start};
                std::size_t d{0};

                for (std::size_t i{0}; i < runLen; ++i, s += 3, d += 3)
                {
                    buf[d] = data[s + 2];
                    buf[d + 1] = data[s + 1];
                    buf[d + 2] = data[s];
                }

                const std::size_t bytes{runLen * 3};

                if (!out.write(reinterpret_cast<const char*>(buf.data()),
                               static_cast<std::streamsize>(bytes)))
                    return false;
            }
            else
            {
                std::size_t s{start};
                std::size_t d{0};

                for (std::size_t i{0}; i < runLen; ++i, s += 4, d += 4)
                {
                    buf[d] = data[s + 2];
                    buf[d + 1] = data[s + 1];
                    buf[d + 2] = data[s];
                    buf[d + 3] = data[s + 3];
                }

                const std::size_t bytes{runLen * 4};

                if (!out.write(reinterpret_cast<const char*>(buf.data()),
                               static_cast<std::streamsize>(bytes)))
                    return false;
            }
        }
        else
        {
            const std::size_t s{chunkStartByte};

            if (BPP == 1)
            {
                if (!out.write(reinterpret_cast<const char*>(&data[s]), 1))
                    return false;
            }
            else if (BPP == 3)
            {
                const std::uint8_t triplet[3]{data[s + 2], data[s + 1],
                                              data[s]};

                if (!out.write(reinterpret_cast<const char*>(triplet), 3))
                    return false;
            }
            else
            {
                const std::uint8_t quad[4]{data[s + 2], data[s + 1], data[s],
                                           data[s + 3]};

                if (!out.write(reinterpret_cast<const char*>(quad), 4))
                    return false;
            }
        }

        curPix += runLen;
    }

    return true;
}
