# Red Light, Green Light by rj45jack
#include <Adafruit_NeoPixel.h>

#define PIN            0
int traffic;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
  randomSeed(27);
}

void loop() {
    
    traffic = 0;
    traffic = random(3);
    pixels.setPixelColor(0,0,0,0);
    pixels.show(); 
    delay(500);
    
    if (traffic == 1)
    {
    pixels.setPixelColor(0,0,255,0); 
    pixels.show(); 
    delay(random(3000,10000)); 
    }
    else if (traffic == 2)
    {
    pixels.setPixelColor(0,255,0,0); 
    pixels.show(); 
    delay(random(3000,10000)); 
    }
  }

