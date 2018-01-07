#include "Mdns.h"

extern "C" {
#include <mdnsresponder.h>
}
enum {
    WIFI_CONNECTED
};

Mdns::Mdns(const char* name)  : VerticleTask("sntp",512,6)
{
}

void Mdns::start()
{
    eb.on("wifi/connected",[this](Message& msg) {
        signal(WIFI_CONNECTED);
        INFO(" SNTP wifi connected ");
        mdns_init();
        mdns_flags flags;
        flags = mdns_TCP;
        mdns_add_facility(Sys::hostname(), "_tcp", "Zoom=1", flags, 23, 600);
        flags = mdns_Browsable;
        mdns_add_facility(Sys::hostname(), "_telnet", "Zoom=1", flags, 23, 600);
    });

//    VerticleTask::start();
}

Mdns::~Mdns()
{
}
