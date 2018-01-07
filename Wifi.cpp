
#include <vertx.h>
#include <Wifi.h>

Wifi::Wifi(const char *name) : VerticleTask(name,284,6) {};
#include <lwip/api.h>
#include <lwip/netif.h>
void Wifi::run()
{

    while (true) {
        uint8_t retries = 30;
        uint8_t status = 0;
        // https://github.com/SuperHouse/esp-open-rtos/issues/333
        sdk_wifi_station_disconnect();
        netif_set_hostname(netif_default, "ikke");
        sdk_wifi_station_connect();

        ZERO(config);
        strcpy((char *)config.ssid, WIFI_SSID);
        strcpy((char *)config.password, WIFI_PASS);
        INFO("WiFi: connecting to WiFi");
        wait(5000);
DISCONNECTED : {
            INFO("WiFi: disconnected");
            eb.publish("wifi/disconnected");
            while(true) {
                retries=30;
                sdk_wifi_set_opmode(STATION_MODE);
                sdk_wifi_station_set_config(&config);
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
                    wait(1000);
                }
                sdk_wifi_station_disconnect();
            }
        }
CONNECTED : {
            eb.publish("wifi/connected");
            INFO("WiFi: Connected");
            while (true) {
                status = sdk_wifi_station_get_connect_status();
                if ( status == STATION_GOT_IP) {
                    wait(1000);
                } else {
                    sdk_wifi_station_disconnect();
                    goto DISCONNECTED;
                }
            }
        }
    }
}
