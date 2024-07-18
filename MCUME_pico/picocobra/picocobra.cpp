#include "pico.h"
#include "pico/stdlib.h"

extern "C" {
  #include "iopins.h"  
  #include "emuapi.h"  
}
#include "keyboard_osd.h"

extern "C" {
#include "cobra.h"
}
#include <stdio.h>
#include "pico_dsp.h"

volatile bool vbl=true;

bool repeating_timer_callback(struct repeating_timer *t) {   
    if (vbl) {
        vbl = false;
    } else {
        vbl = true;
    }   
    return true;
}

PICO_DSP tft;
static int skip=0;

#include "hardware/clocks.h"
#include "hardware/vreg.h"

int main(void) {
//    vreg_set_voltage(VREG_VOLTAGE_1_05);
//    set_sys_clock_khz(125000, true);    
//    set_sys_clock_khz(150000, true);    
//    set_sys_clock_khz(133000, true);    
    set_sys_clock_khz(200000, true);    
//    set_sys_clock_khz(210000, true);    
//    set_sys_clock_khz(230000, true);    
//    set_sys_clock_khz(225000, true);    
//    set_sys_clock_khz(250000, true);  
    stdio_init_all();

    emu_init();
    char * filename;
    menuActive();
    emu_DebounceLocalKeys();
    toggleMenu(false);
//#ifdef FILEBROWSER
//    while (true) {      
//        if (menuActive()) {
//            uint16_t bClick = emu_DebounceLocalKeys();
//            int action = handleMenu(bClick);
//            filename = menuSelection();   
//            if (action == ACTION_RUN) {
//              break;    
//            }
//            tft.waitSync();
//        }
//    }
//#endif    
    //tft.waitSync();
    emu_start();
    tft.startRefresh();
    Cobra_Init();
    struct repeating_timer timer;
    add_repeating_timer_ms(15, repeating_timer_callback, NULL, &timer);    
    while (true) {
        uint16_t bClick = emu_DebounceLocalKeys();
        Cobra_Input(bClick);  
        Cobra_Step();        
    }
}
static unsigned short palette16[PALETTE_SIZE];
void emu_SetPaletteEntry(unsigned char r, unsigned char g, unsigned char b, int index)
{
    if (index<PALETTE_SIZE) {
        palette16[index]  = RGBVAL16(r,g,b);        
    }
}

void emu_DrawLinePal16(unsigned char * VBuf, int width, int height, int line) 
{
    if (skip == 0) {
         tft.writeLinePal(width,height,line, VBuf, palette16);
    }
}

void emu_DrawLine16(unsigned short * VBuf, int width, int height, int line)
{
     if (skip == 0) {
        tft.writeLine(width,height,line, VBuf);
    }
}

int emu_IsVga(void)
{
    return (tft.getMode() == MODE_VGA_320x240?1:0);
}

void emu_DrawVsync(void)
{
    skip += 1;
    skip &= VID_FRAME_SKIP;
    volatile bool vb=vbl; 
    if ( emu_IsVga() ) {
        tft.waitSync(); 
    }
    else {
        //while (vbl==vb) {};
    }
}


#ifdef HAS_SND
#include "AudioPlaySystem.h"
AudioPlaySystem mymixer;

void emu_sndInit() {
  tft.begin_audio(256, mymixer.snd_Mixer);
  mymixer.start();    
}

void emu_sndPlaySound(int chan, int volume, int freq)
{
  if (chan < 6) {
    mymixer.sound(chan, freq, volume); 
  }
}

void emu_sndPlayBuzz(int size, int val) {
  mymixer.buzz(size,val); 
}

#endif
