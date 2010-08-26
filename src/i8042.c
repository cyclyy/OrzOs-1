#include "i8042.h"
#include "screen.h"
#include "dev.h"
#include "isr.h"
#include "kheap.h"
#include "task.h"
#include "key.h"
#include "module.h"

u32int kbdus[128] =
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

dev_t *dev_i8042;

wait_queue_t *wq = 0;

u8int in_escape = 0;
u32int key;

void module_i8042_init()
{
    /*printk("I'm module_i8042_init\n");*/
    dev_i8042 = (dev_t*)kmalloc(sizeof(dev_t));
    memset(dev_i8042, 0, sizeof(dev_t));
    dev_i8042->dev_id = 0x10000;
    dev_i8042->read = &i8042_read;
    dev_i8042->write = &i8042_write;
    dev_i8042->open = &i8042_open;
    dev_i8042->close = &i8042_close;

    if (i8042_probe() == 0) {
        wq = (wait_queue_t*)kmalloc(sizeof(wait_queue_t));
        memset(wq, 0, sizeof(wait_queue_t));
        add_dev(dev_i8042);
        outb(0x64,0x60);
        outb(0x64,0xAE);
        /*printk("%p\n",inb(0x60));*/
        register_interrupt_handler(IRQ1, &i8042_irq);
    } else {
        printk("failed to init i8042.\n");
        kfree(dev_i8042);
        dev_i8042 = 0;
    }
}

void module_i8042_cleanup()
{
    /*printk("I'm module_i8042_cleanup\n");*/
    if (dev_i8042) {
        register_interrupt_handler(IRQ1, 0);
        del_dev(dev_i8042);
    }
}

s32int i8042_probe()
{
    while (inb(0x64) & 1)
        inb(0x60);

    // doesn't perform test since it'll failed in VirtualBox
    return 0;

    outb(0x64,0x60);
    outb(0x64,0xAA);
    if (inb(0x60) != 0x55)
        return -1;

    outb(0x64,0x60);
    outb(0x64,0xAB);
    if (inb(0x60) != 0)
        return -1;

    return 0;
}

s32int i8042_read(file_t *f, u32int offset, u32int sz, u8int *buffer)
{
    /*printk("%s\n", __FUNCTION__);*/
    if ((sz<4) || !buffer)
        return 0;

    sleep_on(wq);

    *(u32int*)buffer = key;

    return 4;
}

s32int i8042_write(file_t *f, u32int offset, u32int sz, u8int *buffer)
{
    return 0;
}

s32int i8042_open(file_t *f)
{
    return 0;
}

s32int i8042_close(file_t *f)
{
    return 0;
}

void i8042_irq(registers_t *regs)
{
    u8int ch = inb(0x60);
    u8int c = ch;

    if (ch == 0xe0) {
        in_escape = 1;
        return;
    }

    if (c & 0x80) {
        c -= 0x80;
    }
    key = kbdus[c];

    if (in_escape) {
        in_escape = 0;
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

    /*printk("i8042_irq\n");*/
    wake_up_all(wq);
}

MODULE_INIT(module_i8042_init);
MODULE_CLEANUP(module_i8042_cleanup);
MEXPORT(i8042_irq);

