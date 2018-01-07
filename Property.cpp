#include "Property.h"

uint32_t now;
Property<uint32_t> nowProp("now",now,2000);

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

void PropertyVerticle::run()
{
    crSTART(handle());
    while(true) {
        while ( !_mqttConnected ) {
            crDELAY(handle(),MS_TO_TICK(1000));
        };
        while(_mqttConnected ) {
            if ( _currentProp == 0 ) _currentProp=PropertyBase::first();
            if ( _currentProp ==0 ) {

            } else  {
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
                _currentProp=_currentProp->next();
            }
            crDELAY(handle(),MS_TO_TICK(1000));
        }

    }
    crEND();
}
