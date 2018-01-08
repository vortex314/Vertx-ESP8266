#include "Property.h"

uint32_t now;
PropertyReference<uint32_t> nowProp("now",now,2000);

PropertyVerticle::PropertyVerticle(const char* name) : VerticleCoRoutine(name),_toMqttMsg(100),_topic(50),_message(100)
{
    _currentProp=0;
    _mqttConnected=false;
}

void PropertyVerticle::start()
{
    eb.on("mqtt/connected",[this](Message& msg) {
        _mqttConnected=true;
    });
    eb.on("mqtt/disconnected",[this](Message& msg) {
        _mqttConnected=false;
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
    _topic +=UID.label(_currentProp->_uid);
    _currentProp->toJson(_message);

    _toMqttMsg.put(H("topic"),_topic);
    _toMqttMsg.put(H("message"),_message);
    eb.publish("mqtt/publish",_toMqttMsg);
    p->_timeout = Sys::millis()+p->_interval;
}

void PropertyVerticle::run()
{
    crSTART(handle());
    while(true) {
        while ( !_mqttConnected ) {
            crDELAY(handle(),MS_TO_TICK(1000));
        };

        while(_mqttConnected ) {
            _currentProp=Property::first();
            while (_currentProp ) {
                if ( _currentProp->_timeout < Sys::millis()) {
                    sendProp(_currentProp);
                    crDELAY(handle(),MS_TO_TICK(10));
                }
                _currentProp=_currentProp->next();
            }
            crDELAY(handle(),MS_TO_TICK(1000));
        };

    }
    crEND();
}
