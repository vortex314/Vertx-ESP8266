#ifndef HARDWARE_H
#define HARDWARE_H
#include <Bytes.h>
#include <Erc.h>

class Bytes;

namespace HW
{

typedef void (*FunctionPointer)(void *);

typedef unsigned int uint32_t;
typedef uint32_t Erc;
typedef unsigned char uint8_t;
typedef uint32_t PhysicalPin;
typedef enum {
    LP_TXD = 0,
    LP_RXD,
    LP_SCL,
    LP_SDA,
    LP_MISO,
    LP_MOSI,
    LP_SCK,
    LP_CS
} LogicalPin;

class Driver
{
public:
    virtual Erc init() = 0;
    virtual Erc deInit() = 0;
};

class UART : public Driver
{
public:
    static UART &create(PhysicalPin txd, PhysicalPin rxd);
    virtual Erc init() = 0;
    virtual Erc deInit() = 0;
    virtual Erc setClock(uint32_t clock) = 0;

    virtual Erc write(const uint8_t *data, uint32_t length) = 0;
    virtual Erc write(uint8_t b) = 0;
    virtual Erc read(Bytes &bytes) = 0;
    virtual uint8_t read() = 0;
    virtual void onRxd(FunctionPointer, void *) = 0;
    virtual void onTxd(FunctionPointer, void *) = 0;
    virtual uint32_t hasSpace() = 0;
    virtual uint32_t hasData() = 0;
};

//===================================================== GPIO DigitalIn ========

class DigitalIn : public Driver
{
public:
    typedef enum { DIN_NONE,
                   DIN_RAISE,
                   DIN_FALL,
                   DIN_CHANGE
                 } PinChange;
    static DigitalIn &create(PhysicalPin pin);
    virtual int read() = 0;
    virtual Erc init() = 0;
    virtual Erc deInit() = 0;
    virtual Erc onChange(PinChange pinChange, FunctionPointer fp, void *object) = 0;
    virtual PhysicalPin getPin() = 0;
};
//===================================================== GPIO DigitalOut
class DigitalOut : public Driver
{
    PhysicalPin _gpio;

public:
    static DigitalOut &create(PhysicalPin pin);
    virtual Erc init();
    virtual Erc deInit();
    virtual Erc write(int);
    virtual PhysicalPin getPin();
};
//===================================================== I2C ===

#define I2C_WRITE_BIT
#define I2C_READ_BIT 0

class I2C : public Driver
{
public:
    static I2C &create(PhysicalPin scl, PhysicalPin sda);
    ~I2C();
    virtual Erc init() = 0;
    virtual Erc deInit() = 0;
    virtual Erc setClock(uint32_t) = 0;
    virtual Erc setSlaveAddress(uint8_t address) = 0;
    virtual Erc write(uint8_t *data, uint32_t size) = 0;
    virtual Erc write(uint8_t data) = 0;
    virtual Erc read(uint8_t *data, uint32_t size) = 0;
};

class SPI : public Driver
{
public:
    typedef enum { SPI_MODE_0,
                   SPI_MODE_1,
                   SPI_MODE_2,
                   SPI_MODE_3
                 } SPIMode;

    static SPI &create(PhysicalPin miso, PhysicalPin mosi, PhysicalPin sck,
                       PhysicalPin cs);
    ~SPI();
    virtual Erc init() = 0;
    virtual Erc deInit() = 0;
    virtual Erc exchange(Bytes &in, Bytes &out) = 0;
    virtual Erc onExchange(FunctionPointer, void *) = 0;
    virtual Erc setClock(uint32_t) = 0;
    virtual Erc setMode(SPIMode) = 0;
    virtual Erc setLsbFirst(bool) = 0;
};

class ADC
{
    uint32_t _pin;

public:
    ADC(uint32_t pin);
    Erc init();
    float getValue();
};

class Connector
{
    uint32_t _pinsUsed;
    uint32_t _connectorIdx;
    uint32_t _physicalPins[8];
    UART *_uart;
    SPI *_spi;
    I2C *_i2c;

private:
    uint32_t toPin(uint32_t logicalPin);
    void lockPin(LogicalPin);
    bool isUsedPin(LogicalPin lp) {
        return _pinsUsed & lp;
    }
    void freePin(LogicalPin);

public:
    Connector(uint32_t idx);
    UART &getUART();
    SPI &getSPI();
    I2C &getI2C();
    DigitalIn &getDigitalIn(LogicalPin);
    DigitalOut &getDigitalOut(LogicalPin);
    ADC &getADC();
    // PWM& getPWM();
};

}

#endif
