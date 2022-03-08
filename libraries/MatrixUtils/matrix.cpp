#include "matrix.h"
#include <stdlib.h>

void matrixPutPixel(Adafruit_NeoPixel* strip, RGB color, unsigned int x, unsigned int y, unsigned int matWidth, unsigned int matHeight, bool zigzag, bool colMajor) {
    if (zigzag && ((colMajor ? x : y) & 1) == 0) {
        y = colMajor ? (matHeight - 1) - y : y;
        x = colMajor ? x : (matWidth - 1 - x);
    }

    int pixelIdx = 0;
    if (colMajor) pixelIdx = x * matHeight + y;
    else pixelIdx = y * matWidth + x;

    strip->setPixelColor(pixelIdx, color.r, color.g, color.b);
}

void matrixPutImage(
    Adafruit_NeoPixel* strip,
    unsigned char* image,
    unsigned int x, unsigned int y,
    unsigned int matWidth, unsigned int matHeight,
    unsigned int imgWidth, unsigned int imgHeight,
    bool imgColMajor,
    bool matZigzag,
    bool matColMajor
) {
    matrixPutImageRegion(
        strip,
        image,
        x, y,
        imgWidth, imgHeight,
        0, 0,
        matWidth, matHeight,
        imgWidth, imgHeight,
        imgColMajor,
        matZigzag,
        matColMajor
    );
}

// Euclidean remainder (as opposed to modulus, which is a pain for negative numbers)
int erem(int a, int b) {
    int r = a % b;
    return r >= 0 ? r : r + (b < 0 ? -b : b);
}

void matrixPutImageRegion(
    Adafruit_NeoPixel* strip,
    unsigned char* image,
    unsigned int x, unsigned int y,
    unsigned int w, unsigned int h,
    unsigned int offsetX, unsigned int offsetY,
    unsigned int matWidth, unsigned int matHeight,
    unsigned int imgWidth, unsigned int imgHeight,
    bool imgColMajor,
    bool matZigzag,
    bool matColMajor
) {
    for (int px = max(0, x); px < min(matWidth, x + w); ++px) {
        for (int py = max(0, y); py < min(matHeight, y + h); ++py) {
            unsigned int imgx = erem(px - offsetX, imgWidth), imgy = erem(py - offsetY, imgHeight);
            RGB color = imageGetPixelRGB(image, imgx, imgy, imgWidth, imgHeight, imgColMajor);
            matrixPutPixel(strip, color, px, py, matWidth, matHeight, matZigzag, matColMajor);
        }
    }
}