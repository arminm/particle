// LED strip
void rainbow(int index);
void simple(int index);
void dot(int index);
void zebra(int index);
void lowPower(int index, float battery);
void sparkle(int index);
void turnStripOff();

// Battery
float readBattery();

int LOW_POWER_MODE = 1;

int lowPowerMode()
{
    return LOW_POWER_MODE;
}

void dispayModeAtIndex(int mode, int index)
{
    if (mode == LOW_POWER_MODE)
    {
        lowPower(index, readBattery());
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
        dot(index);
    }
    else if (mode == 5)
    {
        zebra(index);
    }
    else if (mode == 6)
    {
        sparkle(index);
    }
    else
    {
        turnStripOff();
    }
}