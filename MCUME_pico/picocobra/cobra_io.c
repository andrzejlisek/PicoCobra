unsigned int in(int AddrH, int AddrL)
{
    int Reg = 0x00;

    // Status drukarki Mera-Blonie D-100
    /*if (Printer == 1)
    {
        if (AddrL == 0x14)
        {
            Reg = 0xff;
        }
    }*/

    // Status drukarki Seikosha GP-500A
    /*if (Printer == 2)
    {
        if (AddrL == 0x04)
        {
            Reg = 0x00;
        }
    }*/

    // Adresowanie niepelne - pozostawianie istotnych bitow
    AddrL = AddrL & 0x80;

    // Klawiatura i magnetofon
    if (AddrL == 0x80)
    {
        // Klawiatura
        //NoStd = false;
        Reg = keyboard_system_state(AddrH);

        // Magnetofon
        Reg = Reg & 0x7F;
    }
    return Reg;
}

unsigned int out(int AddrH, int AddrL, int Reg)
{
    /*
    // Wysylanie znaku do drukarki Mera-Blonie D-100
    if (Printer == 1)
    {
        if (AddrL == 0x16)
        {
            int X = Reg;
            X = 255 - X;
            uchar XX = X;
            PrintBuffer.push_back(XX);
        }
    }

    // Wysylanie znaku do drukarki Seikosha GP-500A
    if (Printer == 2)
    {
        if (AddrL == 0x00)
        {
            uchar XX = Reg;
            PrintBuffer.push_back(XX);
        }
    }

    // Sterowanie dzwiekiem
    if (AddrL == 0xFE)
    {
        //cout << "Dzwiek " << Eden::IntToHex8(Reg) << endl;
        SoundLevel1 = 1 - SoundLevel1;
    }

    // AY-3-8910 - numer rejestr
    if ((AddrH == 0xFF) && (AddrL == 0xFD))
    {
        AudioAY_->SetRegN(Reg);
    }

    // AY-3-8910 - wartosc rejestru
    if ((AddrH == 0xBF) && (AddrL == 0xFD))
    {
        AudioAY_->SetRegV(Reg);
    }
    */

    //cout << "XXX  "  << Eden::IntToHex8(AddrH) << Eden::IntToHex8(AddrL) << "  " << Eden::IntToHex8(Reg) << endl;

    // Adresowanie niepelne - pozostawianie istotnych bitow
    AddrL = AddrL & 0x9C; /*b10011100*/;

    /*
    // Katarynka
    if (AddrL == 0x00)
    {
        //cout << "Katarynka1 " << Eden::IntToHex8(Reg) << endl;
        if (Reg > 0)
        {
            SoundLevel2 = 2;
        }
        else
        {
            SoundLevel2 = 0;
        }
    }*/

    // Rom monitor
    if (AddrL == 0x04)
    {
        //cout << "Monitor " << Eden::IntToHex8(Reg) << endl;
        MemorySetRom(Reg);
    }

    // Katarynka
    /*if (AddrL == 0x08)
    {
        //cout << "Katarynka2 " << Eden::IntToHex8(Reg) << endl;
        if (Reg > 0)
        {
            SoundLevel2 = 2;
        }
        else
        {
            SoundLevel2 = 0;
        }
    }*/

    // Zmiana czcionki
    if (AddrL == 0x0C)
    {
        MemorySetFont(Reg);
    }

    // Zmiana banku pamieci
    if (AddrL == 0x10)
    {
        //cout << "Bank " << Eden::IntToHex8(Reg) << endl;
        MemorySetRom(Reg);
    }

    // Sygnal dzwiekowy
    /*if (AddrL == 0x18)
    {
        SoundSignal = true;
        //NoStd = false;
    }
    */

    // Zapis na magnetofon, przelaczenie offsetu
    if (AddrL == 0x1C)
    {
        //Tape_->Pulse();
        //NoStd = false;

        memory_offset = 0;
    }
    return 1;
}

void screen_debug_reset()
{
}

void screen_debug_info_(int pc, int sp, int RegA, int RegF, int RegB, int RegC, int RegD, int RegE, int RegH, int RegL, int RegBC, int RegDE, int RegHL)
{
}

void screen_debug_info(int pc)
{
    if (screen_second)
    {
        screen_hex2( 0, 54, keyboard_new[0]);
        screen_hex2( 2, 54, keyboard_new[1]);
        screen_hex2( 4, 54, keyboard_new[2]);
        screen_hex2( 6, 54, keyboard_new[3]);
        screen_hex2( 8, 54, keyboard_new[4]);
        screen_hex2(10, 54, keyboard_new[5]);
        screen_hex2(12, 54, keyboard_new[6]);
        screen_hex2(14, 54, keyboard_new[7]);

        screen_char(16, 54, keyboard_capslock ? '#' : '_');
        screen_char(17, 54, keyboard_event_buffer_shift_state1 ? '#' : '_');
        screen_char(18, 54, keyboard_event_buffer_shift_state2 ? '#' : '_');
    }
}

