#include "Sntp.h"
#include <esp/uart.h>
#include <espressif/esp_common.h>

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>


#include <lwip/err.h>
#include <lwip/dns.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>

#include <ssid_config.h>
/* Add extras/sntp component to makefile for this include to work */
#include <sntp.h>
#include <time.h>

#define SNTP_SERVERS                                                           \
    "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org", "3.pool.ntp.org"

enum {
    WIFI_CONNECTED
};

Sntp::Sntp(const char* name)  : VerticleTask("sntp",250,1)
{
}

Sntp::~Sntp()
{
}

void Sntp::start()
{
    eb.on("wifi/connected",[this](Message& msg) {
        signal(WIFI_CONNECTED);
    });
}

void Sntp::run()
{


    while(true) {
        wait(1000);
        if ( hasSignal(WIFI_CONNECTED)) {
            const char *servers[] = {SNTP_SERVERS};
            /* SNTP will request an update each 5 minutes */
            sntp_set_update_delay(5 * 60000);
            /* Set GMT+1 zone, daylight savings off */
            const struct timezone tz = {1 * 60, 0};
            /* SNTP initialization */
            sntp_initialize(&tz);
            /* Servers must be configured right after initialization */
            sntp_set_servers((char**)servers, sizeof(servers) / sizeof(char *));
        } else {
            time_t ts = time(NULL);
            INFO("TIME: %s", ctime(&ts));
        }
    }
}


/*
 * Test code for SNTP on esp-open-rtos.
 *
 * Jesus Alonso (doragasu)
 */
