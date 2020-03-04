#include <iostream>
#include <string>

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

Renderer renderer(width, height);

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

void drawModel(TGAImage& texture)
{
    Model m = Model("obj/african_head.obj");

    Matrix sumTransform = renderer.GetTransform();

    FaceInfo face[3];
    for(int i = 0; i < m.nfaces(); i++) {
        std::tie(face[0], face[1], face[2]) = m.face(i);

        Vec3f screen[3];
        Vec3f world[3];
        for(int j = 0; j < 3; j++) {
            Vec3f v = m.vert(face[j].vertIndex);

            screen[j] = m2v(sumTransform * v2m(v));
            world[j] = v;
        }

        Vec3f n = (world[2] - world[1]) ^ (world[2] - world[0]);
        n.normalize();
        
        Vec2f uvs[3];
        uvs[0] = m.uv(face[0].uvIndex);
        uvs[1] = m.uv(face[1].uvIndex);
        uvs[2] = m.uv(face[2].uvIndex);

        renderer.DrawTriangle(screen[0], screen[1], screen[2], texture, uvs[0], uvs[1], uvs[2]);
    }
}

int main(void)
{
    renderer.SetCamera(Vec3f(3, 3, 5), Vec3f(0, 0, 0), Vec3f(0, 1, 0));

    TGAImage texture;
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();

    drawModel(texture);

    renderer.WriteRenderTarget();

    return 0;
}
