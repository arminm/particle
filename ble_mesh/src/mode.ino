// LED strip
void rainbow(int index);
void simple(int index);
void dot(int index);
void zebra(int index);

void dispayModeAtIndex(int mode, int index)
{
    if (mode == 1)
    {
        rainbow(index);
    }
    else if (mode == 2)
    {
        dot(index);
    }
    else if (mode == 3)
    {
        zebra(index);
    }
    else
    {
        simple(index);
    }
}