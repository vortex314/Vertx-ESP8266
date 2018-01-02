#ifndef _VERTICLE_H_
#define _VERTICLE_H_

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>
#include <croutine.h>
#include "esp/uart.h"
#include "espressif/esp_common.h"

#include <Log.h>
#include <Uid.h>
#include <Cbor.h>
#include <LinkedList.hpp>
#include <Uid.h>

class Verticle;

class Verticle : public LinkedList<Verticle>
{
  public:
    virtual const char *name() { return "ERRORE!"; };
    virtual void start() { };
    virtual void stop() {};
    virtual void onMessage(Cbor &msg) {};
    virtual bool isTask() {return false;};
    //    virtual void onTimer() = 0;
    //   virtual void onInterrupt() = 0;
};

typedef enum {
    N_TIMEOUT = 1,
    N_MESSAGE = 2,
    N_TIMER = 4,
    N_INTERRUPT = 8
} Notification;

class VerticleTask : public Verticle
{
    char *_name;
    uint16_t _stackSize;
    uint8_t _priority;
    uint32_t _nextEvent;
    
  public:
TaskHandle_t _taskHandle;
    VerticleTask(const char *name,  uint16_t stack, uint8_t priority);
    const char *name();
    virtual void run();
    virtual void start();
    virtual void onMessage(Cbor &msg);
    void stop();
    uint32_t newEvent();

    void notify(Notification n);
    uint32_t wait(uint32_t time);
    bool isTask() { return true; };

    static void handler(void *p);

    static void timerHandler(TimerHandle_t th);
    void print();
    TaskHandle_t getHandle();
};

typedef void (Verticle::*MethodHandler)(Cbor &);
typedef void (*StaticHandler)(Cbor &);

class VerticleCoRoutine : public Verticle
{
    char *_name;
    CoRoutineHandle_t _xHandle;

  public:
    VerticleCoRoutine(const char *name) ;
    CoRoutineHandle_t getHandle();
    const char* name();
    virtual void run(CoRoutineHandle_t xHandle);
    static void handler(CoRoutineHandle_t xHandle, UBaseType_t uxIndex);
    void start();
    void stop();
    void onMessage(Cbor &msg);
    bool isTask();
};


typedef Cbor Message;


class Address {
    uid_t _uid;
    public:
    Address(const char* s){
        _uid =  UID.add(s);
    }
    uid_t uid(){
        return _uid;
    }
    const char* label(){
       return UID.label(_uid);
    }
    bool match(Address& addr){
        return _uid == addr._uid;
    }
};

typedef void (*FunctionPointer)();
typedef void (Verticle::*VerticleMethod)(Message &);
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))

class MessageHandler
{
    Verticle &_verticle;
    VerticleMethod _method;

  public:
    MessageHandler(Verticle &verticle, VerticleMethod method) : _verticle(verticle), _method(method)
    {
    }
    void handle(Message &msg) {
        CALL_MEMBER_FN(_verticle,_method)(msg);
    }
};



class EventBus
{
  public:
    EventBus(uint32_t size);
    Erc publish(Address, Message &);
    Erc send(Address, Message &);
    Erc consumer(Address, MessageHandler &);
    Erc localConsumer(Address, MessageHandler &);
    Erc addInterceptor(MessageHandler &);
    Erc removeInterceptor(MessageHandler &);
    static void eventLoop();
};

extern EventBus EB;

#endif