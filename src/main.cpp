#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>

#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

constexpr int width{800};
constexpr int height{800};

double signedTriangleArea(int ax, int ay, int bx, int by, int cx, int cy)
{
    return 0.5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) +
                  (ay - cy) * (ax + cx));
}

void triangle(int ax, int ay, double az, int bx, int by, double bz, int cx,
              int cy, double cz, std::vector<double>& zbuffer,
              TGAImage& framebuffer, TGAColor color)
{
    int bbminx{std::max(0, std::min(std::min(ax, bx), cx))};
    int bbminy{std::max(0, std::min(std::min(ay, by), cy))};
    int bbmaxx{
        std::min(framebuffer.width() - 1, std::max(std::max(ax, bx), cx))};
    int bbmaxy{
        std::min(framebuffer.height() - 1, std::max(std::max(ay, by), cy))};
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

            double z{alpha * az + beta * bz + gamma * cz};

            if (z <= zbuffer[x + y * width])
                continue;

            zbuffer[x + y * width] = z;
            framebuffer.set(x, y, color);
        }
    }
}

vec3 rotate(vec3 v)
{
    constexpr double a{M_PI / 6};
    double c{std::cos(a)};
    double s{std::sin(a)};
    mat<3, 3> ry{{{c, 0, s}, {0, 1, 0}, {-s, 0, c}}};

    return ry * v;
}

vec3 persp(vec3 v)
{
    constexpr double c{3.0};
    return v / (1 - v.z / c);
}

std::tuple<int, int, double> project(vec3 v)
{
    return {(v.x + 1.0) * width / 2, (v.y + 1.0) * height / 2, v.z};
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    std::vector<double> zbuffer(width * height,
                                -std::numeric_limits<double>::max());
    TGAImage framebuffer(width, height, TGAImage::RGB);
    Model model(argv[1]);
    int nfaces = model.nfaces();

    for (int i{0}; i < nfaces; ++i)
    {
        auto [ax, ay, az] = project(persp(rotate(model.vert(i, 0))));
        auto [bx, by, bz] = project(persp(rotate(model.vert(i, 1))));
        auto [cx, cy, cz] = project(persp(rotate(model.vert(i, 2))));

        TGAColor rnd;
        for (int c{0}; c < 3; ++c) rnd[c] = std::rand() % 255;
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.writeTGAFile("assets/output.tga");
    return 0;
}
