#include <paging.h>
#include <kheap.h>
#include <bitset.h>
#include <kutil.h>
#include <asm.h>
#include <idt.h>
#include <trace.h>

extern heap_t kernel_heap;
bitset_t glb_frames;

page_directory_t *kernel_page_directory = 0x0;
page_directory_t *current_page_directory = 0x0;

void alloc_frame(page_t *page, int is_writable, int is_kernel)
{
    int32_t idx = bitset_first_unset(&glb_frames);
    bitset_set(&glb_frames, idx, 1);
    if (idx == -1)
    {
        return;
    }
    page->frame = idx;
    page->present = 1;
    page->rw = is_writable ? 1 : 0;
    page->user = is_kernel ? 0 : 1;
}

void free_frame(page_t *page)
{
    if (page->frame)
    {
        bitset_set(&glb_frames, page->frame, 0);
        page->frame = 0x0;
        page->present = 0;
    }
}

page_t *get_page(uint32_t address, uint8_t init, page_directory_t *dir)
{
    address /= 0x1000;
    uint32_t table_index = address / 1024;
    uint32_t entry_index = address % 1024;
    if (dir->tables[table_index] == 0x0)
    {
        dir->tables[table_index] = (page_table_t *)kmalloc_a(sizeof(page_table_t));
        memset(dir->tables[table_index], 0, 0x1000);
        if (init)
        {
            dir->tables_physical[table_index] = (uint32_t)dir->tables[table_index] | 0x7;
        }
        else
        {
            dir->tables_physical[table_index] = get_physical_address((uint32_t)dir->tables[table_index]) | 0x7;
        }
    }
    page_table_t *table = (page_table_t *)((uint32_t)dir->tables[table_index] & 0xFFFFF000);
    return &table->pages[entry_index];
}

uint32_t get_physical_address(uint32_t virtual_address)
{
    if (current_page_directory)
    {
        page_t *page = get_page(virtual_address, 0, current_page_directory);
        return page->frame * 0x1000 + virtual_address % 0x1000;
    }
    return virtual_address;
}

void paging_init()
{
    uint32_t total_frames = 0x100000;
    uint32_t frames_size = total_frames / 8;
    bitset_init(&glb_frames, kmalloc(frames_size), total_frames);

    kernel_page_directory = kmalloc_a(sizeof(page_directory_t));
    memset(kernel_page_directory, 0, sizeof(page_directory_t));

    for (uint32_t i = 0x0; i < (uint32_t)kernel_heap.start + kernel_heap.size; i += 0x1000)
    {
        page_t *page = get_page(i, 1, kernel_page_directory);
        alloc_frame(page, 0, 0);
    }
    kernel_page_directory->physical = (uint32_t)kernel_page_directory->tables_physical;
    current_page_directory = page_directory_clone(kernel_page_directory);
    switch_page_directory((page_table_t **)current_page_directory->physical);

    load_int_handler(INTCODE_PAGEFAULT, page_fault);
}

page_directory_t *page_directory_clone(page_directory_t *dir)
{
    page_directory_t *newdir = kmalloc_a(sizeof(page_directory_t));
    memset(newdir, 0, sizeof(page_directory_t));
    newdir->physical = (uint32_t)get_physical_address((uint32_t)newdir) + ((uint32_t)newdir->tables_physical - (uint32_t)newdir);
    for (uint32_t i = 0; i < 1024; i++)
    {
        if (dir->tables[i])
        {
            if (dir->tables[i] == kernel_page_directory->tables[i])
            {
                newdir->tables[i] = dir->tables[i];
                newdir->tables_physical[i] = dir->tables_physical[i];
            }
            else
            {
                newdir->tables[i] = page_table_clone(dir->tables[i]);
                newdir->tables_physical[i] = (uint32_t)get_physical_address((uint32_t)newdir->tables[i]) | 0x7;
            }
        }
    }
    return newdir;
}

page_table_t *page_table_clone(page_table_t *table)
{
    page_table_t *new_table = kmalloc_a(sizeof(page_table_t));
    memset(new_table,0,sizeof(page_table_t));
    for (uint32_t i = 0; i < 1024; i++)
    {
        if (table->pages[i].frame)
        {
            alloc_frame(&new_table->pages[i], 0, 0);
            new_table->pages[i].rw = table->pages[i].rw;
            new_table->pages[i].accessed = table->pages[i].accessed;
            new_table->pages[i].present = table->pages[i].present;
            new_table->pages[i].user = table->pages[i].user;
            new_table->pages[i].dirty = table->pages[i].dirty;
            paging_physcpy(table->pages[i].frame * 0x1000, new_table->pages[i].frame * 0x1000);
        }
    }

    return new_table;
}

void page_fault(registers *regs)
{
    const char *present = !(regs->err_code & 0x1) ? "present " : ""; // Page not present
    const char *rw = regs->err_code & 0x2 ? "read-only " : "";       // Write operation?
    const char *us = regs->err_code & 0x4 ? "user-mode " : "";       // Processor was in user-mode?
    const char *reserved = regs->err_code & 0x8 ? "reserved " : "";  // Overwritten CPU-reserved bits of page entry?
    const char *fetch = regs->err_code & 0x10 ? "fetch " : "";

    trace(regs->eip, regs->ebp);
    kpanic("paging fault [%x] ( %s%s%s%s%s) ", asm_get_cr2(), present, rw, us, reserved, fetch);
}