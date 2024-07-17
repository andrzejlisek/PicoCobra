#ifndef cobra_memory_c
#define cobra_memory_c

#define rom_prog
#include "cobrarom.h"
#undef rom_prog



unsigned char memory_common = 1;


unsigned char memory_offset = 1;

unsigned char memory_raw[65536];

int memory_rom_size = 0;

///#define fetch(x) (memory_raw[x])

unsigned char fetch(unsigned short ad)
{
    if (memory_offset && (ad < 0x4000))
    {
        ad = ad + 0xC000;
    }
    if (memory_common && (ad >= 0xFC00) && (ad <= 0xFFFF)) { ad = ad - 0x400; }
    return memory_raw[ad];
}

#define fetch2(x) ((fetch((x)+1)<<8)|fetch(x))

void store(unsigned short ad, unsigned char b)
{
    if (memory_offset && (ad < 0x4000))
    {
        ad = ad + 0xC000;
    }
    if (memory_common && (ad >= 0xFC00) && (ad <= 0xFFFF)) { ad = ad - 0x400; }
    if ((ad >= 0xC000) && (ad < (0xC000 + memory_rom_size)))
    {
        return;
    }
    memory_raw[ad] = b;
    if ((ad >= 0xF800) && (ad < (0xF800 + 768)))
    {
        int ScrY = ad;
        int ScrX = ad;
        ScrX = (ScrX - 0xF800) % 32;
        ScrY = (ScrY - 0xF800) / 32;
        screen_char(ScrX, ScrY, b);
    }
}

static void store2b(unsigned short ad, unsigned char b1, unsigned char b2)
{
    store(ad, b2);
    store(ad + 1, b1);
}

//#define store(x,y) do {\
//    memory_raw[x] = y;\
//} while(0)
//
//#define store2b(x,hi,lo) do {\
//    memory_raw[x] = lo;\
//    memory_raw[x + 1] = hi;\
//} while(0)

#define store2(x,y) store2b(x,(y)>>8,(y)&255)

#ifdef __GNUC__
static void inline storefunc(unsigned short ad,unsigned char b){
   store(ad,b);
}
#undef store
#define store(x,y) storefunc(x,y)

static void inline store2func(unsigned short ad,unsigned char b1,unsigned char b2){
   store2b(ad,b1,b2);
}
#undef store2b
#define store2b(x,hi,lo) store2func(x,hi,lo)
#endif

void memory_init()
{
    memory_offset = 1;
    memset(memory_raw, 0, 65536);
    //memcpy(memory_raw + 0xC000, CobraROM, CobraROM_size);
    //memcpy(memory_raw, CobraRAM, CobraRAM_size);
}

#endif

