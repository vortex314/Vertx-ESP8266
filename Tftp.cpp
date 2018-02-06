#include "Tftp.h"

Tftp::Tftp(const char* name) : VerticleCoRoutine(name)
{
}

Tftp::~Tftp()
{
}

/* A very simple OTA example
 *
 * Tries to run both a TFTP client and a TFTP server simultaneously, either will accept a TTP firmware and update it.
 *
 * Not a realistic OTA setup, this needs adapting (choose either client or server) before you'd want to use it.
 *
 * For more information about esp-open-rtos OTA see https://github.com/SuperHouse/esp-open-rtos/wiki/OTA-Update-Configuration
 *
 * NOT SUITABLE TO PUT ON THE INTERNET OR INTO A PRODUCTION ENVIRONMENT!!!!
 */
#include <string.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include "ssid_config.h"
extern "C" {
#include "rboot-api.h"
#include "ota-tftp.h"
}


#define TFTP_PORT 69

void Tftp::start()
{
    rboot_config conf = rboot_get_config();
    INFO("OTA Basic demo.Currently running on flash slot %d / %d.",
         conf.current_rom, conf.count);
    int slot = (conf.current_rom + 1) % conf.count;
    INFO("Image will be saved in OTA slot %d.", slot);
    if(slot == conf.current_rom) {
        INFO("FATAL ERROR: Only one OTA slot is configured!");
        while(1) {}
    }
    INFO("Image addresses in flash:");
    for(int i = 0; i <conf.count; i++) {
        INFO("%c%d: offset 0x%08x", i == conf.current_rom ? '*':' ', i, conf.roms[i]);
    }
    INFO("Starting TFTP server...");
    ota_tftp_init_server(TFTP_PORT);

}
