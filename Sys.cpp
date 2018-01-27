/*
 * Sys.cpp
 *
 *  Created on: May 15, 2016
 *      Author: lieven
 */
#include <FreeRTOS.h>
#include <task.h>
#include <Log.h>
#include <Sys.h>
#include <stdint.h>
#include <sys/time.h>


uint64_t Sys::_upTime;

void Sys::init()
{
    // uint8_t cpuMhz = sdk_system_get_cpu_frequency();
}

uint32_t Sys::getFreeHeap()
{
    return xPortGetFreeHeapSize();
};

char Sys::_hostname[30];
uint64_t Sys::_boot_time = 0;
uint64_t Sys::millis()
{
    /*
    timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);*/
    return Sys::micros() / 1000;
}

uint64_t Sys::micros()
{

    static uint32_t lsClock = 0;
    static uint32_t msClock = 0;


    vPortEnterCritical();
    uint32_t ccount;
    __asm__ __volatile__("esync; rsr %0,ccount"
                         : "=a"(ccount));
    if (ccount < lsClock) {
        msClock++;
    }
    lsClock = ccount;
    portEXIT_CRITICAL();


    uint64_t micros = msClock;
    micros <<= 32;
    micros += lsClock;

    return micros / 80;

}

uint64_t Sys::now()
{
    return _boot_time + Sys::millis();
}

void Sys::setNow(uint64_t n)
{
    _boot_time = n - Sys::millis();
}

void Sys::hostname(const char *h)
{
    strncpy(_hostname, h, strlen(h) + 1);
}

void Sys::setHostname(const char *h)
{
    strncpy(_hostname, h, strlen(h) + 1);
}

void Sys::delay(unsigned int delta)
{
    uint32_t end = Sys::millis() + delta;
    while (Sys::millis() < end)
        ;
}

extern "C" uint64_t SysMillis()
{
    return Sys::millis();
}

const char *Sys::hostname()
{
    return _hostname;
}

/*
 *
 *  ATTENTION : LOGF call Sys::hostname, could invoke another call to
 * Sys::hostname with LOGF,....
 *
 *
 *
 */
#ifdef __ARDUINO__
#include <Arduino.h>

uint32_t Sys::getFreeHeap()
{
    return ESP.getFreeHeap();
}

uint64_t Sys::millis()
{
    return ::millis();
}

#endif
