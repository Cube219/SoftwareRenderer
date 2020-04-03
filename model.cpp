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
            for(int i = 0; i < 3; i++) iss >> v[i];
            verts_.push_back(v);
        } else if(!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f n;
            for(int i = 0; i < 3; i++) iss >> n[i];
            norms_.push_back(n);
        } else if(!line.compare(0, 2, "vt")) {
            iss >> trash >> trash;
            Vec2f uv;
            float ftrash;

            iss >> uv.x >> uv.y >> ftrash;
            uvs_.push_back(uv);
        } else if (!line.compare(0, 2, "f ")) {
            FaceInfo f[3];
            int idx, uv, norm;
            iss >> trash;

            // v1
            iss >> idx >> trash >> uv >> trash >> norm;
            f[0].vertIndex = idx - 1; // in wavefront obj all indices start at 1, not zero
            f[0].uvIndex = uv - 1; // in wavefront obj all indices start at 1, not zero
            f[0].normIndex = norm - 1;

            // v2
            iss >> idx >> trash >> uv >> trash >> norm;
            f[1].vertIndex = idx - 1; // in wavefront obj all indices start at 1, not zero
            f[1].uvIndex = uv - 1; // in wavefront obj all indices start at 1, not zero
            f[1].normIndex = norm - 1;

            // v3
            iss >> idx >> trash >> uv >> trash >> norm;
            f[2].vertIndex = idx - 1; // in wavefront obj all indices start at 1, not zero
            f[2].uvIndex = uv - 1; // in wavefront obj all indices start at 1, not zero
            f[2].normIndex = norm - 1;
   
            faces_.push_back(std::make_tuple(f[0], f[1], f[2]));
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uvs_.size() << " vn# " << norms_.size() << std::endl;

    calculateTangent();
}

Model::~Model() {
}

void Model::calculateTangent()
{
    tangents_.resize(norms_.size(), Vec3f(0, 0, 0));

    FaceInfo face[3];
    for(auto& f : faces_) {
        std::tie(face[0], face[1], face[2]) = f;
        
        Vec2f uv1 = uvs_[face[0].uvIndex];
        Vec2f uv2 = uvs_[face[1].uvIndex];
        Vec2f uv3 = uvs_[face[2].uvIndex];

        Vec3f pos1 = verts_[face[0].vertIndex];
        Vec3f pos2 = verts_[face[1].vertIndex];
        Vec3f pos3 = verts_[face[2].vertIndex];

        Vec3f e1 = pos2 - pos1;
        Vec3f e2 = pos3 - pos1;
        Vec2f dUV1 = uv2 - uv1;
        Vec2f dUV2 = uv3 - uv1;

        float f = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);

        Vec3f t;
        t.x = f * (dUV2.y * e1.x - dUV1.y * e2.x);
        t.y = f * (dUV2.y * e1.y - dUV1.y * e2.y);
        t.z = f * (dUV2.y * e1.z - dUV1.y * e2.z);

        int index = face[0].normIndex;
        tangents_[index] = tangents_[index] + t;
        index = face[1].normIndex;
        tangents_[index] = tangents_[index] + t;
        index = face[2].normIndex;
        tangents_[index] = tangents_[index] + t;
    }

    for(auto& f : tangents_) {
        f.normalize();
    }
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

Vec3f Model::normal(int i)
{
    return norms_[i];
}

Vec3f Model::tangent(int i)
{
    return tangents_[i];
}
