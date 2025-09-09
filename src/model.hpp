#include <vector>

#include "geometry.hpp"

class Model
{
   public:
    Model(const std::string filename);
    int nverts() const;
    int nfaces() const;
    vec3 vert(const int i) const;
    vec3 vert(const int iface, const int nthvert) const;
    vec3 normal(const int iface, const int nthvert) const;

   private:
    std::vector<vec3> verts{};
    std::vector<vec3> norms{};
    std::vector<int> facesVert{};
    std::vector<int> facesNorm{};
};
