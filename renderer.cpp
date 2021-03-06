﻿#include "renderer.h"

Renderer::Renderer(int width, int height) :
    mWidth(width), mHeight(height),
    mRenderTarget(width, height, TGAImage::RGB),
    mZBuffer(width * height)
{
}

void Renderer::SetCamera(Vec3f eye, Vec3f center, Vec3f up)
{
    Matrix proj = Matrix::identity();
    proj[3][2] = -1 / (eye-center).norm();

    Matrix view = GetLookAt(eye, center, up);

    mViewPort = Matrix::identity();
    mViewPort[0][3] = mWidth / 2.0f;
    mViewPort[1][3] = mHeight / 2.0f;
    mViewPort[2][3] = 250 / 2.0f;
    mViewPort[0][0] = mWidth / 2.0f;
    mViewPort[1][1] = -mHeight / 2.0f;
    mViewPort[2][2] = 250 / 2.0f;

    mProjView = proj * view;
}

void Renderer::DrawLine(Vec2i l1, Vec2i l2, TGAColor color)
{
    bool isSteep = false;
    if(std::abs(l1.x - l2.x) < std::abs(l1.y - l2.y)) {
        std::swap(l1.x, l1.y);
        std::swap(l2.x, l2.y);
        isSteep = true;
    }

    if(l1.x > l2.x) {
        std::swap(l1, l2);
    }

    int dx = l2.x - l1.x;
    int dy = l2.y - l1.y;

    // float yGap = std::abs(dy / (float)dx);
    // float currentYGap = 0;
    int yGap2 = std::abs(dy);
    int currentYGap2 = 0;

    int y = l1.y;
    int yIncrement = (l2.y > l1.y ? 1 : -1);

    for(int x = l1.x; x <= l2.x; x++) {
        if(isSteep) {
            mRenderTarget.set(y, x, color);
        } else {
            mRenderTarget.set(x, y, color);
        }
        /*
        currentYGap += yGap;
        if(currentYGap > 1.0f) {
            y += yIncrement;
            currentYGap -= 1;
        }
        */
        currentYGap2 += yGap2;
        if(currentYGap2 > dx) {
            y += yIncrement;
            currentYGap2 -= dx;
        }
    }
}

void Renderer::DrawTriangle(Vec3f t1, Vec3f t2, Vec3f t3, IShader& shader)
{
    Vec3f prevT1 = t1;
    Vec3f prevT2 = t2;
    Vec3f prevT3 = t3;

    if(t1.y > t2.y) {
        std::swap(t1, t2);
    }
    if(t1.y > t3.y) {
        std::swap(t1, t3);
    }
    if(t2.y > t3.y) {
        std::swap(t2, t3);
    }

    Vec2i t1i = Vec2i((int)t1.x, (int)t1.y);
    Vec2i t2i = Vec2i((int)t2.x, (int)t2.y);
    Vec2i t3i = Vec2i((int)t3.x, (int)t3.y);

    // Bottom half of triangle (t1-t3, t1-t2)
    if(t1i.y != t2i.y) {
        for(int y = t1i.y; y <= t2i.y; y++) {
            if(y < 0) continue;
            if(y >= mHeight) break;

            float a = (float)(y - t1i.y) / (t3i.y - t1i.y);
            float b = (float)(y - t1i.y) / (t2i.y - t1i.y);

            Vec2i begin = t1i + (t3i - t1i) * a;
            Vec2i end = t1i + (t2i - t1i) * b;
            float zBegin = t1.z + (t3.z - t1.z) * a;
            float zEnd = t1.z + (t2.z - t1.z) * b;

            if(begin.x == end.x) continue;

            if(begin.x > end.x) {
                std::swap(begin, end);
            }
            for(int x = begin.x; x <= end.x; x++) {
                if(x < 0) {
                    x = 0;
                    continue;
                }
                if(x >= mWidth) break;

                float t = (float)(x - begin.x) / (end.x - begin.x);

                Vec2i p = begin + (end - begin) * t;
                float z = zBegin + (zEnd - zBegin) * t;

                int zBufIndex = x + y * mWidth;
                if(mZBuffer[zBufIndex] < z) {
                    mZBuffer[zBufIndex] = z;

                    Vec3f s = GetBarycentricCoord(prevT1, prevT2, prevT3, Vec3f(p.x, p.y, 0));

                    TGAColor c;
                    bool isDiscard = shader.fragment(s, c);

                    if(isDiscard == false)
                        mRenderTarget.set(x, y, c);
                }
            }
        }
    }

    // Top half of triangle (t1-t3, t2-t3)
    if(t2i.y != t3i.y) {
        for(int y = t2i.y; y <= t3i.y; y++) {
            if(y < 0) continue;
            if(y >= mHeight) break;

            float a = (float)(y - t1i.y) / (t3i.y - t1i.y);
            float b = (float)(y - t2i.y) / (t3i.y - t2i.y);

            Vec2i begin = t1i + (t3i - t1i) * a;
            Vec2i end = t2i + (t3i - t2i) * b;
            float zBegin = t1.z + (t3.z - t1.z) * a;
            float zEnd = t2.z + (t3.z - t2.z) * b;

            if(begin.x == end.x) continue;

            if(begin.x > end.x) {
                std::swap(begin, end);
            }
            for(int x = begin.x; x <= end.x; x++) {
                if(x < 0) {
                    x = 0;
                    continue;
                }
                if(x >= mWidth) break;

                float t = (float)(x - begin.x) / (end.x - begin.x);

                Vec2i p = begin + (end - begin) * t;
                float z = zBegin + (zEnd - zBegin) * t;

                int zBufIndex = x + y * mWidth;
                if(mZBuffer[zBufIndex] < z) {
                    mZBuffer[zBufIndex] = z;

                    Vec3f s = GetBarycentricCoord(prevT1, prevT2, prevT3, Vec3f(p.x, p.y, 0));

                    TGAColor c;
                    bool isDiscard = shader.fragment(s, c);

                    if(isDiscard == false)
                        mRenderTarget.set(x, y, c);
                }
            }
        }
    }
}

void Renderer::DrawTriangle2(Vec3f t1, Vec3f t2, Vec3f t3, IShader& shader)
{
    Vec2i t1i = Vec2i((int)t1.x, (int)t1.y);
    Vec2i t2i = Vec2i((int)t2.x, (int)t2.y);
    Vec2i t3i = Vec2i((int)t3.x, (int)t3.y);

    Vec2i boxMin = Vec2i(t1i.x, t1i.y);
    Vec2i boxMax = Vec2i(t1i.x, t1i.y);

    if(boxMin.x > t2i.x) boxMin.x = t2i.x;
    if(boxMax.x < t2i.x) boxMax.x = t2i.x;
    if(boxMin.y > t2i.y) boxMin.y = t2i.y;
    if(boxMax.y < t2i.y) boxMax.y = t2i.y;
    if(boxMin.x > t3i.x) boxMin.x = t3i.x;
    if(boxMax.x < t3i.x) boxMax.x = t3i.x;
    if(boxMin.y > t3i.y) boxMin.y = t3i.y;
    if(boxMax.y < t3i.y) boxMax.y = t3i.y;

    if(boxMin.x < 0) boxMin.x = 0;
    if(boxMin.y < 0) boxMin.y = 0;
    if(boxMax.x > mWidth - 1) boxMax.x = mWidth - 1;
    if(boxMax.y > mHeight - 1) boxMax.y = mHeight - 1;

    Vec2i p;
    for(p.x = boxMin.x; p.x <= boxMax.x; p.x++) {
        for(p.y = boxMin.y; p.y <= boxMax.y; p.y++) {
            Vec3f s = GetBarycentricCoord(t1, t2, t3, Vec3f((float)p.x, (float)p.y, 0));
            if(s.x < 0 || s.y < 0 || s.z < 0) continue;

            float z = t1.z * s.x;
            z += t2.z * s.y;
            z += t3.z * s.z;

            int zBufIndex = p.x + p.y * mWidth;
            if(mZBuffer[zBufIndex] < z) {
                mZBuffer[zBufIndex] = z;

                TGAColor c;
                bool isDiscard = shader.fragment(s, c);

                if(isDiscard == false)
                    mRenderTarget.set(p.x, p.y, c);
            }
        }
    }
}

void Renderer::DrawModel(Model& model, IShader& shader)
{
    FaceInfo face[3];
    for(int i = 0; i < model.nfaces(); i++) {
        std::tie(face[0], face[1], face[2]) = model.face(i);

        Vec4f screen[3];
        for(int j = 0; j < 3; j++) {
            screen[j] = shader.vertex(face[j], j);
            screen[j] = mViewPort * screen[j];
        }
        DrawTriangle(embed<3>(screen[0]), embed<3>(screen[1]), embed<3>(screen[2]), shader);
    }
}

Vec3f Renderer::GetBarycentricCoord(Vec3f a, Vec3f b, Vec3f c, Vec3f p)
{
    Vec3f s1 = Vec3f(c.x - a.x, b.x - a.x, a.x - p.x);
    Vec3f s2 = Vec3f(c.y - a.y, b.y - a.y, a.y - p.y);

    Vec3f u = cross(s1, s2);

    /* `pts` and `P` has integer value as coordinates
        so `abs(u[2])` < 1 means `u[2]` is 0, that means
        triangle is degenerate, in this case return something with negative coordinates */
    if(std::abs(u.z) <= 1e-2) return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

Matrix Renderer::GetLookAt(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = eye - center;
    z.normalize();
    Vec3f x = cross(up, z);
    x.normalize();
    Vec3f y = cross(z, x);
    y.normalize();

    Matrix mInv = Matrix::identity();
    mInv[0][0] = x.x;
    mInv[0][1] = x.y;
    mInv[0][2] = x.z;
    mInv[1][0] = y.x;
    mInv[1][1] = y.y;
    mInv[1][2] = y.z;
    mInv[2][0] = z.x;
    mInv[2][1] = z.y;
    mInv[2][2] = z.z;

    Matrix move = Matrix::identity();
    move[0][3] = -center.x;
    move[1][3] = -center.y;
    move[2][3] = -center.z;

    return mInv * move;
}
