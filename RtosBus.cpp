

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
#include <Sntp.h>
#include <Mdns.h>
#include <Property.h>
#include <Hardware.h> //
#include <DWM1000_Anchor.h>
#include <DWM1000_Tag.h>
#include <Config.h>



/*
The Router receives messages  and sends them  to subscribers or destinations

    calls handlers which are synchronous or asynchronous
    if ( async

    create filters :

    Header : dst

*/
extern "C" void __cxa_pure_virtual()
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
        : VerticleTask(name, 500, 1)
    {
    }
    void run()
    {
        while (true) {
//            vCoRoutineSchedule();
            eb.eventLoop();
            VerticleCoRoutine::loop();
        }
    }
};






class Task : public VerticleTask
{
public:
    Task(const char *name)
        : VerticleTask(name, 256, 3)
    {
    }
    void start()
    {
        VerticleTask::start();
    }
    void run()
    {
        while (true) {
            waitSignal(1000);
            //            EventBus::eventLoop();
        }
    }
};


EventBus eb(1024);
Log logger(256);
Telnet telnet("telnet");
Wifi wifi("wifi");
LedBlinker ledBlue("LED_BLUE",DigitalOut::create(2));
LedBlinker ledRed("LED_RED",DigitalOut::create(16));
Monitor monitor("monitor");
CoRoutineTask coRoutines("CoRout");
//Sntp sntp("sntp");


Mqtt mqtt("mqtt");
Task task("task");
Mdns mdns("mdns");
PropertyVerticle propSender("props");

#include <SpiVerticle.h>

//SpiVerticle spi("spi");


class DummyVerticle : public VerticleCoRoutine
{
    uint32_t _interval;
public:
    DummyVerticle(const char* name):VerticleCoRoutine(name)
    {

    }
    void start()
    {
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
    void run()
    {
        PT_BEGIN();
        for (;;) {
            PT_WAIT_SIGNAL(10000);
            INFO("_____________________________________________", name());
        }
        PT_END();
    }
};

DummyVerticle dummy("DUMMY");

void addMac(Str& str)
{
    uint8_t my_id[6];
    sdk_wifi_get_macaddr(STATION_IF, my_id);
    str.appendHex(my_id,6,0);
}
#include <sys/unistd.h>





extern "C" void user_init(void)
{


    uart_set_baud(0, 921600);
    INFO("SDK version:%s", sdk_system_get_sdk_version());


    new PropertyFunction<const char*>("system/sdk",sdk_system_get_sdk_version,5000);
    config.load();
    config.setNameSpace("system");

    Str hn(20);
    hn ="ESP-";
    addMac(hn);
    config.get("host",hn,hn.c_str());
    INFO(" host : %s",hn.c_str());
    Sys::hostname(hn.c_str());

    union {
        uint8_t mac[6];
        uint64_t ll;
        uint16_t lw[4];
    } un;
    sdk_wifi_get_macaddr(STATION_IF, un.mac);
    int role;
    config.get("dwm1000/roleNbr",role,1);
    role=2;
    if ( role==1 ) {
        DWM1000_Tag* tag=new DWM1000_Tag("tag",Spi::create(12,13,14,15),DigitalIn::create(4),DigitalOut::create(5));
        tag->setShortAddress(un.lw[2]);
        tag->setLongAddress(un.mac);
    } else {
        DWM1000_Anchor* anchor=new DWM1000_Anchor("anchor",Spi::create(12,13,14,15),DigitalIn::create(4),DigitalOut::create(5));
        anchor->setShortAddress(un.lw[2]);
        anchor->setLongAddress(un.mac);
    }


    Verticle* pv;
    for(pv=Verticle::first(); pv; pv=pv->next())
        pv->start();

    config.save();

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
