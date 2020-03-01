#include <iostream>
#include <string>

#include "tgaimage.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width = 800;
const int height = 800;
const int depth = 250;
Vec3f camera(0, 0, 3);

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

void drawLine(int x1, int y1, int x2, int y2, TGAImage& image, TGAColor color)
{
    bool isSteep = false;
    if(std::abs(x1 - x2) < std::abs(y1 - y2)) {
        std::swap(x1, y1);
        std::swap(x2, y2);
        isSteep = true;
    }

    if(x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    int dx = x2 - x1;
    int dy = y2 - y1;

    // float yGap = std::abs(dy / (float)dx);
    // float currentYGap = 0;
    int yGap2 = std::abs(dy);
    int currentYGap2 = 0;

    int y = y1;
    int yIncrement = (y2 > y1 ? 1 : -1);

    for(int x = x1; x <= x2; x++) {
        if(isSteep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
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

Vec3f cross(Vec3f a, Vec3f b)
{
    return Vec3f(
        a.y * b.z - a.z * b.y,
        -(a.x * b.z - a.z * b.x),
        a.x * b.y - a.y * b.x
    );
}

Vec3f getBarycentricCoord(Vec3f a, Vec3f b, Vec3f c, Vec3f p)
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

void drawTriangle(Vec3f t1, Vec3f t2, Vec3f t3, float* pZBuf, TGAImage& image, TGAColor color, TGAImage* pTexture = nullptr,
    Vec2f uv1 = {}, Vec2f uv2 = {}, Vec2f uv3 = {}, float intensity = 0.0f)
{
    if(t1.y > t2.y) {
        std::swap(t1, t2);
        std::swap(uv1, uv2);
    }
    if(t1.y > t3.y) {
        std::swap(t1, t3);
        std::swap(uv1, uv3);
    }
    if(t2.y > t3.y) {
        std::swap(t2, t3);
        std::swap(uv2, uv3);
    }

    Vec2i t1i = Vec2i((int)t1.x, (int)t1.y);
    Vec2i t2i = Vec2i((int)t2.x, (int)t2.y);
    Vec2i t3i = Vec2i((int)t3.x, (int)t3.y);

    // Bottom half of triangle (t1-t3, t1-t2)
    if(t1i.y != t2i.y) {
        for(int y = t1i.y; y <= t2i.y; y++) {
            if(y < 0) continue;
            if(y > height) break;

            float a = (float)(y - t1i.y) / (t3i.y - t1i.y);
            float b = (float)(y - t1i.y) / (t2i.y - t1i.y);

            Vec2i begin = t1i + (t3i - t1i) * a;
            Vec2i end = t1i + (t2i - t1i) * b;
            float zBegin = t1.z + (t3.z - t1.z) * a;
            float zEnd = t1.z + (t2.z - t1.z) * b;
            Vec2f uvBegin = uv1 + (uv3 - uv1) * a;
            Vec2f uvEnd = uv1 + (uv2 - uv1) * b;

            if(begin.x == end.x) continue;

            if(begin.x > end.x) {
                std::swap(begin, end);
                std::swap(uvBegin, uvEnd);
            }
            for(int x = begin.x; x <= end.x; x++) {
                if(x < 0) {
                    x = 0;
                    continue;
                }
                if(x > width) break;

                float t = (float)(x - begin.x) / (end.x - begin.x);

                Vec2i p = begin + (end - begin) * t;
                float z = zBegin + (zEnd - zBegin) * t;
                Vec2f uvP = uvBegin + (uvEnd - uvBegin) * t;

                int zBufIndex = x + y * width;
                if(pZBuf[zBufIndex] < z) {
                    pZBuf[zBufIndex] = z;

                    TGAColor c = color;
                    if(pTexture != nullptr) {
                        c = pTexture->get(int(uvP.x * pTexture->get_width()), int(uvP.y * pTexture->get_height()));
                        c.r *= intensity;
                        c.g *= intensity;
                        c.b *= intensity;
                    }
                    image.set(x, y, c);
                }
            }
        }
    }

    // Top half of triangle (t1-t3, t2-t3)
    if(t2i.y != t3i.y) {
        for(int y = t2i.y; y <= t3i.y; y++) {
            if(y < 0) continue;
            if(y > height) break;

            float a = (float)(y - t1i.y) / (t3i.y - t1i.y);
            float b = (float)(y - t2i.y) / (t3i.y - t2i.y);

            Vec2i begin = t1i + (t3i - t1i) * a;
            Vec2i end = t2i + (t3i - t2i) * b;
            float zBegin = t1.z + (t3.z - t1.z) * a;
            float zEnd = t2.z + (t3.z - t2.z) * b;
            Vec2f uvBegin = uv1 + (uv3 - uv1) * a;
            Vec2f uvEnd = uv2 + (uv3 - uv2) * b;

            if(begin.x == end.x) continue;

            if(begin.x > end.x) {
                std::swap(begin, end);
                std::swap(uvBegin, uvEnd);
            }
            for(int x = begin.x; x <= end.x; x++) {
                if(x < 0) {
                    x = 0;
                    continue;
                }
                if(x > width) break;

                float t = (float)(x - begin.x) / (end.x - begin.x);

                Vec2i p = begin + (end - begin) * t;
                float z = zBegin + (zEnd - zBegin) * t;
                Vec2f uvP = uvBegin + (uvEnd - uvBegin) * t;

                int zBufIndex = x + y * width;
                if(pZBuf[zBufIndex] < z) {
                    pZBuf[zBufIndex] = z;

                    TGAColor c = color;
                    if(pTexture != nullptr) {
                        c = pTexture->get(int(uvP.x * pTexture->get_width()), int(uvP.y * pTexture->get_height()));
                        c.r *= intensity;
                        c.g *= intensity;
                        c.b *= intensity;
                    }
                    image.set(x, y, c);
                }
            }
        }
    }
}

void drawTriangle2(Vec3f t1, Vec3f t2, Vec3f t3, float* pZBuf, TGAImage& image, TGAColor color,
    TGAImage* pTexture = nullptr, Vec2f uv1 = {}, Vec2f uv2 = {}, Vec2f uv3 = {}, float intensity = 0.0f)
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
    int imageWidth = image.get_width() - 1;
    int imageHeight = image.get_height() - 1;
    if(boxMax.x > imageWidth) boxMax.x = imageWidth;
    if(boxMax.y > imageHeight) boxMax.y = imageHeight;

    Vec2i p;
    for(p.x = boxMin.x; p.x <= boxMax.x; p.x++) {
        for(p.y = boxMin.y; p.y <= boxMax.y; p.y++) {
            Vec3f s = getBarycentricCoord(t1, t2, t3, Vec3f((float)p.x, (float)p.y, 0));
            if(s.x < 0 || s.y < 0 || s.z < 0) continue;

            float z = t1.z * s.x;
            z += t2.z * s.y;
            z += t3.z * s.z;
            
            int zBufIndex = p.x + p.y * width;
            if(pZBuf[zBufIndex] < z) {
                pZBuf[zBufIndex] = z;

                TGAColor c = color;
                if(pTexture != nullptr) {
                    Vec2f uv;
                    uv.x = uv1.x * s.x;
                    uv.x += uv2.x * s.y;
                    uv.x += uv3.x * s.z;
                    uv.y = uv1.y * s.x;
                    uv.y += uv2.y * s.y;
                    uv.y += uv3.y * s.z;
                    
                    c = pTexture->get(int(uv.x * pTexture->get_width()), int(uv.y * pTexture->get_height()));
                    c.r *= intensity;
                    c.g *= intensity;
                    c.b *= intensity;
                }

                image.set(p.x, p.y, c);
            }
        }
    }
}

void drawModel(float* pZBuf, TGAImage& image, TGAImage& texture)
{
    Model m = Model("obj/african_head.obj");

    Vec3f lightDir = Vec3f(0, 0, -1);

    Matrix proj = Matrix::identity(4);
    proj[3][2] = -1 / camera.z;
    Matrix view = Matrix::identity(4);
    view[0][3] = width / 2.0f;
    view[1][3] = height / 2.0f;
    view[2][3] = depth / 2.0f;
    view[0][0] = width / 2.0f;
    view[1][1] = height / 2.0f;
    view[2][2] = depth / 2.0f;

    Matrix viewproj = view * proj;

    FaceInfo face[3];
    for(int i = 0; i < m.nfaces(); i++) {
        std::tie(face[0], face[1], face[2]) = m.face(i);

        Vec3f screen[3];
        Vec3f world[3];
        for(int j = 0; j < 3; j++) {
            Vec3f v = m.vert(face[j].vertIndex);

            screen[j] = m2v(viewproj * v2m(v));
            world[j] = v;
        }

        Vec3f n = (world[2] - world[1]) ^ (world[2] - world[0]);
        n.normalize();
        float intensity = n * lightDir;
        
        Vec2f uvs[3];
        uvs[0] = m.uv(face[0].uvIndex);
        uvs[1] = m.uv(face[1].uvIndex);
        uvs[2] = m.uv(face[2].uvIndex);

        if(intensity > 0) {
            // drawTriangle2(screen[0], screen[1], screen[2], pZBuf, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
            drawTriangle(screen[0], screen[1], screen[2], pZBuf, image, {}, &texture, uvs[0], uvs[1], uvs[2], intensity);
        }
    }
}

int main(void)
{
    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();

    float* zBuffer = new float[width * height];
    for(int i = 0; i < width * height; i++) {
        zBuffer[i] = -std::numeric_limits<float>::max();
    }
    TGAImage image(width, height, TGAImage::RGB);

    Vec3f t0[3] = { Vec3f(10, 70, 0),   Vec3f(50, 160, 0),  Vec3f(70, 80, 0) };
    Vec3f t1[3] = { Vec3f(180, 50, 1),  Vec3f(150, 1, 1),   Vec3f(70, 180, 1) };
    Vec3f t2[3] = { Vec3f(180, 150, -1), Vec3f(120, 160, -1), Vec3f(130, 180, -1) };
    drawTriangle(t0[0], t0[1], t0[2], zBuffer, image, red);
    drawTriangle(t1[0], t1[1], t1[2], zBuffer, image, white);
    drawTriangle(t2[0], t2[1], t2[2], zBuffer, image, green);

    drawTriangle2(Vec3f(10, 10, 2), Vec3f(100, 30, 2), Vec3f(190, 160, 2), zBuffer, image, green);

    drawModel(zBuffer, image, texture);

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete[] zBuffer;

    return 0;
}
