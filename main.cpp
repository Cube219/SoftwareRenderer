#include <iostream>

#include "tgaimage.h"

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
    TGAImage image(100, 100, TGAImage::RGB);

    drawLine(13, 20, 80, 40, image, TGAColor(255, 255, 255, 255));
    drawLine(20, 13, 40, 80, image, TGAColor(255, 0, 0, 255));
    drawLine(80, 40, 13, 20, image, TGAColor(255, 0, 0, 255));

    image.flip_vertically();
    image.write_tga_file("output.tga");

    return 0;
}
