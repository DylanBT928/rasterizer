#include <cstdlib>
#include <ctime>

#include "geometry.hpp"
#include "gl.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

extern mat<4, 4> ModelView, Perspective;
extern std::vector<double> zbuffer;

struct RandomShader : IShader
{
    const Model& model;
    TGAColor color = {};
    vec3 tri[3];

    RandomShader(const Model& m) : model(m) {}

    virtual vec4 vertex(const int face, const int vert)
    {
        vec3 v{model.vert(face, vert)};
        vec4 glPosition{ModelView * vec4{v.x, v.y, v.z, 1.0}};

        tri[vert] = glPosition.xyz();

        return Perspective * glPosition;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
    {
        return {false, color};
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
        RandomShader shader(model);
        int nfaces{model.nfaces()};

        for (int f{0}; f < nfaces; ++f)
        {
            TGAColor rnd{{0, 0, 0, 255}};
            for (int c{0}; c < 3; ++c) rnd[c] = std::rand() % 255;
            shader.color = rnd;

            Triangle clip{shader.vertex(f, 0), shader.vertex(f, 1),
                          shader.vertex(f, 2)};

            rasterize(clip, shader, framebuffer);
        }
    }

    framebuffer.writeTGAFile("assets/framebuffer.tga");
    return 0;
}
