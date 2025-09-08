#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

static_assert(true);
#pragma pack(push, 1)

struct TGAHeader
{
    std::uint8_t idLength{0};
    std::uint8_t colorMapType{0};
    std::uint8_t dataTypeCode{0};
    std::uint16_t colorMapOrigin{0};
    std::uint16_t colorMapLength{0};
    std::uint8_t colorMapDepth{0};
    std::uint16_t xOrigin{0};
    std::uint16_t yOrigin{0};
    std::uint16_t width{0};
    std::uint16_t height{0};
    std::uint8_t bitsPerPixel{0};
    std::uint8_t imageDescriptor{0};
};

#pragma pack(pop)

struct TGAColor
{
    std::array<std::uint8_t, 4> rgba{0, 0, 0, 0};
    std::uint8_t bytesPerPixel{4};

    std::uint8_t& operator[](const int i) noexcept
    {
        return rgba[static_cast<size_t>(i)];
    }

    const std::uint8_t& operator[](const int i) const noexcept
    {
        return rgba[static_cast<size_t>(i)];
    }
};

struct TGAImage
{
    enum Format
    {
        GRAYSCALE = 1,
        RGB = 3,
        RGBA = 4
    };

    TGAImage() = default;
    TGAImage(const int w, const int h, const int bpp, TGAColor c = {}) noexcept;

    TGAColor get(const int x, const int y) const;
    void set(const int x, const int y, const TGAColor& c);

    int width() const noexcept { return w; }
    int height() const noexcept { return h; }

    bool readTGAFile(const std::filesystem::path& filename);
    bool writeTGAFile(const std::filesystem::path& filename,
                      const bool vflip = true, const bool rle = true) const;

    void flipHorizontally();
    void flipVertically();

   private:
    int w{0};
    int h{0};
    std::uint8_t bpp{0};
    std::vector<std::uint8_t> data{};

    bool loadRLEData(std::ifstream& in);
    bool unloadRLEData(std::ofstream& out) const;
};
