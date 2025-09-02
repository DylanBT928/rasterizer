#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

constexpr int width{800};
constexpr int height{800};

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

double signedTriangleArea(int ax, int ay, int bx, int by, int cx, int cy)
{
    return 0.5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) +
                  (ay - cy) * (ax + cx));
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy,
              TGAImage& framebuffer, TGAColor color)
{
    int bbminx{std::min(std::min(ax, bx), cx)};
    int bbminy{std::min(std::min(ay, by), cy)};
    int bbmaxx{std::max(std::max(ax, bx), cx)};
    int bbmaxy{std::max(std::max(ay, by), cy)};
    double totalArea{signedTriangleArea(ax, ay, bx, by, cx, cy)};

    if (totalArea < 1)
        return;

#pragma omp parallel for

    for (int x{bbminx}; x <= bbmaxx; ++x)
    {
        for (int y{bbminy}; y <= bbmaxy; ++y)
        {
            double alpha{signedTriangleArea(x, y, bx, by, cx, cy) / totalArea};
            double beta{signedTriangleArea(x, y, cx, cy, ax, ay) / totalArea};
            double gamma{signedTriangleArea(x, y, ax, ay, bx, by) / totalArea};

            if (alpha < 0 || beta < 0 || gamma < 0)
                continue;

            framebuffer.set(x, y, color);
        }
    }
}

std::tuple<int, int> project(vec3 v)
{
    return {(v.x + 1.0) * width / 2, (v.y + 1.0) * height / 2};
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
    int nfaces = model.nfaces();

    for (int i{0}; i < nfaces; ++i)
    {
        auto [ax, ay] = project(model.vert(i, 0));
        auto [bx, by] = project(model.vert(i, 1));
        auto [cx, cy] = project(model.vert(i, 2));

        TGAColor rnd;
        for (int c{0}; c < 3; ++c) rnd[c] = std::rand() % 255;
        triangle(ax, ay, bx, by, cx, cy, framebuffer, rnd);
    }

    framebuffer.writeTGAFile("assets/output.tga");
    return 0;
}
