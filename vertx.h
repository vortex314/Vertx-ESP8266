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
#include <CborQueue.h>

#define MS_TO_TICK(xx) (xx/portTICK_PERIOD_MS)

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



class Message :  public Cbor
{

public:
    Message(uint32_t size):Cbor(size) {
    }
    template <typename T> bool get(uid_t uid,T& var) {
        return getKeyValue(uid,var);
    };
    template <typename T> Message& put(uid_t uid,T& var) {
        addKeyValue(uid,var);
        return *this;
    }
    void clear() {
        Cbor::clear();
    }
};

typedef const char* EventLabel;
#define EventHandler std::function<void (Message&)>

const uid_t EB_DST = H("dst");
const uid_t EB_SRC = H("src");

class EventBus
{
    CborQueue _queue;
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
    void eventLoop();
};

extern EventBus eb;


#endif
