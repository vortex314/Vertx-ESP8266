
#include <vertx.h>
#include <Wifi.h>
#include <Config.h>

Wifi::Wifi(const char *name) : VerticleTask(name,500,6),_ssid(30),_pswd(40) {};
#include <lwip/api.h>
#include <lwip/netif.h>

void Wifi::start()
{
    config.setNameSpace("wifi");
    config.get("ssid",_ssid,WIFI_SSID);
    config.get("pswd",_pswd,WIFI_PASS);
    VerticleTask::start();
}

void startAP()
{
    struct sdk_softap_config apconfig;

    if(sdk_wifi_softap_get_config(&apconfig)) {

        strncpy((char *)apconfig.ssid, Sys::hostname(), 32);
        strncpy((char *)apconfig.password, "password", 32);

        apconfig.authmode = AUTH_WPA2_PSK;
        apconfig.ssid_hidden = 0;
        apconfig.max_connection = 4;
        apconfig.channel=7;

        if(!sdk_wifi_softap_set_config(&apconfig)) {
            printf("ESP8266 not set ap config!\r\n");
        }
    }
    struct ip_info info;
    IP4_ADDR(&info.ip, 192, 168, 22, 1);
//IP4_ADDR(&info.gw, 192, 168, 22, 1);
    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
}

void Wifi::run()
{

    while (true) {
        uint8_t retries = 30;
        uint8_t status = 0;
        // https://github.com/SuperHouse/esp-open-rtos/issues/333

        ZERO(_config);
        strcpy((char *)_config.ssid, _ssid.c_str());
        strcpy((char *)_config.password, _pswd.c_str());
        INFO("WiFi: connecting to WiFi");
//        waitSignal(1000);
DISCONNECTED : {
            INFO("WiFi: disconnected");
            eb.publish("wifi/disconnected");
            while(true) {
                retries=30;
                sdk_wifi_station_disconnect();
                netif_set_hostname(netif_default, Sys::hostname());
                sdk_wifi_set_opmode(STATION_MODE);
                sdk_wifi_station_set_config(&_config);
                sdk_wifi_station_connect();

 //               startAP();
                while (true) {
                    status = sdk_wifi_station_get_connect_status();
                    if ( status == STATION_GOT_IP ) goto CONNECTED;
                    INFO("Wifi status =%d , retries left: %d", status,retries);
                    if (status == STATION_WRONG_PASSWORD) {
                        INFO("WiFi: wrong password");
                        break;
                    } else if (status == STATION_NO_AP_FOUND) {
                        INFO("WiFi: AP not found");
                        break;
                    } else if (status == STATION_CONNECT_FAIL) {
                        INFO("WiFi: connection failed");
                        break;
                    }
                    --retries;
                    if ( retries==0) break;
                    waitSignal(1000);
                }
            }
        }
CONNECTED : {
            eb.publish("wifi/connected");
            INFO("WiFi: Connected");
            while (true) {
                status = sdk_wifi_station_get_connect_status();
                if ( status == STATION_GOT_IP) {
                    waitSignal(1000);
                } else {
                    sdk_wifi_station_disconnect();
                    goto DISCONNECTED;
                }
            }
        }
    }
}
