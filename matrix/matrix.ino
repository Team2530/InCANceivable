#include <Adafruit_NeoPixel.h>
#include <marquee.h>
#include <matrix.h>
#include <logo.h>
#include <imgconv_utils.h>

#define PIN_STRIP1 11
#define MATRIX_WIDTH 9
#define MATRIX_HEIGHT 8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(MATRIX_WIDTH * MATRIX_HEIGHT, PIN_STRIP1, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  // Anything more is hard to look at :P
  strip.setBrightness(64);
  // Serial.println("Setup");
}

void loop() {
  static unsigned int offset = 0;
  static unsigned int t = 0;

  // Display the sqrt(-1) inconceivable logo for 8k millis before scrolling the banner.
  if (t < 100) {
    matrixPutImage(
      &strip, 
      (unsigned char*)logo,
      0, 0,
      MATRIX_WIDTH, MATRIX_HEIGHT,
      logo_width, logo_height
    );
    ++t; // In here to prevent overflow repetition.
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
  delay(80);
}
