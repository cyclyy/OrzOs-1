#include "pci.h"
#include "screen.h"
#include "kheap.h"
#include "module.h"

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA 0xcfc

static pci_dev_t *pci_devs = 0;

static void dump_pci_dev(pci_dev_t *dev)
{
    printk("bus %p slot %p func %p\n", dev->bus, dev->slot, dev->func);
    printk("device %p, vendor %p, class %p, subclass %p prog_if %p\n", 
            dev->config->device,dev->config->vendor,dev->config->class_code,dev->config->subclass,dev->config->prog_if);
}

u32int read_pci_config_dword(u32int bus, u32int slot, u32int func, u32int off)
{
    u32int addr = 0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | (off&0xfc);
    outl(PCI_CONFIG_ADDRESS, addr);
    return inl(PCI_CONFIG_DATA);
}

void write_pci_config_dword(u32int bus, u32int slot, u32int func, u32int off, u32int data)
{
    u32int addr = 0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | (off&0xfc);
    outl(PCI_CONFIG_ADDRESS, addr);
    outl(PCI_CONFIG_DATA, data);
}

void write_pci_config_word(u32int bus, u32int slot, u32int func, u32int off, u16int data)
{
    u32int addr = 0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | (off&0xfe);
    outl(PCI_CONFIG_ADDRESS, addr);
    outw(PCI_CONFIG_DATA, data);
}

void write_pci_config_byte(u32int bus, u32int slot, u32int func, u32int off, u8int data)
{
    u32int addr = 0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | (off&0xff);
    outl(PCI_CONFIG_ADDRESS, addr);
    outb(PCI_CONFIG_DATA, data);
}

u32int read_pci_dev_config_dword(pci_dev_t *dev, u32int off)
{
    return read_pci_config_dword(dev->bus,dev->slot,dev->func,off);
}

void write_pci_dev_config_dword(pci_dev_t *dev, u32int off, u32int data)
{
    return write_pci_config_dword(dev->bus,dev->slot,dev->func,off,data);
}

void write_pci_dev_config_word(pci_dev_t *dev, u32int off, u16int data)
{
    return write_pci_config_word(dev->bus,dev->slot,dev->func,off,data);
}

void write_pci_dev_config_byte(pci_dev_t *dev, u32int off, u8int data)
{
    return write_pci_config_byte(dev->bus,dev->slot,dev->func,off,data);
}

s32int pci_present()
{
    return 1;
}

static void init_pci()
{
    u32int bus, slot, func;
    u32int dw, vendor;
    pci_config_t *config = 0;

    for (bus=0; bus<255; bus++) 
        for (slot=0; slot<31; slot++)
            for (func=0; func<7; func++) {
                vendor = read_pci_config_dword(bus,slot,func,0) & 0xffff;
                if (vendor != 0xffff) {
                    config = (pci_config_t*)kmalloc(sizeof(pci_config_t));
                    memset(config,0,sizeof(pci_config_t));
                    dw = read_pci_config_dword(bus,slot,func,0x0);
                    config->vendor = dw & 0xffff;
                    config->device = dw >> 16;
                    dw = read_pci_config_dword(bus,slot,func,0x8);
                    config->revision = dw & 0xff;
                    config->prog_if = (dw & 0xffff) >> 8;
                    config->subclass = (dw & 0xffffff) >> 16;
                    config->class_code = dw >> 24;
                    dw = read_pci_config_dword(bus,slot,func,0xc);
                    config->header_type = (dw & 0xffffff) >> 16;
                    dw = read_pci_config_dword(bus,slot,func,0x3c);
                    config->irq = dw & 0xff;
                    config->interrupt_pin = (dw & 0xffff) >> 8;

                    pci_dev_t *pci_dev = (pci_dev_t*)kmalloc(sizeof(pci_dev_t));
                    memset(pci_dev, 0, sizeof(pci_dev_t));
                    pci_dev->bus = bus;
                    pci_dev->slot = slot;
                    pci_dev->func = func;
                    pci_dev->config = config;
                    pci_dev->next = 0;

                    if (pci_devs) {
                        pci_dev_t *p = pci_devs; 
                        while (p->next)
                            p = p->next;
                        p->next = pci_dev;
                    } else {
                        pci_devs = pci_dev;
                    }

                    if ((config->header_type & 0x80) == 0)
                        break;

                }
            }

    pci_dev_t *p = pci_devs;
    printk("=====Dump discovered pci device======\n");
    while(p) {
        dump_pci_dev(p);
        p = p->next;
    }
    printk("=====End Dump======\n");
}

void register_pci_driver(pci_driver_t *drv)
{
    pci_dev_t *p = pci_devs;
    while (p) {
        if ((!p->driver) && (drv->probe(p) == 0)) {
            p->driver = drv;
            drv->attach(p);
            break;
        }
        p = p->next;
    }
}

void unregister_pci_driver(pci_driver_t *drv)
{
    pci_dev_t *p = pci_devs;
    while (p) {
        if (p->driver == drv) {
            drv->deattach(p);
            p->driver = 0;
            break;
        }
        p = p->next;
    }
}

void module_pci_init()
{
    init_pci();
}

void module_pci_cleanup()
{
}

MODULE_INIT(module_pci_init);
MODULE_CLEANUP(module_pci_cleanup);

MEXPORT(read_pci_config_dword);
MEXPORT(write_pci_config_dword);
MEXPORT(write_pci_config_word);
MEXPORT(write_pci_config_byte);
MEXPORT(read_pci_dev_config_dword);
MEXPORT(write_pci_dev_config_dword);
MEXPORT(write_pci_dev_config_word);
MEXPORT(write_pci_dev_config_byte);
MEXPORT(pci_present);
MEXPORT(register_pci_driver);
MEXPORT(unregister_pci_driver);
