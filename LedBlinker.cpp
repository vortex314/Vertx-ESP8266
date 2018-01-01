#include <LedBlinker.h>
LedBlinker::LedBlinker(const char *name) : Actor(name)
{
    _isOn = false;
    _gpio = 2;
    _gpio2 = 16;
    _interval = 100;
};
void LedBlinker::setup()
{
    gpio_enable(_gpio, GPIO_OUTPUT);
    gpio_enable(_gpio2, GPIO_OUTPUT);
    eb.onSrc(H("wifi")).call(this, (MethodHandler)&LedBlinker::changeInterval);
    /*    eb.onSrc(H("wifi")).call([=](Cbor &msg) -> void {
      // this->changeInterval(msg);
    });*/
    timeout(100);
};
void LedBlinker::onEvent(Cbor &msg)
{
    if (_isOn)
    {
        gpio_write(_gpio, 0);
        gpio_write(_gpio2, 1);
        _isOn = false;
    }
    else
    {
        gpio_write(_gpio, 1);
        gpio_write(_gpio2, 0);
        _isOn = true;
    }
    timeout(_interval);
}
void LedBlinker::changeInterval(Cbor &msg)
{
    uid_t event;
    if (msg.getKeyValue(EB_EVENT, event))
    {
        if (event == H("connected"))
            _interval = 1000;
        if (event == H("disconnected"))
            _interval = 100;
    }
}
