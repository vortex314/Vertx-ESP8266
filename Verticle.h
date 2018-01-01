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

class AbstractVerticle : public LinkedList<AbstractVerticle>
{
  public:
    virtual const char *name() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void onMessage(Cbor &msg) = 0;
    virtual bool isTask() = 0;
    //    virtual void onTimer() = 0;
    //   virtual void onInterrupt() = 0;
};

typedef enum {
    N_TIMEOUT = 1,
    N_MESSAGE = 2,
    N_TIMER = 4,
    N_INTERRUPT = 8
} Notification;

class VerticleTask : public AbstractVerticle
{
    char *_name;
    bool _worker;

    uint16_t _stackSize;
    uint8_t _priority;
    uint32_t _nextEvent;

  public:
    TaskHandle_t _taskHandle;
    VerticleTask(const char *name, bool worker, uint16_t stack, uint8_t priority);
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
};

typedef void (AbstractVerticle::*MethodHandler)(Cbor &);
typedef void (*StaticHandler)(Cbor &);

class VerticleRoutine : public AbstractVerticle
{
    char *_name;

  public:
    VerticleRoutine(const char *name);
    const char *name();
    virtual void start();
    virtual void onMessage(Cbor &msg);
    void stop(){};
    uint32_t newEvent();
};

#endif