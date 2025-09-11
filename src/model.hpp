#include "geometry.hpp"
#include "tgaimage.hpp"

class Model
{
   public:
    Model(const std::string filename);
    int nverts() const;
    int nfaces() const;
    vec4 vert(const int i) const;
    vec4 vert(const int iface, const int nthvert) const;
    vec4 normal(const int iface, const int nthvert) const;
    vec4 normal(const vec2& uv) const;
    vec2 uv(const int iface, const int nthvert) const;

   private:
    TGAImage normalMap;
    std::vector<vec4> verts{};
    std::vector<vec4> norms{};
    std::vector<vec2> tex{};
    std::vector<int> facesVert{};
    std::vector<int> facesNorm{};
    std::vector<int> facesTex{};
};
