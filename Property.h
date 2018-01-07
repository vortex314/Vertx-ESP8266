#ifndef PROPERTY_H
#define PROPERTY_H
#include <LinkedList.hpp>
#include <vertx.h>
#include <memory.h>
#include <ArduinoJson.h>

class PropertyBase : public LinkedList<PropertyBase>
{
public:
    uid_t _uid;
    uint32_t _interval;
    PropertyBase(const char* name,uint32_t interval) : _interval(interval) {
        _uid = UID.add(name);
        add(this);
    }
    virtual void toJson(Str&) {};

};

template <typename T>
class Property : public PropertyBase
{
public:
    T& _var;

    Property(const char* name,T& var,uint32_t interval) : PropertyBase(name,interval),_var(var) {
        _uid = UID.add(name);
    }

    ~Property() {
    }
    void toJson(Str& msg) {
        msg.append(_var);
    }
};

template <typename T>
class PropertyFunction : public PropertyBase
{
public:
    std::function<T()> _f;
    ;


    PropertyFunction(const char* name,std::function<T ()> f,uint32_t interval) : PropertyBase(name,interval),_f(f) {
        _uid = UID.add(name);
    }

    ~PropertyFunction() {
    }
    void toJson(Str& msg) {
        StaticJsonBuffer <200> js;
        msg.append(_f());
    }
};


class PropertyVerticle : public VerticleCoRoutine
{
    PropertyBase* _currentProp;
    bool  _mqttConnected;
    Message _toMqttMsg;
    Str _topic;
    Str _message;
public:
    PropertyVerticle(const char* name);
    void start();
    void run();
};

extern PropertyVerticle propertyVerticle;



#endif // PROPERTY_H
