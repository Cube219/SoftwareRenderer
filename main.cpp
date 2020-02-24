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

void drawTriangle(Vec2i t1, Vec2i t2, Vec2i t3, TGAImage& image, TGAColor color)
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
            image.set(x, y, color);
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
            image.set(x, y, color);
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

Vec3f getBarycenticCoord(Vec2i* pTriangle, Vec2i p)
{
    Vec3f u = cross(
        Vec3f(
            (float)(pTriangle[2].x - pTriangle[0].x),
            (float)(pTriangle[1].x - pTriangle[0].x),
            (float)(pTriangle[0].x - p.x)
        ),
        Vec3f(
            (float)(pTriangle[2].y - pTriangle[0].y),
            (float)(pTriangle[1].y - pTriangle[0].y),
            (float)(pTriangle[0].y - p.y)
        )
    );
    /* `pts` and `P` has integer value as coordinates
       so `abs(u[2])` < 1 means `u[2]` is 0, that means
       triangle is degenerate, in this case return something with negative coordinates */
    if(std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void drawTriangle2(Vec2i t1, Vec2i t2, Vec2i t3, TGAImage& image, TGAColor color)
{
    Vec2i tri[3] = { t1, t2, t3 };
    Vec2i boxMin = t1;
    Vec2i boxMax = t1;

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

    Vec2i p;
    for(p.x = boxMin.x; p.x <= boxMax.x; p.x++) {
        for(p.y = boxMin.y; p.y <= boxMax.y; p.y++) {
            Vec3f s = getBarycenticCoord(tri, p);
            if(s.x < 0 || s.y < 0 || s.z < 0) continue;
            image.set(p.x, p.y, color);
        }
    }
}

void drawModel(TGAImage& image)
{
    Model m = Model("obj/african_head.obj");

    Vec3f lightDir = Vec3f(0, 0, -1);

    for(int i = 0; i < m.nfaces(); i++) {
        std::vector<int> face = m.face(i);

        Vec2i screen[3];
        Vec3f world[3];
        for(int j = 0; j < 3; j++) {
            Vec3f v = m.vert(face[j]);

            screen[j] = Vec2i(int((v.x + 1.0f) * (width / 2)), int((v.y + 1.0f) * (height / 2)));
            world[j] = v;
        }

        Vec3f n = (world[2] - world[1]) ^ (world[2] - world[0]);
        n.normalize();
        float intensity = n * lightDir;
        
        if(intensity > 0) {
            drawTriangle(screen[0], screen[1], screen[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }
}

int main(void)
{

    TGAImage image(width, height, TGAImage::RGB);

    Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
    Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
    Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
    drawTriangle(t0[0], t0[1], t0[2], image, red);
    drawTriangle(t1[0], t1[1], t1[2], image, white);
    drawTriangle(t2[0], t2[1], t2[2], image, green);

    drawTriangle2(Vec2i(10, 10), Vec2i(100, 30), Vec2i(190, 160), image, green);

    drawModel(image);

    image.flip_vertically();
    image.write_tga_file("output.tga");

    return 0;
}
