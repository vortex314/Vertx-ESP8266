#ifndef PROPERTY_H
#define PROPERTY_H
#include <LinkedList.hpp>
#include <vertx.h>

class Property : public LinkedList<Property>
{
public:
    uid_t _uid;
    uint32_t _interval;



    Property(uid_t uid,uint32_t interval);
    ~Property();
    virtual void  get(Str& s);
    virtual void set(Str& s);

};

template <typename T>
class Prop : public Property
{
    T& _var;
public:
    Prop(const char* name,T& var,uint32_t interval): Property(UID.add(name),interval) ,_var(var) {

    }
};


#endif // PROPERTY_H
