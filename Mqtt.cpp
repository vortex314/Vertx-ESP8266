
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

Mqtt::Mqtt(const char *name) : VerticleTask(name, 512, 1),_topicAlive(40)
{

};

void Mqtt::start()
{
    mqtt_network_new(&_network);
    ZERO(_mqtt_client_id);
    strcpy(_mqtt_client_id, "ESP-");
    strcat(_mqtt_client_id, get_my_id());
    _topicAlive = "src/";
    _topicAlive +=_mqtt_client_id;
    _topicAlive += "/system/alive";
    VerticleTask::start();
}

Erc Mqtt::publish(Str& topic,Str& msg)
{
    int ret;
    mqtt_message_t message;
    message.payload = (void*)msg.c_str();
    message.payloadlen = msg.length();
    message.dup = 0;
    message.qos = MQTT_QOS1;
    message.retained = 0;
    ret = mqtt_publish(&_client, topic.c_str() ,&message);
    if (ret != MQTT_SUCCESS) {
        ERROR("error while publishing message: %d", ret);
        return EIO;
    }
    return E_OK;
}

void Mqtt::run()
{


    int ret = 0;
    INFO(" clientId : %s , topic alive : %s", _mqtt_client_id,_topicAlive.c_str());
    wait(15000);

    while (true) {
        goto TCP_CONNECTING;
TCP_DISCONNECTING: {
            mqtt_network_disconnect(&_network);
        }
TCP_CONNECTING : {
            while(true) {
                wait(5000);
                INFO(" (Re)connecting to MQTT server %s ... ",  MQTT_HOST);
                ret = mqtt_network_connect(&_network, MQTT_HOST, MQTT_PORT);
                if (ret) {
                    INFO("error: %d", ret);
                } else {
                    INFO (" TCP connected.");
                    goto MQTT_CONNECTING;
                }
            }
        };
MQTT_CONNECTING : {
            while(true) {
                mqtt_client_new(&_client, &_network, 20000, _mqtt_buf, 100, _mqtt_readbuf, 100);

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

                    goto TCP_DISCONNECTING;
                } else {
                    INFO("MQTT connected ");
                    goto PUBLISHING;
                }
            }
        }
        goto SUBSCRIBING;
SUBSCRIBING: {
            mqtt_subscribe(&_client, "#", MQTT_QOS1, topic_received);
            goto PUBLISHING;
        }
PUBLISHING: {
            while (true) {
                Str b(10);
                b = "true";
                INFO(" publishing : %s = %s ",_topicAlive.c_str(),b.c_str());
                Erc erc = publish(_topicAlive,b);
                if ( erc != E_OK ) {
                    goto TCP_DISCONNECTING;
                }
                _ret = mqtt_yield(&_client, 1000);
                if (_ret == MQTT_DISCONNECTED) {
                    goto TCP_DISCONNECTING;
                }
                wait(1000);
            }
        }

    }
}
