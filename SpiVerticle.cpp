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
    _spi.setClock(Spi::SPI_CLOCK_1M);
    _spi.setHwSelect(true);
    _spi.setMode(Spi::SPI_MODE_PHASE1_POL0);
    _spi.setLsbFirst(false);
    _spi.init();
    VerticleCoRoutine::start();
}

void SpiVerticle::run()
{
    PT_BEGIN();
    while(true) {
        _spi.init();
        PT_WAIT(1000);
        Bytes out(10);
        for(int i=0; i<5; i++)
            out.write((uint8_t)0);
        Bytes in(10);
        _spi.exchange(in,out);
        Str line(40);
        line.appendHex(in);
        INFO(" SPI DEV_ID : %s ",line.c_str());
//       PT_WAIT(1000);
        out.clear();
        uint8_t writeEuid[]= {0x81,1,2,3,4,5,6,7,8};
        out.write(writeEuid,0,sizeof(writeEuid));
        _spi.exchange(in,out);

        line.clear();
        line.appendHex(in);
        INFO(" SPI  EUID W : %s ",line.c_str());

        out.clear();
        uint8_t readEuid[]= {0x1,0,0,0,0,0,0,0,0};
        out.write(readEuid,0,sizeof(readEuid));
        _spi.exchange(in,out);

        line.clear();
        line.appendHex(in);
        INFO(" SPI EUID R : %s ",line.c_str());

    }
    PT_END();
}
