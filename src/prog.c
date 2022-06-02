#include <prog.h>
#include <util.h>
#include <paging.h>
#include <elf.h>
#include <task.h>
#include <kutil.h>

int32_t prog_load(const char *file, uint32_t laddr, uint32_t *entry)
{
    Elf32_Ehdr elf_header = *(Elf32_Ehdr *)file;
    char elf_signature [5];
    memcpy(elf_signature,elf_header.e_ident,4);
    elf_signature[4] = 0;
    if(strcmp(elf_signature,"\x7f""ELF") != 0)
    {
        return -1;
    }
    Elf32_Phdr *prog_arr = (Elf32_Phdr *)(file + elf_header.e_phoff);
    uint32_t prog_arrlen = elf_header.e_phnum;
    uint32_t brk = 0;
    for (uint32_t i = 1; i < prog_arrlen; i++)
    {
        uint32_t start = prog_arr[i].p_vaddr;
        uint32_t end = prog_arr[i].p_vaddr + prog_arr[i].p_memsz - 1;
        if (start == 0)
            continue;
        if (start < laddr)
        {
            kprintf("KERNEL FAILURE: invalid load address for elf file\n");
        }
        for (uint32_t i = start; i <= end; i += 0x1000)
        {
            page_t *page = get_page(i, 0, current_page_directory);
            if (!page->frame)
            {
                alloc_frame(page, 1, 0);
            }
            if(brk < end)
            {
                brk = end;
                if(brk % 0x1000)
                {
                    brk &= 0xfffff000;
                    brk += 0x1000;
                }
            }
        }
        task_curtask()->brk = brk;
        const char *data = file + prog_arr[i].p_offset;
        memcpy((void *)start, data, prog_arr[i].p_memsz);
    }
    *entry = elf_header.e_entry;
    return 0;
}