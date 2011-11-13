#ifndef VBE_H
#define VBE_H

#include "sysdef.h"
#define MIN_VESA_VERSION    0x200

#define SEG(address) \
        ((unsigned short)(((unsigned long)address & 0xffff0000) >> 4))

#define OFF(address) \
        ((unsigned short)((unsigned long)address & 0x0000ffff))

#define VBEMakeUInt32(x)                   \
        (((unsigned long)x##_high << 24) | \
         ((unsigned long)x##_2    << 16) | \
         ((unsigned long)x##_1    <<  8) | \
          (unsigned long)x##_low)

#define VBEDecodeFP(t, fp) \
        ((t)(((fp ## _low) | ((fp ## _1 ) << 8)) + \
             (((fp ## _2) << 4) | ((fp ## _high ) << 12))))

/*
 * Functions
 */
enum {
    funcGetControllerInfo    = 0x4F00,
    funcGetModeInfo          = 0x4F01,
    funcSetMode              = 0x4F02,
    funcGetCurrentMode       = 0x4F03,
    funcSaveRestoreState     = 0x4F04,
    funcWindowControl        = 0x4F05,
    funcGetSetScanLineLength = 0x4F06,
    funcGetSetDisplayStart   = 0x4F07,
    funcGetSetPaletteFormat  = 0x4F08,
    funcGetSetPaletteData    = 0x4F09,
    funcGetProtModeInterdace = 0x4F0A
};

enum {
    subfuncSet          = 0x00,
    subfuncGet          = 0x01,
    subfuncSetSecondary = 0x02,
    subfuncGetSecondary = 0x03
};

/*
 * errors.
 */
enum {
    errSuccess          = 0,
    errFuncFailed       = 1,
    errFuncNotSupported = 2,
    errFuncInvalid      = 3
};

/*
 * Per-controller info, returned in function 4f00.
 */
struct VBEInfoBlock {
    unsigned char   VESASignature[4];
    unsigned short  VESAVersion;
    /*
     * Avoid packing problem...
     */
    unsigned char   OEMStringPtr_low;
    unsigned char   OEMStringPtr_1;
    unsigned char   OEMStringPtr_2;
    unsigned char   OEMStringPtr_high;
    unsigned char   Capabilities_low;
    unsigned char   Capabilities_1;
    unsigned char   Capabilities_2;
    unsigned char   Capabilities_high;
    unsigned char   VideoModePtr_low;
    unsigned char   VideoModePtr_1;
    unsigned char   VideoModePtr_2;
    unsigned char   VideoModePtr_high;
    unsigned short  TotalMemory;
    unsigned char   Reserved1[236];
    unsigned char   Reserved2[256];
} __attribute__((packed));

/*
 * Capabilites
 */
enum {
    capDACWidthIsSwitchableBit          = (1 << 0), /* 1 = yes; 0 = no  */
    capControllerIsNotVGACompatableBit  = (1 << 1), /* 1 = no;  0 = yes */
    capOldRAMDAC                        = (1 << 2)  /* 1 = yes; 0 = no  */
};

/*
 * Per-mode info, returned in function 4f02.
 */
struct VBEModeInfoBlock {
    unsigned short  ModeAttributes;
    unsigned char   WinAAttributes;
    unsigned char   WinBAttributes;
    unsigned short  WinGranularity;
    unsigned short  WinSize;
    unsigned short  WinASegment;
    unsigned short  WinABegment;
    u32int          WinFuncPtr;
    u16int  BytesPerScanline;
    unsigned short  XResolution;
    unsigned short  YResolution;
    unsigned char   XCharSize;
    unsigned char   YCharSize;
    unsigned char   NumberOfPlanes;
    unsigned char   BitsPerPixel;
    unsigned char   NumberOfBanks;
    unsigned char   MemoryModel;
    unsigned char   BankSize;
    unsigned char   NumberOfImagePages;
    unsigned char   Reserved;
    unsigned char   RedMaskSize;
    unsigned char   RedFieldPosition;
    unsigned char   GreenMaskSize;
    unsigned char   GreenFieldPosition;
    unsigned char   BlueMaskSize;
    unsigned char   BlueFieldPosition;
    unsigned char   RsvdMaskSize;
    unsigned char   RsvdFieldPosition;
    unsigned char   DirectColorModeInfo;
    u32int          PhysBasePtr;
    u32int          OffScreenMemOffset;
    unsigned short  OffScreenMemSize;
    unsigned char   Reserved1[206];
} __attribute__((packed));

/* 
 * ModeAttributes bits
 */
enum {
    maModeIsSupportedBit        = (1 << 0), /* mode is supported */
    maExtendedInfoAvailableBit  = (1 << 1), /* extended info available */
    maOutputFuncSupportedBit    = (1 << 2), /* output functions supported */
    maColorModeBit              = (1 << 3), /* 1 = color; 0 = mono */
    maGraphicsModeBit           = (1 << 4), /* 1 = graphics; 0 = text */
    maModeIsNotVGACompatableBit = (1 << 5), /* 1 = not compat; 0 = compat */
    maVGAMemoryModeNotAvailBit  = (1 << 6), /* 1 = not avail; 0 = avail */
    maLinearFrameBufferAvailBit = (1 << 7)  /* 1 = avail; 0 = not avail */
};

/*
 * Modes
 */
enum {
    mode640x400x256   = 0x100,
    mode640x480x256   = 0x101,
    mode800x600x16    = 0x102,
    mode800x600x256   = 0x103,
    mode1024x768x16   = 0x104,
    mode1024x768x256  = 0x105,
    mode1280x1024x16  = 0x106,
    mode1280x1024x256 = 0x107,
    mode80Cx60R       = 0x108,
    mode132Cx25R      = 0x109,
    mode132Cx43R      = 0x10A,
    mode132Cx50R      = 0x10B,
    mode132Cx60R      = 0x10C,
    mode320x200x555   = 0x10D,
    mode320x200x565   = 0x10E,
    mode320x200x888   = 0x10F,
    mode640x480x555   = 0x110,
    mode640x480x565   = 0x111,
    mode640x480x888   = 0x112,
    mode800x600x555   = 0x113,
    mode800x600x565   = 0x114,
    mode800x600x888   = 0x115,
    mode1024x768x555  = 0x116,
    mode1024x768x565  = 0x117,
    mode1024x768x888  = 0x118,
    mode1280x1024x555 = 0x119,
    mode1280x1024x565 = 0x11A,
    mode1280x1024x888 = 0x11B,
    modeSpecial       = 0x81FF,
    modeEndOfList     = 0xFFFF
};

/*
 * Get/Set VBE Mode parameters
 */
enum {
    kLinearFrameBufferBit  =  (1 << 14),
    kPreserveMemoryBit     =  (1 << 15)
};

/*
 * Palette
 */
typedef unsigned long VBEPalette[256];

int getVBEInfo(struct VBEInfoBlock *vinfo_p);
int getVBEModeInfo(int mode, struct VBEModeInfoBlock *minfo_p);
int getVBEDACFormat(unsigned char *format);
int setVBEDACFormat(unsigned char format);
int setVBEPalette(void *palette);
int getVBEPalette(void *palette);
int setVBEMode(u16int mode);
int getVBECurrentMode(u16int *mode);

#endif

