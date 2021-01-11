#include "Particle.h"
#include <neopixel.h>

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 144
#define PIXEL_PIN D8
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

void rainbow(int index)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, wheel((i + index) & 255));
  }
  strip.show();
}

void simple(int index)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, wheel(index & 255));
  }
  strip.show();
}

void dot(int index)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    if (i == index) {
      strip.setPixelColor(i, wheel(index & 255));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

void zebra(int index)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    if ((i + index) % 5 == 0) {
      strip.setPixelColor(i, wheel(index & 255));
    } else {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(byte WheelPos)
{
  if (WheelPos < 85)
  {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}