#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <tuple>

#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

constexpr int width{128};
constexpr int height{128};

constexpr TGAColor red{{255, 0, 0, 255}};
constexpr TGAColor green{{0, 255, 0, 255}};
constexpr TGAColor blue{{0, 0, 255, 255}};

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

void triangle(int ax, int ay, int bx, int by, int cx, int cy,
              TGAImage& framebuffer, TGAColor color)
{
    if (ay > by)
    {
        std::swap(ax, bx);
        std::swap(ay, by);
    }

    if (ay > cy)
    {
        std::swap(ax, cx);
        std::swap(ay, cy);
    }

    if (by > cy)
    {
        std::swap(bx, cx);
        std::swap(by, cy);
    }

    int totalHeight{cy - ay};

    if (ay != by)
    {
        int segmentHeight{by - ay};

        for (int y{ay}; y <= by; ++y)
        {
            int x1{ax + ((cx - ax) * (y - ay)) / totalHeight};
            int x2{ax + ((bx - ax) * (y - ay)) / segmentHeight};
            int x{std::min(x1, x2)};
            int xMax{std::max(x1, x2)};

            for (; x < xMax; ++x) framebuffer.set(x, y, color);
        }
    }

    if (by != cy)
    {
        int segmentHeight{cy - by};

        for (int y{by}; y <= cy; ++y)
        {
            int x1{ax + ((cx - ax) * (y - ay)) / totalHeight};
            int x2{bx + ((cx - bx) * (y - by)) / segmentHeight};
            int x{std::min(x1, x2)};
            int xMax{std::max(x1, x2)};

            for (; x < xMax; ++x) framebuffer.set(x, y, color);
        }
    }
}

int main(int argc, char** argv)
{
    TGAImage framebuffer(width, height, TGAImage::RGB);

    triangle(7, 45, 35, 100, 45, 60, framebuffer, red);
    triangle(120, 35, 90, 5, 45, 110, framebuffer, green);
    triangle(115, 83, 80, 90, 85, 120, framebuffer, blue);

    framebuffer.writeTGAFile("assets/output.tga");
    return 0;
}
