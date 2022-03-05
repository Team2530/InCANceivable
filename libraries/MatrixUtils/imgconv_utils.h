#ifndef IMGCONV_H
#define IMGCONV_H

typedef struct RGB {
    unsigned char r, g, b;
} RGB;

RGB imageGetPixelRGB(unsigned char* image, unsigned int x, unsigned int y, unsigned int imageWidth, unsigned int imageHeight, bool columnMajor = true);

#endif