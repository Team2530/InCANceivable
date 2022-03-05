#include "imgconv_utils.h"
#include <avr/pgmspace.h>

// Gets RGB values of a pixel from an image stored in PROGMEM.
// 3 channels, expects 24-bit RGB images
RGB imageGetPixelRGB(unsigned char* image, unsigned int x, unsigned int y, unsigned int imageWidth, unsigned int imageHeight, bool columnMajor) {
    RGB color;
    unsigned int offs = 0;

    // Get pixel offset
    if (columnMajor) offs = (x * imageHeight) + y;
    else offs = (y * imageWidth) + x;
    offs *= 3; // 3 channels, a byte a piece

    color.r = pgm_read_byte_near(image + offs);
    color.g = pgm_read_byte_near(image + offs + 1);
    color.b = pgm_read_byte_near(image + offs + 2);
    return color;
}