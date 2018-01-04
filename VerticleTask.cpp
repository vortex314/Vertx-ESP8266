#include <vertx.h>

VerticleTask::VerticleTask(const char *name, uint16_t stack, uint8_t priority)
{
    // _next = 0;
    _taskHandle=0;
    _name = new char[strlen(name) + 1];
    strcpy(_name, name);
    _stackSize = stack;
    _priority = priority;

    INFO(" name : %s : %X ", _name, this);
    _nextEvent = 8;
    INFO(" VerticleTask : %s = 0x%X", _name, this);
    add(this);
    // LinkedList<Verticle>::add(this);
};

void VerticleTask::run()
{
    while (true) {
        INFO(" default  run()   : %s : %X ", _name, this);
        wait(1000);
    }
}

void VerticleTask::handler(void *p)
{
    INFO(" running %X ",p);
    VerticleTask *pV = (VerticleTask *)p;
    pV->run();
}

void VerticleTask::start()
{

    /*    TimerHandle_t timer = xTimerCreate("LED", 10000 / portTICK_PERIOD_MS, pdTRUE, (void *)0, timerHandler);
    vTimerSetTimerID(timer, this);
    xTimerStart(timer, 0);*/
    if (xTaskCreate(&handler, _name, _stackSize, this, _priority, &_taskHandle)==pdPASS) {
        INFO(" Task created %s : %X ",_name,_taskHandle);
    } else {
        ERROR(" Task creation failed  %s : %X ",_name,_taskHandle);
    }
}

void VerticleTask::stop()
{
    vTaskSuspend( _taskHandle );
}

void VerticleTask::onMessage(Cbor &msg)
{
    // copy message
    // awake thread
    notify(N_MESSAGE);
    INFO(" received message in %s , default handler invoked.", _name);
}

uint32_t VerticleTask::newEvent()
{
    _nextEvent <<= 1;
    return _nextEvent;
}

void VerticleTask::notify(Notification n)
{
    xTaskNotify(_taskHandle, n, eSetBits);
}

uint32_t VerticleTask::wait(uint32_t time)
{
    uint32_t notification;
    xTaskNotifyWait(0x00, UINT32_MAX, &notification, time/portTICK_PERIOD_MS);
    if (notification)
        INFO(" %s notification received %d", _name, notification);
    return notification;
}



void VerticleTask::timerHandler(TimerHandle_t th)
{
    VerticleTask *pv = (VerticleTask *)pvTimerGetTimerID(th);
    pv->notify(N_TIMER);
}

const char *VerticleTask::name()
{
    return _name;
}

void VerticleTask::print()
{
    INFO(" name : %s , worker : %d, stack : %d, prio : %d",_name,isTask(),_stackSize,_priority);
}

TaskHandle_t VerticleTask::getHandle()
{
    return _taskHandle;;
}
