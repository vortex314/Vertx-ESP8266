#ifndef _WIFI_H_
#define _WIFI_H_

#include <vertx.h>
#include <FreeRTOS.h>
#include <ssid_config.h>
#include <task.h>
#include "espressif/esp_common.h"
#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

class Wifi : public VerticleTask
{
    uint8_t status = 0;
    struct sdk_station_config config;

  public:
    Wifi(const char *name);
    void run();
};
#endif