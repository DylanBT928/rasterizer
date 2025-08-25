#include "tgaimage.hpp"

constexpr TGAColor white{{255, 255, 255, 255}};
constexpr TGAColor red{{255, 0, 0, 255}};
constexpr TGAColor green{{0, 255, 0, 255}};
constexpr TGAColor blue{{0, 0, 255, 255}};
constexpr TGAColor yellow{{255, 200, 0, 255}};

void line(int x1, int y1, int x2, int y2, TGAImage &framebuffer, TGAColor color)
{
    bool steep{std::abs(x1 - x2) < std::abs(y1 - y2)};

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

    for (int x{x1}; x <= x2; ++x)
    {
        float t{(x - x1) / static_cast<float>(x2 - x1)};
        int y{static_cast<int>(std::round(y1 + (y2 - y1) * t))};

        if (steep)
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
    }
}

int main()
{
    constexpr int width{64};
    constexpr int height{64};
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax{7};
    int ay{3};

    int bx{12};
    int by{37};

    int cx{62};
    int cy{53};

    line(ax, ay, bx, by, framebuffer, red);
    line(cx, cy, bx, by, framebuffer, green);
    line(cx, cy, ax, ay, framebuffer, blue);
    line(ax, ay, cx, cy, framebuffer, yellow);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.writeTGAFile("assets/output.tga");
    return 0;
}
