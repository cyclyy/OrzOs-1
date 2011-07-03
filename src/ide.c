#include "ide.h"
#include "pci.h"
#include "util.h"
#include "isr.h"
#include "task.h"
#include "kmm.h"
#include "vmm.h"
#include "waitqueue.h"
#include "disk.h"

u8int ideBuf[2048] = {0};
//u8int ideirq_invoked = 0;
u8int atapiPacket[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct WaitQueue wq = {0};

struct ideChannelRegisters {
    u16int base;  // I/O Base.
    u16int ctrl;  // Control Base
    u16int bmide; // Bus Master IDE
    u8int  nIEN;  // No Interrupt;
} channels[2];

struct ideDevice {
    u8int exists;    // 0 (Empty) or 1 (This Drive really exists).
    u8int channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
    u8int drive;       // 0 (Master Drive) or 1 (Slave Drive).
    u8int type;        // 0: ATA, 1:ATAPI.
    u16int signature;   // Drive Signature
    u16int capabilities;        // Features.
    u32int command_sets; // Command Sets Supported.
    u16int udma;        // Active udma mode.
    u32int size;        // size in Sectors.
    u8int model[41];   // model in string.
} ideDevices[4];

void ideWrite(u8int channel, u8int reg, u8int data);

s64int ideReadSectors(u8int drive, u8int numsects, u32int lba, u8int *buf);

s64int ideWriteSectors(u8int drive, u8int numsects, u32int lba, u8int *buf);

u64int ideReadBlocks(disk_t *disk, u32int start_blk, u32int nr_blks, u8int *buf)
{
    u8int drive = (u32int)disk->priv;

    while(nr_blks >= 256) {
        ide_read_sectors(drive, 0, start_blk, buf);
        buf += 256*512;
        start_blk += 256;
        nr_blks -= 256;
    }

    if (nr_blks)
        ide_read_sectors(drive, nr_blks, start_blk, buf);
    
    return nr_blks;
}

u32int ide_write_blks(disk_t *disk, u32int start_blk, u32int nr_blks, u8int *buf)
{
    u8int drive = (u32int)disk->priv;

    while(nr_blks >= 256) {
        ide_write_sectors(drive, 0, start_blk, buf);
        buf += 256*512;
        start_blk += 256;
        nr_blks -= 256;
    }

    if (nr_blks)
        ide_write_sectors(drive, nr_blks, start_blk, buf);

    return nr_blks;
}


u8int ide_read(u8int channel, u8int reg) {
    u8int result;
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    if (reg < 0x08)
        result = inb(channels[channel].base + reg - 0x00);
    else if (reg < 0x0C)
        result = inb(channels[channel].base  + reg - 0x06);
    else if (reg < 0x0E)
        result = inb(channels[channel].ctrl  + reg - 0x0A);
    else if (reg < 0x16)
        result = inb(channels[channel].bmide + reg - 0x0E);
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
    return result;
}

void ide_write(u8int channel, u8int reg, u8int data) {
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    if (reg < 0x08)
        outb(channels[channel].base  + reg - 0x00,data);
    else if (reg < 0x0C)
        outb(channels[channel].base  + reg - 0x06,data);
    else if (reg < 0x0E)
        outb(channels[channel].ctrl  + reg - 0x0A,data);
    else if (reg < 0x16)
        outb(channels[channel].bmide + reg - 0x0E,data);
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

s32int ide_probe(pci_dev_t *dev)
{
    if ((dev->config->class_code == 1) && ((dev->config->subclass==1)))
        return 0;
    return -1;
}

void ide_irq(registers_t *regs)
{
    ide_irq_invoked = 1;
}

void ide_read_buffer(u8int channel, u8int reg, u32int buffer, u32int quads) {
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
    if (reg < 0x08)
        insl(channels[channel].base  + reg - 0x00, buffer, quads);
    else if (reg < 0x0C)
        insl(channels[channel].base  + reg - 0x06, buffer, quads);
    else if (reg < 0x0E)
        insl(channels[channel].ctrl  + reg - 0x0A, buffer, quads);
    else if (reg < 0x16)
        insl(channels[channel].bmide + reg - 0x0E, buffer, quads);
    if (reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

u8int ide_polling(u8int channel, u32int advanced_check) {
    u32int i=0;
    // (I) Delay 400 nanosecond for BSY to be set:
    // -------------------------------------------------
    for(i = 0; i < 4; i++)
        ide_read(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.

    // (II) Wait for BSY to be cleared:
    // -------------------------------------------------
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
        ; // Wait for BSY to be zero.

    if (advanced_check) {
        u8int state = ide_read(channel, ATA_REG_STATUS); // Read Status Register.

        // (III) Check For Errors:
        // -------------------------------------------------
        if (state & ATA_SR_ERR)
            return 2; // Error.

        // (IV) Check If Device fault:
        // -------------------------------------------------
        if (state & ATA_SR_DF)
            return 1; // Device Fault.

        // (V) Check DRQ:
        // -------------------------------------------------
        // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
        if ((state & ATA_SR_DRQ) == 0)
            return 3; // DRQ should be set

    }

    return 0; // No Error.

}

u8int ide_ata_access(u8int direction, u8int drive, u32int lba, u8int numsects, u8int *buf) {
    u8int lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */, dma /* 0: No DMA, 1: DMA */, cmd;
    u8int lba_io[6];
    u32int  channel  = ide_devices[drive].channel; // Read the Channel.
    u32int  slavebit = ide_devices[drive].drive; // Read the Drive [Master/Slave]
    u32int  port      = channels[channel].base; // The Bus Base, like [0x1F0] which is also data port.
    u32int  words    = 256; // Approximatly all ATA-Drives has sector-size of 512-byte.
    u16int cyl, i; 
    u8int head, sect, err;

    // We want to do polling only, disable interrupt
    ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = (ide_irq_invoked = 0x0) + 0x02);

   // (I) Select one from LBA28, LBA48 or CHS;
   if (lba >= 0x10000000) { // Sure Drive should support LBA in this case, or you are giving a wrong LBA.
      // LBA48:
      lba_mode  = 2;
      lba_io[0] = (lba & 0x000000FF)>> 0;
      lba_io[1] = (lba & 0x0000FF00)>> 8;
      lba_io[2] = (lba & 0x00FF0000)>>16;
      lba_io[3] = (lba & 0xFF000000)>>24;
      lba_io[4] = 0; // We said that we lba is integer, so 32-bit are enough to access 2TB.
      lba_io[5] = 0; // We said that we lba is integer, so 32-bit are enough to access 2TB.
      head      = 0; // Lower 4-bits of HDDEVSEL are not used here.
   } else if (ide_devices[drive].capabilities & 0x200)  { // Drive supports LBA?
      // LBA28:
      lba_mode  = 1;
      lba_io[0] = (lba & 0x00000FF)>> 0;
      lba_io[1] = (lba & 0x000FF00)>> 8;
      lba_io[2] = (lba & 0x0FF0000)>>16;
      lba_io[3] = 0; // These Registers are not used here.
      lba_io[4] = 0; // These Registers are not used here.
      lba_io[5] = 0; // These Registers are not used here.
      head      = (lba & 0xF000000)>>24;
   } else {
      // CHS:
      lba_mode  = 0;
      sect      = (lba % 63) + 1;
      cyl       = (lba + 1  - sect)/(16*63);
      lba_io[0] = sect;
      lba_io[1] = (cyl>>0) & 0xFF;
      lba_io[2] = (cyl>>8) & 0xFF;
      lba_io[3] = 0;
      lba_io[4] = 0;
      lba_io[5] = 0;
      head      = (lba + 1  - sect)%(16*63)/(63); // Head number is written to HDDEVSEL lower 4-bits.
   }

   // (II) See if Drive Supports DMA or not;
   dma = 0; // Supports or doesn't, we don't support !!!

   // (III) Wait if the drive is busy;
   while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait if Busy.

   // (IV) Select Drive from the controller;
   if (lba_mode == 0) 
        ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit<<4) | head);   // Select Drive CHS.
   else         
        ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit<<4) | head);   // Select Drive LBA.

   // (V) Write Parameters;
   if (lba_mode == 2) {
       ide_write(channel, ATA_REG_SECCOUNT1,   0);
       ide_write(channel, ATA_REG_LBA3,   lba_io[3]);
       ide_write(channel, ATA_REG_LBA4,   lba_io[4]);
       ide_write(channel, ATA_REG_LBA5,   lba_io[5]);
   }
   ide_write(channel, ATA_REG_SECCOUNT0,   numsects);
   ide_write(channel, ATA_REG_LBA0,   lba_io[0]);
   ide_write(channel, ATA_REG_LBA1,   lba_io[1]);
   ide_write(channel, ATA_REG_LBA2,   lba_io[2]);

   // (VI) Select the command and send it;
   // Routine that is followed:
   // If ( DMA & LBA48)   DO_DMA_EXT;
   // If ( DMA & LBA28)   DO_DMA_LBA;
   // If ( DMA & LBA28)   DO_DMA_CHS;
   // If (!DMA & LBA48)   DO_PIO_EXT;
   // If (!DMA & LBA28)   DO_PIO_LBA;
   // If (!DMA & !LBA#)   DO_PIO_CHS;

   if (lba_mode == 0 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
   if (lba_mode == 1 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;   
   if (lba_mode == 2 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;   
   if (lba_mode == 0 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
   if (lba_mode == 1 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
   if (lba_mode == 2 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA_EXT;
   if (lba_mode == 0 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
   if (lba_mode == 1 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
   if (lba_mode == 2 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
   if (lba_mode == 0 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
   if (lba_mode == 1 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
   if (lba_mode == 2 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
   ide_write(channel, ATA_REG_COMMAND, cmd);               // Send the Command.

   if (dma) {
       if (direction == 0) {
           // DMA Read.
       } else {
           // DMA Write.
       }
   } else {
       if (direction == ATA_READ)
           // PIO Read.
           for (i = 0; i < numsects; i++) {
               if ((err = ide_polling(channel, 1))) return err; // Polling, then set error and exit if there is.
               insw(port, (u32int)buf, words);
               buf += words*2;
           } else {
               // PIO Write.
               for (i = 0; i < numsects; i++) {
                   ide_polling(channel, 0); // Polling.
                   outsw(port, (u32int)buf, words);
                   buf += words*2;
               }
               ide_write(channel, ATA_REG_COMMAND, (char []) {   ATA_CMD_CACHE_FLUSH,
                       ATA_CMD_CACHE_FLUSH,
                       ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
               ide_polling(channel, 0); // Polling.
           }
   }

   return 0; // Easy, ... Isn't it?
}

u8int ide_print_error(u32int drive, u8int err) {
    if (err == 0)
        return err;

    printk("IDE:");
    if (err == 1) {printk("- Device Fault\n     "); err = 19;}
    else if (err == 2) {
        u8int st = ide_read(ide_devices[drive].channel, ATA_REG_ERROR);
        if (st & ATA_ER_AMNF)   {printk("- No Address Mark Found\n   ");   err = 7;}
        if (st & ATA_ER_TK0NF)  {printk("- No Media or Media Error\n ");   err = 3;}
        if (st & ATA_ER_ABRT)   {printk("- Command Aborted\n         ");   err = 20;}
        if (st & ATA_ER_MCR)    {printk("- No Media or Media Error\n ");   err = 3;}
        if (st & ATA_ER_IDNF)   {printk("- ID mark not Found\n       ");   err = 21;}
        if (st & ATA_ER_MC)     {printk("- No Media or Media Error\n ");   err = 3;}
        if (st & ATA_ER_UNC)    {printk("- Uncorrectable Data Error\n");   err = 22;}
        if (st & ATA_ER_BBK)    {printk("- Bad Sectors\n             ");   err = 13;}
    } else  if (err == 3)       {printk("- Reads Nothing\n           ");   err = 23;}
    else  if (err == 4)         {printk("- Write Protected\n         ");   err = 8;}
    printk("- [%s %s] %s\n",
            (const char *[]){"Primary", "Secondary"}[ide_devices[drive].channel], // Use the channel as an index into the array
            (const char *[]){"Master", "Slave"}[ide_devices[drive].drive], // Same as above, using the drive
            ide_devices[drive].model);

    return err;
}

void ide_initialize(u32int bar0, u32int bar1, u32int bar2, u32int bar3, u32int bar4)
{
    int i, j, k, count = 0;

    // 1- Detect I/O Ports which interface IDE Controller:
    // if bar0==0, !bar0==1, .base = bar0, else .base = bar0 with io port mask
    channels[ATA_PRIMARY  ].base  = (bar0 & 0xFFFFFFFC) + 0x1F0 * (!bar0);
    channels[ATA_PRIMARY  ].ctrl  = (bar1 & 0xFFFFFFFC) + 0x3F4 * (!bar1);
    channels[ATA_SECONDARY].base  = (bar2 & 0xFFFFFFFC) + 0x170 * (!bar2);
    channels[ATA_SECONDARY].ctrl  = (bar3 & 0xFFFFFFFC) + 0x374 * (!bar3);
    channels[ATA_PRIMARY  ].bmide = (bar4 & 0xFFFFFFFC) + 0; // Bus Master IDE
    channels[ATA_SECONDARY].bmide = (bar4 & 0xFFFFFFFC) + 8; // Bus Master IDE

    // 2- Software reset:
    ide_write(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

    // 3- Detect ATA-ATAPI Devices:
    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++) {

            u8int err = 0, type = IDE_ATA, status;
            ide_devices[count].exists = 0; // Assuming that no drive here.

            // (I) Select Drive:
            ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
            sys_msleep(1); // Wait 1ms for drive select to work.

            // (II) Send ATA Identify Command:
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            sys_msleep(1);

            // (III) Polling:
            if (ide_read(i, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.

            while(1) {
                status = ide_read(i, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
            }

            // (IV) Probe for ATAPI Devices:

            if (err != 0) {
                u8int cl = ide_read(i, ATA_REG_LBA1);
                u8int ch = ide_read(i, ATA_REG_LBA2);

                if (cl == 0x14 && ch ==0xEB)
                    type = IDE_ATAPI;
                else if (cl==0 && ch == 0) 
                    type = IDE_ATA;
                else
                    continue; // Unknown type (may not be a device).

                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                sys_msleep(1);
            }

            // (V) Read Identification Space of the Device:
            ide_read_buffer(i, ATA_REG_DATA, (u32int) ide_buf, 128);

            // (VI) Read Device Parameters:
            ide_devices[count].exists       = 1;
            ide_devices[count].type         = type;
            ide_devices[count].channel      = i;
            ide_devices[count].drive        = j;
            ide_devices[count].signature    = *((u16int *)(ide_buf + ATA_IDENT_DEVICETYPE));
            ide_devices[count].capabilities         = *((u16int *)(ide_buf + ATA_IDENT_CAPABILITIES));
            ide_devices[count].command_sets = *((u32int *)(ide_buf + ATA_IDENT_COMMANDSETS));
            ide_devices[count].udma  = *((u32int *)(ide_buf + ATA_IDENT_UDMA)) >> 8;

            // (VII) Get size:
            if (ide_devices[count].command_sets & (1 << 26)) {
                // Device uses 48-Bit Addressing:
                ide_devices[count].size   = *((u32int *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
            } else {
                // Device uses CHS or 28-bit Addressing:
                ide_devices[count].size   = *((u32int *)(ide_buf + ATA_IDENT_MAX_LBA));
            }

            // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
            for(k = 0; k < 40; k += 2) {
                ide_devices[count].model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                ide_devices[count].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];}
                ide_devices[count].model[40] = 0; // Terminate String.

                count++;
        }
    // 4- Print Summary:
    for (i = 0; i < 4; i++)
        if (ide_devices[i].exists == 1) {
            printk(" Found %s Drive %dMB - %s\n",
                    (const char *[]){"ATA", "ATAPI"}[ide_devices[i].type],         /* type */
                    ide_devices[i].size / 1024 / 2,               /* size */
                    ide_devices[i].model);
            if (ide_devices[i].type == IDE_ATA) {
               disk_t *d = (disk_t*)kmalloc(sizeof(disk_t));
               memset(d, 0, sizeof(disk_t));
               d->dev_id = 0x400000 + i*0x10000;
               d->name = strdup((char*)ide_devices[i].model);
               d->blk_size = 512;
               d->nr_blks = ide_devices[i].size;
               d->read_blks = &ide_read_blks;
               d->write_blks = &ide_write_blks;
               add_disk(d);
            }

        }

    /*register_interrupt_handler(IRQ14, &ide_irq);*/

}

s32int ide_read_sectors(u8int drive, u8int numsects, u32int lba, u8int *buf) 
{
    s32int ret;

    // 1: Check if the drive presents:
    // ==================================
    if (drive > 3 || ide_devices[drive].exists == 0) ret = 0x1;      // Drive Not Found!

    // 2: Check if inputs are valid:
    // ==================================
    else if (((lba + numsects) > ide_devices[drive].size) && (ide_devices[drive].type == IDE_ATA))
        ret = 0x2;                     // Seeking to invalid position.

    // 3: Read in PIO Mode through Polling & IRQs:
    // ============================================
    else {
        u8int err;
        if (ide_devices[drive].type == IDE_ATA)
            err = ide_ata_access(ATA_READ, drive, lba, numsects, buf);
        else if (ide_devices[drive].type == IDE_ATAPI) {
            /*
            for (i = 0; i < numsects; i++)
                err = ide_atapi_read(drive, lba + i, 1, es, edi + (i*2048));
             */
        }
        ret = ide_print_error(drive, err);
    }

    return ret;
}

s32int ide_write_sectors(u8int drive, u8int numsects, u32int lba, u8int *buf)
{
    s32int ret;

    // 1: Check if the drive presents:
    // ==================================
    if (drive > 3 || ide_devices[drive].exists == 0) ret = 0x1;      // Drive Not Found!
    // 2: Check if inputs are valid:
    // ==================================
    else if (((lba + numsects) > ide_devices[drive].size) && (ide_devices[drive].type == IDE_ATA))
        ret = 0x2;                     // Seeking to invalid position.
    // 3: Read in PIO Mode through Polling & IRQs:
    // ============================================
    else {
        u8int err;
        if (ide_devices[drive].type == IDE_ATA)
            err = ide_ata_access(ATA_WRITE, drive, lba, numsects, buf);
        else if (ide_devices[drive].type == IDE_ATAPI)
            err = 4; // Write-Protected.
        ret = ide_print_error(drive, err);
    }

    return ret;
}

s32int ide_attach(pci_dev_t *dev)
{
    u32int bar0, bar1, bar2, bar3, bar4;
    bar0 = read_pci_dev_config_dword(dev, PCI_BAR0_OFFSET);
    bar1 = read_pci_dev_config_dword(dev, PCI_BAR1_OFFSET);
    bar2 = read_pci_dev_config_dword(dev, PCI_BAR2_OFFSET);
    bar3 = read_pci_dev_config_dword(dev, PCI_BAR3_OFFSET);
    bar4 = read_pci_dev_config_dword(dev, PCI_BAR4_OFFSET);

   if (dev->config->class_code == 0x01 && dev->config->subclass == 0x01 
           && (dev->config->prog_if == 0x8A || dev->config->prog_if == 0x80)) {
       // This is a Parallel IDE Controller which uses IRQs 14 and 15.
       printk("Parallel IDE Controller detected\n");

   }
   ide_initialize(bar0, bar1, bar2, bar3, bar4);

   return 0;
}

s32int ide_deattach(pci_dev_t *dev)
{
    return 0;
}

pci_driver_t drv = { 
    .name = "ide",
    .probe = &ide_probe,
    .attach = &ide_attach,
    .deattach = &ide_deattach,
};

void ide_Init()
{
    printk("module_ide_init %p\n", &drv);
    registerPCIDriver(&drv);
}

void ide_Cleanup()
{
    unregisterPCIDriver(&drv);
}


MODULE_INIT(module_ide_init);
MODULE_CLEANUP(module_ide_cleanup);
MEXPORT(ide_read_sectors);
MEXPORT(ide_write_sectors);

