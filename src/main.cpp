#include <cstdlib>
#include <ctime>

#include "geometry.hpp"
#include "gl.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

extern mat<4, 4> ModelView, Perspective;
extern std::vector<double> zbuffer;

struct PhongShader : IShader
{
    const Model& model;
    vec3 tri[3];
    vec3 l;

    PhongShader(const vec3 light, const Model& m) : model(m)
    {
        l = normalized(
            (ModelView * vec4{light.x, light.y, light.z, 0.0}).xyz());
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        vec3 v{model.vert(face, vert)};
        vec4 glPosition{ModelView * vec4{v.x, v.y, v.z, 1.0}};

        tri[vert] = glPosition.xyz();

        return Perspective * glPosition;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
    {
        TGAColor glFragColor{{255, 255, 255, 255}};

        vec3 n{normalized(cross(tri[1] - tri[0], tri[2] - tri[0]))};
        vec3 r{normalized(2 * n * (n * l) - l)};

        double ambient{0.3};
        double diff{std::max(0.0, n * l)};
        double spec{std::pow(std::max(r.z, 0.0), 35)};

        for (int channel : {0, 1, 2})
            glFragColor[channel] *=
                std::min(1.0, ambient + 0.4 * diff + 0.9 * spec);

        return {false, glFragColor};
    }
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width{800};
    constexpr int height{800};

    constexpr vec3 light{1, 1, 1};
    constexpr vec3 eye{-1, 0, 2};
    constexpr vec3 center{0, 0, 0};
    constexpr vec3 up{0, 1, 0};

    lookAt(eye, center, up);
    initPerspective(norm(eye - center));
    initViewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);
    initZBuffer(width, height);

    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int m{1}; m < argc; ++m)
    {
        Model model(argv[m]);
        PhongShader shader(light, model);
        int nfaces{model.nfaces()};

        for (int f{0}; f < nfaces; ++f)
        {
            Triangle clip{shader.vertex(f, 0), shader.vertex(f, 1),
                          shader.vertex(f, 2)};

            rasterize(clip, shader, framebuffer);
        }
    }

    framebuffer.writeTGAFile("assets/framebuffer.tga");
    return 0;
}
