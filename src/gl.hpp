#pragma once

#include "geometry.hpp"
#include "tgaimage.hpp"

void lookAt(const vec3 eye, const vec3 center, const vec3 up);
void initPerspective(const double f);
void initViewport(const int x, const int y, const int w, const int h);
void initZBuffer(const int width, const int height);

struct IShader
{
    static TGAColor sample2D(const TGAImage& img, const vec2& uvf)
    {
        return img.get(uvf[0] * img.width(), uvf[1] * img.height());
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};

typedef vec4 Triangle[3];

void rasterize(const Triangle& clip, const IShader& shader,
               TGAImage& framebuffer);
