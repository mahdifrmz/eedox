#ifndef ASM_H
#define ASM_H

#include <stdint.h>
#include <gdt.h>
#include <idt.h>

void asm_outb(unsigned short port, unsigned char byte);
void asm_outw(unsigned short port, unsigned short word);
uint8_t asm_inb(unsigned short port);
void asm_lgdt(gdtarray arr);
void asm_lidt(idtarray arr);

void asm_disk_write(uint32_t sector, uint32_t count, uint32_t buffer);
void asm_disk_read(uint32_t sector, uint32_t count, uint32_t buffer);

void asm_cli();
void asm_sti();
void asm_insw(uint16_t port, void *address, uint32_t count);
void asm_outsw(uint16_t port, void *address, uint32_t count);
uint32_t asm_get_cr2();
uint32_t asm_get_eip();
uint32_t asm_get_esp();
uint32_t asm_get_ebp();
void asm_multsk_switch();
void asm_usermode(void *userprog);
void asm_set_sps(uint32_t ebp, uint32_t esp);
void asm_flush_TLB();
void asm_flush_tss();
#endif