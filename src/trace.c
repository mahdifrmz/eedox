#include <trace.h>
#include <kutil.h>
#include <asm.h>
#include <multsk.h>

// extern uint32_t symtable_count;
// extern char symtable;
// fnrec *symtable_ptr = (fnrec *)&symtable;

// uint32_t fnrec_find(uint32_t eip)
// {
//     uint32_t index = 0;
//     while (index + 1 < symtable_count && eip >= symtable_ptr[index + 1].ptr)
//     {
//         index++;
//     }
//     return index;
// }

void common_interrupt_handler();
void set_interrupt_handler();

void trace(uint32_t eip, uint32_t ebp)
{
    // fnrec rec = symtable_ptr[fnrec_find(eip)];
    // kprintf("trace -> %s(%s)\n", rec.name, rec.address);
    if (!eip)
    {
        eip = asm_get_eip();
    }
    if (!ebp)
    {
        ebp = asm_get_ebp();
    }
    kprintf("trace -> %x\n", eip);
    if (!(eip >= (uint32_t)common_interrupt_handler && eip < (uint32_t)set_interrupt_handler))
    {
        uint32_t prev_ebp = *(uint32_t *)ebp;
        uint32_t prev_eip = *(uint32_t *)(ebp + 4);
        trace(prev_eip, prev_ebp);
    }
}