#ifndef MDNS_H
#define MDNS_H
#include <vertx.h>
class Mdns : public VerticleTask
{
public:
    Mdns(const char* name);
    ~Mdns();
    void start();

};

#endif // MDNS_H
