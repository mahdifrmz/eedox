#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <idt.h>

#define SECTOR_SIZE 512

void ata_read(uint32_t sector, void *buffer);
void ata_write(uint32_t sector, void *buffer);
void ata_ihandler(__attribute__((unused)) registers *regs);
void ata_init();

typedef enum
{
    ATA_OP_READ,
    ATA_OP_WRITE,
    ATA_OP_FLUSH,
} ata_op;

#endif