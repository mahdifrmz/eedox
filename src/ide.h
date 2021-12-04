#ifndef IDE_H
#define IDE_H

#include <stdint.h>

void ata_read(uint32_t sector, void *buffer);
void ata_write(uint32_t sector, void *buffer);
void ata_ihandler();

typedef enum
{
    ATA_OP_READ,
    ATA_OP_WRITE,
    ATA_OP_FLUSH,
} ata_op;

#endif