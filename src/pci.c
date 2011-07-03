#include "pci.h"
#include "util.h"
#include "kmm.h"

static struct PCIDevice *pci_devs = 0;

static void pciDumpDevice(struct PCIDevice *dev)
{
    printk("%d:%d:%d, device %p, vendor %p, class %p, subclass %p\n", 
            dev->bus, dev->slot, dev->func,
            dev->config->device,dev->config->vendor,dev->config->classCode,dev->config->subclass);
}

u32int pciReadConfigDWord(u64int bus, u64int slot, u64int func, u64int off)
{
    u32int addr = 0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | (off&0xfc);
    outl(PCI_CONFIG_ADDRESS, addr);
    return inl(PCI_CONFIG_DATA);
}

void pciWriteConfigDWord(u64int bus, u64int slot, u64int func, u64int off, u32int data)
{
    u32int addr = 0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | (off&0xfc);
    outl(PCI_CONFIG_ADDRESS, addr);
    outl(PCI_CONFIG_DATA, data);
}

void pciWriteConfigWord(u64int bus, u64int slot, u64int func, u64int off, u16int data)
{
    u32int addr = 0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | (off&0xfe);
    outl(PCI_CONFIG_ADDRESS, addr);
    outw(PCI_CONFIG_DATA, data);
}

void pciWriteConfigByte(u64int bus, u64int slot, u64int func, u64int off, u8int data)
{
    u32int addr = 0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | (off&0xff);
    outl(PCI_CONFIG_ADDRESS, addr);
    outb(PCI_CONFIG_DATA, data);
}

u32int pciReadDeviceConfigDWord(struct PCIDevice *dev, u64int off)
{
    return pciReadConfigDWord(dev->bus,dev->slot,dev->func,off);
}

void pciWriteDeviceConfigDWord(struct PCIDevice *dev, u64int off, u32int data)
{
    return pciWriteConfigDWord(dev->bus,dev->slot,dev->func,off,data);
}

void pciWriteDeviceConfigWord(struct PCIDevice *dev, u64int off, u16int data)
{
    return pciWriteConfigWord(dev->bus,dev->slot,dev->func,off,data);
}

void pciWriteDeviceConfigByte(struct PCIDevice *dev, u64int off, u8int data)
{
    return pciWriteConfigByte(dev->bus,dev->slot,dev->func,off,data);
}

/*
s32int pci_present()
{
    return 1;
}
*/


static void initPCI()
{
    u64int bus, slot, func;
    u64int dw;
    u16int vendor;
    struct PCIConfig *config = 0;

    for (bus=0; bus<255; bus++) 
        for (slot=0; slot<31; slot++)
            for (func=0; func<7; func++) {
                vendor = pciReadConfigDWord(bus,slot,func,0) & 0xffff;
                if (vendor != 0xffff) {
                    config = (struct PCIConfig*)kMalloc(sizeof(struct PCIConfig));
                    memset(config,0,sizeof(struct PCIConfig));
                    dw = pciReadConfigDWord(bus,slot,func,0x0);
                    config->vendor = dw & 0xffff;
                    config->device = dw >> 16;
                    dw = pciReadConfigDWord(bus,slot,func,0x8);
                    config->revision = dw & 0xff;
                    config->progIF = (dw & 0xffff) >> 8;
                    config->subclass = (dw & 0xffffff) >> 16;
                    config->classCode = dw >> 24;
                    dw = pciReadConfigDWord(bus,slot,func,0xc);
                    config->headerType = (dw & 0xffffff) >> 16;
                    dw = pciReadConfigDWord(bus,slot,func,0x3c);
                    config->irq = dw & 0xff;
                    config->interruptPin = (dw & 0xffff) >> 8;

                    struct PCIDevice *pci_dev = (struct PCIDevice*)kMalloc(sizeof(struct PCIDevice));
                    memset(pci_dev, 0, sizeof(struct PCIDevice));
                    pci_dev->bus = bus;
                    pci_dev->slot = slot;
                    pci_dev->func = func;
                    pci_dev->config = config;
                    pci_dev->next = 0;

                    if (pci_devs) {
                        struct PCIDevice *p = pci_devs; 
                        while (p->next)
                            p = p->next;
                        p->next = pci_dev;
                    } else {
                        pci_devs = pci_dev;
                    }

                    if ((config->headerType & 0x80) == 0)
                        break;

                }
            }

    struct PCIDevice *p = pci_devs;
    printk("=====Dump discovered pci device======\n");
    while(p) {
        pciDumpDevice(p);
        p = p->next;
    }
    printk("=====End Dump======\n");
}

void registerPCIDriver(struct PCIDriver *drv)
{
    struct PCIDevice *p = pci_devs;
    while (p) {
        if ((!p->driver) && (drv->probe(p) == 0)) {
            p->driver = drv;
            drv->attach(p);
            break;
        }
        p = p->next;
    }
}

void unregisterPCIDriver(struct PCIDriver *drv)
{
    struct PCIDevice *p = pci_devs;
    while (p) {
        if (p->driver == drv) {
            drv->deattach(p);
            p->driver = 0;
            break;
        }
        p = p->next;
    }
}

void pci_Init()
{
    initPCI();
}

void pci_Cleanup()
{
}
