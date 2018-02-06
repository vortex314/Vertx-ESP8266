PROGRAM=vertx-esp8266
EXTRA_COMPONENTS = extras/sntp extras/mdnsresponder extras/rboot-ota ../Common  ../ArduinoJson ../DWM1000 extras/paho_mqtt_c
# EXTRA_COMPONENTS = extras/paho_mqtt_c ../Common ../Ebos ../ArduinoJson
PROGRAM_SRC_DIR=. 
PROGRAM_INC_DIR=. ../esp-open-rtos/include ../Common  ../ArduinoJson ../DWM1000 ../etl/src  $(ROOT)bootloader $(ROOT)bootloader/rboot
PROGRAM_CXXFLAGS += -fno-threadsafe-statics -std=c++11 -fno-rtti -lstdc++ -fno-exceptions -DPSWD=${PSWD} -DSSID=${SSID}
# PROGRAM_INC_DIR=. ../esp-open-rtos/include ../Common ../Ebos ../ArduinoJson ../esp-open-rtos/lwip/lwip/src/include 
ESPBAUD=921600
TTY ?= USB0
SERIAL_PORT ?= /dev/tty$(TTY)
ESPPORT = $(SERIAL_PORT)
SERIAL_BAUD = 921600
LIBS= m hal gcc 
PROGRAM_CFLAGS= -DESP_OPEN_RTOS
EXTRA_CFLAGS=-DEXTRAS_MDNS_RESPONDER -DLWIP_MDNS_RESPONDER=1 -DLWIP_NUM_NETIF_CLIENT_DATA=1 -DLWIP_NETIF_EXT_STATUS_CALLBACK=1 -DPSWD=${PSWD} -DSSID=${SSID}
# FLAVOR=sdklike

include ../esp-open-rtos/common.mk

term:
	rm -f $(TTY)_minicom.log
	minicom -D $(SERIAL_PORT) -b $(SERIAL_BAUD) -C $(TTY)_minicom.log
