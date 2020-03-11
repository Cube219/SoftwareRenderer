#include <iostream>
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

class GouraudShader : public IShader
{
public:
    GouraudShader(Model& m, TGAImage& t, TGAImage& n) : IShader(m),
        texture(t), normalMap(n)
    {}

    virtual Vec4f vertex(FaceInfo f, int nthVertex) override
    {
        uvs.set_col(nthVertex, model.uv(f.uvIndex));
        normals.set_col(nthVertex, proj<3>(projViewInvertTranspose * embed<4>(model.normal(f.normIndex), 0.0f)));

        Vec4f resV = projView * embed<4>(model.vert(f.vertIndex));
        ndc_tri.set_col(nthVertex, proj<3>(resV / resV[3]));

        return resV;
    }

    virtual bool fragment(Vec3f bar, TGAColor& result) override
    {
        Vec2f uv = uvs * bar;
        Vec3f normal = (normals * bar).normalize();

        mat<3, 3, float> A;
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = normal;

        mat<3, 3, float> AI = A.invert();
        
        Vec3f i = AI * Vec3f(uvs[0][1] - uvs[0][0], uvs[0][2] - uvs[0][0], 0);
        Vec3f j = AI * Vec3f(uvs[1][1] - uvs[1][0], uvs[1][2] - uvs[1][0], 0);

        mat<3, 3, float> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, normal);

        TGAColor normalColor = normalMap.get(int(uv.x * normalMap.get_width()), int(uv.y * normalMap.get_height()));
        Vec3f n;
        n.x = (float)normalColor.r / 255 * 2 - 1;
        n.y = (float)normalColor.g / 255 * 2 - 1;
        n.z = (float)normalColor.b / 255 * 2 - 1;
        n = (B * n).normalize();

        float intensity = n * light;
        if(intensity < 0.0f) intensity = 0.0f;
        if(intensity > 1.0f) intensity = 1.0f;
        result = texture.get(int(uv.x * texture.get_width()), int(uv.y * texture.get_height()));
        result.r *= intensity;
        result.g *= intensity;
        result.b *= intensity;

        return false;
    }

    Matrix projView;
    Matrix projViewInvertTranspose;
    mat<2, 3, float> uvs;
    mat<3, 3, float> normals;
    mat<3, 3, float> ndc_tri;
    TGAImage& texture;
    TGAImage& normalMap;
};

int main(void)
{
    renderer.SetCamera(Vec3f(1, 3, 6), Vec3f(0, 0, 0), Vec3f(0, 1, 0));

    Model m = Model("obj/african_head.obj");

    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();

    TGAImage normalMap;
    normalMap.read_tga_file("obj/african_head_nm_tangent.tga");
    normalMap.flip_vertically();

    GouraudShader shader(m, texture, normalMap);

    shader.projView = renderer.GetProjView();
    light = proj<3>(shader.projView * embed<4>(light, 0.0f)).normalize();
    shader.projViewInvertTranspose = shader.projView.invert_transpose();

    renderer.DrawModel(m, shader);

    renderer.WriteRenderTarget();

    return 0;
}
