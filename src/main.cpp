#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>

#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

mat<4, 4> ModelView, Viewport, Perspective;

void lookAt(const vec3 eye, const vec3 center, const vec3 up)
{
    vec3 n{normalized(eye - center)};
    vec3 l{normalized(cross(up, n))};
    vec3 m{normalized(cross(n, l))};

    ModelView = mat<4, 4>{{{l.x, l.y, l.z, 0},
                           {m.x, m.y, m.z, 0},
                           {n.x, n.y, n.z, 0},
                           {0, 0, 0, 1}}} *
                mat<4, 4>{{{1, 0, 0, -center.x},
                           {0, 1, 0, -center.y},
                           {0, 0, 1, -center.z},
                           {0, 0, 0, 1}}};
}

void perspective(const double f)
{
    Perspective = {
        {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}};
}

void viewport(const int x, const int y, const int w, const int h)
{
    Viewport = {{{w / 2.0, 0, 0, x + w / 2.0},
                 {0, h / 2.0, 0, y + h / 2.0},
                 {0, 0, 1, 0},
                 {0, 0, 0, 1}}};
}

void rasterize(const vec4 clip[3], std::vector<double>& zbuffer,
               TGAImage& framebuffer, const TGAColor color)
{
    vec4 ndc[3]{clip[0] / clip[0].w, clip[1] / clip[1].w, clip[2] / clip[2].w};
    vec2 screen[3] = {(Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(),
                      (Viewport * ndc[2]).xy()};
    mat<3, 3> ABC = {{{screen[0].x, screen[0].y, 1.0},
                      {screen[1].x, screen[1].y, 1.0},
                      {screen[2].x, screen[2].y, 1.0}}};

    if (ABC.det() < 1)
        return;

    auto [bbminx, bbmaxx]{std::minmax({screen[0].x, screen[1].x, screen[2].x})};
    auto [bbminy, bbmaxy]{std::minmax({screen[0].y, screen[1].y, screen[2].y})};

#pragma omp parallel for

    for (int x{std::max<int>(bbminx, 0)};
         x <= std::min<int>(bbmaxx, framebuffer.width() - 1); ++x)
    {
        for (int y{std::max<int>(bbminy, 0)};
             y <= std::min<int>(bbmaxy, framebuffer.height() - 1); ++y)
        {
            vec3 bc{ABC.invertTranspose() *
                    vec3{static_cast<double>(x), static_cast<double>(y), 1.0}};

            if (bc.x < 0 || bc.y < 0 || bc.z < 0)
                continue;

            double z{bc * vec3{ndc[0].z, ndc[1].z, ndc[2].z}};

            if (z <= zbuffer[x + y * framebuffer.width()])
                continue;

            zbuffer[x + y * framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width{800};
    constexpr int height{800};

    constexpr vec3 eye{-1, 0, 2};
    constexpr vec3 center{0, 0, 0};
    constexpr vec3 up{0, 1, 0};

    lookAt(eye, center, up);
    perspective(norm(eye - center));
    viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width * height,
                                -std::numeric_limits<double>::max());

    for (int m{1}; m < argc; ++m)
    {
        Model model(argv[m]);
        int nfaces{model.nfaces()};

        for (int i{0}; i < nfaces; ++i)
        {
            vec4 clip[3];
            for (int d : {0, 1, 2})
            {
                vec3 v{model.vert(i, d)};
                clip[d] = Perspective * ModelView * vec4{v.x, v.y, v.z, 1.0};
            }

            TGAColor rnd;
            for (int c{0}; c < 3; ++c) rnd[c] = std::rand() % 255;
            rasterize(clip, zbuffer, framebuffer, rnd);
        }
    }

    framebuffer.writeTGAFile("assets/framebuffer.tga");
    return 0;
}
