#include <neopixel.h>

// LED strip
void rainbow(int index);
void simple(int index);
void dot(int index);
void zebra(int index);
void takeoff(int index);

void dispayModeAtIndex(int mode, int index) {
    if (mode == 1) {
        rainbow(index);
    } else if (mode == 2) {
        dot(index);
    } else if (mode == 3) {
        zebra(index);
    } else if (mode == 4) {
        takeoff(index);
    } else {
        simple(index);
    }
}
