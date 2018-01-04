#include "LedBlinker.h"


LedBlinker::LedBlinker(const char *name)
    : VerticleCoRoutine(name)
{
    _gpio = 2;
    _gpio2 = 16;
}

void LedBlinker::run()
{
    crSTART(handle());
    gpio_enable(_gpio, GPIO_OUTPUT);
    gpio_enable(_gpio2, GPIO_OUTPUT);
    while (true) {
        crDELAY(handle(),100);
        gpio_write(_gpio, 1);
        gpio_write(_gpio2, 0);
        crDELAY(handle(),100);
        gpio_write(_gpio, 0);
        gpio_write(_gpio2, 1);
    }
    crEND();
}
