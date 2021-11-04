#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct
{
    uint16_t offset_1; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t zero;      // unused, set to 0
    uint8_t type_attr; // type and attributes, see below
    uint16_t offset_2; // offset bits 16..31
} __attribute__((packed)) idtrec;

typedef struct
{
    unsigned short len;
    idtrec *ptr;
} __attribute__((packed)) idtarray;

#endif