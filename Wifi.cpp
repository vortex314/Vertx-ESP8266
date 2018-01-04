
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



        while (1) {
            retries=30;
            while ((status != STATION_GOT_IP) && (retries)) {
                status = sdk_wifi_station_get_connect_status();
                printf("%s: status = %d\n\r", __func__, status);
                if (status == STATION_WRONG_PASSWORD) {
                    printf("WiFi: wrong password\n\r");
                    break;
                } else if (status == STATION_NO_AP_FOUND) {
                    printf("WiFi: AP not found\n\r");
                    break;
                } else if (status == STATION_CONNECT_FAIL) {
                    printf("WiFi: connection failed\r\n");
                    break;
                }
                wait(1000);
                --retries;
            }
            if (status == STATION_GOT_IP) {
                printf("WiFi: Connected\n\r");
                wait(1000);
            }

            while ((status = sdk_wifi_station_get_connect_status()) == STATION_GOT_IP) {
                wait(1000);
            }
            printf("WiFi: disconnected\n\r");
            sdk_wifi_station_disconnect();
            wait(1000);
        }
    }
}
