extern unsigned char keyboard_new[8];

unsigned char keyboard_old[8];

unsigned char keyboard_diff[8];

void keyboard_init()
{
    keyboard_new[0] = 0;
    keyboard_new[1] = 0;
    keyboard_new[2] = 0;
    keyboard_new[3] = 0;
    keyboard_new[4] = 0;
    keyboard_new[5] = 0;
    keyboard_new[6] = 0;
    keyboard_new[7] = 0;

    keyboard_old[0] = 0;
    keyboard_old[1] = 0;
    keyboard_old[2] = 0;
    keyboard_old[3] = 0;
    keyboard_old[4] = 0;
    keyboard_old[5] = 0;
    keyboard_old[6] = 0;
    keyboard_old[7] = 0;
}

int keyboard_system_FE = 0xFF;
int keyboard_system_FD = 0xFF;
int keyboard_system_FB = 0xFF;
int keyboard_system_F7 = 0xFF;
int keyboard_system_EF = 0xFF;
int keyboard_system_DF = 0xFF;
int keyboard_system_BF = 0xFF;
int keyboard_system_7F = 0xFF;


#define keyboard_event_buffer_size 20

unsigned short keyboard_event_buffer[keyboard_event_buffer_size];
unsigned char keyboard_event_buffer_i = 0;
unsigned char keyboard_event_buffer_o = 0;

unsigned char keyboard_event_buffer_shift_state1 = 0;
unsigned char keyboard_event_buffer_shift_state2 = 0;

unsigned char keyboard_capslock = 0;

void keyboard_event_buffer_add(unsigned short Addr, unsigned short Bit, unsigned short Shift, unsigned short Release)
{
    if (keyboard_capslock)
    {
        switch (Shift)
        {
            case 2: Shift = 3; break;
            case 3: Shift = 2; break;
        }
    }

    switch (Shift)
    {
        case 0:
            if (keyboard_event_buffer_shift_state1)
            {
                keyboard_event_buffer_add(0xFE, 0, 9, 1);
                keyboard_event_buffer_shift_state1 = 0;
            }
            break;
        case 1:
            if (!keyboard_event_buffer_shift_state1)
            {
                keyboard_event_buffer_add(0xFE, 0, 9, 0);
                keyboard_event_buffer_shift_state1 = 1;
            }
            break;
        case 2:
            if (keyboard_event_buffer_shift_state2)
            {
                if (!keyboard_event_buffer_shift_state1)
                {
                    keyboard_event_buffer_add(0xFE, 0, 9, 0);
                }
                keyboard_event_buffer_add(0xFB, 4, 9, 0);
                keyboard_event_buffer_add(0xFB, 4, 9, 1);
                keyboard_event_buffer_add(0xFE, 0, 9, 1);
                keyboard_event_buffer_shift_state2 = 0;
                keyboard_event_buffer_shift_state1 = 0;
            }
            else
            {
                if (keyboard_event_buffer_shift_state1)
                {
                    keyboard_event_buffer_add(0xFE, 0, 9, 1);
                    keyboard_event_buffer_shift_state1 = 0;
                }
            }
            break;
        case 3:
            if (!keyboard_event_buffer_shift_state2)
            {
                if (!keyboard_event_buffer_shift_state1)
                {
                    keyboard_event_buffer_add(0xFE, 0, 9, 0);
                }
                keyboard_event_buffer_add(0xFB, 4, 9, 0);
                keyboard_event_buffer_add(0xFB, 4, 9, 1);
                keyboard_event_buffer_add(0xFE, 0, 9, 1);
                keyboard_event_buffer_shift_state2 = 1;
                keyboard_event_buffer_shift_state1 = 0;
            }
            else
            {
                if (keyboard_event_buffer_shift_state1)
                {
                    keyboard_event_buffer_add(0xFE, 0, 9, 1);
                    keyboard_event_buffer_shift_state1 = 0;
                }
            }
            break;
    }
    
    keyboard_event_buffer[keyboard_event_buffer_i] = (Addr << 8) + (Release << 3) + (Bit);
    keyboard_event_buffer_i++;
    if (keyboard_event_buffer_i >= keyboard_event_buffer_size)
    {
        keyboard_event_buffer_i = 0;
    }
}

int keyboard_scan()
{
    int Chr = 0;
    int DiffSign = 0;
    for (int I = 0; I < 6; I++)
    {
        if ((keyboard_new[7] & 15) > 0)
        {
            if (keyboard_new[7] & 1) { keyboard_new[0] = keyboard_new[0] | 0x40; }
            if (keyboard_new[7] & 2) { keyboard_new[1] = keyboard_new[1] | 0x40; }
            if (keyboard_new[7] & 4) { keyboard_new[2] = keyboard_new[2] | 0x40; }
            if (keyboard_new[7] & 8) { keyboard_new[3] = keyboard_new[3] | 0x40; }
        }
    
        keyboard_diff[I] = keyboard_new[I] ^ keyboard_old[I];
        if (keyboard_diff[I] != 0)
        {
            if (keyboard_new[I] > keyboard_old[I])
            {
                DiffSign++;
            }
            else
            {
                DiffSign--;
            }
            keyboard_old[I] = keyboard_new[I];
        }
    }

    if (DiffSign != 0)
    {
        unsigned long long KeybDiff = 0;
        KeybDiff = KeybDiff + (((unsigned long long)(keyboard_diff[0])) << 32LL);
        KeybDiff = KeybDiff + (((unsigned long long)(keyboard_diff[1])) << 24LL);
        KeybDiff = KeybDiff + (((unsigned long long)(keyboard_diff[2])) << 16LL);
        KeybDiff = KeybDiff + (((unsigned long long)(keyboard_diff[3])) <<  8LL);
        KeybDiff = KeybDiff + (((unsigned long long)(keyboard_diff[4])));

        unsigned char Chr0 = 0;
        unsigned char Chr1 = 0;
        
        switch (KeybDiff)
        {
            case 0x2000000000LL: Chr0 = 'Q'; Chr1 = '1'; break;
            case 0x0020000000LL: Chr0 = 'W'; Chr1 = '2'; break;
            case 0x0000200000LL: Chr0 = 'E'; Chr1 = '3'; break;
            case 0x0000002000LL: Chr0 = 'R'; Chr1 = '4'; break;
            case 0x0000000020LL: Chr0 = 'T'; Chr1 = '5'; break;
            case 0x0000000008LL: Chr0 = 'Y'; Chr1 = '6'; break;
            case 0x0000000800LL: Chr0 = 'U'; Chr1 = '7'; break;
            case 0x0000080000LL: Chr0 = 'I'; Chr1 = '8'; break;
            case 0x0008000000LL: Chr0 = 'O'; Chr1 = '9'; break;
            case 0x0800000000LL: Chr0 = 'P'; Chr1 = '0'; break;
            case 0x0200000000LL: Chr0 = 8;   Chr1 = 31;  break; // Delete

            case 0x1000000000LL: Chr0 = 'A'; Chr1 = '!'; break;
            case 0x0010000000LL: Chr0 = 'S'; Chr1 = '@'; break;
            case 0x0000100000LL: Chr0 = 'D'; Chr1 = '#'; break;
            case 0x0000001000LL: Chr0 = 'F'; Chr1 = '$'; break;
            case 0x0000000010LL: Chr0 = 'G'; Chr1 = '%'; break;
            case 0x0000000001LL: Chr0 = 'H'; Chr1 = '+'; break;
            case 0x0000000100LL: Chr0 = 'J'; Chr1 = '&'; break;
            case 0x0000010000LL: Chr0 = 'K'; Chr1 = '*'; break;
            case 0x0001000000LL: Chr0 = 'L'; Chr1 = '-'; break;
            case 0x0100000000LL: Chr0 = 13;  Chr1 = 13;  break; // Enter

            case 0x0002000000LL: Chr0 = 'Z'; Chr1 = '('; break;
            case 0x0000020000LL: Chr0 = 'X'; Chr1 = ')'; break;
            case 0x0000000200LL: Chr0 = 'C'; Chr1 = '?'; break;
            case 0x0000000002LL: Chr0 = 'V'; Chr1 = '/'; break;
            case 0x0000000004LL: Chr0 = 'B'; Chr1 = '"'; break;
            case 0x0000000400LL: Chr0 = 'N'; Chr1 = '<'; break;
            case 0x0000040000LL: Chr0 = 'M'; Chr1 = '>'; break;
            case 0x0004000000LL: Chr0 = '.'; Chr1 = ','; break;
            case 0x0400000000LL: Chr0 = ' '; Chr1 = ' '; break;

            case 0x4000000000LL: Chr0 = 1;   Chr1 = 1;   break; // Strzalka w lewo
            case 0x0040000000LL: Chr0 = 2;   Chr1 = 2;   break; // Strzalka w prawo
            case 0x0000400000LL: Chr0 = 3;   Chr1 = 3;   break; // Strzalka w gore
            case 0x0000004000LL: Chr0 = 4;   Chr1 = 4;   break; // Strzalka w dol
        }
        
        if (Chr0 || Chr1)
        {
            if (keyboard_new[7] & 0x40)
            {
                Chr = Chr1;
            }
            else
            {
                Chr = Chr0;
                if ((Chr >= 'A') && (Chr <= 'Z'))
                {
                    if (keyboard_new[7] & 0x10)
                    {
                        Chr += 32;
                    }
                }
                if ((Chr == 8) && (keyboard_new[7] & 0x10))
                {
                    Chr = 30;
                }
            }
        }
        
        if (Chr)
        {
            switch (Chr)
            {
                case '1':
                    keyboard_event_buffer_add(0xF7, 0, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '!':
                    keyboard_event_buffer_add(0xF7, 0, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case '2':
                    keyboard_event_buffer_add(0xF7, 1, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '"':
                    keyboard_event_buffer_add(0xF7, 1, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case '3':
                    keyboard_event_buffer_add(0xF7, 2, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '#':
                    keyboard_event_buffer_add(0xF7, 2, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case '4':
                    keyboard_event_buffer_add(0xF7, 3, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '$':
                    keyboard_event_buffer_add(0xF7, 3, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case '5':
                    keyboard_event_buffer_add(0xF7, 4, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '%':
                    keyboard_event_buffer_add(0xF7, 4, 1, (DiffSign < 0) ? 1 : 0);
                    break;

                case 'Q':
                case 'q':
                    keyboard_event_buffer_add(0xFB, 0, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 3:
                    keyboard_event_buffer_add(0xFB, 0, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'W':
                case 'w':
                    keyboard_event_buffer_add(0xFB, 1, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'E':
                case 'e':
                    keyboard_event_buffer_add(0xFB, 2, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'R':
                case 'r':
                    keyboard_event_buffer_add(0xFB, 3, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'T':
                case 't':
                    keyboard_event_buffer_add(0xFB, 4, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;

                case 'A':
                case 'a':
                    keyboard_event_buffer_add(0xFD, 0, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 1:
                    keyboard_event_buffer_add(0xFD, 0, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'S':
                case 's':
                    keyboard_event_buffer_add(0xFD, 1, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 2:
                    keyboard_event_buffer_add(0xFD, 1, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'D':
                case 'd':
                    keyboard_event_buffer_add(0xFD, 2, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'F':
                case 'f':
                    keyboard_event_buffer_add(0xFD, 3, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'G':
                case 'g':
                    keyboard_event_buffer_add(0xFD, 4, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;

                case 8: // shift
                    keyboard_event_buffer_add(0xFE, 0, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'Z':
                case 'z':
                    keyboard_event_buffer_add(0xFE, 1, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case 4:
                    keyboard_event_buffer_add(0xFE, 1, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'X':
                case 'x':
                    keyboard_event_buffer_add(0xFE, 2, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case ':':
                    keyboard_event_buffer_add(0xFE, 2, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'C':
                case 'c':
                    keyboard_event_buffer_add(0xFE, 3, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case ';':
                    keyboard_event_buffer_add(0xFE, 3, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'V':
                case 'v':
                    keyboard_event_buffer_add(0xFE, 4, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '=':
                    keyboard_event_buffer_add(0xFE, 4, 1, (DiffSign < 0) ? 1 : 0);
                    break;


                case '6':
                    keyboard_event_buffer_add(0xEF, 4, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '&':
                    keyboard_event_buffer_add(0xEF, 4, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case '7':
                    keyboard_event_buffer_add(0xEF, 3, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '\'':
                    keyboard_event_buffer_add(0xEF, 3, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case '8':
                    keyboard_event_buffer_add(0xEF, 2, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '(':
                    keyboard_event_buffer_add(0xEF, 2, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case '9':
                    keyboard_event_buffer_add(0xEF, 1, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case ')':
                    keyboard_event_buffer_add(0xEF, 1, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case '0':
                    keyboard_event_buffer_add(0xEF, 0, 0, (DiffSign < 0) ? 1 : 0);
                    break;

                case 'Y':
                case 'y':
                    keyboard_event_buffer_add(0xDF, 4, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '@':
                    keyboard_event_buffer_add(0xDF, 4, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'U':
                case 'u':
                    keyboard_event_buffer_add(0xDF, 3, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '[':
                    keyboard_event_buffer_add(0xDF, 3, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'I':
                case 'i':
                    keyboard_event_buffer_add(0xDF, 2, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case ']':
                    keyboard_event_buffer_add(0xDF, 2, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'O':
                case 'o':
                    keyboard_event_buffer_add(0xDF, 1, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '^':
                    keyboard_event_buffer_add(0xDF, 1, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'P':
                case 'p':
                    keyboard_event_buffer_add(0xDF, 0, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;

                case 'H':
                case 'h':
                    keyboard_event_buffer_add(0xBF, 4, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '+':
                    keyboard_event_buffer_add(0xBF, 4, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'J':
                case 'j':
                    keyboard_event_buffer_add(0xBF, 3, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '-':
                    keyboard_event_buffer_add(0xBF, 3, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'K':
                case 'k':
                    keyboard_event_buffer_add(0xBF, 2, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '*':
                    keyboard_event_buffer_add(0xBF, 2, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'L':
                case 'l':
                    keyboard_event_buffer_add(0xBF, 1, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '/':
                    keyboard_event_buffer_add(0xBF, 1, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 13: // Enter
                    keyboard_event_buffer_add(0xBF, 0, 0, (DiffSign < 0) ? 1 : 0);
                    break;

                case 'B':
                case 'b':
                    keyboard_event_buffer_add(0x7F, 4, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '?':
                    keyboard_event_buffer_add(0x7F, 4, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'N':
                case 'n':
                    keyboard_event_buffer_add(0x7F, 3, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '<':
                    keyboard_event_buffer_add(0x7F, 3, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case 'M':
                case 'm':
                    keyboard_event_buffer_add(0x7F, 2, (Chr >= 96) ? 3 : 2, (DiffSign < 0) ? 1 : 0);
                    break;
                case '>':
                    keyboard_event_buffer_add(0x7F, 2, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case ',':
                    keyboard_event_buffer_add(0x7F, 1, 0, (DiffSign < 0) ? 1 : 0);
                    break;
                case '.':
                    keyboard_event_buffer_add(0x7F, 1, 1, (DiffSign < 0) ? 1 : 0);
                    break;
                case ' ':
                    keyboard_event_buffer_add(0x7F, 0, 0, (DiffSign < 0) ? 1 : 0);
                    break;

                case 30:
                    if (DiffSign > 0)
                    {
                        keyboard_capslock = 1 - keyboard_capslock;
                    }
                    break;
            }
        }
    }

    if (DiffSign > 0)
    {
        return Chr;
    }
    return 0;
}

int keyboard_system_update_counter = 0;

void keyboard_system_update()
{
    if (keyboard_event_buffer_o != keyboard_event_buffer_i)
    {
        if (keyboard_system_update_counter <= 0)
        {
            unsigned short BufVal = keyboard_event_buffer[keyboard_event_buffer_o];
            unsigned char BufAddr = BufVal >> 8;
            unsigned char BufBit = BufVal & 15;
        
            unsigned char KeyLineVal = 0;
            switch (BufAddr)
            {
                case 0xF7: KeyLineVal = keyboard_system_F7; break;
                case 0x7F: KeyLineVal = keyboard_system_7F; break;
                case 0xFE: KeyLineVal = keyboard_system_FE; break;
                case 0xEF: KeyLineVal = keyboard_system_EF; break;
                case 0xFB: KeyLineVal = keyboard_system_FB; break;
                case 0xBF: KeyLineVal = keyboard_system_BF; break;
                case 0xFD: KeyLineVal = keyboard_system_FD; break;
                case 0xDF: KeyLineVal = keyboard_system_DF; break;
            }

            switch (BufBit)
            {
                case  0: KeyLineVal = KeyLineVal & 0xFE; break;
                case  1: KeyLineVal = KeyLineVal & 0xFD; break;
                case  2: KeyLineVal = KeyLineVal & 0xFB; break;
                case  3: KeyLineVal = KeyLineVal & 0xF7; break;
                case  4: KeyLineVal = KeyLineVal & 0xEF; break;
                case  5: KeyLineVal = KeyLineVal & 0xDF; break;
                case  6: KeyLineVal = KeyLineVal & 0xBF; break;
                case  7: KeyLineVal = KeyLineVal & 0x7F; break;
                case  8: KeyLineVal = KeyLineVal | 0x01; break;
                case  9: KeyLineVal = KeyLineVal | 0x02; break;
                case 10: KeyLineVal = KeyLineVal | 0x04; break;
                case 11: KeyLineVal = KeyLineVal | 0x08; break;
                case 12: KeyLineVal = KeyLineVal | 0x10; break;
                case 13: KeyLineVal = KeyLineVal | 0x20; break;
                case 14: KeyLineVal = KeyLineVal | 0x40; break;
                case 15: KeyLineVal = KeyLineVal | 0x80; break;
            }

            switch (BufAddr)
            {
                case 0xFE: keyboard_system_FE = KeyLineVal; break;
                case 0xFD: keyboard_system_FD = KeyLineVal; break;
                case 0xFB: keyboard_system_FB = KeyLineVal; break;
                case 0xF7: keyboard_system_F7 = KeyLineVal; break;
                case 0xEF: keyboard_system_EF = KeyLineVal; break;
                case 0xDF: keyboard_system_DF = KeyLineVal; break;
                case 0xBF: keyboard_system_BF = KeyLineVal; break;
                case 0x7F: keyboard_system_7F = KeyLineVal; break;
            }
           
            keyboard_event_buffer_o++;
            if (keyboard_event_buffer_o >= keyboard_event_buffer_size)
            {
                keyboard_event_buffer_o = 0;
            }
            keyboard_system_update_counter = 3;
        }
    }
    if (keyboard_system_update_counter > 0)
    {
        keyboard_system_update_counter--;
    }
}

int keyboard_system_state(int Addr)
{
    int KeyState = 0xFF;
    if (!(Addr & 0x01)) KeyState = KeyState & keyboard_system_FE;
    if (!(Addr & 0x02)) KeyState = KeyState & keyboard_system_FD;
    if (!(Addr & 0x04)) KeyState = KeyState & keyboard_system_FB;
    if (!(Addr & 0x08)) KeyState = KeyState & keyboard_system_F7;
    if (!(Addr & 0x10)) KeyState = KeyState & keyboard_system_EF;
    if (!(Addr & 0x20)) KeyState = KeyState & keyboard_system_DF;
    if (!(Addr & 0x40)) KeyState = KeyState & keyboard_system_BF;
    if (!(Addr & 0x80)) KeyState = KeyState & keyboard_system_7F;
    return KeyState;
}

