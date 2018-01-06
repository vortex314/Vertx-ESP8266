#ifndef _VERTICLE_H_
#define _VERTICLE_H_

#include <memory>

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
    virtual const char *name() {
        return "ERRORE!";
    };
    virtual void start() { };
    virtual void stop() {};
    virtual void onMessage(Cbor &msg) {};
    virtual bool isTask() {
        return false;
    };

    //    virtual void onTimer() = 0;
    //   virtual void onInterrupt() = 0;
};

#define SIGNAL_MESSAGE 0
#define SIGNAL_TIMER 1

class VerticleTask : public Verticle
{
    char *_name;
    uint16_t _stackSize;
    uint8_t _priority;
    uint32_t _nextEvent;
    uint32_t  _lastNotify;
public:
    TaskHandle_t _taskHandle;
    VerticleTask(const char *name,  uint16_t stack, uint8_t priority);
    const char *name();
    virtual void run();
    virtual void start();
    virtual void onMessage(Cbor &msg);
    void stop();
    uint32_t newEvent();

    void signal(uint32_t  n);
    void signalSys(uint32_t n);
    bool hasSignal(uint32_t n);
    uint32_t wait(uint32_t time);
    bool isTask() {
        return true;
    };

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
    inline CoRoutineHandle_t handle() {
        return _xHandle;
    };
    const char* name();
    virtual void run();
    static void handler(CoRoutineHandle_t xHandle, UBaseType_t uxIndex);
    void start();
    void stop();
    void onMessage(Cbor &msg);
    bool isTask();
};

typedef uint16_t offset_t;

typedef struct {
    uid_t uid;
    offset_t next;

} TokenHeader;

class Message
{
    uint32_t* _start;
    offset_t _size;
    offset_t _lastToken;
    offset_t _nextFree;
    void next16() {
        while(_nextFree & 0x1 ) _nextFree++;
    };
    void next32() {
        while(_nextFree & 0x3 ) _nextFree++;
    };
    void add(void* item,uint32_t itemSize, int alignment);
    offset_t seek(uid_t);
    bool valid(offset_t);
    bool hasSpace(uint32_t size ) {
        if ( _size > 4 )
            return (_nextFree + size ) < (offset_t)(_size-4);
        else
            return false;
    }
public:
    Message(uint32_t size);

    Message& put(uid_t , uint32_t );
    Message& put(uid_t , Bytes& );

    bool get(uid_t,uint32_t&);
    bool get(uid_t,Bytes&);

    void finish();
};

#define EventHandler std::function<void (Message&)>


//typedef  EventHandler ;
typedef const char* EventLabel;

class EventBus
{
public:
    EventBus(uint32_t size);
    Erc publish(EventLabel);
    Erc publish(uid_t);
    Erc publish(EventLabel, Message &);
    Erc publish(uid_t ,Message& );
    Erc send(EventLabel, Message &);
    Erc consumer(EventLabel , EventHandler );
    Erc localConsumer(EventLabel, EventHandler );
    Erc addInterceptor(EventHandler );
    Erc removeInterceptor(EventHandler );
    Erc on(EventLabel  address,EventHandler);
    static void eventLoop();
};

extern EventBus eb;

#endif
