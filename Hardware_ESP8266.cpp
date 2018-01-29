#include <Hardware.h>
#include <Log.h>


#include "esp/uart.h"
#include "esp/gpio.h"
#include "esp/spi.h"
#include "espressif/esp_common.h"


class DigitalIn_ESP8266 : public DigitalIn
{
    PhysicalPin _gpio;
    void* _object;
    FunctionPointer _fp;
    gpio_inttype_t _interrType;
    
    static DigitalIn_ESP8266* _din;
public:
    static void interruptHandler(uint8_t idx)
    {
        if (_din)
            _din->_fp(_din->_object);
    }
    DigitalIn_ESP8266(uint32_t gpio) : _gpio(gpio)
    {
        _fp=0;
        _object=0;
    }
    Erc init()
    {
        gpio_enable(_gpio, GPIO_INPUT);
        if ( _fp ) {
            gpio_set_interrupt(_gpio, _interrType, interruptHandler);
        }
        return E_OK;
    }

    Erc deInit()
    {
        gpio_enable(_gpio, GPIO_INPUT);
        return E_OK;
    }

    int read()
    {
        return gpio_read(_gpio);
    }

    Erc onChange(DigitalIn::PinChange pinChange, FunctionPointer fp, void *object)
    {
        _fp=fp;
        _object=object;
        _din=this;
        _interrType = GPIO_INTTYPE_EDGE_POS;
        if ( pinChange == DIN_RAISE ) _interrType = GPIO_INTTYPE_EDGE_POS;
        if ( pinChange == DIN_FALL ) _interrType = GPIO_INTTYPE_EDGE_NEG;
        if ( pinChange == DIN_CHANGE ) _interrType = GPIO_INTTYPE_EDGE_ANY;
        return E_OK;
    }
    PhysicalPin getPin()
    {
        return _gpio;
    }
};

DigitalIn_ESP8266* DigitalIn_ESP8266::_din=0;

DigitalIn &DigitalIn::create(PhysicalPin pin)
{
    DigitalIn_ESP8266 *ptr = new DigitalIn_ESP8266(pin);
    return *ptr;
}




class DigitalOut_ESP8266 : public DigitalOut
{
    PhysicalPin _gpio;

public:
    DigitalOut_ESP8266(uint32_t gpio) : _gpio(gpio)
    {
    }
    ~DigitalOut_ESP8266() {};
    Erc init()
    {
        gpio_enable(_gpio, GPIO_OUTPUT);
        return E_OK;
    }

    Erc deInit()
    {
        //      gpio_enable(_gpio, GPIO_INPUT);
        gpio_disable(_gpio);
        return E_OK;
    }

    Erc write(int x)
    {
        gpio_write(_gpio, x);
        return E_OK;
    }
    PhysicalPin getPin()
    {
        return _gpio;
    }
};



DigitalOut_ESP8266 dout(1);

DigitalOut &DigitalOut::create(PhysicalPin pin)
{
    DigitalOut_ESP8266 *ptr = new DigitalOut_ESP8266(pin);
    return *ptr;
}
/*

 *
 */


/*
 *  Bus 1:
 *   - MISO = GPIO 12
 *   - MOSI = GPIO 13
 *   - SCK  = GPIO 14
 *   - CS0  = GPIO 15 (if minimal_pins is false)
 * */



#define CLOCK_80MHZ 80000000

class SPI_ESP8266:public Spi
{
    FunctionPointer _fp;
    void* _arg;
    uint32_t _clock;
    SpiMode _mode;
    bool _lsbFirst;
    bool _hwSelect;
    PhysicalPin _miso,_mosi,_sck,_cs;
public:
    SPI_ESP8266(PhysicalPin miso, PhysicalPin mosi, PhysicalPin sck,
                PhysicalPin cs)
    {
        _clock = 1000000;
        _mode = SPI_MODE_PHASE0_POL0;
        _hwSelect = true;
        _lsbFirst = false;
        _cs=cs;
        _sck=sck;
        _mosi=mosi;
        _miso=miso;
    }
    ~SPI_ESP8266()
    {

    }
    Erc init()
    {
        spi_settings_t settings ;
        ZERO(settings);
        settings.mode = ( spi_mode_t) _mode;
        settings.msb= ! _lsbFirst;
        if ( _clock == SPI_CLOCK_125K ) settings.freq_divider = SPI_FREQ_DIV_125K;
        else if ( _clock == SPI_CLOCK_250K ) settings.freq_divider = SPI_FREQ_DIV_250K;
        else if ( _clock == SPI_CLOCK_500K ) settings.freq_divider = SPI_FREQ_DIV_500K;
        else if ( _clock == SPI_CLOCK_1M ) settings.freq_divider = SPI_FREQ_DIV_1M;
        else if ( _clock == SPI_CLOCK_2M ) settings.freq_divider = SPI_FREQ_DIV_2M;
        else if ( _clock == SPI_CLOCK_4M ) settings.freq_divider = SPI_FREQ_DIV_4M;
        else if ( _clock == SPI_CLOCK_10M ) settings.freq_divider = SPI_FREQ_DIV_10M;
        else if ( _clock == SPI_CLOCK_8M ) settings.freq_divider = SPI_FREQ_DIV_8M;
        else if ( _clock == SPI_CLOCK_20M ) settings.freq_divider = SPI_FREQ_DIV_20M;
        else settings.freq_divider = SPI_FREQ_DIV_1M;
        settings.endianness=SPI_LITTLE_ENDIAN;
        settings.minimal_pins=_hwSelect ? false:true;
        spi_set_settings(1,&settings);
        spi_clear_address(1);
        spi_clear_command(1);
        spi_clear_dummy(1);
        /*       uint32_t spi_ctrl2=0;
               spi_ctrl2 = READ_PERI_REG(SPI_CTRL2(1));
               spi_ctrl2 += ( 3 << SPI_MISO_DELAY_NUM_S );
               spi_ctrl2 += (1 << SPI_MISO_DELAY_MODE_S); // add delay for going trough mux , see ESP32 ref manual
               WRITE_PERI_REG(SPI_CTRL2(1), spi_ctrl2);*/


        /*    uint32_t spi_user=READ_PERI_REG(SPI_USER(1));
            spi_user  |= (SPI_USER0_CS_HOLD | SPI_USER0_CS_SETUP);
            WRITE_PERI_REG(SPI_USER(1), spi_user);*/
        return E_OK;
    };
    Erc deInit()
    {
        return E_OK;
    }
    // Black magic, for one reason or another the ESP8266 is always 1 bit off !
    Erc exchange(Bytes &in, Bytes &out)
    {
        spi_transfer(1, out.data(), in.data(), out.length(), SPI_8BIT);
        in.length((int)out.length());
        bool lowBit=0;
        for(uint32_t  i=0; i<out.length(); i++) {
            uint8_t b=in.peek(i);
            if ( lowBit ) in.poke(i,(b>>1)+0x80);
            else  in.poke(i,(b>>1));
            lowBit =  b & 1;
        }

        return E_OK;
    }
    Erc onExchange(FunctionPointer fp, void *arg)
    {
        _fp=fp;
        _arg=arg;
        return E_OK;
    }
    Erc setClock(uint32_t clock)
    {
        _clock=clock;
        return E_OK;
    }
    Erc setMode(SpiMode mode)
    {
        _mode=mode;
        return E_OK;
    }
    Erc setLsbFirst(bool lsbFirst)
    {
        _lsbFirst = lsbFirst;
        return E_OK;
    }
    Erc setHwSelect(bool hwSelect)
    {
        _hwSelect=hwSelect;
        return E_OK;
    }
};


Spi& Spi::create(PhysicalPin miso, PhysicalPin mosi, PhysicalPin sck,
                 PhysicalPin cs)
{
    Spi *ptr=new SPI_ESP8266( miso,  mosi,  sck,cs);
    return *ptr;
}
