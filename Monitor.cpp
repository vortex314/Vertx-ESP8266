#include "Monitor.h"
#include <Property.h>

char taskState(uint8_t eCurrentState)
{
    char cStatus;
    switch (eCurrentState) {
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


Monitor::Monitor(const char *name)
    : VerticleCoRoutine(name)
{
    new PropertyReference<uint32_t>("system/lowestStack",_lowestStack,5000);
}

void Monitor::run()
{
    crSTART(handle());
    while (true) {
        crDELAY(handle(),5000);
        _lowestStack = 100000;
        INFO(" free heap : %d", xPortGetFreeHeapSize());
        INFO(" Address  | S |   TASK     |  STACK | prio");
        INFO("----------|---|------------|--------|-------");
        for (Verticle *v = first(); v != 0; v = v->next()) {
            if (v->isTask()) {
                VerticleTask* vt = (VerticleTask*)v;
                TaskHandle_t th=vt->getHandle();
                if ( th != 0)
                    INFO(" %8X | %c | %10s |  %5ld | %3ld",
                         vt->getHandle(),
                         taskState(eTaskGetState(th)),
                         vt->name(),
                         uxTaskGetStackHighWaterMark(th),
                         uxTaskPriorityGet(th));
                if ( uxTaskGetStackHighWaterMark(th) < _lowestStack ) _lowestStack = uxTaskGetStackHighWaterMark(th);
            } else {
                VerticleCoRoutine *vc = (VerticleCoRoutine *)v;
                INFO(" %8X | - | %10s |  ", vc->getHandle(),
                     v->name());
            }
        }
    }
    crEND();
}
