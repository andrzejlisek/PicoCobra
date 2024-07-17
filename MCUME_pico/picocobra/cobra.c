
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
int zx80=0;
int autoload=1;

unsigned char keyboard_new[8];

#define PaletteC1 128
#define PaletteC2 255

struct { unsigned char R,G,B; } Palette[16] = {
  {         0,         0,         0},
  {         0,         0, PaletteC1},
  { PaletteC1,         0,         0},
  { PaletteC1,         0, PaletteC1},
  {         0, PaletteC1,         0},
  {         0, PaletteC1, PaletteC1},
  { PaletteC1, PaletteC1,         0},
  { PaletteC1, PaletteC1, PaletteC1},
  {         0,         0,         0},
  {         0,         0, PaletteC2},
  { PaletteC2,         0,         0},
  { PaletteC2,         0, PaletteC2},
  {         0, PaletteC2,         0},
  {         0, PaletteC2, PaletteC2},
  { PaletteC2, PaletteC2,         0},
  { PaletteC2, PaletteC2, PaletteC2}
};

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

void z81_Input(int bClick) {
  InKey_ = emu_GetPad();
  InKey0 = emu_ReadI2CKeyboard2(0);
  InKey1 = emu_ReadI2CKeyboard2(1);
  InKey2 = emu_ReadI2CKeyboard2(2);
  InKey3 = emu_ReadI2CKeyboard2(3);
  InKey4 = emu_ReadI2CKeyboard2(4);
  InKey5 = emu_ReadI2CKeyboard2(5);
  InKey6 = emu_ReadI2CKeyboard2(6);
}

void bitbufBlit(unsigned char * buf)
{
  emu_DrawVsync();  
  memset( XBuf, 0, WIDTH*8 ); 
  buf = buf + (ZX_VID_MARGIN*(ZX_VID_FULLWIDTH/8)) + 1;
  int y,x,i;
  byte d;
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
          *dst++=0;
        }
        else
        {
          *dst++=15;
        }
        d <<= 1;
      }       
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


/* despite the name, this also works for the ZX80 :-) */
void reset81()
{
  //interrupted=2;  /* will cause a reset */
  //memset(mem+0x4000,0,0xc000);  
}

void load_p(int a)
{
  emu_printf("loading...");

  emu_printf(tapename);
  int size = emu_FileSize(tapename);
  int f = emu_FileOpen(tapename, "r+b");  
  if ( !f ) {
    /* the partial snap will crash without a file, so reset */
    if(autoload)
      reset81(),autoload=0;
    return;
  }

  autoload=0;
  //emu_FileRead(mem + (zx80?0x4000:0x4009), size, f);
  //emu_FileClose(f);

  //if(zx80)
  //  store(0x400b,fetch(0x400b)+1);         
}

void save_p(int a)
{
  
}

static void initmem()
{

}


void emu_KeyboardOnDown(int keymodifer, int key) {
}

void emu_KeyboardOnUp(int keymodifer, int key) {
}


void z81_Init(void) 
{

#if HAS_SND
  emu_sndInit(); 
#endif 
  
  if (XBuf == 0) XBuf = (byte *)emu_Malloc(WIDTH*8);
  /* Set up the palette */
  int J;
  for(J=0;J<16;J++)
    emu_SetPaletteEntry(Palette[J].R,Palette[J].G,Palette[J].B, J);


  //????????memcpy(memchrset,CobraCHR,CobraCHR_size);
  
  Reset8910(&ay,3500000,0);
  
  
  //?????????memory_init();
  
  initmem();
 
  /* reset the keyboard state */
  memset( keyboard, 255, sizeof( keyboard ) );  

  ResetZ80();
 }


void SetConfigZ80_(void)
{
    SetConfigZ80();
}

void z81_Step(void)
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

void z81_Start(char * filename)
{
  char c;
  strncpy(tapename,filename,64);
  int f = emu_FileOpen(tapename, "r+b");
  if ( f ) {
    int fsize = emu_FileRead(&c, 1, f);
    if ( fsize == 0) { 
      autoload = 0;
      emu_printf("no autoload");
    }
    emu_FileClose(f);
  }
}
