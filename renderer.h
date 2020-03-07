#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include <vector>

struct VertexAtribute
{
    Vec3f vert;
    Vec2f uv;
};

class IShader
{
public:
    IShader(Model& m) : model(m)
    {}
    virtual ~IShader() {}
    virtual Vec4f vertex(FaceInfo f, int nthVertex) = 0;
    virtual bool fragment(Vec3f bar, TGAColor& result) = 0;

    Model& model;
};

class Renderer
{
public:
    Renderer(int width, int height);

    void SetCamera(Vec3f eye, Vec3f center, Vec3f up);

    Matrix GetTransform() const { return mSumTransform; }

    void DrawLine(Vec2i l1, Vec2i l2, TGAColor color);
    void DrawTriangle(Vec3f t1, Vec3f t2, Vec3f t3, IShader& shader);
    void DrawTriangle2(Vec3f t1, Vec3f t2, Vec3f t3, IShader& shader);

    void DrawModel(Model& model, IShader& shader);

    void WriteRenderTarget()
    {
        mRenderTarget.write_tga_file("output.tga");
    }

private:
    Vec3f GetBarycentricCoord(Vec3f a, Vec3f b, Vec3f c, Vec3f p);
    Matrix GetLookAt(Vec3f eye, Vec3f center, Vec3f up);

    int mWidth;
    int mHeight;
    TGAImage mRenderTarget;
    std::vector<float> mZBuffer;
    Matrix mSumTransform;
};
