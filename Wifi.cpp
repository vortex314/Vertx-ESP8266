
#include <vertx.h>
#include <Wifi.h>

Wifi::Wifi(const char *name) : VerticleTask(name,284,6) {};

void Wifi::run()
{

    Address myAddress("src/wifi");

    Cbor connected(10);
    connected.addKeyValue(UID.add("connected"),true);

    while (true) {
        uint8_t retries = 30;
        uint8_t status = 0;
        ZERO(config);
        strcpy((char *)config.ssid, WIFI_SSID);
        strcpy((char *)config.password, WIFI_PASS);
        INFO("WiFi: connecting to WiFi\n\r");
        sdk_wifi_set_opmode(STATION_MODE);
        sdk_wifi_station_set_config(&config);


        wait(5000);
        while (1) {
            retries=30;
            while ((status != STATION_GOT_IP) && (retries)) {
                status = sdk_wifi_station_get_connect_status();
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
                wait(1000);
                --retries;
            }
            if (status == STATION_GOT_IP) {
                INFO("WiFi: Connected");
                wait(1000);
            }

            while ((status = sdk_wifi_station_get_connect_status()) == STATION_GOT_IP) {
                wait(1000);
            }
            INFO("WiFi: disconnected");
            sdk_wifi_station_disconnect();
            wait(1000);
        }
    }
}
