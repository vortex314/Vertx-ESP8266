#include "Mqtt.h"
#include <Property.h>

//#define MQTT_PORT 1883
#define MQTT_HOST "limero.ddns.net"

enum {
    WIFI_CONNECTED=0,
    WIFI_DISCONNECTED,
    MQTT_CONNECTED,
    MQTT_DISCONNECTED,
    MQTT_SUBSCRIBED,
    MQTT_PUBLISHED,
    MQTT_INCOMING,
    MQTT_FAILURE,
    MQTT_DO_PUBLISH
};



Mqtt::Mqtt(const char* name):VerticleTask(name,512,2) , _topicAlive(40),_topicIncoming(50),_topic(50),_message(100),_topicTxd(50),_messageTxd(100)
{
    _client = mqtt_client_new();
    _wifiConnected=false;
    _mqttConnected=false;
    UID.add("topic");
    UID.add("message");
}

static bool alive=true;

void Mqtt::start()
{
    _topicAlive = "src/";
    _topicAlive += Sys::hostname();
    _topicAlive += "/system/alive";

    eb.on("wifi/connected",[this](Message& msg) {
        _wifiConnected=true;
        signal(WIFI_CONNECTED);
    });

    eb.on("wifi/disconnected",[this](Message& msg) {
        _wifiConnected=false;
        signal(WIFI_DISCONNECTED);
        signal(MQTT_DISCONNECTED);
    });

    eb.on("mqtt/publish",[this](Message& msg) {
        if ( msg.get(H("topic"),_topicTxd) && msg.get(H("message"),_messageTxd)) {
            signal(MQTT_DO_PUBLISH);
        }
    });

    VerticleTask::start();

    new PropertyFunction<uint64_t> ("system/upTime",Sys::millis,1000);
    new PropertyFunction<uint32_t> ("system/heap",Sys::getFreeHeap,1000);
    new Property<bool>("system/alive",alive,1000);
}

void Mqtt::do_connect()
{
    struct mqtt_connect_client_info_t ci;
    ip_addr_t mqttServerIP;
    IP4_ADDR(&mqttServerIP, 192, 168, 0,177);
    err_t err;
    ZERO(ci);
    ci.will_msg = "false";
    ci.will_topic = _topicAlive.c_str();
    ci.will_qos=1;
    ci.client_id =  Sys::hostname();
    ci.keep_alive=5;
    INFO(" %X : mqtt connecting %X ",this,_client);
    err = mqtt_client_connect(_client, &mqttServerIP, MQTT_PORT, mqtt_connection_cb, this, &ci);
    if(err != ERR_OK) {
        ERROR("%X : mqtt_connect return %d", this,err);
        signal(MQTT_DISCONNECTED);
    }


}

void Mqtt::mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    Mqtt* me = (Mqtt*)arg;
    if(status == MQTT_CONNECT_ACCEPTED) {
        INFO("%X : mqtt_connection_cb: Successfully connected",arg);
        me->signal(MQTT_CONNECTED);
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, me);
    } else {
        ERROR("%s : Disconnected, reason: %d", __func__,status);
        me->_mqttConnected=false;
        me->signal(MQTT_DISCONNECTED);
    }
}


void Mqtt::mqtt_sub_request_cb(void *arg, err_t result)
{
    Mqtt* me=(Mqtt*)arg;
    if ( result != ERR_OK ) me->signal(MQTT_FAILURE);
    else me->signal(MQTT_SUBSCRIBED);
    INFO("%X : Subscribe result: %d", arg,result);
}

void Mqtt::mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    Mqtt* me=(Mqtt*)arg;
//   INFO("%X : Incoming publish at topic %s with total length %u", arg,topic, (unsigned int)tot_len);
    me->_topicIncoming = topic;
}

void Mqtt::mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    Mqtt* me=(Mqtt*)arg;
//   INFO("%X : Incoming publish payload with length %d, flags %u", arg,len, (unsigned int)flags);

    if(flags & MQTT_DATA_FLAG_LAST) {
        Str msg((uint8_t*)data,len);
        msg.offset(len-1);
        msg.write((uint8_t)0);
        INFO(" %s = %s ",me->_topicIncoming.c_str(),msg.c_str());
        me->signal(MQTT_INCOMING);
    } else {
//        INFO(" partial buffer  ! length : %d ",len);
    }
}

void Mqtt::publish(Str& topic,Str& message)
{
    if ( !_mqttConnected ) return;
    INFO(" MQTT_PUB %s=%s ",topic.c_str(),message.c_str());
    err_t err;
    u8_t qos = 0; /* 0 1 or 2, see MQTT specification */
    u8_t retain = 0; /* No don't retain such crappy payload... */
    err = mqtt_publish(_client,
                       topic.c_str(),
                       message.data(), message.length(),
                       qos,
                       retain,
                       mqtt_pub_request_cb,
                       this);
    if(err != ERR_OK) {
        INFO("Publish err: %d", err);
        signal(MQTT_FAILURE);
    } else
        signal(MQTT_PUBLISHED); // add erc
}

/* Called when publish is complete either with sucess or failure */
void Mqtt::mqtt_pub_request_cb(void *arg, err_t result)
{
    Mqtt* me=(Mqtt*)arg;
    if(result != ERR_OK) {
        INFO("%X : Publish result: %d", arg,result);
        me->signal(MQTT_FAILURE);
    } else
        me->signal(MQTT_PUBLISHED); //TODO add erc
}

void Mqtt::run()
{
    TimerHandle_t th = xTimerCreate("mqtt",100/portTICK_PERIOD_MS,pdTRUE,this,timerHandler);
    xTimerStart(th,0);
    while(true) {
        wait(500);
        if ( hasSignal(MQTT_FAILURE)) {
        };
        if ( hasSignal(WIFI_CONNECTED)) {
            _wifiConnected=true;
            do_connect();
        };
        if ( hasSignal(MQTT_CONNECTED)) {
            _mqttConnected=true;
            eb.publish("mqtt/connected");
            err_t err = mqtt_subscribe(_client, "src/+/system/alive", 1, mqtt_sub_request_cb, this);
            if ( err ) ERROR(" subscribe failed ");
        };
        if ( hasSignal(MQTT_DISCONNECTED)) {
            _mqttConnected=false;
            wait(1000);
            do_connect();
        };
        if ( hasSignal(MQTT_SUBSCRIBED)) {

        };
        if ( hasSignal(MQTT_PUBLISHED)) {

        };
        if ( hasSignal(MQTT_INCOMING)) {

        };
        if ( hasSignal(MQTT_DISCONNECTED)) {
            _mqttConnected=false;
            eb.publish("mqtt/disconnected");
        };
        if ( hasSignal(WIFI_DISCONNECTED)) {
            _wifiConnected=false;
        };
        if ( hasSignal(MQTT_DO_PUBLISH) && _mqttConnected ) {
            publish(_topicTxd,_messageTxd);
        } /*else if ( (n& (1<<SIGNAL_TIMER) ) && _mqttConnected ) { // timeout
            _message.clear();
            if( cnt==0 ) {
                _message="true";
                publish(_topicAlive,_message);
            } else if ( cnt==1) {
                _topic = "src/";
                _topic += Sys::hostname();
                _topic += "/system/upTime";
                _message.append(Sys::millis());
                publish(_topic,_message);
            } else if ( cnt==2) {
                _topic = "src/";
                _topic += Sys::hostname();
                _topic += "/system/heap";
                _message.append(Sys::getFreeHeap());
                publish(_topic,_message);
            }
            if (cnt++==3 ) cnt=0;

        }*/

    }

}
