#ifndef MQTT_H
#define MQTT_H
#include <vertx.h>
#include <lwip/apps/mqtt.h>
#include "lwip/apps/mqtt_opts.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/prot/iana.h"
class Mqtt2 : public VerticleTask
{
    mqtt_client_t* _client;
    Str _topicAlive;
    bool _wifiConnected;
    bool _mqttConnected;
    Str _topicIncoming;
    Str _topic;
    Str _message;
public:
    Mqtt2(const char*);
    void run();
    void start();
    void do_connect();
    static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
    static void mqtt_sub_request_cb(void *arg, err_t result);
    static void mqtt_pub_request_cb(void *arg, err_t result);

    static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
    static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
    void publish(Str& topic,Str& message);


private :
    void mqtt_do_connect();
};

#endif // MQTT_H
