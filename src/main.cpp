#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <tuple>

#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

constexpr int width{800};
constexpr int height{800};

constexpr TGAColor white{{255, 255, 255, 255}};
constexpr TGAColor red{{255, 0, 0, 255}};

void line(int x1, int y1, int x2, int y2, TGAImage& framebuffer, TGAColor color)
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

std::tuple<int, int> project(vec3 v)
{
    return {(v.x + 1.) * width / 2, (v.y + 1.) * height / 2};
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    TGAImage framebuffer(width, height, TGAImage::RGB);
    Model model(argv[1]);
    int nverts = model.nverts();
    int nfaces = model.nfaces();

    for (int i{0}; i < nfaces; ++i)
    {
        auto [ax, ay] = project(model.vert(i, 0));
        auto [bx, by] = project(model.vert(i, 1));
        auto [cx, cy] = project(model.vert(i, 2));
        line(ax, ay, bx, by, framebuffer, red);
        line(bx, by, cx, cy, framebuffer, red);
        line(cx, cy, ax, ay, framebuffer, red);
    }

    for (int i{0}; i < nverts; ++i)
    {
        vec3 v = model.vert(i);
        auto [x, y] = project(v);
        framebuffer.set(x, y, white);
    }

    framebuffer.writeTGAFile("assets/output.tga");
    return 0;
}
