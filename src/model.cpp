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
        else if (!line.compare(0, 2, "f "))
        {
            int f, t, n;
            int cnt{0};

            iss >> trash;

            while (iss >> f >> trash >> t >> trash >> n)
            {
                faces.push_back(--f);
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

    std::vector<int> idx(nfaces());

    for (int i{0}; i < nfaces(); ++i) idx[i] = i;

    std::sort(idx.begin(), idx.end(),
              [&](const int& a, const int& b)
              {
                  float aminz = std::min(vert(a, 0).z,
                                         std::min(vert(a, 1).z, vert(a, 2).z));

                  float bminz = std::min(vert(b, 0).z,
                                         std::min(vert(b, 1).z, vert(b, 2).z));

                  return aminz < bminz;
              });

    std::vector<int> faces2(nfaces() * 3);

    for (int i{0}; i < nfaces(); ++i)
        for (int j{0}; j < 3; ++j) faces2[i * 3 + j] = faces[idx[i] * 3 + j];

    faces = faces2;
}

int Model::nverts() const { return verts.size(); }

int Model::nfaces() const { return faces.size() / 3; }

vec3 Model::vert(const int i) const { return verts[i]; }

vec3 Model::vert(const int iface, const int nthvert) const
{
    return verts[faces[iface * 3 + nthvert]];
}
