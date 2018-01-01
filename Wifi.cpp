
#include <Wifi.h>

Wifi::Wifi(const char *name) : Actor(name){};

void Wifi::setup()
{
    ZERO(config);
    strcpy((char *)config.ssid, WIFI_SSID);
    strcpy((char *)config.password, WIFI_PASS);
    INFO("WiFi: connecting to WiFi\n\r");
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    timeout(1000);
    state(uid.add("disconnected"));
    uid.add("connected");
};

void Wifi::onEvent(Cbor &msg)
{
    bool stateChanged = false;
    status = sdk_wifi_station_get_connect_status();
    // INFO("%s: status = %d", __func__, status);
    if (status == STATION_GOT_IP)
    {
        stateChanged = state(H("connected"));
        timeout(1000);
    }
    else
    {
        stateChanged = state(H("disconnected"));
        if (status == STATION_WRONG_PASSWORD)
        {
            ERROR("WiFi: wrong password");
        }
        else if (status == STATION_NO_AP_FOUND)
        {
            ERROR("WiFi: AP not found");
        }
        else if (status == STATION_CONNECT_FAIL)
        {
            ERROR("WiFi: connection failed");
        }
        if (stateChanged)
        {
            sdk_wifi_station_disconnect();
        }
        timeout(1000);
    };
}
