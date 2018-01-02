#ifndef _MQTT_H_
#define _MQTT_H_

#include <vertx.h>
#include <FreeRTOS.h>
#include <ssid_config.h>
#include <task.h>
#include "espressif/esp_common.h"
#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>
extern "C" {
#include <paho_mqtt_c/MQTTClient.h>
#include <paho_mqtt_c/MQTTESP8266.h>
};

class Mqtt : public VerticleTask
{
  int _ret = 0;
  uint8_t _status = 0;
  struct sdk_station_config _config;
  uid_t _wifi;
  struct mqtt_network _network;
  mqtt_client_t _client = mqtt_client_default;
  char _mqtt_client_id[20];
  uint8_t _mqtt_buf[100];
  uint8_t _mqtt_readbuf[100];
  mqtt_packet_connect_data_t _data = mqtt_packet_connect_data_initializer;

public:
  Mqtt(const char *name);
  void run();
  void setWifi(uid_t wifi) { _wifi = wifi; };
};
#endif