#ifndef MATRIX_H
#define MATRIX_H

#include <Adafruit_NeoPixel.h>
#include "imgconv_utils.h"

void matrixPutPixel(Adafruit_NeoPixel* strip, RGB color, unsigned int x, unsigned int y, unsigned int matWidth, unsigned int matHeight, bool zigzag = true, bool colMajor = true);
void matrixPutImage(
    Adafruit_NeoPixel* strip,
    unsigned char* image,
    unsigned int x, unsigned int y,
    unsigned int matWidth, unsigned int matHeight,
    unsigned int imgWidth, unsigned int imgHeight,
    bool imgColMajor = true,
    bool matZigzag = true,
    bool matColMajor = true
);
void matrixPutImageRegion(
    Adafruit_NeoPixel* strip,
    unsigned char* image,
    unsigned int x, unsigned int y,
    unsigned int w, unsigned int h,
    unsigned int offsetX, unsigned int offsetY,
    unsigned int matWidth, unsigned int matHeight,
    unsigned int imgWidth, unsigned int imgHeight,
    bool imgColMajor = true,
    bool matZigzag = true,
    bool matColMajor = true
);

#endif