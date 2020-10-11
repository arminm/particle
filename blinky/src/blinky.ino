#include <math.h>
#include <neopixel.h>

SYSTEM_MODE(AUTOMATIC);

// accelerometer
void setXYZ(float xyz[]);
// int ReadAxis(int axisPin, int sampleSize);
// void AutoCalibrate(int xRaw, int yRaw, int zRaw);
void AutoCalibrate();

// LED strip
void rainbow(Adafruit_NeoPixel strip, uint8_t wait);
uint32_t Wheel(Adafruit_NeoPixel strip, byte WheelPos);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D2
#define PIXEL_COUNT 144
#define PIXEL_TYPE WS2812B
#define ONBOARD_LED D7

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

bool calibrating = true;
uint calibrationPeriodMs = 10000;
int startTime = 0;

void setup()
{
  pinMode(ONBOARD_LED, OUTPUT);

  Serial.begin(9600);
  strip.begin();
  // strip.setBrightness(10);
  strip.show(); // Initialize all pixels to 'off'

  startTime = millis(); // start delay
}

void loop()
{
  digitalWrite(ONBOARD_LED, LOW);
  if (calibrating && (millis() - startTime <= calibrationPeriodMs))
  {
    digitalWrite(ONBOARD_LED, HIGH);
    AutoCalibrate();
  }
  else
  {
    float xyz[3];
    setXYZ(xyz);

    for (int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, colorFromG(xyz[0]), colorFromG(xyz[1]), colorFromG(xyz[2]));
    }
    // strip.setPixelColor(4, xyz[0], 0, 0);
    // strip.setPixelColor(5, 0, xyz[1], 0);
    // strip.setPixelColor(6, 0, 0, xyz[2]);
    strip.show();
    delay(100);
  }
}

int minG = -1.5;
int maxG = 1.5;

int colorFromG(float g) {
  int gInt = floor(g * 10);
  // g = constrain(g, -1.0, 1.0);
  if (gInt < minG) {
    minG = gInt;
  } else if (gInt > maxG) {
    maxG = gInt;
  }
  return map(gInt, minG, maxG, 0, 255);
}

// OLD stuff
const int x_out = A3; /* connect x_out of module to A1 of Particle Photon board */
const int y_out = A4; /* connect y_out of module to A2 of Particle Photon board */
const int z_out = A5; /* connect z_out of module to A3 of Particle Photon board */

void oldAcc()
{
  int x_adc_value, y_adc_value, z_adc_value;
  double x_g_value, y_g_value, z_g_value;
  double roll, pitch, yaw;
  x_adc_value = analogRead(x_out); /* Digital value of voltage on x_out pin */
  y_adc_value = analogRead(y_out); /* Digital value of voltage on y_out pin */
  z_adc_value = analogRead(z_out); /* Digital value of voltage on z_out pin */

  x_g_value = ((((double)(x_adc_value * 3.3) / 4095) - 1.65) / 0.330); /* Acceleration in x-direction in g units */
  y_g_value = ((((double)(y_adc_value * 3.3) / 4095) - 1.65) / 0.330); /* Acceleration in y-direction in g units */
  z_g_value = ((((double)(z_adc_value * 3.3) / 4095) - 1.80) / 0.330); /* Acceleration in z-direction in g units */

  roll = (((atan2(y_g_value, z_g_value) * 180) / 3.14) + 180);  /* Formula for roll */
  pitch = (((atan2(z_g_value, x_g_value) * 180) / 3.14) + 180); /* Formula for pitch */
  //yaw = ( ( (atan2(x_g_value,y_g_value) * 180) / 3.14 ) + 180 ); /* Formula for yaw */
  /* Not possible to measure yaw using accelerometer. Gyroscope must be used if yaw is also required */

  int xc = abs((int)(x_g_value * 255));
  int yc = abs((int)(y_g_value * 255));
  int zc = abs((int)(z_g_value * 255));
  int rc = abs((int)(roll * 255));
  int pc = abs((int)(pitch * 255));

  // for(int i=0; i<strip.numPixels(); i++) {
  // }
  strip.setPixelColor(1, xc, 0, 0);
  strip.setPixelColor(2, 0, yc, 0);
  strip.setPixelColor(3, 0, 0, zc);
  strip.show();
  Serial.print("x = ");
  Serial.print(x_g_value);
  Serial.print(" y = ");
  Serial.print(y_g_value);
  Serial.print(" z = ");
  Serial.print(z_g_value);
  Serial.print(" roll = ");
  Serial.print(roll);
  Serial.print(" pitch = ");
  Serial.print(pitch);
  Serial.print("\n\n");
  delay(200);
}
