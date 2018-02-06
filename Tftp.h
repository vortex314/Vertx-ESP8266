#ifndef TFTP_H
#define TFTP_H

#include <vertx.h>

class Tftp :public VerticleCoRoutine
{
public:
    Tftp(const char* name);
    ~Tftp();
    void start();
};

#endif // TFTP_H
