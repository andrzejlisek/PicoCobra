
#define rom_scrn
#include "cobrarom.h"
#undef rom_scrn

char screen_second = 0;

unsigned char screen_charmap = 0;

unsigned char screen_buf[32 * 24 * 3];

#define ZX_VID_MARGIN_		55
#define ZX_VID_HMARGIN_		(8*8)
#define ZX_VID_FULLWIDTH_	(2*ZX_VID_HMARGIN_+32*8)	/* sic */
#define ZX_VID_FULLHEIGHT_	(2*ZX_VID_MARGIN_+192)

#define screen_blit_offfset (screen_second ? (384 * 31) : 0)
#define screen_blit_offfset_color (screen_second ? 768 : 0)

char * CobraCHR;

unsigned char screen_raw[2 * (ZX_VID_FULLWIDTH_*ZX_VID_FULLHEIGHT_/8)];
unsigned char * screen_raw_color;

char screen_use_colors = 0;

void screen_char(int X, int Y, unsigned char Chr)
{
    int PtrC = Y * 32 + X;
    if (screen_buf[PtrC] != Chr)
    {
        screen_buf[PtrC] = Chr;

        int Ptr = 2645;
        Ptr += Y * 8 * 48 + X;
        int Chr_ = Chr << 3;
        for (int CharLine = 0; CharLine < 8; CharLine++)
        {
            screen_raw[Ptr] = CobraCHR[screen_charmap + Chr_ + CharLine];
            Ptr += 48;
        }
    }
}

void screen_color(int X, int Y, unsigned char Clr)
{
    int PtrC = Y * 32 + X;
    screen_raw_color[PtrC] = Clr;
}

void screen_clear()
{
    memset(screen_raw,0xff,ZX_VID_FULLWIDTH_*ZX_VID_FULLHEIGHT_/8);
    for (int Y = 0; Y < 24; Y++)
    {
        for (int X = 0; X < 32; X++)
        {
            screen_char(X, Y, 33);
            screen_color(X, Y, 15);
            screen_color(X, Y + 24, 240);
        }
    }
}

void screen_text(int X, int Y, unsigned char * Txt)
{
    int I = 0;
    while ((Txt[I] != 0) && (X < 32))
    {
        screen_char(X, Y, Txt[I]);
        X++;
        I++;
    }
}

void screen_num(int X, int Y, int Num)
{
    int NumL = 1;
    if (Num >= 10) NumL = 2;
    if (Num >= 100) NumL = 3;
    if (Num >= 1000) NumL = 4;
    if (Num >= 10000) NumL = 5;
    if (Num >= 100000) NumL = 6;
    if (Num >= 1000000) NumL = 7;
    if (Num >= 10000000) NumL = 8;
    
    if (Num == 0)
    {
        screen_char(X, Y, '0');
    }
    
    while (Num > 0)
    {
        screen_char(X + NumL - 1, Y, (Num % 10) + 48);
        Num = Num / 10;
        NumL--;
    }
}

void screen_hex1(int X, int Y, int Num)
{
    switch (Num)
    {
        case  0: screen_char(X, Y, '0'); break;
        case  1: screen_char(X, Y, '1'); break;
        case  2: screen_char(X, Y, '2'); break;
        case  3: screen_char(X, Y, '3'); break;
        case  4: screen_char(X, Y, '4'); break;
        case  5: screen_char(X, Y, '5'); break;
        case  6: screen_char(X, Y, '6'); break;
        case  7: screen_char(X, Y, '7'); break;
        case  8: screen_char(X, Y, '8'); break;
        case  9: screen_char(X, Y, '9'); break;
        case 10: screen_char(X, Y, 'A'); break;
        case 11: screen_char(X, Y, 'B'); break;
        case 12: screen_char(X, Y, 'C'); break;
        case 13: screen_char(X, Y, 'D'); break;
        case 14: screen_char(X, Y, 'E'); break;
        case 15: screen_char(X, Y, 'F'); break;
    }
}

void screen_hex2(int X, int Y, int Num)
{
    screen_hex1(X + 0, Y, Num >> 4);
    screen_hex1(X + 1, Y, Num & 15);
}

void screen_hex4(int X, int Y, int Num)
{
    screen_hex2(X + 0, Y, Num >> 8);
    screen_hex2(X + 2, Y, Num & 255);
}

void screen_bin4(int X, int Y, int Num)
{
    switch (Num)
    {
        case  0: screen_char(X, Y, '0'); screen_char(X + 1, Y, '0'); screen_char(X + 2, Y, '0'); screen_char(X + 3, Y, '0'); break;
        case  1: screen_char(X, Y, '0'); screen_char(X + 1, Y, '0'); screen_char(X + 2, Y, '0'); screen_char(X + 3, Y, '1'); break;
        case  2: screen_char(X, Y, '0'); screen_char(X + 1, Y, '0'); screen_char(X + 2, Y, '1'); screen_char(X + 3, Y, '0'); break;
        case  3: screen_char(X, Y, '0'); screen_char(X + 1, Y, '0'); screen_char(X + 2, Y, '1'); screen_char(X + 3, Y, '1'); break;
        case  4: screen_char(X, Y, '0'); screen_char(X + 1, Y, '1'); screen_char(X + 2, Y, '0'); screen_char(X + 3, Y, '0'); break;
        case  5: screen_char(X, Y, '0'); screen_char(X + 1, Y, '1'); screen_char(X + 2, Y, '0'); screen_char(X + 3, Y, '1'); break;
        case  6: screen_char(X, Y, '0'); screen_char(X + 1, Y, '1'); screen_char(X + 2, Y, '1'); screen_char(X + 3, Y, '0'); break;
        case  7: screen_char(X, Y, '0'); screen_char(X + 1, Y, '1'); screen_char(X + 2, Y, '1'); screen_char(X + 3, Y, '1'); break;
        case  8: screen_char(X, Y, '1'); screen_char(X + 1, Y, '0'); screen_char(X + 2, Y, '0'); screen_char(X + 3, Y, '0'); break;
        case  9: screen_char(X, Y, '1'); screen_char(X + 1, Y, '0'); screen_char(X + 2, Y, '0'); screen_char(X + 3, Y, '1'); break;
        case 10: screen_char(X, Y, '1'); screen_char(X + 1, Y, '0'); screen_char(X + 2, Y, '1'); screen_char(X + 3, Y, '0'); break;
        case 11: screen_char(X, Y, '1'); screen_char(X + 1, Y, '0'); screen_char(X + 2, Y, '1'); screen_char(X + 3, Y, '1'); break;
        case 12: screen_char(X, Y, '1'); screen_char(X + 1, Y, '1'); screen_char(X + 2, Y, '0'); screen_char(X + 3, Y, '0'); break;
        case 13: screen_char(X, Y, '1'); screen_char(X + 1, Y, '1'); screen_char(X + 2, Y, '0'); screen_char(X + 3, Y, '1'); break;
        case 14: screen_char(X, Y, '1'); screen_char(X + 1, Y, '1'); screen_char(X + 2, Y, '1'); screen_char(X + 3, Y, '0'); break;
        case 15: screen_char(X, Y, '1'); screen_char(X + 1, Y, '1'); screen_char(X + 2, Y, '1'); screen_char(X + 3, Y, '1'); break;
    }
}

void screen_bin8(int X, int Y, int Num)
{
    screen_bin4(X + 0, Y, Num >> 4);
    screen_bin4(X + 4, Y, Num & 15);
}

