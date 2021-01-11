/*
 * Project joy
 * Description: Joy by pixels
 * Author: Armin Mahmoudi
 * Date: 11/03/2020
 */

// SYSTEM_MODE(AUTOMATIC);
SYSTEM_MODE(MANUAL);
SerialLogHandler logHandler;

// declare other functions
void beginStrip();
void dispayModeAtIndex(int mode, int index);
uint32_t wheel(byte WheelPos);

const int redLED = A5;
int startTime = 0;
int lastChanged = 0;
const uint calibrationPeriodMs = 10000;
const uint changeDelayMs = 500;

// setup() runs once, when the device is first turned on.
void setup()
{
  RGB.control(true);
  pinMode(redLED, OUTPUT);
  beginStrip();
  startTime = millis(); // start delay
  lastChanged = millis();
}

// Joystick pins
const int joystickX = A2;
const int joystickY = A1;
const int joystickSelect = A0;

const int minDelay = 10;
const int maxDelay = 1000;
const float delayMultiplier = 0.5;
const int maxModes = 5;

int MODE = 4;
int DELAY_MS = 100;
int RAINBOW_INDEX = 0;

int minX, avgX, maxX, minY, avgY, maxY = 1000;
int selectThreshold = 20;
float dirThreshold = 0.5; // =50% of the max distance from neutral
int x, y, selectPressed = 0;
int selectPressStarted = 0;

int automaticThreshold = 1000;
int modeThreshold = 200;

int redLEDOn = 1;
int automatic = 1;

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  int x_adc_val, y_adc_val, select_adc_val;
  x_adc_val = analogRead(joystickX);
  y_adc_val = analogRead(joystickY);
  select_adc_val = analogRead(joystickSelect);

  digitalWrite(redLED, redLEDOn);
  delay(DELAY_MS);
  if (millis() - startTime <= calibrationPeriodMs)
  {
    DELAY_MS = 200;
    redLEDOn = redLEDOn ^ 1; // will toggle the value between 0 and 1
    calibrate(x_adc_val, y_adc_val);
    delay(DELAY_MS);
    digitalWrite(redLED, LOW);
    return;
  }
  else
  {
    redLEDOn = 1;
  }

  processInput(x_adc_val, y_adc_val, select_adc_val);

  // calibrate
  calibrate(x_adc_val, y_adc_val);

  if (selectPressed == 1 && selectPressStarted == 0)
  {
    selectPressStarted = millis();
  }

  // config
  if ((x != 0 || (selectPressed == 0 && selectPressStarted > 0)) && millis() - lastChanged >= changeDelayMs)
  {
    // Speed (delay)
    DELAY_MS += int(x * DELAY_MS * delayMultiplier);
    if (DELAY_MS < minDelay)
    {
      DELAY_MS = minDelay;
    }
    else if (DELAY_MS > maxDelay)
    {
      DELAY_MS = maxDelay;
    }

    if (selectPressStarted > 0)
    {
      const int lengthPressed = millis() - selectPressStarted;
      if (lengthPressed >= automaticThreshold)
      {
        automatic = automatic ^ 1; // toggle
      }
      else if (lengthPressed >= modeThreshold)
      {
        // Mode
        MODE++;
        if (MODE > maxModes)
        {
          MODE = 1;
        }
      }
      selectPressStarted = 0; // reset
    }
    lastChanged = millis();
  }

  // LET'S PLAY!
  if (automatic)
  {
    RAINBOW_INDEX++;
  }
  else
  {
    RAINBOW_INDEX += y;
  }
  if (RAINBOW_INDEX < 0 || RAINBOW_INDEX >= 255)
  {
    RAINBOW_INDEX = 0;
  }

  dispayModeAtIndex(MODE, RAINBOW_INDEX);
  RGB.color(wheel(RAINBOW_INDEX & 255));

  // Log.info("minX = %d, maxX = %d, avgX = %d", minX, maxX, avgX);
  // Log.info("minY = %d, maxY = %d, avgY = %d", minY, maxY, avgY);
  // Log.info("x_val = %4d, y_val = %4d, select_val = %4d", x_adc_val, y_adc_val, select_adc_val);
  // Log.info("X = %d, Y = %d, Select = %d", x, y, selectPressed);
}

void processInput(int x_adc_val, int y_adc_val, int select_adc_val)
{
  // process X
  const int xThreshold = int(dirThreshold * avgX);
  if (x_adc_val < avgX - xThreshold)
  {
    x = -1;
  }
  else if (x_adc_val > avgX + xThreshold)
  {
    x = 1;
  }
  else
  {
    x = 0;
  }

  // process Y
  const int yThreshold = int(dirThreshold * avgY);
  if (y_adc_val < avgY - yThreshold)
  {
    y = -1;
  }
  else if (y_adc_val > avgY + yThreshold)
  {
    y = 1;
  }
  else
  {
    y = 0;
  }

  // process Select
  if (select_adc_val < selectThreshold)
  {
    selectPressed = 1;
  }
  else
  {
    selectPressed = 0;
  };
}

void calibrate(int x_adc_val, int y_adc_val)
{
  // calibrate X
  if (minX > x_adc_val)
  {
    minX = x_adc_val;
  }
  else if (maxX < x_adc_val)
  {
    maxX = x_adc_val;
  }
  avgX = int((minX + maxX) / 2);
  // calibrate Y
  if (minY > y_adc_val)
  {
    minY = y_adc_val;
  }
  else if (maxY < y_adc_val)
  {
    maxY = y_adc_val;
  }
  avgY = int((minY + maxY) / 2);
}