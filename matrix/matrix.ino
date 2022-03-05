#include <Adafruit_NeoPixel.h>
#include <img.h>

#define PIN_STRIP1 11
Adafruit_NeoPixel strip = Adafruit_NeoPixel(img_width * img_height, PIN_STRIP1, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(64);
  Serial.println("Setup");
}

void loop() {
  for (int pi = 0; pi < img_width * img_height; ++pi) {
    int ri = pi * img_depth;
    strip.setPixelColor(pi, 
      pgm_read_byte_near(img + ri), 
      pgm_read_byte_near(img + ri+1),
      pgm_read_byte_near(img + ri+2)
    );
  }
  strip.show();
  Serial.println("Wrote pixels");
  delay(1000);
}
