
typedef unsigned char  byte;
//typedef unsigned short  word;

#define WIDTH  320
#define HEIGHT 192
#define BORDER 32

#define CYCLES_PER_FRAME 65000//3500000/50


/* full internal image with overscan (but not hsync/vsync areas) */
#define ZX_VID_MARGIN		55
#define ZX_VID_HMARGIN		(8*8)
#define ZX_VID_FULLWIDTH	(2*ZX_VID_HMARGIN+32*8)	/* sic */
#define ZX_VID_FULLHEIGHT	(2*ZX_VID_MARGIN+192)


/* AY board types */
#define AY_TYPE_NONE		0
#define AY_TYPE_QUICKSILVA	1
#define AY_TYPE_ZONX		2


extern unsigned char keyboard_new[8];

extern int memattr[64];
extern unsigned long tstates,tsmax;
extern int vsync_visuals;
extern int ramsize;
extern int interrupted;
extern int nmigen,hsyncgen,vsync;
extern int autoload;

extern void sighandler(int a);
extern unsigned int in(int AddrH, int AddrL);
extern unsigned int out(int AddrH, int AddrL, int Reg);
extern void do_interrupt();
extern void frame_pause(void);
extern void bitbufBlit(unsigned char * buf, unsigned char * bufpal);

