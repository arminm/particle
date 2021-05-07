#include "Particle.h"
#include <neopixel.h>

// IMPORTANT: Set pixel COUNT, PIN and TYPE
// cane has 150, 1m strips have 144
#define PIXEL_COUNT 150
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
    if (index < PIXEL_COUNT) // initial startup
    {
      int j = index / 2;
      if (i < j || i > PIXEL_COUNT - j)
      {
        strip.setPixelColor(i, wheel((i + index) % 360));
      }
      else
      {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
      }
    }
    else
    {
      strip.setPixelColor(i, wheel((i + index) % 360));
    }
  }
  strip.show();
}

void simple(int index)
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    strip.setPixelColor(i, wheel(index % 360));
  }
  strip.show();
}

void dot(int index)
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    if (i == index % PIXEL_COUNT)
    {
      strip.setPixelColor(i, wheel(index % 360));
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
    if ((i + index) % 7 == 0)
    {
      strip.setPixelColor(i, wheel(index % 360));
    }
    else
    {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

void zebraRainbow(int index)
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    if ((i + index) % 7 == 0)
    {
      strip.setPixelColor(i, wheel((i + index) % 360));
    }
    else
    {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

int sparkles[PIXEL_COUNT] = {};
int sparkleRatio = 40;

void sparkle(int index)
{
  int decrements = 100;
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

void charging(int index, float battery)
{
  int step = 20;
  int bat = int(battery / step);
  int max = 100 / step;
  int offset = 5;
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    if (i > offset && i < offset + bat + 1)
    {
      strip.setPixelColor(i, strip.Color(50, 0, 0));
    }
    else if (bat == max && i == offset + bat + 1)
    {
      strip.setPixelColor(i, strip.Color(0, 50, 0));
    }
    else
    {
      strip.setPixelColor(i, 0);
    }
  }
  strip.show();
}

// Input a value 0 to 360 to get a color value.
// https://www.instructables.com/How-to-Make-Proper-Rainbow-and-Random-Colors-With-/
uint32_t wheel(int angle)
{
  byte red, green, blue;

  if (angle < 60)
  {
    red = 255;
    green = angle * 4.25;
    blue = 0;
  }
  else if (angle < 120)
  {
    red = (120 - angle) * 4.25;
    green = 255;
    blue = 0;
  }
  else if (angle < 180)
  {
    red = 0;
    green = 255;
    blue = (angle - 120) * 4.25;
  }
  else if (angle < 240)
  {
    red = 0;
    green = (240 - angle) * 4.25;
    blue = 255;
  }
  else if (angle < 300)
  {
    red = (angle - 240) * 4.25;
    green = 0;
    blue = 255;
  }
  else
  {
    red = 255;
    green = 0;
    blue = round((360 - angle) * 4.25);
  }
  return strip.Color(red, green, blue);
}

// // Input a value 0 to 255 to get a color value.
// // The colours are a transition r - g - b - back to r.
// uint32_t wheel(byte WheelPos)
// {
//   if (WheelPos < 85)
//   {
//     return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
//   }
//   else if (WheelPos < 170)
//   {
//     WheelPos -= 85;
//     return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
//   }
//   else
//   {
//     WheelPos -= 170;
//     return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
//   }
// }