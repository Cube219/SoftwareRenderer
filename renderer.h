#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include <vector>

class IShader
{
    virtual ~IShader() {}
    virtual Vec3i vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor color) = 0; 
};

class Renderer
{
public:
    Renderer(int width, int height);

    void SetCamera(Vec3f eye, Vec3f center, Vec3f up);

    Matrix GetTransform() const { return mSumTransform; }

    void DrawLine(Vec2i l1, Vec2i l2, TGAColor color);
    void DrawTriangle(Vec3f t1, Vec3f t2, Vec3f t3, TGAImage& texture, Vec2f uv1, Vec2f uv2, Vec2f uv3);
    void DrawTriangle2(Vec3f t1, Vec3f t2, Vec3f t3, TGAImage& texture, Vec2f uv1, Vec2f uv2, Vec2f uv3);

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
