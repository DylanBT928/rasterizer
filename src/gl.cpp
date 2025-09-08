#include "gl.hpp"

#include <algorithm>

mat<4, 4> ModelView, Viewport, Perspective;
std::vector<double> zbuffer;

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

void initPerspective(const double f)
{
    Perspective = {
        {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}};
}

void initViewport(const int x, const int y, const int w, const int h)
{
    Viewport = {{{w / 2.0, 0, 0, x + w / 2.0},
                 {0, h / 2.0, 0, y + h / 2.0},
                 {0, 0, 1, 0},
                 {0, 0, 0, 1}}};
}

void initZBuffer(const int width, const int height)
{
    zbuffer = std::vector(width * height, -1000.0);
}

void rasterize(const Triangle& clip, const IShader& shader,
               TGAImage& framebuffer)
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

            auto [discard, color]{shader.fragment(bc)};

            if (discard)
                continue;

            zbuffer[x + y * framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}
