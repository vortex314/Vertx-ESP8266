#ifndef MQTT_H
#define MQTT_H
#include <vertx.h>
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/prot/iana.h"

#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>

#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

extern "C" {
#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>
}

class Mqtt : public VerticleTask
{
    int _ret         = 0;
    struct mqtt_network _network;
    mqtt_client_t _client   = mqtt_client_default;
    char _mqtt_client_id[20];
    uint8_t _mqtt_buf[100];
    uint8_t _mqtt_readbuf[100];
    mqtt_packet_connect_data_t _data;
    Str _topicAlive;
	Str _topicsForDevice;
    bool _wifiConnected;
    bool _mqttConnected;
    Str _topicRxd;
	Str _messageRxd;
    Str _topic;
    Str _message;
    Str _topicTxd;
    Str _messageTxd;
	Message _messageEb;
    bool _busyTxd;

public:
	static Mqtt* _mqtt;
    Mqtt(const char*);
    void run();
    void start();
    void publish(Str& topic,Str& message);
	static void  topic_received_cb(mqtt_message_data_t *md);
	void onMessageReceived(Str& topic, Str& payload);

private :
    void mqtt_do_connect();
    bool do_connect();
};

#endif // MQTT_H
