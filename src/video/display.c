#include "display.h"
#include "device.h"
#include "util.h"
#include "kmm.h"
#include "vmm.h"
#include "task.h"
#include "waitqueue.h"
#include "rmi.h"
#include "vbe.h"

s64int display_Open(struct Device *dev);
s64int display_Close(struct Device *dev);
s64int display_Read(struct Device *dev, u64int offset, u64int size, char *buffer);
s64int display_Write(struct Device *dev, u64int offset, u64int size, char *buffer);
s64int display_IoControl(struct Device *dev, s64int request, u64int size, void *data);
s64int display_Probe();

static struct Device *dev = 0;

static struct DeviceOperation display_Ops = {
    .open  = &display_Open,
    .close = &display_Close,
    .read  = &display_Read,
    .write  = &display_Write,
    .ioctl = &display_IoControl,
};

static struct DisplayModeInfo currentDMI;
static struct VBEInfoBlock *vbeInfo;
static struct VBEModeInfoBlock *vbeModeInfo;

static s64int getCurrentDMI();
static s64int setDMI(struct DisplayModeInfo *mi);

s64int setDMI(struct DisplayModeInfo *mi)
{
    if (mi->mode == DISPLAY_MODE_TEXT) {
        if (mi->color) {
            setVBEMode(0x3);
        } else {
            setVBEMode(0x1);
        }
        getCurrentDMI();
        return 0;
    } else if (mi->mode == DISPLAY_MODE_VESA) {
        if ((mi->width = 640) && (mi->height = 480)) {
            switch (mi->cellBits) {
            case 8:
                break;
            case 16:
                break;
            default:
                return -1;
            }
            getCurrentDMI();
            return 0;
        } else if ((mi->width = 800) && (mi->height = 600)) {
            switch (mi->cellBits) {
            case 8:
                setVBEMode(0x103);
                break;
            case 16:
                setVBEMode(0x114);
                break;
            case 24:
                setVBEMode(0x115);
                break;
            default:
                return -1;
            }
            getCurrentDMI();
            return 0;

        } else if ((mi->width = 1024) && (mi->height = 768)) {
        } else if ((mi->width = 1280) && (mi->height = 1024)) {
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}


s64int getCurrentDMI()
{
    u16int mode;

    getVBECurrentMode(&mode);
    getVBEModeInfo(mode,vbeModeInfo);
    memset(&currentDMI, 0, sizeof(struct DisplayModeInfo));
    switch (mode) {
    case 0x3:
        currentDMI.mode = DISPLAY_MODE_TEXT;
        currentDMI.color = 1;
        currentDMI.width = 80;
        currentDMI.height = 25;
        currentDMI.cellBits = 16;
        currentDMI.addr = 0xB8000;
        break;
    case 0x115:
        currentDMI.mode = DISPLAY_MODE_VESA;
        currentDMI.color = 1;
        currentDMI.width = vbeModeInfo->XResolution;
        currentDMI.height = vbeModeInfo->YResolution;
        currentDMI.cellBits = vbeModeInfo->BitsPerPixel;
        currentDMI.addr = vbeModeInfo->PhysBasePtr;
        break;
    default:
        printk("Not supported display mode:%x\n", mode);
        return -1;
    }
    return 0;
}

void display_Init()
{
    struct Device *dev;

    memset(&currentDMI, 0, sizeof(struct DisplayModeInfo));
    vbeInfo = realModeAlloc(sizeof(struct VBEInfoBlock));
    vbeModeInfo = realModeAlloc(sizeof(struct VBEModeInfoBlock));
    getVBEInfo(vbeInfo);
    getCurrentDMI();
    dev = (struct Device*)kMalloc(sizeof(struct Device));
    memset(dev, 0, sizeof(struct Device));
    dev->id = 0x20000;
    dev->op  = &display_Ops;
    addDevice(dev);
    vfsCreateObject("Device:/Display",dev->id);
}

void display_Cleanup()
{
    /*printk("I'm module_display_cleanup\n");*/
    removeDevice(dev);
    kFree(dev);
    realModeFree(vbeInfo);
    realModeFree(vbeModeInfo);
    //vfsRemove("Device:/Display");
}

s64int display_Read(struct Device *dev, u64int offset, u64int size, char *buffer)
{
    u64int n;

    n = currentDMI.width * currentDMI.height * (currentDMI.cellBits/8);
    if (offset >= n) {
        return 0;
    }
    size = MIN(size, n - offset);
    return copyToUser(buffer, (void*)(currentDMI.addr + offset), size);
}

s64int display_Write(struct Device *dev, u64int offset, u64int size, char *buffer)
{
    u64int n;

    n = currentDMI.width * currentDMI.height * (currentDMI.cellBits/8);
    if (offset >= n) {
        return 0;
    }
    size = MIN(size, n - offset);
    return copyFromUser((void*)(currentDMI.addr + offset), buffer, size);
}

s64int display_Open(struct Device *dev)
{
    return 0;
}

s64int display_Close(struct Device *dev)
{
    return 0;
}

s64int display_IoControl(struct Device *dev, s64int request, u64int size, void *data)
{
    u64int n;
    struct DisplayModeInfo mi;

    switch (request) {
    case DISPLAY_IOCTL_GET_CURRENT_MODE_INFO:
        if (size < sizeof(struct DisplayModeInfo)) {
            return -1;
        }
        n = copyToUser(data, &currentDMI, sizeof(struct DisplayModeInfo));
        if (n < sizeof(struct DisplayModeInfo)) {
            return -1;
        }
        return 0;
        break;
    case DISPLAY_IOCTL_SET_MODE:
        if (size < sizeof(struct DisplayModeInfo)) {
            return -1;
        }
        n = copyFromUser(&mi, data, sizeof(struct DisplayModeInfo));
        if (n < sizeof(struct DisplayModeInfo)) {
            return -1;
        }
        return setDMI(&mi);
        break;
    default:
        return -1;
    }
}

