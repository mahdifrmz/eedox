#include <gdt.h>
#include <asm.h>

void load_gdt_recs(gdtrec *gdt_records)
{
    // null : 0x00
    gdt_records[0].length = 0;
    gdt_records[0].base_l = 0;
    gdt_records[0].base_m = 0;
    gdt_records[0].flags = 0;
    gdt_records[0].fllen = 0;
    gdt_records[0].base_h = 0;

    // code : 0x08
    gdt_records[1].length = 0xffff;
    gdt_records[1].base_l = 0;
    gdt_records[1].base_m = 0;
    gdt_records[1].flags = 0b10011010;
    gdt_records[1].fllen = 0b11001111;
    gdt_records[1].base_h = 0;

    // data : 0x10
    gdt_records[2].length = 0xffff;
    gdt_records[2].base_l = 0;
    gdt_records[2].base_m = 0;
    gdt_records[2].flags = 0b10010010;
    gdt_records[2].fllen = 0b11001111;
    gdt_records[2].base_h = 0;

    gdtarray arr;
    arr.ptr = gdt_records;
    arr.len = 3 * sizeof(gdtrec) - 1;

    asm_lgdt(arr);
}