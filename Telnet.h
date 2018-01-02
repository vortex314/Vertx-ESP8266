#ifndef _TELNET_H_
#define _TELNET_H_
#include <vertx.h>
#include <vertx.h>
#include "lwip/err.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/mem.h"
class Telnet : public VerticleTask
{
  public:
    Telnet(const char *name);
    void run();
};

#endif

