#include <Adafruit_NeoPixel.h>
#include <marquee.h>
#include <matrix.h>
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
  static int offset = 0;

  // Ooh more concise library functions :)
  matrixPutImageRegion(
    &strip, 
    marquee, 
    0, 0, // x, y 
    9, 8, // w, h
    -offset, 0, // image offset
    9, 8, // mat size
    marquee_width, marquee_height, // img size
    MARQUEE_COLMAJOR
  );
  
  strip.show();
  delay(80);
  offset = (offset+1) % marquee_width;
}
