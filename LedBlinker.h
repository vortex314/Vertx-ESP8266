#ifndef LEDBLINKER_H
#define LEDBLINKER_H

#include <vertx.h>

class LedBlinker : public VerticleCoRoutine
{
    uint32_t _gpio = 2;
    uint32_t _gpio2 = 16;
    uint32_t notifyValue;

public:
    LedBlinker(const char *name);

    void run();
};
#endif // LEDBLINKER_H
