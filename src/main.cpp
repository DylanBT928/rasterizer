#include "tgaimage.hpp"

constexpr TGAColor red{{255, 0, 0, 255}};
constexpr TGAColor green{{0, 255, 0, 255}};
constexpr TGAColor blue{{0, 0, 255, 255}};

int main()
{
    constexpr int width{64};
    constexpr int height{64};
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax{10};
    int ay{12};

    int bx{16};
    int by{47};

    int cx{50};
    int cy{53};

    framebuffer.set(ax, ay, red);
    framebuffer.set(bx, by, green);
    framebuffer.set(cx, cy, blue);

    framebuffer.writeTGAFile("assets/output.tga");
    return 0;
}
