#include "model.hpp"

#include <fstream>
#include <sstream>

Model::Model(const std::string filename)
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);

    if (!in)
        return;

    std::string line;

    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;

        if (!line.compare(0, 2, "v "))
        {
            iss >> trash;
            vec4 v{0, 0, 0, 1};
            for (int i : {0, 1, 2}) iss >> v[i];
            verts.push_back(v);
        }
        else if (!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            vec4 n;
            for (int i : {0, 1, 2}) iss >> n[i];
            norms.push_back(normalized(n));
        }
        else if (!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            vec2 uv;
            for (int i : {0, 1}) iss >> uv[i];
            tex.push_back({uv.x, 1 - uv.y});
        }
        else if (!line.compare(0, 2, "f "))
        {
            int f, t, n;
            int cnt{0};

            iss >> trash;

            while (iss >> f >> trash >> t >> trash >> n)
            {
                facesVert.push_back(--f);
                facesTex.push_back(--t);
                facesNorm.push_back(--n);
                ++cnt;
            }

            if (cnt != 3)
            {
                std::cerr
                    << "Error: the obj file is supposed to be triangulated\n";
                return;
            }
        }
    }

    std::cerr << "# v# " << nverts() << " f# " << nfaces() << std::endl;

    auto loadTexture{
        [&filename](const std::string suffix, TGAImage& img)
        {
            std::size_t dot{filename.find_last_of(".")};

            if (dot == std::string::npos)
                return;

            std::string texFile{filename.substr(0, dot) + suffix};
            std::cerr << "Texture file " << texFile << " loading "
                      << (img.readTGAFile(texFile.c_str()) ? "ok" : "failed")
                      << std::endl;
        }};

    loadTexture("_nm.tga", normalMap);
}

int Model::nverts() const { return verts.size(); }

int Model::nfaces() const { return facesVert.size() / 3; }

vec4 Model::vert(const int i) const { return verts[i]; }

vec4 Model::vert(const int iface, const int nthvert) const
{
    return verts[facesVert[iface * 3 + nthvert]];
}

vec4 Model::normal(const int iface, const int nthvert) const
{
    return norms[facesNorm[iface * 3 + nthvert]];
}

vec4 Model::normal(const vec2& uv) const
{
    TGAColor c{
        normalMap.get(uv[0] * normalMap.width(), uv[1] * normalMap.height())};
    return vec4{(double)c[2], (double)c[1], (double)c[0], 0} * 2.0 / 255.0 -
           vec4{1, 1, 1, 0};
}

vec2 Model::uv(const int iface, const int nthvert) const
{
    return tex[facesTex[iface * 3 + nthvert]];
}
