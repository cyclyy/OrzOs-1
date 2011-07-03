#ifndef PCI_H
#define PCI_H 

#include "sysdef.h"

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA 0xcfc

#define PCI_BAR0_OFFSET 0x10
#define PCI_BAR1_OFFSET 0x14
#define PCI_BAR2_OFFSET 0x18
#define PCI_BAR3_OFFSET 0x1c
#define PCI_BAR4_OFFSET 0x20
#define PCI_BAR5_OFFSET 0x24

struct PCIConfig {
    u16int vendor, device;
    u8int revision, progIF, subclass, classCode;
    u8int headerType;
    u8int irq, interruptPin;
    union { 
        struct {
            u32int CIS;
            u16int subvendor;
            u16int subsystem;
            u16int romBar;
        } type0;
    };
} __attribute__((packed));

struct PCIDevice;

struct PCIDriver {
    char name[MAX_NAME_LEN];
    s64int (*probe)(struct PCIDevice *dev);
    s64int (*attach)(struct PCIDevice *dev);
    s64int (*deattach)(struct PCIDevice *dev);
};

struct PCIDevice {
    u32int bus, slot, func;
    struct PCIConfig *config;
    struct PCIDriver *driver;
    struct PCIDevice *next;
};

u32int pciReadConfigDWord(u64int bus, u64int slot, u64int func, u64int off);
void pciWriteConfigDWord(u64int bus, u64int slot, u64int func, u64int off, u32int data);
void pciWriteConfigWord(u64int bus, u64int slot, u64int func, u64int off, u16int data);
void pciWriteConfigByte(u64int bus, u64int slot, u64int func, u64int off, u8int data);

u32int pciReadDeviceConfigDWord(struct PCIDevice *dev, u64int off);
void pciWriteDeviceConfigDWord(struct PCIDevice *dev, u64int off, u32int data);
void pciWriteDeviceConfigWord(struct PCIDevice *dev, u64int off, u16int data);
void pciWriteDeviceConfigByte(struct PCIDevice *dev, u64int off, u8int data);

// s32int pci_present();

void registerPCIDriver(struct PCIDriver *drv);

void unregisterPCIDriver(struct PCIDriver *drv);

void pci_Init();

void pci_Cleanup();

#endif /* PCI_H */
