#include <Adafruit_NeoPixel.h>
#include <marquee.h>
#include <matrix.h>
#include <splash.h>
#include <imgconv_utils.h>

#define PIN_STRIP1 11
#define MATRIX_WIDTH 9
#define MATRIX_HEIGHT 8
#define BRIGHTNESS 64

#define FRAME_DURATION 80
#define SPLASH_FRAMES 100

Adafruit_NeoPixel strip = Adafruit_NeoPixel(MATRIX_WIDTH * MATRIX_HEIGHT, PIN_STRIP1, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  // Serial.println("Setup");
}

void loop() {
  static unsigned int offset = 0;
  static unsigned int t = 0;

  // Display the splash image for the specified amount of frames before scrolling the banner.
  // -1 will permanently display the splash image.
  if (t < SPLASH_FRAMES || t == -1) {
    matrixPutImage(
      &strip, 
      (unsigned char*)splash,
      0, 0,
      MATRIX_WIDTH, MATRIX_HEIGHT,
      splash_width, splash_height
    );
    if (t != -1) ++t; // In here to prevent overflow repetition.
    strip.show();
  } else {
    // Ooh more concise library functions :)
    // Use offset to scroll because it wraps the image
    matrixPutImageRegion(
      &strip, 
      (unsigned char*)marquee, 
      0, 0, // x, y 
      MATRIX_WIDTH, MATRIX_HEIGHT, // w, h
      -offset, 0, // image offset
      MATRIX_WIDTH, MATRIX_HEIGHT, // mat size
      marquee_width, marquee_height, // img size
      MARQUEE_COLMAJOR
    );
    
    strip.show();
    // Prevent overflow, even though it isn't really a problem
    offset = (offset+1) % marquee_width;
  }

  // Scroll delay
  delay(FRAME_DURATION);
}
