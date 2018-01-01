#ifndef _LEDBLINKER_H_
#define _LEDBLINKER_H
#include <EventBus.h>

#include <esp/gpio.h>

class LedBlinker : public Actor
{
    bool _isOn;
    uint32_t _gpio = 2;
    uint32_t _gpio2 = 16;
    uint32_t _interval;

  public:
    LedBlinker(const char *name);
    void setup();
    void onEvent(Cbor &msg);
    void changeInterval(Cbor &msg);
};
#endif
