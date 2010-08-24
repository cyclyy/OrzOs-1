#ifndef PCI_H
#define PCI_H 

#include "common.h"

#define PCI_BAR0_OFFSET 0x10
#define PCI_BAR1_OFFSET 0x14
#define PCI_BAR2_OFFSET 0x18
#define PCI_BAR3_OFFSET 0x1c
#define PCI_BAR4_OFFSET 0x20
#define PCI_BAR5_OFFSET 0x24

typedef struct {
    u16int vendor, device;
    u8int revision, prog_if, subclass, class_code;
    u8int header_type;
    u8int irq, interrupt_pin;
    union { 
        struct {
            u32int CIS;
            u16int subvendor;
            u16int subsystem;
            u16int rom_bar;
        } type0;
    };

} pci_config_t;

typedef struct pci_dev_struct pci_dev_t;

typedef struct {
    char name[MAX_NAME_LEN];
    s32int (*probe)(pci_dev_t *pci_dev);
    s32int (*attach)(pci_dev_t *pci_dev);
    s32int (*deattach)(pci_dev_t *pci_dev);
} pci_driver_t;

struct pci_dev_struct {
    u32int bus, slot, func;
    pci_config_t *config;
    pci_driver_t *driver;
    pci_dev_t *next;
};

u32int read_pci_config_dword(u32int bus, u32int slot, u32int func, u32int off);

void write_pci_config_dword(u32int bus, u32int slot, u32int func, u32int off, u32int data);

s32int pci_present();

void register_pci_driver(pci_driver_t *drv);

void unregister_pci_driver(pci_driver_t *drv);

#endif /* PCI_H */
