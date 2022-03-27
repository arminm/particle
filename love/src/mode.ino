// LED strip
void rainbow(int index);
void simple(int index);
void dot(int index);
void zebra(int index);
void zebraRainbow(int index);
void charging(int index, float battery);
void sparkle(int index);
void bounce(int index);
void turnStripOff();

void dispayModeAtIndex(int mode, int index)
{
    if (mode == 1)
    {
        rainbow(index);
    }
    else if (mode == 2)
    {
        simple(index);
    }
    else if (mode == 3)
    {
        zebra(index);
    }
    else if (mode == 4)
    {
        zebraRainbow(index);
    }
    else if (mode == 5)
    {
        sparkle(index);
    }
    else if (mode == 6)
    {
        bounce(index);
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