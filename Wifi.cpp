
#include <vertx.h>
#include <Wifi.h>
#include <Config.h>
#include <Property.h>


Wifi *Wifi::_wifi = 0; // system sigleton, needed in callback

Wifi::Wifi(const char *name) : VerticleTask(name, 500, 6)
	, _ssid(30)
	, _ssidPattern(30)
	, _pswd(40)
	,_ipAddress(20)
{
	_wifi = this;
	_foundAP = false;
};
#include <lwip/api.h>
#include <lwip/netif.h>

void Wifi::start()
{
	config.setNameSpace("wifi");
	config.get("ssid", _ssidPattern, WIFI_SSID);
	config.get("pswd", _pswd, WIFI_PASS);
	new PropertyFunction<const char *>("wifi/ssid", [this]() {
		return _ssid.c_str();
	},
	4000);
	new PropertyReference<int>("wifi/rssi", _rssi, 5000);
	new PropertyReference<Str>("wifi/ipAddress", _ipAddress, 5000);

	VerticleTask::start();
}
/*
void startAP()
{
	struct sdk_softap_config apconfig;

	if (sdk_wifi_softap_get_config(&apconfig)) {

		strncpy((char *)apconfig.ssid, Sys::hostname(), 32);
		strncpy((char *)apconfig.password, "password", 32);

		apconfig.authmode = AUTH_WPA2_PSK;
		apconfig.ssid_hidden = 0;
		apconfig.max_connection = 4;
		apconfig.channel = 7;

		if (!sdk_wifi_softap_set_config(&apconfig)) {
			printf("ESP8266 not set ap config!\r\n");
		}
	}
	struct ip_info info;
	IP4_ADDR(&info.ip, 192, 168, 22, 1);
	//IP4_ADDR(&info.gw, 192, 168, 22, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
}
*/
static const char *const auth_modes[] = {
	[AUTH_OPEN] = "Open",
	[AUTH_WEP] = "WEP",
	[AUTH_WPA_PSK] = "WPA/PSK",
	[AUTH_WPA2_PSK] = "WPA2/PSK",
	[AUTH_WPA_WPA2_PSK] = "WPA/WPA2/PSK"
};
/*
void	Wifi::wifi_handle_event_cb(System_Event_t	*evt)
{
	os_printf("event	%x\n",	evt->event);
	switch	(evt->event)	{
	case	EVENT_STAMODE_CONNECTED:
		os_printf("connect	to	ssid	%s,	channel	%d\n",
		          evt->event_info.connected.ssid,
		          evt->event_info.connected.channel);
		break;
	case	EVENT_STAMODE_DISCONNECTED:
		os_printf("disconnect	from	ssid	%s,	reason	%d\n",
		          evt->event_info.disconnected.ssid,
		          evt->event_info.disconnected.reason);
		break;
	case	EVENT_STAMODE_AUTHMODE_CHANGE:
		os_printf("mode:	%d	->	%d\n",
		          evt->event_info.auth_change.old_mode,
		          evt->event_info.auth_change.new_mode);
		break;
	case	EVENT_STAMODE_GOT_IP:
		os_printf("ip:"	IPSTR	",mask:"	IPSTR	",gw:"	IPSTR,
		          IP2STR(&evt->event_info.got_ip.ip),
		          IP2STR(&evt->event_info.got_ip.mask),
		          IP2STR(&evt->event_info.got_ip.gw));
		os_printf("\n");
		break;
	case	EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("station:	"	MACSTR	"join,	AID	=	%d\n",
		          MAC2STR(evt->event_info.sta_connected.mac),
		          evt->event_info.sta_connected.aid);
		break;
	case	EVENT_SOFTAPMODE_STADISCONNECTED:
		os_printf("station:	"	MACSTR	"leave,	AID	=	%d\n",
		          MAC2STR(evt->event_info.sta_disconnected.mac),
		          evt->event_info.sta_disconnected.aid);
		break;
	default:
		break;
	}
}*/

void Wifi::scan_done_cb(void *arg, sdk_scan_status_t status)
{
	char ssid[33]; // max SSID length + zero byte
	Str ssidStr(40);

	if (status != SCAN_OK) {
		WARN("Error: WiFi scan failed\n");
		return;
	}

	struct sdk_bss_info *bss = (struct sdk_bss_info *)arg;
	// first one is invalid
	bss = bss->next.stqe_next;

	struct sdk_bss_info *strongestAP = 0;
	int strongestRssi = -1000;
	while (NULL != bss) {
		size_t len = strlen((const char *)bss->ssid);
		memcpy(ssid, bss->ssid, len);
		ssid[len] = 0;

		ssidStr = ssid;

		INFO("%32s (" MACSTR ") RSSI: %02d, security: %s", ssid,
		     MAC2STR(bss->bssid), bss->rssi, auth_modes[bss->authmode]);
		if (ssidStr.startsWith(_wifi->_ssidPattern) && bss->rssi > strongestRssi) {
			strongestAP = bss;
			strongestRssi = bss->rssi;
		}
		bss = bss->next.stqe_next;
	}
	if (strongestAP) {
		_wifi->_ssid = (const char *)strongestAP->ssid;
		_wifi->_rssi = strongestAP->rssi;
		_wifi->_foundAP = true;
	}
}

void Wifi::run()
{

	while (true) {
		uint8_t retries = 5;
		uint8_t status = 0;
		// https://github.com/SuperHouse/esp-open-rtos/issues/333
		ZERO(_config);
		strcpy((char *)_config.password, _pswd.c_str());
		INFO("WiFi: connecting to WiFi");
SCAN: {
			_foundAP = false;
			while (true) {
				sdk_wifi_station_disconnect();
				netif_set_hostname(netif_default, Sys::hostname());
				sdk_wifi_set_opmode(STATION_MODE);
				sdk_wifi_station_scan(NULL, scan_done_cb);
				if (_foundAP) {
					strcpy((char *)_config.ssid, _ssid.c_str());
					break;
				}
				waitSignal(5000);
			}
		}
		//        waitSignal(1000);
DISCONNECTED: {
			INFO("WiFi: disconnected");
			eb.publish("wifi/disconnected");
			while (true) {
				retries = 10;
				sdk_wifi_station_disconnect();
				netif_set_hostname(netif_default, Sys::hostname());
				sdk_wifi_set_opmode(STATION_MODE);
				sdk_wifi_station_set_config(&_config);
//				sdk_wifi_set_event_hander_cb(wifi_handle_event_cb);

				sdk_wifi_station_connect();

				//               startAP();
				while (true) {
					status = sdk_wifi_station_get_connect_status();
					if (status == STATION_GOT_IP) {
						ip_info ipInfo;
						if ( sdk_wifi_get_ip_info(STATION_IF,&ipInfo)) {
							_ipAddress.clear();
							for(int i=0; i<4; i++ ) {
								if ( i) _ipAddress+=".";
								_ipAddress.append(((uint8_t*)&ipInfo.ip.addr)[i]);
							}
							INFO(" ip info %s",_ipAddress.c_str());
						}
						goto CONNECTED;

					}
					INFO("Wifi status =%d , retries left: %d", status, retries);
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
					if (retries == 0)
						goto SCAN;
					waitSignal(1000);
				}
			}
		}
CONNECTED: {
			eb.publish("wifi/connected");
			INFO("WiFi: Connected");
			while (true) {
				status = sdk_wifi_station_get_connect_status();
				if (status == STATION_GOT_IP) {
					waitSignal(1000);
				} else {
					sdk_wifi_station_disconnect();
					_foundAP = false;
					goto SCAN;
				}
			}
		}
	}
}
