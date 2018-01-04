

#include <vertx.h>

VerticleCoRoutine::VerticleCoRoutine(const char *name) : Verticle()
{
    _name = new char[strlen(name) + 1];
    strcpy(_name, name);
    add(this);
}
const char *VerticleCoRoutine::name()
{
    return _name;
}
CoRoutineHandle_t VerticleCoRoutine::getHandle()
{
    return _xHandle;
}
void VerticleCoRoutine::run()
{
    crSTART(handle());
    for (;;) {
        crDELAY(handle(), 1000);
        INFO(" coroutine-%X running", handle());
    }
    crEND();
}
void VerticleCoRoutine::handler(CoRoutineHandle_t xHandle, UBaseType_t uxIndex)
{
    VerticleCoRoutine *pvc = (VerticleCoRoutine *)uxIndex;
    pvc->_xHandle = xHandle;
    pvc->run();
}

void VerticleCoRoutine::start()
{
    xCoRoutineCreate(handler, 0, (UBaseType_t)this);
}
void VerticleCoRoutine::stop()
{
}
void VerticleCoRoutine::onMessage(Cbor &msg)
{
}
bool VerticleCoRoutine::isTask()
{
    return false;
}
