#include <iostream>

#include "tgaimage.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width = 800;
const int height = 800;

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
    if(std::abs(u.z) < 1e-2) return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void drawTriangle(Vec3f t1, Vec3f t2, Vec3f t3, float* pZBuf, TGAImage& image, TGAColor color)
{
    if(t1.y > t2.y) std::swap(t1, t2);
    if(t1.y > t3.y) std::swap(t1, t3);
    if(t2.y > t3.y) std::swap(t2, t3);

    // Bottom half of triangle (t1-t3, t1-t2)
    for(int y = t1.y; y <= t2.y; y++) {
        float a = (float)(y - t1.y) / (t3.y - t1.y);
        float b = (float)(y - t1.y) / (t2.y - t1.y);

        float aX = t1.x + (t3.x - t1.x) * a;
        float bX = t1.x + (t2.x - t1.x) * b;

        if(aX > bX) std::swap(aX, bX);
        for(int x = (int)aX; x <= (int)bX; x++) {
            Vec3f s = getBarycentricCoord(t1, t2, t3, Vec3f(x, y, 0));

            float z = t1.z * s.x;
            z += t2.z * s.y;
            z += t3.z * s.z;

            int zBufIndex = x + y * width;
            if(pZBuf[zBufIndex] < z) {
                pZBuf[zBufIndex] = z;
                image.set(x, y, color);
            }
        }
    }

    // Top half of triangle (t1-t3, t2-t3)
    for(int y = t2.y; y <= t3.y; y++) {
        float a = (float)(y - t1.y) / (t3.y - t1.y);
        float b = (float)(y - t2.y) / (t3.y - t2.y);

        float aX = t1.x + (t3.x - t1.x) * a;
        float bX = t2.x + (t3.x - t2.x) * b;

        if(aX > bX) std::swap(aX, bX);
        for(int x = (int)aX; x <= (int)bX; x++) {
            Vec3f s = getBarycentricCoord(t1, t2, t3, Vec3f(x, y, 0));

            float z = t1.z * s.x;
            z += t2.z * s.y;
            z += t3.z * s.z;

            int zBufIndex = x + y * width;
            if(pZBuf[zBufIndex] < z) {
                pZBuf[zBufIndex] = z;
                image.set(x, y, color);
            }
        }
    }
}

void drawTriangle2(Vec3f t1, Vec3f t2, Vec3f t3, float* pZBuf, TGAImage& image, TGAColor color)
{
    // Vec3f tri[3] = { t1, t2, t3 };
    Vec2f boxMin = Vec2f(t1.x, t1.y);
    Vec2f boxMax = Vec2f(t1.x, t1.y);

    if(boxMin.x > t2.x) boxMin.x = t2.x;
    if(boxMax.x < t2.x) boxMax.x = t2.x;
    if(boxMin.y > t2.y) boxMin.y = t2.y;
    if(boxMax.y < t2.y) boxMax.y = t2.y;
    if(boxMin.x > t3.x) boxMin.x = t3.x;
    if(boxMax.x < t3.x) boxMax.x = t3.x;
    if(boxMin.y > t3.y) boxMin.y = t3.y;
    if(boxMax.y < t3.y) boxMax.y = t3.y;

    if(boxMin.x < 0) boxMin.x = 0;
    if(boxMin.y < 0) boxMin.y = 0;
    int imageWidth = image.get_width() - 1;
    int imageHeight = image.get_height() - 1;
    if(boxMax.x > imageWidth) boxMax.x = imageWidth;
    if(boxMax.y > imageHeight) boxMax.y = imageHeight;

    Vec3f p;
    for(p.x = boxMin.x; p.x <= boxMax.x; p.x++) {
        for(p.y = boxMin.y; p.y <= boxMax.y; p.y++) {
            Vec3f s = getBarycentricCoord(t1, t2, t3, p);
            if(s.x < 0 || s.y < 0 || s.z < 0) continue;

            p.z = t1.z * s.x;
            p.z += t2.z * s.y;
            p.z += t3.z * s.z;
            
            int zBufIndex = int(p.x) + int(p.y * width);
            if(pZBuf[zBufIndex] < p.z) {
                pZBuf[zBufIndex] = p.z;
                image.set(p.x, p.y, color);
            }
        }
    }
}

void drawModel(float* pZBuf, TGAImage& image)
{
    Model m = Model("obj/african_head.obj");

    Vec3f lightDir = Vec3f(0, 0, -1);

    for(int i = 0; i < m.nfaces(); i++) {
        std::vector<int> face = m.face(i);

        Vec3f screen[3];
        Vec3f world[3];
        for(int j = 0; j < 3; j++) {
            Vec3f v = m.vert(face[j]);

            screen[j] = Vec3f(int((v.x + 1.0f) * (width / 2)), int((v.y + 1.0f) * (height / 2)), v.z);
            world[j] = v;
        }

        Vec3f n = (world[2] - world[1]) ^ (world[2] - world[0]);
        n.normalize();
        float intensity = n * lightDir;
        
        if(intensity > 0) {
            drawTriangle(screen[0], screen[1], screen[2], pZBuf, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }
}

int main(void)
{
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

    drawModel(zBuffer, image);

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete[] zBuffer;

    return 0;
}
