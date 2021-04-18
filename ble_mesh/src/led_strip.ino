#include "Particle.h"
#include <neopixel.h>

// IMPORTANT: Set pixel COUNT, PIN and TYPE
// cane has 154, 1m strips have 144
#define PIXEL_COUNT 154
#define PIXEL_PIN D13
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

void setBrightness(int b)
{
  strip.setBrightness(b);
}

void turnStripOff()
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    strip.setPixelColor(i, 0);
  }
  strip.show();
  Log.info("Turn strip OFF...");
}

void rainbow(int index)
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    strip.setPixelColor(i, wheel((i + index) & 255));
  }
  strip.show();
}

void simple(int index)
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    strip.setPixelColor(i, wheel(index & 255));
  }
  strip.show();
}

void dot(int index)
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    if (i == index % PIXEL_COUNT)
    {
      strip.setPixelColor(i, wheel(index & 255));
    }
    else
    {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

void zebra(int index)
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    if ((i + index) % 5 == 0)
    {
      strip.setPixelColor(i, wheel(index & 255));
    }
    else
    {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

int sparkles[PIXEL_COUNT] = {};
int sparkleRatio = 20;

void sparkle(int index)
{
  int decrements = 50;
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    int brightness = sparkles[i];
    if (brightness > decrements)
    {
      sparkles[i] -= decrements;
    }
    else if (brightness > 0)
    {
      sparkles[i] = 0;
    }
  }
  for (int i = 0; i < PIXEL_COUNT / sparkleRatio; i++)
  {
    int rand = random(PIXEL_COUNT);
    sparkles[rand] = 255;
  }
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    int v = sparkles[i];
    strip.setPixelColor(i, v, v, v);
  }
  strip.show();
}

void lowPower(int index, float battery)
{
  int max = int(battery / 25) + 1;
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    if (i < max)
    {
      if ((i + index) % max == 0)
      {
        strip.setPixelColor(i, strip.Color(50, 0, 0));
      }
      else
      {
        strip.setPixelColor(i, 0);
      }
    }
    else
    {
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