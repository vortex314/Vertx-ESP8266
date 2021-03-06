#ifndef _WIFI_H_
#define _WIFI_H_

#include <vertx.h>
#include <FreeRTOS.h>
#include <ssid_config.h>
#include <task.h>
#include "espressif/esp_common.h"
#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>

class Wifi : public VerticleTask
{
    uint8_t status = 0;
    struct sdk_station_config _config;
	Str _ssidPattern;
    Str  _ssid;
    Str _pswd;
	Str _ipAddress;
	bool _foundAP;
	static Wifi* _wifi;
    int _rssi;

public:
    Wifi(const char *name);
    void start();
    void run();
	static void scan_done_cb(void *arg, sdk_scan_status_t status);
//	static void	wifi_handle_event_cb(System_Event_t	*evt);

	void scan();
};
#endif
