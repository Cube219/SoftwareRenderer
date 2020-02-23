#include <iostream>

#include "tgaimage.h"
#include "model.h"

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

int main(void)
{
    Model m = Model("obj/african_head.obj");

    TGAImage image(width, height, TGAImage::RGB);

    for(int i = 0; i < m.nfaces(); i++) {
        std::vector<int> face = m.face(i);
        
        for(int j = 0; j < 3; j++) {
            Vec3f v1 = m.vert(face[j]);
            Vec3f v2 = m.vert(face[(j+1)%3]);

            int x1 = (v1.x + 1.0f) * (width / 2);
            int y1 = (v1.y + 1.0f) * (height / 2);
            int x2 = (v2.x + 1.0f) * (width / 2);
            int y2 = (v2.y + 1.0f) * (height / 2);

            drawLine(x1, y1, x2, y2, image, TGAColor(255, 255, 255, 255));
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    return 0;
}
