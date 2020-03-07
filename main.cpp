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
Vec3f camera(3, 3, 5);
Vec3f light(1, 1, 1);

Renderer renderer(width, height);

class GouraudShader : public IShader
{
public:
    GouraudShader(Model& m, TGAImage& t) : IShader(m),
        texture(t)
    {}

    virtual Vec4f vertex(FaceInfo f, int nthVertex) override
    {
        uvs.set_col(nthVertex, model.uv(f.uvIndex));

        float t = model.normal(f.normIndex) * light;
        if(t < 0.0f) t = 0.0f;
        intensities[nthVertex] = t;
        return sumTransform * embed<4>(model.vert(f.vertIndex));
    }
    virtual bool fragment(Vec3f bar, TGAColor& result) override
    {
        Vec2f uv = uvs * bar;
        float intensity = intensities * bar;
        if(intensity < 0.0f) intensity = 0.0f;
        if(intensity > 1.0f) intensity = 1.0f;
        result = texture.get(int(uv.x * texture.get_width()), int(uv.y * texture.get_height()));
        result.r *= intensity;
        result.g *= intensity;
        result.b *= intensity;

        return false;
    }

    Matrix sumTransform;
    Matrix sumTransfomInvertTranspose;
    Vec3f intensities;
    mat<2, 3, float> uvs;
    TGAImage& texture;
};

int main(void)
{
    renderer.SetCamera(Vec3f(1, 1, 4), Vec3f(0, 0, 0), Vec3f(0, 1, 0));

    Model m = Model("obj/african_head.obj");

    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();

    GouraudShader shader(m, texture);

    shader.sumTransform = renderer.GetTransform();
    shader.sumTransfomInvertTranspose = renderer.GetTransform().invert_transpose();

    renderer.DrawModel(m, shader);

    renderer.WriteRenderTarget();

    return 0;
}
