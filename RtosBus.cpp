

#include <memory>
#include <thread>

#include <Verticle.h>
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

class Logger : public VerticleTask
{
  public:
    Logger(const char *name, bool worker, uint16_t stack, uint8_t priority)
        : VerticleTask(name, worker, stack, priority)
    {
    }

    void run()
    {
        while (true)
        {
            wait(5000);
            INFO(" free heap : %d", xPortGetFreeHeapSize());
            INFO(" S |   TASK     |  STACK | prio");
            INFO("---|------------|--------|-------");
            for (AbstractVerticle *v = first(); v != 0; v = v->next())
            {
                //                if (v->isTask())
                {
                    VerticleTask *vt = (VerticleTask *)v;
                    INFO(" %c | %10s |  %5d | %3d",
                         taskState(eTaskGetState(vt->_taskHandle)),
                         vt->name(),
                         uxTaskGetStackHighWaterMark(vt->_taskHandle),
                         uxTaskPriorityGet(vt->_taskHandle));
                }
            }
        }
    }
};

class Led : public VerticleTask
{
    uint32_t _gpio = 2;
    uint32_t _gpio2 = 16;
    uint32_t notifyValue;

  public:
    Led(const char *name, bool worker, uint16_t stack, uint8_t priority)
        : VerticleTask(name, worker, stack, priority)
    {
    }
    void start()
    {
        gpio_enable(_gpio, GPIO_OUTPUT);
        gpio_enable(_gpio2, GPIO_OUTPUT);
        vTaskDelay(1000);
        VerticleTask::start();
        //        eb.consumer("LED", [this](Cbor &msg) {});
    }
    void run()
    {
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

// Co-routine to be created.
void vFlashCoRoutine(CoRoutineHandle_t xHandle, UBaseType_t uxIndex)
{
    crSTART(xHandle);
    for (;;)
    {
        crDELAY(xHandle, 1000);
        INFO(" coroutine running");
    }
    crEND();
}

class VerticleCoRoutine : public AbstractVerticle
{
    char *_name;

  public:
    VerticleCoRoutine(const char *name) : AbstractVerticle()
    {
        _name = new char[strlen(name) + 1];
        strcpy(_name, name);
    }
    const char *name()
    {
        return _name;
    }
    static void run(CoRoutineHandle_t xHandle, UBaseType_t uxIndex)
    {
        crSTART(xHandle);
        for (;;)
        {
            crDELAY(xHandle, 1000);
            INFO(" coroutine2 running");
        }
        crEND();
    }
    void start()
    {
        xCoRoutineCreate(run, 0, 125);
    }
    void stop()
    {
    }
    void onMessage(Cbor &msg)
    {
    }
    bool isTask()
    {
        return false;
    }
};

class CoRoutineTask : public VerticleTask
{
  public:
    CoRoutineTask(const char *name)
        : VerticleTask(name, true, 256, 1)
    {
    }
    void run()
    {
        while (true)
        {
            vCoRoutineSchedule();
        }
    }
};

class TelnetServer : public VerticleTask
{
  public:
    TelnetServer(const char *name)
        : VerticleTask(name, true, 512, 5)
    {
    }

    void run()
    {
        while (true)
        {
            struct netconn *conn, *newconn;
            err_t err;
//            LWIP_UNUSED_ARG(arg);

            /* Create a new connection identifier. */
            /* Bind connection to well known port number 7. */
#if LWIP_IPV6
            conn = netconn_new(NETCONN_TCP_IPV6);
            netconn_bind(conn, IP6_ADDR_ANY, 23);
#else  /* LWIP_IPV6 */
            conn = netconn_new(NETCONN_TCP);
            netconn_bind(conn, IP_ADDR_ANY, 23);
#endif /* LWIP_IPV6 */
            LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

            /* Tell connection to go into listening mode. */
            netconn_listen(conn);

            while (1)
            {

                /* Grab new connection. */
                err = netconn_accept(conn, &newconn);
                /*printf("accepted new connection %p\n", newconn);*/
                /* Process the new connection. */
                if (err == ERR_OK)
                {
                    struct netbuf *buf;
                    void *data;
                    u16_t len;

                    netconn_write(newconn, " $ >", 4, NETCONN_COPY);

                    while ((err = netconn_recv(newconn, &buf)) == ERR_OK)
                    {
                        /*printf("Recved\n");*/
                        do
                        {
                            netbuf_data(buf, &data, &len);
                            err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 1
                            if (err != ERR_OK)
                            {
                                printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
                            }
#endif
                        } while (netbuf_next(buf) >= 0);
                        netbuf_delete(buf);
                    }
                    printf("Got EOF, looping\n");
                    /* Close connection and discard connection identifier. */
                    netconn_close(newconn);
                    netconn_delete(newconn);
                }
            }
        }
    }
};

extern "C" void user_init(void)
{

    uart_set_baud(0, 921600);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    (new Logger("logger", true, 256, 20))->start();
    VerticleTask* vt = new Led("LED", true, 512, 2);
    INFO(" VT created ");
    vt->print();
    vt->start();
    (new TelnetServer("telnetd"))->start();
    xCoRoutineCreate(vFlashCoRoutine, 0, 125);
    for(int i=0;i<20;i++)
    (new VerticleCoRoutine("cor2"))->start();
    (new CoRoutineTask("CoRout"))->start();
}