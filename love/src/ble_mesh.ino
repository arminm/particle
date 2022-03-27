/*
 * Project love
 * Description: Love sign
 * Author: Armin Mahmoudi
 * Date: 6/29/2021
 */

// SYSTEM_THREAD(ENABLED);

// SYSTEM_MODE(AUTOMATIC);
SYSTEM_MODE(MANUAL);
SerialLogHandler logHandler;

// declare other functions
void dispayModeAtIndex(int mode, int index);
void setBrightness(int b);
void beginStrip();
int MODE = 1; // rainbow
int DELAY_MS = 20;
unsigned int MODE_INDEX = 0;
int BRIGHTNESS = 50;

system_tick_t startTime = 0;

void setRGBColor(unsigned int index)
{
  RGB.color(index & 255, 0, 0);
}

void debugLogs()
{
  Log.info("MODE: %d, DELAY_MS: %d, BRIGHTNESS: %d", MODE, DELAY_MS, BRIGHTNESS);
}

// setup() runs once, when the device is first turned on.
void setup()
{
  startTime = millis();

  RGB.control(true);
  beginStrip();
  // Set LED Strip brightness
  setBrightness(BRIGHTNESS);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  delay(DELAY_MS);
  MODE_INDEX++;
  setRGBColor(MODE_INDEX);
  static unsigned int offset = 1;
  static unsigned int modes = 5;
  static unsigned int delay = 120000; // 2 minutes
  unsigned int mode = MODE;
  mode = (((millis() - startTime) / delay) % modes) + offset;
  dispayModeAtIndex(mode, MODE_INDEX);
  // debugLogs();
}
