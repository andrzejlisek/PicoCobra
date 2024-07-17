#ifndef cobra_files_c
#define cobra_files_c

#include "emuapi.h"

#define FileDir "cobra/"
#define FileDirLen 6
#define FileConfig "cobra/cobra1.cfg"

int FileConfigRomSize;
int FileConfigRamSize;
int FileConfigChrSize;
int FileConfigLstSize;

char * FileConfigRomName;
char * FileConfigRamName;
char * FileConfigChrName;
char * FileConfigLstName;

int FileConfigChrSizeType = 0;

int * MemoryRomTable;

int FileRomBankCurrentSize = 0x800;
int FileRomBankCurrent = 0;
int FileChrBankCurrent = 0;

int FileReadLine1(int Pos, char * Buf)
{
    if (Pos < 0)
    {
        return -1;
    }
    int Ptr = 0;
    while (Buf[Pos + Ptr] < 32)
    {
        if (Buf[Pos + Ptr] == 0)
        {
            return -1;
        }
        Ptr++;
    }
    return Ptr;
}

int FileReadLine2(int Pos, char * Buf)
{
    if (Pos < 0)
    {
        return -1;
    }
    if (Buf[Pos] < 32)
    {
        return -1;
    }
    int Ptr = 0;
    while (Buf[Pos + Ptr] >= 32)
    {
        Ptr++;
    }
    return Ptr;
}

char FileLoadConfigParam(char * Buf, int BufPos, int BufLen, const char * Param)
{
    if (strlen(Param) > BufLen)
    {
        return 0;
    }
    for (int I = 0; I < strlen(Param); I++)
    {
        if (Buf[BufPos + I] != Param[I])
        {
            return 0;
        }
    }
    return 1;
}

int FileLoadConfigValue(char * Buf, int BufPos, int BufLen)
{
    int Val = 0;
    int ValChr;
    for (int I = 0; I < BufLen; I++)
    {
        Val = Val * 10;
        if ((Buf[BufPos + I] >= '0') && (Buf[BufPos + I] <= '9'))
        {
            ValChr = Buf[BufPos + I] - 48;
            Val = Val + ValChr;
        }
    }
    return Val;
}

int FileLoadConfigValueHex(char * Buf, int BufPos, int BufLen)
{
    int Val = 0;
    int ValChr;
    for (int I = 0; I < BufLen; I++)
    {
        Val = Val * 16;
        if ((Buf[BufPos + I] >= '0') && (Buf[BufPos + I] <= '9'))
        {
            ValChr = Buf[BufPos + I] - 48;
            Val = Val + ValChr;
        }
        if ((Buf[BufPos + I] >= 'A') && (Buf[BufPos + I] <= 'F'))
        {
            ValChr = Buf[BufPos + I] - 65;
            Val = Val + ValChr;
        }
        if ((Buf[BufPos + I] >= 'a') && (Buf[BufPos + I] >= 'f'))
        {
            ValChr = Buf[BufPos + I] - 97;
            Val = Val + ValChr;
        }
    }
    return Val;
}

int FileLoadToMem(char * MemPtr, char * FileName, int FileOffset, int DataSize)
{
    if (FileName)
    {
        if (DataSize > 0)
        {
            int FileHandle = emu_FileOpen(FileName, "r+b");
            if (FileHandle)
            {
                emu_FileSeek(FileHandle, FileOffset, 0);
                emu_FileRead(MemPtr, DataSize, FileHandle);
                emu_FileClose(FileHandle);
            }
        }
    }
}

void MemorySetRom(int Page)
{
    int MemoryRomPageOffset = MemoryRomTable[(Page << 1) + 0];
    int MemoryRomPageSize = MemoryRomTable[(Page << 1) + 1];
    FileLoadToMem(memory_raw + 0xC000, FileConfigRomName, MemoryRomPageOffset, MemoryRomPageSize);
    memory_rom_size = MemoryRomPageSize;
}

void MemorySetFont(int Page)
{
    // Jezeli liczba jest co najmniej 128, to nalezy uzyc czcionki z pamieci RAM
    //if (Page >= 128)
    //{
        //Page = 256;
    //}
    
    if (FileConfigChrSize >= 2048)
    {
        if (FileConfigChrSizeType > 0)
        {
            int FontFileOffset = Page & FileConfigChrSizeType;
            FontFileOffset = FontFileOffset * 2048;
            FileLoadToMem(CobraCHR, FileConfigChrName, FontFileOffset, 2048);
        }
        else
        {
            FileLoadToMem(CobraCHR, FileConfigChrName, 0, 2048);
        }
    }
}

void FileLoadConfig()
{
    screen_raw_color = emu_Malloc(32 * 24 * 3);
    CobraCHR = emu_Malloc(2048);
    for (int I = 0; I < 2048; I++)
    {
        CobraCHR[I] = CobraCHR_[I];
    }

    MemoryRomTable = emu_Malloc(512 * sizeof(int));

    FileConfigRomSize = 0;
    FileConfigRamSize = 0;
    FileConfigChrSize = 0;
    FileConfigLstSize = 0;

    FileConfigRomName = 0;
    FileConfigRamName = 0;
    FileConfigChrName = 0;
    FileConfigLstName = 0;

    int StringSize = 100;
    int f = emu_FileOpen(FileConfig, "r+b");
    if (f)
    {

        int ConfFileSize = emu_FileSize(FileConfig);

        char * c = emu_Malloc(ConfFileSize + 5);

        char * StrX = emu_Malloc(StringSize + 5);

        int fsize = emu_FileRead(c, ConfFileSize, f);
        c[ConfFileSize] = 0;
        emu_FileClose(f);

        int c_ptr = 0;
        int Line1 = 0;
        int Line2 = 0;

        int TI = 32;
        while (1)
        {
            Line1 = FileReadLine1(c_ptr, c);
            if (Line1 >= 0)
            {
                c_ptr += Line1;

                Line2 = FileReadLine2(c_ptr, c);
                if (Line2 >= 0)
                {
                    if (FileLoadConfigParam(c, c_ptr, Line2, "CpuMem_ScreenMemCommon="))
                    {
                        int Val = FileLoadConfigValue(c, c_ptr + 23, Line2 - 23);
                        if (Val)
                        {
                            screen_text(0, 31, "Common");
                            memory_common = 1;
                        }
                        else
                        {
                            screen_text(0, 31, "Separa");
                            memory_common = 0;
                        }
                    }
                    if (FileLoadConfigParam(c, c_ptr, Line2, "Screen_ScreenColor="))
                    {
                        int Val = FileLoadConfigValue(c, c_ptr + 19, Line2 - 19);
                        if (Val)
                        {
                            screen_text(7, 31, "Color");
                            screen_use_colors = 1;
                        }
                        else
                        {
                            screen_text(7, 31, "Mono ");
                            screen_use_colors = 0;
                        }
                    }
                    if (FileLoadConfigParam(c, c_ptr, Line2, "FileRom="))
                    {
                        TI += 2;

                        int ParamLen = 8;
                    
                        screen_char(0, TI, 'R');
                        screen_char(1, TI, 'O');
                        screen_char(2, TI, 'M');

                        for (int I = 0; I < FileDirLen; I++)
                        {
                            StrX[I] = FileDir[I];
                        }
                        for (int I = 0; I < (Line2 - ParamLen); I++)
                        {
                            StrX[I + FileDirLen] = c[c_ptr + ParamLen + I];
                        }
                        StrX[FileDirLen + Line2 - ParamLen] = 0;
                        
                        if (strlen(StrX) > (FileDirLen + 3))
                        {
                            int FileConfigRomHandle = emu_FileOpen(StrX, "r+b");
                            if (FileConfigRomHandle)
                            {
                                FileConfigRomName = emu_Malloc(strlen(StrX) + 2);
                                strcpy(FileConfigRomName, StrX);
                                FileConfigRomSize = emu_FileSize(StrX);
                                screen_text(10, TI, FileConfigRomName);
                                emu_FileClose(FileConfigRomHandle);
                            }
                        }
                        screen_num(4, TI, FileConfigRomSize);

                        TI -= 2;
                    }
                    if (FileLoadConfigParam(c, c_ptr, Line2, "FileRam="))
                    {
                        TI += 3;
                    
                        int ParamLen = 8;
                    
                        screen_char(0, TI, 'R');
                        screen_char(1, TI, 'A');
                        screen_char(2, TI, 'M');
                        
                        for (int I = 0; I < FileDirLen; I++)
                        {
                            StrX[I] = FileDir[I];
                        }
                        for (int I = 0; I < (Line2 - ParamLen); I++)
                        {
                            StrX[I + FileDirLen] = c[c_ptr + ParamLen + I];
                        }
                        StrX[FileDirLen + Line2 - ParamLen] = 0;
                        
                        if (strlen(StrX) > (FileDirLen + 3))
                        {
                            int FileConfigRamHandle = emu_FileOpen(StrX, "r+b");
                            if (FileConfigRamHandle)
                            {
                                FileConfigRamName = emu_Malloc(strlen(StrX) + 2);
                                strcpy(FileConfigRamName, StrX);
                                FileConfigRamSize = emu_FileSize(StrX);
                                screen_text(10, TI, FileConfigRamName);
                                emu_FileClose(FileConfigRamHandle);
                            }
                        }
                        screen_num(4, TI, FileConfigRamSize);

                        TI -= 3;
                    }
                    if (FileLoadConfigParam(c, c_ptr, Line2, "FileChr="))
                    {
                        int ParamLen = 8;

                        screen_char(0, TI, 'C');
                        screen_char(1, TI, 'H');
                        screen_char(2, TI, 'R');
                        
                        for (int I = 0; I < FileDirLen; I++)
                        {
                            StrX[I] = FileDir[I];
                        }
                        for (int I = 0; I < (Line2 - ParamLen); I++)
                        {
                            StrX[I + FileDirLen] = c[c_ptr + ParamLen + I];
                        }
                        StrX[FileDirLen + Line2 - ParamLen] = 0;
                        
                        if (strlen(StrX) > (FileDirLen + 3))
                        {
                            int FileConfigChrHandle = emu_FileOpen(StrX, "r+b");
                            if (FileConfigChrHandle)
                            {
                                FileConfigChrName = emu_Malloc(strlen(StrX) + 2);
                                strcpy(FileConfigChrName, StrX);
                                FileConfigChrSize = emu_FileSize(StrX);
                                screen_text(10, TI, FileConfigChrName);
                                emu_FileClose(FileConfigChrHandle);
                                
                                if (FileConfigChrSize >=   2048) FileConfigChrSizeType = 0;
                                if (FileConfigChrSize >=   4096) FileConfigChrSizeType = 1;
                                if (FileConfigChrSize >=   8192) FileConfigChrSizeType = 3;
                                if (FileConfigChrSize >=  16384) FileConfigChrSizeType = 7;
                                if (FileConfigChrSize >=  32768) FileConfigChrSizeType = 15;
                                if (FileConfigChrSize >=  65536) FileConfigChrSizeType = 31;
                                if (FileConfigChrSize >= 131072) FileConfigChrSizeType = 63;
                                if (FileConfigChrSize >= 262144) FileConfigChrSizeType = 127;
                                if (FileConfigChrSize >= 524288) FileConfigChrSizeType = 255;
                            }
                        }
                        screen_num(4, TI, FileConfigChrSize);
                    }
                    if (FileLoadConfigParam(c, c_ptr, Line2, "FileLst="))
                    {
                        TI += 1;

                        int ParamLen = 8;
                    
                        screen_char(0, TI, 'L');
                        screen_char(1, TI, 'S');
                        screen_char(2, TI, 'T');
                        
                        for (int I = 0; I < FileDirLen; I++)
                        {
                            StrX[I] = FileDir[I];
                        }
                        for (int I = 0; I < (Line2 - ParamLen); I++)
                        {
                            StrX[I + FileDirLen] = c[c_ptr + ParamLen + I];
                        }
                        StrX[FileDirLen + Line2 - ParamLen] = 0;
                        
                        if (strlen(StrX) > (FileDirLen + 3))
                        {
                            int FileConfigLstHandle = emu_FileOpen(StrX, "r+b");
                            if (FileConfigLstHandle)
                            {
                                FileConfigLstName = emu_Malloc(strlen(StrX) + 2);
                                strcpy(FileConfigLstName, StrX);
                                FileConfigLstSize = emu_FileSize(StrX);
                                screen_text(10, TI, FileConfigLstName);
                                emu_FileClose(FileConfigLstHandle);
                            }
                        }
                        screen_num(4, TI, FileConfigLstSize);

                        TI -= 1;
                    }

                    c_ptr += Line2;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }

        emu_Free(c);
        emu_Free(StrX);


        if ((FileConfigRomSize > 0) && (FileConfigLstSize > 0))
        {
            int FileHandle = emu_FileOpen(FileConfigLstName, "r+b");
            if (FileHandle)
            {
                char * c2 = emu_Malloc(FileConfigLstSize + 5);
                emu_FileRead(c2, FileConfigLstSize, FileHandle);
                emu_FileClose(FileHandle);

                int Line1 = 0;
                int Line2 = 0;

                //screen_num(26, 32, FileConfigLstSize_);

                c_ptr = 0;
                
                for (int II = 0; II < 512; II++)
                {
                    MemoryRomTable[II] = 0;
                    Line1 = FileReadLine1(c_ptr, c2);
                    if (Line1 >= 0)
                    {
                        c_ptr += Line1;

                        Line2 = FileReadLine2(c_ptr, c2);
                        if (Line2 >= 0)
                        {
                            for (int I = 0; I < Line2; I++)
                            {
                                StrX[I] = c2[c_ptr + I];
                            }
                            StrX[Line2] = 0;

                            MemoryRomTable[II] = FileLoadConfigValueHex(c2, c_ptr, Line2);

                            c_ptr += Line2;
                        }
                    }
                }
            }

        }
    }




  //emu_setKeymap(1); 
  //
  //if ( (endsWith(filename, ".80")) || (endsWith(filename, ".o")) || (endsWith(filename, ".O")))  {
  //  zx80 = 1;
  //  ramsize = 48;
  //  //emu_setKeymap(0);    
  //}
  //else if (endsWith(filename, ".56") ) {
  //  ramsize = 56;
  //}


}

#endif

