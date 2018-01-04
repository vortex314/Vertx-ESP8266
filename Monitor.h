#ifndef MONITOR_H
#define MONITOR_H

#include <vertx.h>

class Monitor : public VerticleCoRoutine
{
public:
    Monitor(const char *name);

    void run();
};

#endif // MONITOR_H
