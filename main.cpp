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

        return sumTransform * embed<4>(model.vert(f.vertIndex));
    }

    virtual bool fragment(Vec3f bar, TGAColor& result) override
    {
        Vec2f uv = uvs * bar;

        TGAColor normal = normalMap.get(int(uv.x * normalMap.get_width()), int(uv.y * normalMap.get_height()));
        Vec3f n;
        n.x = (float)normal.r / 255 * 2 - 1;
        n.y = (float)normal.g / 255 * 2 - 1;
        n.z = (float)normal.b / 255 * 2 - 1;

        Matrix tangentSpaceToObjectSpace;

        n = proj<3>(projViewInvertTranspose * embed<4>(n)).normalize();
        Vec3f l = proj<3>(projView * embed<4>(light)).normalize();

        float intensity = n * l;
        if(intensity < 0.0f) intensity = 0.0f;
        if(intensity > 1.0f) intensity = 1.0f;
        result = texture.get(int(uv.x * texture.get_width()), int(uv.y * texture.get_height()));
        result.r *= intensity;
        result.g *= intensity;
        result.b *= intensity;

        return false;
    }

    Matrix sumTransform;
    Matrix projView;
    Matrix projViewInvertTranspose;
    mat<2, 3, float> uvs;
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
    normalMap.read_tga_file("obj/african_head_nm.tga");
    normalMap.flip_vertically();

    GouraudShader shader(m, texture, normalMap);

    shader.sumTransform = renderer.GetSumTransform();
    shader.projView = renderer.GetProjView();
    shader.projViewInvertTranspose = shader.projView.invert_transpose();

    renderer.DrawModel(m, shader);

    renderer.WriteRenderTarget();

    return 0;
}
