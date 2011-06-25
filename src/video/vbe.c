#include "vbe.h"
#include "x86emu.h"
#include "rmi.h"
#include "util.h"

int getVBEInfo( struct VBEInfoBlock * infoBlock )
{
    memcpy(infoBlock->VESASignature, "VBE2", 4);
    memset(&M.x86,0,sizeof(M.x86));
    M.x86.R_AX = funcGetControllerInfo;
    M.x86.R_ES = SEG(infoBlock);
    M.x86.R_DI = OFF(infoBlock);
    realModeInterrupt(0x10);
    return M.x86.R_AH;
    /*
    bb.intno  = 0x10;
    bb.eax.rr = funcGetControllerInfo;
    bb.es     = SEG( infoBlock );
    bb.edi.rr = OFF( infoBlock );
    bios( &bb );
    return(bb.eax.r.h);
    */
}

int getVBEModeInfo( int mode, struct VBEModeInfoBlock * minfo_p )
{
    memset(&M.x86,0,sizeof(M.x86));
    M.x86.R_AX = funcGetModeInfo;
    M.x86.R_CX = mode;
    M.x86.R_ES = SEG(minfo_p);
    M.x86.R_DI = OFF(minfo_p);
    realModeInterrupt(0x10);
    return M.x86.R_AH;
    /*
    bb.intno  = 0x10;
    bb.eax.rr = funcGetModeInfo;
    bb.ecx.rr = mode;
    bb.es     = SEG(minfo_p);
    bb.edi.rr = OFF(minfo_p);
    bios(&bb);
    return(bb.eax.r.h);
    */
}

int getVBEDACFormat(unsigned char *format)
{
    /*
    bb.intno = 0x10;
    bb.eax.rr = funcGetSetPaletteFormat;
    bb.ebx.r.l = subfuncGet;
    bios(&bb);
    *format = bb.ebx.r.h;
    return(bb.eax.r.h);
    */
}

int setVBEDACFormat(unsigned char format)
{
    /*
    bb.intno = 0x10;
    bb.eax.rr = funcGetSetPaletteFormat;
    bb.ebx.r.l = subfuncSet;
    bb.ebx.r.h = format;
    bios(&bb);
    return(bb.eax.r.h);
    */
}

int setVBEMode(u16int mode)
{
    memset(&M.x86,0,sizeof(M.x86));
    M.x86.R_EAX = funcSetMode;
    M.x86.R_EBX = mode | (1<<14);
    realModeInterrupt(0x10);
    return M.x86.R_AH;
    /*
    bb.intno = 0x10;
    bb.eax.rr = funcSetMode;
    bb.ebx.rr = mode;
    bios(&bb);
    return(bb.eax.r.h);
    */
}

int setVBEPalette(void *palette)
{
    /*
    bb.intno = 0x10;
    bb.eax.rr = funcGetSetPaletteData;
    bb.ebx.r.l = subfuncSet;
    bb.ecx.rr = 256;
    bb.edx.rr = 0;
    bb.es = SEG(palette);
    bb.edi.rr = OFF(palette);
    bios(&bb);
    return(bb.eax.r.h);
    */
}

int getVBEPalette(void *palette)
{
    /*
    bb.intno = 0x10;
    bb.eax.rr = funcGetSetPaletteData;
    bb.ebx.r.l = subfuncGet;
    bb.ecx.rr = 256;
    bb.edx.rr = 0;
    bb.es = SEG(palette);
    bb.edi.rr = OFF(palette);
    bios(&bb);
    return(bb.eax.r.h);
    */
}

int getVBECurrentMode(u16int *mode)
{
    memset(&M.x86,0,sizeof(M.x86));
    M.x86.R_EAX = funcGetCurrentMode;
    realModeInterrupt(0x10);
    *mode = M.x86.R_EBX;
    return M.x86.R_AH;
    /*
    bb.intno = 0x10;
    bb.eax.rr = funcGetCurrentMode;
    bios(&bb);
    *mode = bb.ebx.rr;
    return(bb.eax.r.h);
    */
}

// vim: sw=4 sts=4 et tw=100
