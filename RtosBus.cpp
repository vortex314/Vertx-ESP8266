

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
//#include <Mqtt.h>
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
#include <Mqtt2.h>


EventBus eb(1024);
Log logger(256);
Telnet telnet("telnet");
Wifi wifi("wifi");
LedBlinker ledBlue("LED_BLUE");
LedBlinker ledRed("LED_RED");
Monitor monitor("monitor");
CoRoutineTask coRoutines("CoRout");

Mqtt2 mqtt("mqtt");
Task task("task");




class DummyVerticle : public VerticleCoRoutine
{
    uint32_t _interval;
public:
    DummyVerticle(const char* name):VerticleCoRoutine(name) {

    }
    void start() {
        eb.on("wifi/disconnected",[this](Message& evt) {
            _interval=100;
            INFO(" interval : %d",_interval);
        });
        eb.on("wifi/connected",[this](Message& evt) {
            _interval=1000;
            INFO(" interval : %d",_interval);
        });
        VerticleCoRoutine::start();
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

DummyVerticle dummy("DUMMY");

void addMac(Str& str)
{
    uint8_t my_id[13];
    sdk_wifi_get_macaddr(STATION_IF, my_id);
    str.appendHex(my_id,6,0);
}

extern "C" void user_init(void)
{

    uart_set_baud(0, 921600);
    INFO("SDK version:%s\n", sdk_system_get_sdk_version());
    Str hn(20);
    hn ="ESP-";
    addMac(hn);
    INFO(" host : %s",hn.c_str());
    Sys::hostname(hn.c_str());

    ledBlue.setGpio(2);
    ledBlue.setInterval(100);

    ledRed.setGpio(16);
    ledRed.setInterval(100);

    Verticle* pv;
    for(pv=Verticle::first(); pv; pv=pv->next())
        pv->start();

    eb.on("wifi/connected", [](Message& evt) {
        ledBlue.setInterval(1000);
    });
    eb.on("wifi/disconnected", [](Message& evt) {
        ledBlue.setInterval(100);
    });

    eb.on("mqtt/connected", [](Message& evt) {
        ledRed.setInterval(1000);
    });
    eb.on("mqtt/disconnected", [](Message& evt) {
        ledRed.setInterval(100);
    });
}
