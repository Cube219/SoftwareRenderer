#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include <vector>

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
    Vec3f m2v(Matrix m)
    {
        return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
    }

    Matrix v2m(Vec3f v)
    {
        Matrix m(4, 1);
        m[0][0] = v.x;
        m[1][0] = v.y;
        m[2][0] = v.z;
        m[3][0] = 1.0f;

        return m;
    }

    Vec3f GetBarycentricCoord(Vec3f a, Vec3f b, Vec3f c, Vec3f p);
    Matrix GetLookAt(Vec3f eye, Vec3f center, Vec3f up);

    int mWidth;
    int mHeight;
    TGAImage mRenderTarget;
    std::vector<float> mZBuffer;
    Matrix mSumTransform;
};
