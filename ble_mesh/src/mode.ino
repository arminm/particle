// LED strip
void rainbow(int index);
void simple(int index);
void dot(int index);
void zebra(int index);
void zebraRainbow(int index);
void charging(int index, float battery);
void sparkle(int index);
void turnStripOff();

// Battery
float readBattery();

int CHARGING_MODE = 1;

int chargingMode()
{
    return CHARGING_MODE;
}

void dispayModeAtIndex(int mode, int index)
{
    if (mode == CHARGING_MODE)
    {
        charging(index, readBattery());
    }
    else if (mode == 2)
    {
        rainbow(index);
    }
    else if (mode == 3)
    {
        simple(index);
    }
    else if (mode == 4)
    {
        zebra(index);
    }
    else if (mode == 5)
    {
        zebraRainbow(index);
    }
    else if (mode == 6)
    {
        sparkle(index);
    }
    else if (mode == 7)
    {
        dot(index);
    }
    else
    {
        turnStripOff();
    }
}