#include <paging.h>
#include <kheap.h>
#include <bitset.h>
#include <kutil.h>

extern heap_t glb_heap;
bitset_t glb_frames;
page_directory_t *glb_page_directory;

void alloc_frame(bitset_t *frames, page_t *page, int is_writable, int is_kernel)
{
    int32_t idx = bitset_first_unset(frames);
    bitset_set(frames, idx, 1);
    if (idx == -1)
    {
        return;
    }
    page->frame = idx;
    page->present = 1;
    page->rw = is_writable ? 1 : 0;
    page->user = is_kernel ? 0 : 1;
}

void free_frame(bitset_t *frames, page_t *page)
{
    if (page->frame)
    {
        bitset_set(frames, page->frame, 0);
        page->frame = 0x0;
        page->present = 0;
    }
}

page_t *get_page(uint32_t address, page_directory_t *dir)
{
    address /= 0x1000;
    uint32_t table_index = address / 1024;
    uint32_t entry_index = address % 1024;
    if (dir->tables[table_index] == 0x0)
    {
        dir->tables[table_index] = (page_table_t *)((uint32_t)kmalloc_a(sizeof(page_table_t)) | 0x7);
    }
    page_table_t *table = (page_table_t *)((uint32_t)dir->tables[table_index] & 0xFFFFF000);
    return &table->pages[entry_index];
}

void paging_init()
{
    uint32_t total_frames = 0x100000;
    uint32_t frames_size = total_frames / 8;
    bitset_init(&glb_frames, kmalloc(frames_size), total_frames);

    glb_page_directory = kmalloc_a(sizeof(page_directory_t));
    for (int i = 0; i < 1024; i++)
    {
        glb_page_directory->tables[i] = 0x0;
    }
    for (uint32_t i = 0x0; i < (uint32_t)glb_heap.start + glb_heap.size; i += 0x1000)
    {
        page_t *page = get_page(i, glb_page_directory);
        alloc_frame(&glb_frames, page, 0, 0);
    }
    switch_page_directory((page_table_t **)glb_page_directory->tables);
}