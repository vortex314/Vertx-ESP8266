#include "LedBlinker.h"


LedBlinker::LedBlinker(const char *name)
    : VerticleCoRoutine(name)
{
    _gpio = 2;
    _interval=100;
}

void LedBlinker::run()
{
    PT_BEGIN();
    gpio_enable(_gpio, GPIO_OUTPUT);

    while (true) {
        PT_WAIT_SIGNAL(_interval);
        gpio_write(_gpio, 1);

        PT_WAIT_SIGNAL(_interval);
        gpio_write(_gpio, 0);

    }
    PT_END();
}

void LedBlinker::setGpio(uint32_t gpio)
{
    _gpio=gpio;
}

void LedBlinker::setInterval(uint32_t interval)
{
    INFO(" %s  => interval : %d ",name(),interval);
    _interval=interval;
}
