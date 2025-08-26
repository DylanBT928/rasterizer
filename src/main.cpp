#include <algorithm>
#include <cstdlib>
#include <ctime>

#include "tgaimage.hpp"

void line(int x1, int y1, int x2, int y2, TGAImage &framebuffer, TGAColor color)
{
    bool steep{std::abs(x2 - x1) < std::abs(y2 - y1)};

    if (steep)
    {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if (x1 > x2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    const int dx{x2 - x1};
    const int dy{std::abs(y2 - y1)};
    const int ystep{(y1 < y2) ? 1 : -1};

    int error{dx / 2};
    int y{y1};

    if (steep)
    {
        for (int x{x1}; x <= x2; ++x)
        {
            framebuffer.set(y, x, color);
            error -= dy;

            if (error < 0)
            {
                y += ystep;
                error += dx;
            }
        }
    }
    else
    {
        for (int x{x1}; x <= x2; ++x)
        {
            framebuffer.set(x, y, color);
            error -= dy;

            if (error < 0)
            {
                y += ystep;
                error += dx;
            }
        }
    }
}

int main()
{
    constexpr int width{64};
    constexpr int height{64};
    TGAImage framebuffer(width, height, TGAImage::RGB);

    std::srand(std::time({}));
    const std::size_t n{std::size_t{1} << 24};

    for (std::size_t i{0}; i < n; ++i)
    {
        int ax{rand() % width};
        int ay{rand() % height};
        int bx{rand() % width};
        int by{rand() % height};

        TGAColor color{};
        color.rgba[0] = static_cast<std::uint8_t>(rand() % 256);
        color.rgba[1] = static_cast<std::uint8_t>(rand() % 256);
        color.rgba[2] = static_cast<std::uint8_t>(rand() % 256);
        color.rgba[3] = 255;
        color.bytesPerPixel = 3;

        line(ax, ay, bx, by, framebuffer, color);
    }

    framebuffer.writeTGAFile("assets/output.tga");
    return 0;
}
