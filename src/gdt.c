#include <gdt.h>
#include <asm.h>
#include <util.h>

gdtrec create_gdt_rec(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    gdtrec rec;
    rec.base_h = base >> 24;
    rec.base_m = (base >> 16) & 0xff;
    rec.base_l = base & 0xffff;
    rec.length = limit & 0xffff;
    rec.access = access;
    rec.fllen = (limit >> 16) & 0xf;
    rec.fllen |= (flags & 0xf0);
    return rec;
}

void load_gdt_recs(gdtrec *gdt_records, tss_rec *tss_entry)
{
    gdt_records[0] = create_gdt_rec(0, 0, 0, 0);                                                             // Null segment
    gdt_records[1] = create_gdt_rec(0, 0xFFFFFFFF, 0x9A, 0xCF);                                              // Code segment
    gdt_records[2] = create_gdt_rec(0, 0xFFFFFFFF, 0x92, 0xCF);                                              // Data segment
    gdt_records[3] = create_gdt_rec(0, 0xFFFFFFFF, 0xFA, 0xCF);                                              // User mode code segment
    gdt_records[4] = create_gdt_rec(0, 0xFFFFFFFF, 0xF2, 0xCF);                                              // User mode data segment
    gdt_records[5] = create_gdt_rec((uint32_t)tss_entry, (uint32_t)tss_entry + sizeof(tss_rec), 0xE9, 0x00); // tss

    gdtarray arr;
    arr.ptr = gdt_records;
    arr.len = GDTARR_LEN * sizeof(gdtrec) - 1;

    memset(tss_entry, 0, sizeof(tss_rec));
    tss_entry->gs = tss_entry->es = tss_entry->ds = tss_entry->ss = tss_entry->fs = 0x10 | 0x3;
    tss_entry->cs = 0x8 | 0x3;
    tss_entry->ss0 = 0x10;
    tss_entry->esp0 = 0;

    asm_lgdt(arr);
    asm_flush_tss();
}