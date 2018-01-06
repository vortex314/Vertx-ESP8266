#ifndef LEDBLINKER_H
#define LEDBLINKER_H

#include <vertx.h>

class LedBlinker : public VerticleCoRoutine
{
    uint32_t _gpio = 2;
    uint32_t _interval=100;

public:
    LedBlinker(const char *name);
    void run();
    void setGpio(uint32_t gpio);
    void setInterval(uint32_t);
};
#endif // LEDBLINKER_H
