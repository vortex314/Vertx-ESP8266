#ifndef SNTP_H
#define SNTP_H

#include <vertx.h>

class Sntp : public VerticleTask
{
public:
    Sntp(const char* name);
    ~Sntp();
    void run();
    void start();
};

#endif // SNTP_H
