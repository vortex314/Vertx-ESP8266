

#include <memory>
#include <thread>

#include <vertx.h>
#include "lwip/err.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/mem.h"
//#include <Hardware.h> //

Log logger(1024);

/*
The Router receives messages  and sends them  to subscribers or destinations

    calls handlers which are synchronous or asynchronous
    if ( async 
    
    create filters : 

    Header : dst

*/
extern "C" void __cxa_pure_virtual()
{
    while (1)
    {
        ERROR(" pure virtual called !!");
        vTaskDelay(100);
    }
}



class CoRoutineTask : public VerticleTask
{
  public:
    CoRoutineTask(const char *name)
        : VerticleTask(name, 256, 1)
    {
    }
    void run()
    {
        while (true)
        {
            vCoRoutineSchedule();
            //            EventBus::eventLoop();
        }
    }
};

char taskState(uint8_t eCurrentState)
{
    char cStatus;
    switch (eCurrentState)
    {
    case eRunning:
        cStatus = '*';
        break;
    case eReady:
        cStatus = 'R';
        break;
    case eBlocked:
        cStatus = 'B';
        break;
    case eSuspended:
        cStatus = 'S';
        break;
    case eDeleted:
        cStatus = 'D';
        break;
    default:
        cStatus = '?';
        break;
    }
    return cStatus;
}

class Monitor : public VerticleTask
{
  public:
    Monitor(const char *name)
        : VerticleTask(name, 256, 1)
    {
    }

    void run()
    {
        while (true)
        {
            wait(5000);
            INFO(" free heap : %d", xPortGetFreeHeapSize());
            INFO(" Address  | S |   TASK     |  STACK | prio");
            INFO("----------|---|------------|--------|-------");
            for (Verticle *v = first(); v != 0; v = v->next())
            {
                if (v->isTask())
                {
                    VerticleTask* vt = (VerticleTask*)v;
                    TaskHandle_t th=vt->getHandle();
                    if ( th != 0)
                    INFO(" %8X | %c | %10s |  %5ld | %3ld",
                         vt->getHandle(),
                         taskState(eTaskGetState(th)),
                         vt->name(),
                         uxTaskGetStackHighWaterMark(th),
                         uxTaskPriorityGet(th));
                }
                else
                {
                    VerticleCoRoutine *vc = (VerticleCoRoutine *)v;
                    INFO(" %8X | - | %10s |  ", vc->getHandle(),
                         v->name());
                }
            }
        }
    }
};

class LedTask : public VerticleTask
{
    uint32_t _gpio = 2;
    uint32_t _gpio2 = 16;
    uint32_t notifyValue;

  public:
    LedTask(const char *name)
        : VerticleTask(name, 256, 1)
    {
    }

    void run()
    {        
        gpio_enable(_gpio, GPIO_OUTPUT);
        gpio_enable(_gpio2, GPIO_OUTPUT);
        while (true)
        {
            wait(100);
            gpio_write(_gpio, 1);
            gpio_write(_gpio2, 0);
            wait(100);
            gpio_write(_gpio, 0);
            gpio_write(_gpio2, 1);
        }
    }
};


EventBus EB(1024);

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
        while (true)
        {
            wait(1000);
            //            EventBus::eventLoop();
        }
    }
};

Task task("task");

#include <Wifi.h>
#include <Telnet.h>
#include <Mqtt.h>



class DummyVerticle : public VerticleCoRoutine {
    public:
    DummyVerticle(const char* name):VerticleCoRoutine(name){
        
    }
    void run(CoRoutineHandle_t xHandle)
    {
        crSTART(xHandle); 
        for (;;)
        {
            crDELAY(xHandle, 10000);
            INFO(" coroutine-%s running", name());
        }
        crEND();
    }
};

Telnet telnet("telnet");
Wifi wifi("wifi");
LedTask led("LED");
Monitor monitor("monitor");
CoRoutineTask coRoutines("CoRout");
DummyVerticle dummy("DUMMY");
Mqtt mqtt("mqtt");




extern "C" void user_init(void)
{

    uart_set_baud(0, 921600);
    INFO("SDK version:%s\n", sdk_system_get_sdk_version());
    
    Verticle* pv;
    for(pv=Verticle::first();pv;pv=pv->next()) 
        pv->start();

    MessageHandler handler(task, (VerticleMethod)&Task::onMessage);
    EB.consumer("wifi.alive", handler);
}