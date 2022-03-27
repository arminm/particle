#include <neopixel.h>

SYSTEM_MODE(MANUAL);

// LED strip
void rainbow(Adafruit_NeoPixel strip, uint8_t wait);
uint32_t Wheel(Adafruit_NeoPixel strip, byte WheelPos);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D2
#define PIXEL_COUNT 144
#define PIXEL_TYPE WS2812B
#define ONBOARD_LED D7

#define RED strip.Color(255, 0, 0)
#define BLUE strip.Color(0, 255, 0)
#define GREEN strip.Color(0, 0, 255)

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

void setup()
{
  RGB.control(true);
  WiFi.off();

  Serial.begin(9600);
  strip.begin();
  // strip.setBrightness(10);
  strip.setPixelColor(2, 255, 0, 0);
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  int colorIndex = (millis() / 100) % 3;
  RGB.color(255, 0, 0);
  for (int i = 0; i < strip.numPixels(); i++)
  {
    if (colorIndex == 0)
    {
      strip.setPixelColor(i, GREEN);
    }
    else if (colorIndex == 1)
    {
      strip.setPixelColor(i, BLUE);
    }
    else
    {
      strip.setPixelColor(i, RED);
    }
  }
  strip.show();
  delay(10);
}
