#ifndef ASM_H
#define ASM_H

#include <stdint.h>
#include <gdt.h>
#include <idt.h>

void asm_out(unsigned short port, unsigned char byte);
uint8_t asm_in(unsigned short port);
void asm_lgdt(gdtarray arr);
void asm_lidt(idtarray arr);
void asm_cli();
void asm_sti();
uint32_t asm_get_eip();
uint32_t asm_get_esp();
uint32_t asm_get_ebp();
void asm_multsk_switch();
void asm_set_sps(uint32_t ebp, uint32_t esp);
void asm_flush_TLB();
#endif