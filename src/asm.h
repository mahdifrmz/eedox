#ifndef ASM_H
#define ASM_H

#include <stdint.h>
#include <gdt.h>
#include <idt.h>

void asm_out(unsigned short port, unsigned char byte);
void asm_lgdt(gdtarray arr);
void asm_lidt(idtarray arr);

#endif