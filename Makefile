PROGRAM=vertx-esp8266
EXTRA_COMPONENTS = extras/sntp ../Common  ../ArduinoJson  
# EXTRA_COMPONENTS = extras/paho_mqtt_c ../Common ../Ebos ../ArduinoJson
PROGRAM_SRC_DIR=. 
PROGRAM_INC_DIR=. ../esp-open-rtos/include ../Common  ../ArduinoJson ../esp-open-rtos/lwip/lwip/src/include
# PROGRAM_INC_DIR=. ../esp-open-rtos/include ../Common ../Ebos ../ArduinoJson
ESPBAUD=921600
TTY ?= USB0
SERIAL_PORT ?= /dev/tty$(TTY)
ESPPORT = $(SERIAL_PORT)
SERIAL_BAUD = 921600
LIBS= m hal gcc 
PROGRAM_CFLAGS= -DESP8266_OPEN_RTOS  build/sntp.a

include ../esp-open-rtos/common.mk

term:
	rm -f $(TTY)_minicom.log
	minicom -D $(SERIAL_PORT) -b $(SERIAL_BAUD) -C $(TTY)_minicom.log
