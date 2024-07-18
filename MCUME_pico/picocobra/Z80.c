/* Emulation of the Z80 CPU with hooks into the other parts of z81.
 * Copyright (C) 1994 Ian Collier.
 * z81 changes (C) 1995-2001 Russell Marks.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <string.h>	/* for memset/memcpy */
#include "cobra_io_screen.c"
#include "cobra_memory.c"
#include "cobra_io_keyboard.c"
#include "cobra_files.c"
#include "cobra_io.c"
#include "z80.h"

#define parity(a) (partable[a])

unsigned char partable[256]={
      4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
      0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
      0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
      4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
      0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
      4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
      4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
      0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
      0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
      4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
      4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
      0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
      4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
      0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
      0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
      4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4
   };


unsigned long tstates=0,tsmax=65000,frames=0;

/* odd place to have this, but the display does work in an odd way :-) */
unsigned char scrnbmp_new[ZX_VID_FULLWIDTH*ZX_VID_FULLHEIGHT/8]; /* written */

/* checked against for diffs */
int liney=0, lineyi=0;
int vsy=0;
unsigned long linestart=0;
int vsync_toggle=0,vsync_lasttoggle=0;

static int linestate=0, linex=0, nrmvideo=1;

#define LINEX 	((tstates-linestart)>>2)



/* for vsync off -> on */
void vsync_raise(void)
{
  /* save current pos */
  vsy=liney;
}


/* for vsync on -> off */
void vsync_lower(void)
{
  int ny=liney,y;
  
  vsync_toggle++;
  
  /* we don't emulate this stuff by default; if nothing else,
   * it can be fscking annoying when you're typing in a program.
   */
  if(!vsync_visuals)
    return;
  
  /* even when we do emulate it, we don't bother with x timing,
   * just the y. It gives reasonable results without being too
   * complicated, I think.
   */
  if(vsy<0) vsy=0;
  if(vsy>=ZX_VID_FULLHEIGHT) vsy=ZX_VID_FULLHEIGHT-1;
  if(ny<0) ny=0;
  if(ny>=ZX_VID_FULLHEIGHT) ny=ZX_VID_FULLHEIGHT-1;
  
  /* XXX both of these could/should be made into single memset calls */
  if(ny<vsy)
    {
    /* must be wrapping around a frame edge; do bottom half */
    for(y=vsy;y<ZX_VID_FULLHEIGHT;y++)
      memset(scrnbmp_new+y*(ZX_VID_FULLWIDTH/8),0xff,ZX_VID_FULLWIDTH/8);
    vsy=0;
    }
  
  for(y=vsy;y<ny;y++)
    memset(scrnbmp_new+y*(ZX_VID_FULLWIDTH/8),0xff,ZX_VID_FULLWIDTH/8);
}


unsigned char a, f, b, c, d, e, h, l;
unsigned char r, a1, f1, b1, c1, d1, e1, h1, l1, i, iff1, iff2, im;
unsigned short pc;
unsigned short ix, iy, sp;
unsigned long nextlinetime=0,linegap=208,lastvsyncpend=0;
unsigned char radjust;
unsigned char ixoriy, new_ixoriy;
unsigned char intsample=0;
unsigned char op;
int ulacharline=0;
int nmipend=0,intpend=0,vsyncpend=0,vsynclen=0;
int hsyncskip=0;
int framewait=0;


void SetConfigZ80(void)
{
    keyboard_init();
    memory_init();
    FileLoadConfig();
    FileLoadToMem(memory_raw, FileConfigRamName, 0, FileConfigRamSize);
    MemorySetFont(0);
    MemorySetRom(0);
    screen_clear();
    screen_debug_reset();
}


void ExecZ80(void)
{
  int retval=0;
  while(1)
  {
    /* this *has* to be checked before radjust is incr'd */
    if(intsample && !(radjust&64))
      intpend=1;
  
    ixoriy=new_ixoriy;
    new_ixoriy=0;
    intsample=1;
    op=fetch(pc);
  
    if (!nrmvideo) ulacharline = 0;

    screen_debug_info_(pc, sp, a, f, b, c, d, e, h, l, bc, de, hl);
  
    pc++;
    radjust++;
  
    switch(op)
    {
#include "z80ops.h"
    }
  
    if(tstates>=tsmax)
    {
      retval=1;   
      tstates-=tsmax;
      linestart-=tsmax;
      nextlinetime-=tsmax;
      lastvsyncpend-=tsmax;
      vsync_lasttoggle=vsync_toggle;
      vsync_toggle=0;
    
      frames++;
      frame_pause();
    }

    /* the vsync length test is pretty arbitrary, because
     * the timing isn't very accurate (more or less an instruction
     * count) - but it's good enough in practice.
     *
     * the vsync_toggle test is fairly arbitrary too;
     * there has to have been `not too many' for a TV to get
     * confused. In practice, more than one would screw it up,
     * but since we could be looking at over a frames' worth
     * given where vsync_toggle is zeroed, we play it safe.
     * also, we use a copy of the previous chunk's worth,
     * since we need a full frame(-plus) to decide this.
     */
    if(vsynclen && !vsync)
    {
      if(vsynclen>=10)
      {
        if(vsync_lasttoggle<=2)
        {
          vsyncpend=1;	/* start of frame */
          /* FAST mode screws up without this, but it's a bit
           * unpleasant... :-/
           */
          tstates=nextlinetime;
        }
      }
      else
      {
        /* independent timing for this would be awkward, so
         * anywhere on the line is good enough. Also,
         * don't count it as a toggle.
         */
        vsync_toggle--;
        hsyncskip=1;
      }
    }

    /* should do this all the time vsync is set */
    if(vsync)
    {
      ulacharline=0;
      vsynclen++;
    }
    else {
      vsynclen=0;    
    }
  
    if(tstates>=nextlinetime)	/* new line */
    {
      /* generate fake sync if we haven't had one for a while;
       * but if we just loaded/saved, wait for the first real frame instead
       * to avoid jumpiness.
       */
      if(!vsync && tstates-lastvsyncpend>=tsmax && !framewait)
        vsyncpend=1;
  
      /* but that won't ever happen if we always have vsync on -
       * i.e., if we're grinding away in FAST mode. So for that
       * case, we check for vsync being held for a full frame.
       */
      if(vsync_visuals && vsynclen>=tsmax)
      {
        vsyncpend=1;
        vsynclen=1;      
        goto postcopy;				/* skip the usual copying */
      }
  
      if(!vsyncpend)
      {
        if (!lineyi) liney++;
        
        if(hsyncgen && !hsyncskip)
        {
          ulacharline++;
          ulacharline&=7;
        }
      }
      else
      {
        unsigned char KeyChr = keyboard_scan();
        
        if (KeyChr == 31)
        {
            screen_second = 1 - screen_second;
        }
        
        screen_debug_info(pc);
        keyboard_system_update();
        bitbufBlit(screen_raw + screen_blit_offfset, screen_raw_color + screen_blit_offfset_color);
        postcopy:
        memset(scrnbmp_new,0,sizeof(scrnbmp_new));
        lastvsyncpend=tstates;
        vsyncpend=0;
        framewait=0;
        liney=-1;		/* XXX might be something up here */
      }
      
      if(nmigen)
        nmipend=1;
  
      hsyncskip=0;
      linestart=nextlinetime;
      nextlinetime+=linegap;
    }
  
    if(0)
    {
  
    if(intsample && nmipend)
    {
      nmipend=0;
  
      if(nmigen)
      {
        iff2=iff1;
        iff1=0;
        /* hardware syncs tstates to falling of NMI pulse (?),
         * so a slight kludge here...
         */
        if(fetch(pc)==0x76)
        {
          pc++;
          tstates=linestart;
        }
        else
        {
          /* this seems curiously long, but equally, seems
           * to be just about right. :-)
           */
          tstates+=27;
        }
        push2(pc);
        pc=0x66;
      }
    }

    if(intsample && intpend)
    {
      intpend=0;
      if(iff1)
      {
        if(fetch(pc)==0x76)pc++;
        iff1=iff2=0;
        tstates+=5; /* accompanied by an input from the data bus */
        switch(im)
        {
          case 0: /* IM 0 */
          case 1: /* undocumented */
          case 2: /* IM 1 */
            /* there is little to distinguish between these cases */
            tstates+=9; /* perhaps */
            push2(pc);
            pc=0x38;
            break;
          case 3: /* IM 2 */
            /* (seems unlikely it'd ever be used on the '81, but...) */
            tstates+=13; /* perhaps */
            {
              int addr=fetch2((i<<8)|0xff);
              push2(pc);
              pc=addr;
            }
        }
      }
    }
    }

    /* this isn't used for any sort of Z80 interrupts,
     * purely for the emulator's UI.
     */
    if (0) // Przerwania nie sa obslugiwane
    {
        if(interrupted)
        {
          if(interrupted==1)
          {
            do_interrupt();	/* also zeroes it */
          }
          else	/* must be 2 */
          {
            /* a kludge to let us do a reset */
            interrupted=0;
            a=f=b=c=d=e=h=l=a1=f1=b1=c1=d1=e1=h1=l1=i=iff1=iff2=im=r=0;
            ixoriy=new_ixoriy=0;
            ix=iy=sp=pc=0;
            tstates=radjust=0;
            nextlinetime=linegap;
            vsyncpend=vsynclen=0;
            hsyncskip=0;
          }
        }
    }
    
    if (retval) break;  
  }

}

void ResetZ80(void)
{
  a=f=b=c=d=e=h=l=a1=f1=b1=c1=d1=e1=h1=l1=i=iff1=iff2=im=r=0;
  ixoriy=new_ixoriy=0;
  ix=iy=sp=pc=0;
  tstates=radjust=0;
  nextlinetime=linegap; 
  tstates=0;
  frames=0;
  liney=0;
  vsy=0;
  linestart=0;
  vsync_toggle=0;
  vsync_lasttoggle=0;

  pc = 0x0000;
  ix = 0x0000; iy = 0x0000;
      
  radjust = 0x6a;
  sp = 0;
     
     

  //framewait=1;
}


