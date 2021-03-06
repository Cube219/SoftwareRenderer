﻿#include <iostream>
#include <string>
#include <limits>

#include "tgaimage.h"
#include "model.h"
#include "renderer.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width = 800;
const int height = 800;
const int depth = 250;
Vec3f camera(3, 5, 5);
Vec3f light(1, 1, 1);

Renderer renderer(width, height);

int rMax, gMax, bMax;

class GouraudShader : public IShader
{
public:
    GouraudShader(Model& m, TGAImage& t, TGAImage& s, TGAImage& n) : IShader(m),
        texture(t), spec(s), normalMap(n)
    {}

    virtual Vec4f vertex(FaceInfo f, int nthVertex) override
    {
        uvs.set_col(nthVertex, model.uv(f.uvIndex));
        normals.set_col(nthVertex, proj<3>(projViewInvertTranspose * embed<4>(model.normal(f.normIndex), 0.0f)));
        tangents.set_col(nthVertex, proj<3>(projViewInvertTranspose * embed<4>(model.tangent(f.normIndex), 0.0f)));

        Vec4f resV = projView * embed<4>(model.vert(f.vertIndex));

        return resV;
    }

    virtual bool fragment(Vec3f bar, TGAColor& result) override
    {
        Vec2f uv = uvs * bar;
        Vec3f normal = (normals * bar).normalize();
        Vec3f tangent = (tangents * bar).normalize();
        Vec3f bitangent = cross(normal, tangent).normalize();

        mat<3, 3, float> TBN;
        TBN.set_col(0, tangent);
        TBN.set_col(1, bitangent);
        TBN.set_col(2, normal);

        TGAColor normalColor = normalMap.get(int(uv.x * normalMap.get_width()), int(uv.y * normalMap.get_height()));
        Vec3f n;
        n.x = (float)normalColor.r / 255 * 2 - 1;
        n.y = (float)normalColor.g / 255 * 2 - 1;
        n.z = (float)normalColor.b / 255 * 2 - 1;
        n = (TBN * n).normalize();

        // Diffuse
        float diffuse = n * light;
        if(diffuse < 0.0f) diffuse = 0.0f;
        if(diffuse > 1.0f) diffuse = 1.0f;
        result = texture.get(int(uv.x * texture.get_width()), int(uv.y * texture.get_height()));

        // Specular
        Vec3f r = (n * (n * light) * 2) - light;
        r.normalize();
        float rz = r.z;
        if(rz < 0) rz = 0;
        Vec3f s;

        s.x = pow(rz, (float)spec.get(int(uv.x * spec.get_width()), int(uv.y * spec.get_height())).r);
        s.y = pow(rz, (float)spec.get(int(uv.x * spec.get_width()), int(uv.y * spec.get_height())).g);
        s.z = pow(rz, (float)spec.get(int(uv.x * spec.get_width()), int(uv.y * spec.get_height())).b);

        result.r *= (diffuse + s.x * 0.6f);
        result.g *= (diffuse + s.y * 0.6f);
        result.b *= (diffuse + s.z * 0.6f);

        return false;
    }

    Matrix projView;
    Matrix projViewInvertTranspose;
    mat<2, 3, float> uvs;
    mat<3, 3, float> normals;
    mat<3, 3, float> tangents;
    TGAImage& texture;
    TGAImage& spec;
    TGAImage& normalMap;
};

int main(void)
{
    renderer.SetCamera(Vec3f(1, 3, 6), Vec3f(0, 0, 0), Vec3f(0, 1, 0));
    // renderer.SetCamera(Vec3f(0, 0, 6), Vec3f(0, 0, 0), Vec3f(0, 1, 0));

    Model m = Model("obj/african_head.obj");
    // Model m = Model("obj/diablo3_pose.obj");

    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    // texture.read_tga_file("obj/diablo3_pose_diffuse.tga");
    texture.flip_vertically();

    TGAImage spec;
    spec.read_tga_file("obj/african_head_spec.tga");
    // spec.read_tga_file("obj/diablo3_pose_spec.tga");
    spec.flip_vertically();

    TGAImage normalMap;
    normalMap.read_tga_file("obj/african_head_nm_tangent.tga");
    // normalMap.read_tga_file("obj/diablo3_pose_nm_tangent.tga");
    normalMap.flip_vertically();

    GouraudShader shader(m, texture, spec, normalMap);

    shader.projView = renderer.GetProjView();
    light = proj<3>(shader.projView * embed<4>(light, 0.0f)).normalize();
    shader.projViewInvertTranspose = shader.projView.invert_transpose();

    renderer.DrawModel(m, shader);

    renderer.WriteRenderTarget();

    std::cout << rMax << " " << gMax << " " << bMax << std::endl;

    return 0;
}
