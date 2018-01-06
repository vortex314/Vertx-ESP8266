#include "LedBlinker.h"


LedBlinker::LedBlinker(const char *name)
    : VerticleCoRoutine(name)
{
    _gpio = 2;
    _interval=100/portTICK_PERIOD_MS;
}

void LedBlinker::run()
{
    crSTART(handle());
    gpio_enable(_gpio, GPIO_OUTPUT);

    while (true) {
        crDELAY(handle(),_interval);
        gpio_write(_gpio, 1);

        crDELAY(handle(),_interval);
        gpio_write(_gpio, 0);

    }
    crEND();
}

void LedBlinker::setGpio(uint32_t gpio)
{
    _gpio=gpio;
}

void LedBlinker::setInterval(uint32_t interval)
{
    _interval=interval/portTICK_PERIOD_MS;
}
