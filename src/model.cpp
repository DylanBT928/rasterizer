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
            vec3 v;
            for (int i : {0, 1, 2}) iss >> v[i];
            verts.push_back(v);
        }
        else if (!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            vec3 n;

            for (int i : {0, 1, 2}) iss >> n[i];

            norms.push_back(normalized(n));
        }
        else if (!line.compare(0, 2, "f "))
        {
            int f, t, n;
            int cnt{0};

            iss >> trash;

            while (iss >> f >> trash >> t >> trash >> n)
            {
                facesVert.push_back(--f);
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
}

int Model::nverts() const { return verts.size(); }

int Model::nfaces() const { return facesVert.size() / 3; }

vec3 Model::vert(const int i) const { return verts[i]; }

vec3 Model::vert(const int iface, const int nthvert) const
{
    return verts[facesVert[iface * 3 + nthvert]];
}

vec3 Model::normal(const int iface, const int nthvert) const
{
    return norms[facesNorm[iface * 3 + nthvert]];
}
