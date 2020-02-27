#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), uvs_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if(!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for(int i = 0; i < 3; i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if(!line.compare(0, 2, "vt")) {
            iss >> trash >> trash;
            Vec2f uv;
            float ftrash;

            iss >> uv.x >> uv.y >> ftrash;
            uvs_.push_back(uv);
        } else if (!line.compare(0, 2, "f ")) {
            FaceInfo f[3];
            int itrash, idx, uv;
            iss >> trash;

            // v1
            iss >> idx >> trash >> uv >> trash >> itrash;
            f[0].vertIndex = idx - 1; // in wavefront obj all indices start at 1, not zero
            f[0].uvIndex = uv - 1; // in wavefront obj all indices start at 1, not zero

            // v2
            iss >> idx >> trash >> uv >> trash >> itrash;
            f[1].vertIndex = idx - 1; // in wavefront obj all indices start at 1, not zero
            f[1].uvIndex = uv - 1; // in wavefront obj all indices start at 1, not zero

            // v3
            iss >> idx >> trash >> uv >> trash >> itrash;
            f[2].vertIndex = idx - 1; // in wavefront obj all indices start at 1, not zero
            f[2].uvIndex = uv - 1; // in wavefront obj all indices start at 1, not zero
   
            faces_.push_back(std::make_tuple(f[0], f[1], f[2]));
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::tuple<FaceInfo, FaceInfo, FaceInfo> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec2f Model::uv(int i)
{
    return uvs_[i];
}
