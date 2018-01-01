#include <Hardware.h>

#include "esp/uart.h"
#include "esp/gpio.h"
#include "espressif/esp_common.h"
using namespace HW;

class DigitalOut_ESP8266 : public DigitalOut
{
    PhysicalPin _gpio;

  public:
    DigitalOut_ESP8266(uint32_t gpio) : _gpio(gpio)
    {
    }
    Erc init()
    {
        gpio_enable(_gpio, GPIO_OUTPUT);
        return E_OK;
    }

    Erc deInit()
    {
        gpio_enable(_gpio, GPIO_INPUT);
        return E_OK;
    }

    Erc write(int x)
    {
        gpio_write(_gpio, x);
        return E_OK;
    }
    PhysicalPin getPin()
    {
        return _gpio;
    }
};

DigitalOut &DigitalOut::create(PhysicalPin pin)
{
    DigitalOut_ESP8266 *ptr = new DigitalOut_ESP8266(pin);
    return *ptr;
}