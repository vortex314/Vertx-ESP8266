

#include <memory>
#include <thread>

#include <vertx.h>
#include "lwip/err.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/mem.h"


#include <Wifi.h>
#include <Telnet.h>
#include <Mqtt.h>
#include <Monitor.h>
#include <LedBlinker.h>
//#include <Hardware.h> //



/*
The Router receives messages  and sends them  to subscribers or destinations

    calls handlers which are synchronous or asynchronous
    if ( async

    create filters :

    Header : dst

*/
extern "C" void __cxa_pure_virtua2l()
{
    while (1) {
        ERROR(" pure virtual called !!");
        vTaskDelay(100);
    }
}



class CoRoutineTask : public VerticleTask
{
public:
    CoRoutineTask(const char *name)
        : VerticleTask(name, 256, 1) {
    }
    void run() {
        while (true) {
            vCoRoutineSchedule();
            //            EventBus::eventLoop();
        }
    }
};






class Task : public VerticleTask
{
public:
    Task(const char *name)
        : VerticleTask(name, 256, 3) {
    }
    void start() {
        VerticleTask::start();
    }
    void run() {
        while (true) {
            wait(1000);
            //            EventBus::eventLoop();
        }
    }
};






class DummyVerticle : public VerticleCoRoutine
{
public:
    DummyVerticle(const char* name):VerticleCoRoutine(name) {

    }
    void run() {
        crSTART(handle());
        for (;;) {
            crDELAY(handle(), 10000);
            INFO("_____________________________________________", name());
        }
        crEND();
    }
};

EventBus EB(1024);
Log logger(256);
Telnet telnet("telnet");
Wifi wifi("wifi");
LedBlinker led("LED");
Monitor monitor("monitor");
CoRoutineTask coRoutines("CoRout");
DummyVerticle dummy("DUMMY");
Mqtt mqtt("mqtt");
Task task("task");



extern "C" void user_init(void)
{

    uart_set_baud(0, 921600);
    INFO("SDK version:%s\n", sdk_system_get_sdk_version());

    Verticle* pv;
    for(pv=Verticle::first(); pv; pv=pv->next())
        pv->start();

    MessageHandler handler(task, (VerticleMethod)&Task::onMessage);
    EB.consumer("wifi.alive", handler);
}
