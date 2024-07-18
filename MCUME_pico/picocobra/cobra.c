
#define PROGMEM
#include "z80.h"
//#include "Arduino.h"
#include "emuapi.h"
#include "common.h"
#include "AY8910.h"
#include "string.h"

#define MEMORYRAM_SIZE 0x10000

static AY8910 ay;

int unexpanded=0;
int nmigen=0,hsyncgen=0,vsync=0;
int vsync_visuals=0;
int signal_int_flag=0;
int interrupted=0;

/* the keyboard state and other */
static byte keyboard[ 8 ] = {0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff};;
static byte * XBuf=0; 
int autoload=1;

unsigned char keyboard_new[8];

static char tapename[64]={0};
static const int kBuf[]={13,25,19,25,19,40}; //,21,40}; // LOAD "" (J shift p shift p, R ENTER) 
static const int tBuf[]={2,0,2,0,2,2};//200!,2,2};
static int kcount=0;
static int timeout=100;


void sighandler(int a)
{
  signal_int_flag=1;
}

void frame_pause(void)
{
  signal_int_flag=0;

  if(interrupted<2)
    interrupted=1;
}

void do_interrupt()
{
  /* being careful here not to screw up any pending reset... */
  if(interrupted==1)
    interrupted=0;
}

static unsigned char InKey_;
static unsigned char InKey0;
static unsigned char InKey1;
static unsigned char InKey2;
static unsigned char InKey3;
static unsigned char InKey4;
static unsigned char InKey5;
static unsigned char InKey6;

void Cobra_Input(int bClick) {
  InKey_ = emu_GetPad();
  InKey0 = emu_ReadI2CKeyboard2(0);
  InKey1 = emu_ReadI2CKeyboard2(1);
  InKey2 = emu_ReadI2CKeyboard2(2);
  InKey3 = emu_ReadI2CKeyboard2(3);
  InKey4 = emu_ReadI2CKeyboard2(4);
  InKey5 = emu_ReadI2CKeyboard2(5);
  InKey6 = emu_ReadI2CKeyboard2(6);
}

void bitbufBlit(unsigned char * buf, unsigned char * bufpal)
{
  emu_DrawVsync();  
  memset( XBuf, 0, WIDTH*8 ); 
  buf = buf + (ZX_VID_MARGIN*(ZX_VID_FULLWIDTH/8)) + 1;
  int y,x,i;
  byte d;
  int PalPtr = 0;
  for(y=0;y<192;y++)
  {
    byte * src = buf + 4;
    for(x=0;x<32;x++)
    {
      byte * dst=&XBuf[(x<<3)+BORDER];
      d = *src++;
      for (i=0;i<8;i++)
      {
        if ( d & 128 )
        {
          *dst++ = (bufpal[PalPtr] & 15); // Fore
        }
        else
        {
          *dst++ = (bufpal[PalPtr] >> 4); // Back
        }
        d <<= 1;
      }       
      PalPtr++;
    }
    if ((y & 7) != 7)
    {
        PalPtr -= 32;
    }
    emu_DrawLinePal16(&XBuf[0], WIDTH, HEIGHT, y);   
    buf += (ZX_VID_FULLWIDTH/8);
  }
}

static void updateKeyboard (void)
{
  InKey_ = emu_GetPad();
  keyboard_new[0] = InKey0;
  keyboard_new[1] = InKey1;
  keyboard_new[2] = InKey2;
  keyboard_new[3] = InKey3;
  keyboard_new[4] = InKey4;
  keyboard_new[5] = InKey5;
  keyboard_new[6] = InKey6;
  keyboard_new[7] = InKey_;
}


void emu_KeyboardOnDown(int keymodifer, int key)
{
}

void emu_KeyboardOnUp(int keymodifer, int key)
{
}


void Cobra_Init(void) 
{

#if HAS_SND
  emu_sndInit(); 
#endif 
  
  if (XBuf == 0) XBuf = (byte *)emu_Malloc(WIDTH*8);

  /* Set up the palette */
  unsigned char _1 = 128;
  unsigned char _2 = 255;  
  emu_SetPaletteEntry( 0,  0,  0,  0);
  emu_SetPaletteEntry( 0,  0, _1,  1);
  emu_SetPaletteEntry(_1,  0,  0,  2);
  emu_SetPaletteEntry(_1,  0, _1,  3);
  emu_SetPaletteEntry( 0, _1,  0,  4);
  emu_SetPaletteEntry( 0, _1, _1,  5);
  emu_SetPaletteEntry(_1, _1,  0,  6);
  emu_SetPaletteEntry(_1, _1, _1,  7);
  emu_SetPaletteEntry( 0,  0,  0,  8);
  emu_SetPaletteEntry( 0,  0, _2,  9);
  emu_SetPaletteEntry(_2,  0,  0, 10);
  emu_SetPaletteEntry(_2,  0, _2, 11);
  emu_SetPaletteEntry( 0, _2,  0, 12);
  emu_SetPaletteEntry( 0, _2, _2, 13);
  emu_SetPaletteEntry(_2, _2,  0, 14);
  emu_SetPaletteEntry(_2, _2, _2, 15);

  Reset8910(&ay,3500000,0);
  
  

  /* reset the keyboard state */
  memset( keyboard, 255, sizeof( keyboard ) );  

  SetConfigZ80();
  ResetZ80();
}

void Cobra_Step(void)
{
  ExecZ80();
  sighandler(0);
  //if (strlen(tapename)) handleKeyBuf();   
  updateKeyboard();
  Loop8910(&ay,20);
}

static int startsWith(const char * s, const char * prefix)
{
  int retval = 0;
  int len = strlen(s);
  int plen = strlen(prefix);
  if (len > plen ) {
    if (!strncmp(s, prefix, plen)) {
      retval = 1;
    }
  }
  return (retval);  
}

static int endsWith(const char * s, const char * suffix)
{
  int retval = 0;
  int len = strlen(s);
  int slen = strlen(suffix);
  if (len > slen ) {
    if (!strcmp(&s[len-slen], suffix)) {
      retval = 1;
    }
  }
  return (retval);  
}

