#include <math.h>

const int xInput = A3;
const int yInput = A4;
const int zInput = A5;

// Raw Ranges:
// initialize to mid-range and allow calibration to
// find the minimum and maximum for each axis
int xRawMin = 1000;
int xRawMax = 1000;

int yRawMin = 1000;
int yRawMax = 1000;

int zRawMin = 1000;
int zRawMax = 1000;

// Take multiple samples to reduce noise
const int sampleSize = 20;

void setXYZ(float xyz[])
{
    int xRaw = ReadAxis(xInput, sampleSize);
    int yRaw = ReadAxis(yInput, sampleSize);
    int zRaw = ReadAxis(zInput, sampleSize);

    Serial.print("Raw Ranges: X: ");
    Serial.print(xRawMin);
    Serial.print("-");
    Serial.print(xRawMax);

    Serial.print(", Y: ");
    Serial.print(yRawMin);
    Serial.print("-");
    Serial.print(yRawMax);

    Serial.print(", Z: ");
    Serial.print(zRawMin);
    Serial.print("-");
    Serial.print(zRawMax);
    Serial.println();
    Serial.print(xRaw);
    Serial.print(", ");
    Serial.print(yRaw);
    Serial.print(", ");
    Serial.print(zRaw);

    // Convert raw values to 'milli-Gs"
    long xScaled = map(xRaw, xRawMin, xRawMax, -1000, 1000);
    long yScaled = map(yRaw, yRawMin, yRawMax, -1000, 1000);
    long zScaled = map(zRaw, zRawMin, zRawMax, -1000, 1000);

    // re-scale to fractional Gs
    float xAccel = xScaled / 1000.0;
    float yAccel = yScaled / 1000.0;
    float zAccel = zScaled / 1000.0;

    Serial.print(" :: ");
    Serial.print(xAccel);
    Serial.print("G, ");
    Serial.print(yAccel);
    Serial.print("G, ");
    Serial.print(zAccel);
    Serial.println("G");

    xyz[0] = xAccel;
    xyz[1] = yAccel;
    xyz[2] = zAccel;
}

//
// Read "sampleSize" samples and report the average
//
int ReadAxis(int axisPin, int sampleSize)
{
    long reading = 0;
    analogRead(axisPin);
    delay(1);
    for (int i = 0; i < sampleSize; i++)
    {
        reading += analogRead(axisPin);
    }
    return reading / sampleSize;
}

//
// Find the extreme raw readings from each axis
//
void AutoCalibrate()
{
    Serial.println("Calibrate");
    int xRaw = ReadAxis(xInput, 1);
    int yRaw = ReadAxis(yInput, 1);
    int zRaw = ReadAxis(zInput, 1);

    if (xRaw < xRawMin)
    {
        xRawMin = xRaw;
    }
    if (xRaw > xRawMax)
    {
        xRawMax = xRaw;
    }

    if (yRaw < yRawMin)
    {
        yRawMin = yRaw;
    }
    if (yRaw > yRawMax)
    {
        yRawMax = yRaw;
    }

    if (zRaw < zRawMin)
    {
        zRawMin = zRaw;
    }
    if (zRaw > zRawMax)
    {
        zRawMax = zRaw;
    }
}