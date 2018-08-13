#include "Property.h"

uint32_t now;
PropertyReference<uint32_t> nowProp("now",now,2000);
LinkedList<Property*> Property::_properties;

#define MQTT_CONNECTED 0
#define MQTT_DISCONNECTED 1

PropertyVerticle::PropertyVerticle(const char* name) : VerticleCoRoutine(name),_toMqttMsg(100),_topic(50),_message(100)
{
    _currentProp=0;
    _mqttConnected=false;
}

bool alive=true;
Str version(30);


void PropertyVerticle::start()
{
    version = __DATE__ " " __TIME__ " 1.0.3 ";
    new PropertyReference<bool>("system/alive",alive,-1000);
    new PropertyFunction<uint32_t>("system/upTime",Sys::sec,-500);
    new PropertyFunction<uint32_t>("system/heap",Sys::getFreeHeap,-5000);
    new PropertyFunction<uint64_t>("system/serialId",Sys::getSerialId,5000);
    new PropertyReference<Str>("system/version",version,5000);
    eb.on("mqtt/connected",[this](Message& msg) {
        _mqttConnected=true;
    });
    eb.on("mqtt/disconnected",[this](Message& msg) {
        _mqttConnected=false;
    });
    eb.on("property/set",[this](Message& msg) {
        uid_t key;
        if ( msg.get(H("key"),key) && msg.get(H("value"),_message)) {
            Property* p = Property::findByUid(key);
            if ( p ) {
                _message.offset(0);
                p->fromJson(_message);
                INFO(" Propery set  %s=%s",UID.label(key),_message.c_str());
            } else {
                ERROR(" didn't find property ")
            }
        } else {
            ERROR(" didn't find key & value ");
        }

    });
    VerticleCoRoutine::start();
}

void PropertyVerticle::sendProp(Property* p)
{
    _toMqttMsg.clear();
    _topic.clear();
    _message.clear();
    _topic = "src/";
    _topic += Sys::hostname();
    _topic +="/";
    _topic +=UID.label(p->_uid);
    p->toJson(_message);

    _toMqttMsg.put(H("topic"),_topic);
    _toMqttMsg.put(H("message"),_message);
    eb.publish("mqtt/publish",_toMqttMsg);
    p->_timeout = Sys::millis()+p->_interval;
}

void PropertyVerticle::run()
{
//   INFO("%X ",signal());
    PT_BEGIN();
    while(true) {
        while(!_mqttConnected) {
            PT_WAIT(1000);
        }

        while(_mqttConnected ) {
            PT_WAIT(10);
            _currentProp = Property::_properties.findFirst([](Property* p) {
                return  ( p->_timeout < Sys::millis() || p->hasChanged() ) ;
            });
            if ( _currentProp ) sendProp(_currentProp);
        };
    }
    PT_END();
}

Property* Property::findByUid(uid_t uid)
{
    return _properties.findFirst([uid](Property* p) {
        return p->uid()==uid;
    });
}
