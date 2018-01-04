
#include <Mqtt.h>

#define MQTT_HOST ("limero.ddns.net")
#define MQTT_PORT 1883

#define MQTT_USER NULL
#define MQTT_PASS NULL

#define PUB_MSG_LEN 20

static const char *get_my_id(void)
{
    // Use MAC address for Station as unique ID
    static char my_id[13];
    static bool my_id_done = false;
    int8_t i;
    uint8_t x;
    if (my_id_done)
        return my_id;
    if (!sdk_wifi_get_macaddr(STATION_IF, (uint8_t *)my_id))
        return NULL;
    for (i = 5; i >= 0; --i) {
        x = my_id[i] & 0x0F;
        if (x > 9)
            x += 7;
        my_id[i * 2 + 1] = x + '0';
        x = my_id[i] >> 4;
        if (x > 9)
            x += 7;
        my_id[i * 2] = x + '0';
    }
    my_id[12] = '\0';
    my_id_done = true;
    return my_id;
}

static void topic_received(mqtt_message_data_t *md)
{
    mqtt_message_t *message = md->message;
    Str topic((uint8_t *)md->topic->lenstring.data, md->topic->lenstring.len);
    Str payload((uint8_t *)(message->payload), (int)message->payloadlen);
    INFO(" MQTT RXD %s : %s ", topic.c_str(), payload.c_str());
}

Mqtt::Mqtt(const char *name) : VerticleTask(name, 512, 1)
{

};

void Mqtt::run()
{
    mqtt_network_new(&_network);
    memset(_mqtt_client_id, 0, sizeof(_mqtt_client_id));
    strcpy(_mqtt_client_id, "ESP-");
    strcat(_mqtt_client_id, get_my_id());
    Str topicAlive(30);
    topicAlive = "src/";
    topicAlive += get_my_id();
    topicAlive += "/alive";
    int ret = 0;
    INFO(" clientId : %s , topic alive : %s", _mqtt_client_id, topicAlive.c_str());

    while (true) {
        wait(5000);
        INFO("%s: started", __func__);
        INFO("%s: (Re)connecting to MQTT server %s ... ", __func__, MQTT_HOST);
        ret = mqtt_network_connect(&_network, MQTT_HOST, MQTT_PORT);
        if (ret) {
            INFO("error: %d", ret);
            taskYIELD();
            continue;
        }
        INFO("done");
        mqtt_client_new(&_client, &_network, 5000, _mqtt_buf, 100, _mqtt_readbuf, 100);

        _data.willFlag = 0;
        _data.MQTTVersion = 3;
        _data.clientID.cstring = _mqtt_client_id;
        _data.username.cstring = MQTT_USER;
        _data.password.cstring = MQTT_PASS;
        _data.keepAliveInterval = 10;
        _data.cleansession = 0;
        INFO("Send MQTT connect ... ");
        _ret = mqtt_connect(&_client, &_data);
        if (_ret) {
            INFO("error: %d", _ret);
            mqtt_network_disconnect(&_network);
            taskYIELD();
            continue;
        }
        INFO("done");
        mqtt_subscribe(&_client, "#", MQTT_QOS1, topic_received);

        while (true) {
            char msg[PUB_MSG_LEN - 1] = "\0";
            strcpy(msg, "alive");
            wait(1000);

            INFO("got message to publish");
            mqtt_message_t message;
            message.payload = msg;
            message.payloadlen = strlen(msg);
            message.dup = 0;
            message.qos = MQTT_QOS1;
            message.retained = 0;
            ret = mqtt_publish(&_client, topicAlive.c_str(), &message);
            if (ret != MQTT_SUCCESS) {
                INFO("error while publishing message: %d", ret);
                break;
            }

            _ret = mqtt_yield(&_client, 1000);
            if (_ret == MQTT_DISCONNECTED)
                break;
        }

        INFO("Connection dropped, request restart");
        mqtt_network_disconnect(&_network);
        taskYIELD();
    }
}
