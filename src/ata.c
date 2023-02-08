#include <ata.h>
#include <asm.h>
#include <task.h>
#include <util.h>
#include <lock.h>

#define ATA_REG_DATA 0x1f0
#define ATA_REG_ERROR 0x1f1
#define ATA_REG_FEATURES 0x1f1
#define ATA_REG_SECCOUNT 0x1f2
#define ATA_REG_LBA_LOW 0x1f3
#define ATA_REG_LBA_MID 0x1f4
#define ATA_REG_LBA_HIGH 0x1f5
#define ATA_REG_DH 0x1f6
#define ATA_REG_CMD 0x1f7
#define ATA_REG_STATUS 0x1f7
#define ATA_REG_ALTSTATUS 0x3f6
#define ATA_REG_DEVCTRL 0x3f6

#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DF 0x20
#define ATA_STATUS_DSC 0x10
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_CORR 0x04
#define ATA_STATUS_IDX 0x02
#define ATA_STATUS_ERR 0x01

#define ATA_ERR_BBK 0x80
#define ATA_ERR_UNC 0x40
#define ATA_ERR_MC 0x20
#define ATA_ERR_IDNF 0x10
#define ATA_ERR_MCR 0x08
#define ATA_ERR_ABRT 0x04
#define ATA_ERR_TK0NF 0x02
#define ATA_ERR_AMNF 0x01

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC

uint32_t ata_current_lba;
void *ata_current_buffer;
ata_op ata_current_op;
task_t *ata_task = NULL;
ksemaphore_t disk_sem;

void ata_init()
{
    ksemaphore_init(&disk_sem, 1);
    load_int_handler(INTCODE_ATA, ata_ihandler);
}

void ata_read(uint32_t sector, void *buffer)
{
    ksemaphore_wait(&disk_sem);
    ata_current_buffer = buffer;
    ata_current_op = ATA_OP_READ;
    ata_current_lba = sector;

    while (asm_inb(ATA_REG_STATUS) & ATA_STATUS_BSY)
        ;
    while ((asm_inb(ATA_REG_STATUS) & ATA_STATUS_DRDY) == 0)
        ;
    asm_outb(ATA_REG_SECCOUNT, 1);
    asm_outb(ATA_REG_LBA_LOW, sector & 0xff);
    asm_outb(ATA_REG_LBA_MID, (sector >> 8) & 0xff);
    asm_outb(ATA_REG_LBA_HIGH, (sector >> 16) & 0xff);
    asm_outb(ATA_REG_DH, 0xe0 | ((sector >> 24) & 0x0f));
    asm_outb(ATA_REG_CMD, ATA_CMD_READ_PIO);

    ata_task = task_curtask();
    task_sleep();
    ksemaphore_signal(&disk_sem);
}

void ata_write(uint32_t sector, void *buffer)
{
    ksemaphore_wait(&disk_sem);
    ata_current_buffer = buffer;
    ata_current_op = ATA_OP_WRITE;
    ata_current_lba = sector;

    while (asm_inb(ATA_REG_STATUS) & ATA_STATUS_BSY)
        ;
    while ((asm_inb(ATA_REG_STATUS) & ATA_STATUS_DRDY) == 0)
        ;
    asm_outb(ATA_REG_SECCOUNT, 1);
    asm_outb(ATA_REG_LBA_LOW, sector & 0xff);
    asm_outb(ATA_REG_LBA_MID, (sector >> 8) & 0xff);
    asm_outb(ATA_REG_LBA_HIGH, (sector >> 16) & 0xff);
    asm_outb(ATA_REG_DH, 0xe0 | ((sector >> 24) & 0x0f));
    asm_outb(ATA_REG_CMD, ATA_CMD_WRITE_PIO);

    while ((asm_inb(ATA_REG_STATUS) & ATA_STATUS_DRQ) == 0)
        ;

    uint32_t i = 0;
    while (i < SECTOR_SIZE / 2)
    {
        asm_outw(ATA_REG_DATA, ((uint16_t *)buffer)[i++]);
    }

    ata_task = task_curtask();
    task_sleep();
    ksemaphore_signal(&disk_sem);
}

void ata_ihandler(__attribute__((unused)) registers *regs)
{

    if (ata_current_op == ATA_OP_WRITE)
    {
        ata_current_op = ATA_OP_FLUSH;
        asm_outb(ATA_REG_CMD, ATA_CMD_CACHE_FLUSH);
        if (ata_task)
        {
            task_awake(ata_task);
            ata_task = NULL;
        }
    }
    else
    {
        if (ata_current_op == ATA_OP_READ)
        {
            asm_insw(ATA_REG_DATA, ata_current_buffer, SECTOR_SIZE / 2);
        }
        if (ata_task)
        {
            task_awake(ata_task);
            ata_task = NULL;
        }
    }
}