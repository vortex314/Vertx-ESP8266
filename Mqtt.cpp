
#include <Mqtt.h>

#define MQTT_HOST ("limero.ddns.net")
#define MQTT_PORT 1883

#define MQTT_USER NULL
#define MQTT_PASS NULL

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
    for (i = 5; i >= 0; --i)
    {
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

Mqtt::Mqtt(const char *name) : Actor(name){};

void Mqtt::setup()
{
    mqtt_network_new(&_network);
    memset(_mqtt_client_id, 0, sizeof(_mqtt_client_id));
    strcpy(_mqtt_client_id, "ESP-");
    strcat(_mqtt_client_id, get_my_id());
    eb.onEvent(_wifi, EB_UID_ANY).call(this);
    uid.add("waitWifiConnected");
    uid.add("waitMqttConnected");
    uid.add("mqttConnected");
    uid.add("mqttDisconnected");
    state(H("waitWifiConnected"));
};

#define STATE_EVENT(x, y) (H(x) + (H(y) << 16))
#define STATE_EVENT_UID(x, y) ((x) + ((y) << 16))

void Mqtt::onEvent(Cbor &msg)
{
    PT_BEGIN();
WAIT_WIFI:
{
    PT_YIELD_UNTIL(eb.isEvent(_wifi, H("connected")));
};
CONNECT_MQTT:
{
    while (true)
    {
        _ret = mqtt_network_connect(&_network, MQTT_HOST, MQTT_PORT);
        if (_ret)
        {
            ERROR(" MQTT connection failed ");
            timeout(1000);
            PT_YIELD_UNTIL(eb.isTimeout());
            continue;
        }
        mqtt_client_new(&_client, &_network, 5000, _mqtt_buf, 100, _mqtt_readbuf, 100);

        _data.willFlag = 0;
        _data.MQTTVersion = 3;
        _data.clientID.cstring = _mqtt_client_id;
        _data.username.cstring = MQTT_USER;
        _data.password.cstring = MQTT_PASS;
        _data.keepAliveInterval = 10;
        _data.cleansession = 0;
        INFO("MQTT connecting ... ");
        _ret = mqtt_connect(&_client, &_data);
        if (_ret)
        {
            ERROR("error: %d", _ret);
            timeout(1000);
            PT_YIELD_UNTIL(eb.isTimeout());
            mqtt_network_disconnect(&_network);
            state(H("disconnected"));
        }
        else
        {
            state(H("connected"));
        }
        mqtt_subscribe(&_client, "#", MQTT_QOS1, topic_received);
    }
}
    PT_END();
    goto WAIT_WIFI;
    goto CONNECT_MQTT;
}
