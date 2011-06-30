#include "i8042.h"
#include "device.h"
#include "util.h"
#include "interrupt.h"
#include "kmm.h"
#include "task.h"
#include "key.h"
#include "waitqueue.h"

s64int i8042_Open(struct VNode *node);
s64int i8042_Close(struct VNode *node);
s64int i8042_Read(struct VNode *node, u64int size, char *buffer);
s64int i8042_Probe();
void i8042_ISR(struct RegisterState *regs);

static u64int kbdus[128] =
{
    0,  Key_Escape, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, /* 9 */
    Key_9, Key_0, Key_Minus, Key_Equal, Key_Backspace, /* Backspace */
    Key_Tab,           /* Tab */
    Key_Q, Key_W, Key_E, Key_R,   /* 19 */
    Key_T, Key_Y, Key_U, Key_I, Key_O, Key_P, Key_BracketLeft, Key_BracketRight, Key_Return,   /* Enter key */
    Key_LeftControl,          /* 29   - Control */
    Key_A, Key_S, Key_D, Key_F, Key_G, Key_H, Key_J, Key_K, Key_L, Key_Semicolon, /* 39 */
    Key_Apostrophe, Key_QuoteLeft,   Key_LeftShift,      /* Left shift */
    Key_Backslash, Key_Z, Key_X, Key_C, Key_V, Key_B, Key_N,         /* 49 */
    Key_M, Key_Comma, Key_Period, Key_Slash,   Key_RightShift,              /* Right shift */
    Key_Asterisk,
    Key_LeftAlt,  /* Alt */
    Key_Space,  /* Space bar */
    Key_CapsLock,    /* Caps lock */
    Key_F1,    /* 59 - F1 key ... > */
    Key_F2,   Key_F3,   Key_F4,   Key_F5,   Key_F6,   Key_F7,   Key_F8,   Key_F9,
    Key_F10,    /* < ... F10 */
    0,    /* 69 - Num lock*/
    0,    /* Scroll Lock */
    Key_Home,    /* Home key */
    Key_Up,    /* Up Arrow */
    Key_PageUp,    /* Page Up */
    '-',
    Key_Left,  /* Left Arrow */
    0,
    Key_Right,  /* Right Arrow */
    '+',
    Key_End,    /* 79 - End key*/
    Key_Down,    /* Down Arrow */
    Key_PageDown,    /* Page Down */
    Key_Insert,    /* Insert Key */
    Key_Delete,    /* Delete Key */
    0,   0,   0,
    Key_F11,    /* F11 Key */
    Key_F12,    /* F12 Key */
    0,    /* All other keys are undefined */
};      

static struct Device *dev;

static struct WaitQueue *wq = 0;

static u8int inEscape = 0;
static u64int key;

static struct DeviceOperation i8042_Ops = {
    .open  = &i8042_Open,
    .close = &i8042_Close,
    .read  = &i8042_Read,
};

void i8042_Init()
{
    struct Device *dev;
    dev = (struct Device*)kMalloc(sizeof(struct Device));
    memset(dev, 0, sizeof(struct Device));
    dev->id = 0x10000;
    dev->op  = &i8042_Ops;

    if (i8042_Probe() == 0) {
        /*
        cleanupBuffers();
        outb(0x64,0xAE);
        */
        addDevice(dev);
        vfsCreateObject("Device:/Keyboard",dev->id);
        wq = (struct WaitQueue*)kMalloc(sizeof(struct WaitQueue));
        memset(wq, 0, sizeof(struct WaitQueue));
        registerInterruptHandler(IRQ1, &i8042_ISR);
    } else {
        printk("failed to init i8042.\n");
        kFree(dev);
        dev = 0;
    }
}

void i8042_Cleanup()
{
    /*printk("I'm module_i8042_cleanup\n");*/
    if (dev) {
        registerInterruptHandler(IRQ1, 0);
        removeDevice(dev);
        wakeUpAll(wq);
        kFree(wq);
    }
}

void cleanupBuffers()
{
    while ((inb(0x64) & 3))
        inb(0x60);
}

void prepareRead()
{
    while ((inb(0x64) & 1)==0)
        ;
}

void prepareWrite()
{
    while ((inb(0x64) & 2))
        inb(0x60);
}

s64int i8042_Probe()
{
    cleanupBuffers();
    outb(0x64,0xAA);
    prepareRead();
    if (inb(0x60) != 0x55)
        return -1;

    cleanupBuffers();
    outb(0x64,0xAB);
    prepareRead();
    if (inb(0x60) != 0)
        return -1;

    return 0;
}

s64int i8042_Read(struct VNode *node, u64int size, char *buffer)
{
    /*printk("%s\n", __FUNCTION__);*/
    if ((size<4) || !buffer)
        return 0;

    sleepOn(wq);

    *(u64int*)buffer = key;

    return 4;
}

s64int i8042_Open(struct VNode *node)
{
    node->priv = dev;
    return 0;
}

s64int i8042_Close(struct VNode *node)
{
    return 0;
}

void i8042_ISR(struct RegisterState *regs)
{
    u8int ch = inb(0x60);
    u8int c = ch;

    if (ch == 0xe0) {
        inEscape = 1;
        return;
    }

    if (c & 0x80) {
        c -= 0x80;
    }
    key = kbdus[c];

    if (inEscape) {
        inEscape = 0;
        switch (key) {
            case Key_LeftControl:
                key = Key_RightControl;
                break;
            case Key_LeftAlt:
                key = Key_RightAlt;
                break;
            case 0x52:
                key = Key_Insert;
                break;
            case 0x47:
                key = Key_Home;
                break;
            case 0x49:
                key = Key_PageUp;
                break;
            case 0x53:
                key = Key_Delete;
                break;
            case 0x4f:
                key = Key_End;
                break;
            case 0x51:
                key = Key_PageDown;
                break;
            case 0x48:
                key = Key_Up;
                break;
            case 0x4b:
                key = Key_Left;
                break;
            case 0x50:
                key = Key_Down;
                break;
            case 0x4d:
                key = Key_Right;
                break;
            default:
                ;
        }
    }


    if (ch & 0x80) {
        key |= Key_Release_Mask;
    }

    wakeUpAll(wq);
}

