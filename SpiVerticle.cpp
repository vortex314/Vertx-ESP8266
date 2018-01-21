#include "SpiVerticle.h"

SpiVerticle::SpiVerticle(const char* name)  : VerticleCoRoutine(name),_spi(Spi::create(12,13,14,15))
{

}

SpiVerticle::~SpiVerticle()
{
}

void SpiVerticle::start()
{
    eb.on("mqtt/connected",[this](Message& msg) {

    });
    _spi.setClock(Spi::SPI_CLOCK_125K);
    _spi.setHwSelect(true);
    _spi.setMode(Spi::SPI_MODE_PHASE0_POL0);
    _spi.setLsbFirst(false);
    _spi.init();
    VerticleCoRoutine::start();
}

Str line(40);
void logIn(Bytes& in)
{

    line.clear();
    line.appendHex(in);
    INFO(" SPI  : %s ",line.c_str());
}

#include <esp/spi.h>

uint32_t dividers[]= {
    SPI_FREQ_DIV_125K,
    SPI_FREQ_DIV_250K,
    SPI_FREQ_DIV_500K,
    SPI_FREQ_DIV_1M,
    SPI_FREQ_DIV_2M,
    SPI_FREQ_DIV_4M,
    SPI_FREQ_DIV_8M,
    SPI_FREQ_DIV_10M,
    SPI_FREQ_DIV_20M,
    SPI_FREQ_DIV_40M,
    SPI_FREQ_DIV_80M
};

void SpiVerticle::run()
{

    static uint32_t idx=0;
    Bytes out(10);
    Bytes in(10);

    uint8_t readDevice[]= {0,0,0,0,0};
    uint8_t writeEuid[]= {0x81,1,2,3,4,5,6,7,8};
    uint8_t readEuid[]= {0x1,0,0,0,0,0,0,0,0};

    PT_BEGIN();
    while(true) {

        INFO("clock : %d ",idx);
        spi_set_frequency_div(1,dividers[idx]);

        PT_WAIT(10);

        out.clear();
        out.write(readDevice,0,sizeof(readDevice));
        _spi.exchange(in,out);
        logIn(in);

        out.clear();
        out.write(writeEuid,0,sizeof(writeEuid));
        _spi.exchange(in,out);
        logIn(in);

        out.clear();
        out.write(readEuid,0,sizeof(readEuid));
        _spi.exchange(in,out);
        logIn(in);
        idx++;
        if ( idx > 5) idx=0;

    }
    PT_END();
}
