echo " add SSID and PSWD in your profile ! With three preceding backslashes on each quote.  "
echo " add to the path to xtensa esp8266 compiler directory , "
echo " like this PATH=$HOME/workspace/esp-open-sdk/xtensa-lx106-elf/bin:$PATH "
git clone https://github.com/vortex314/vertx-esp8266
git clone https://github.com/vortex314/Common
git clone https://github.com/vortex314/DWM1000
git clone https://github.com/bblanchon/ArduinoJson
git clone https://github.com/ETLCPP/etl
#
# ESP-OPEN-SDK : build will take 30 min
#
git clone https://github.com/pfalcon/esp-open-sdk
sudo apt-get install make unrar-free autoconf automake libtool gcc g++ gperf \
    flex bison texinfo gawk ncurses-dev libexpat-dev python-dev python python-serial \
    sed git unzip bash help2man wget bzip2
sudo apt-get install libtool-bin
cd esp-open-sdk
make clean
git pull
git submodule sync
git submodule update --init
make toolchain esptool libhal STANDALONE=n # adapted build for esp-open-rtos
cd ..
#
#   ESP-OPEN-RTOS
#
git clone --recursive https://github.com/Superhouse/esp-open-rtos.git
#
#
#
cd vertx-esp8266
cp ArduinoJson.h ../ArduinoJson/component.mk
make flash
echo " you can change config parameters by using telnet to go to the device itself based on IP address or .local name"
echo " don't forget to set INCLUDE_eTaskGetState to 1 in FreeRTOS.h"
exit
#
# ESP32 
mkdir -p ~/esp
cd ~/esp
tar -xzf ~/Downloads/xtensa-esp32-elf-linux64-1.22.0-61-gab8375a-5.2.0.tar.gz
export PATH=$PATH:$HOME/esp/xtensa-esp32-elf/bin
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
git submodule update --init
