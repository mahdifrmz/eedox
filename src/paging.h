#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

typedef struct
{
    int32_t present : 1;  // Page present in memory
    int32_t rw : 1;       // Read-only if clear, readwrite if set
    int32_t user : 1;     // Supervisor level only if clear
    int32_t accessed : 1; // Has the page been accessed since last refresh?
    int32_t dirty : 1;    // Has the page been written to since last refresh?
    int32_t unused : 7;   // Amalgamation of unused and reserved bits
    int32_t frame : 20;   // Frame address (shifted right 12 bits)
} page_t;

typedef struct
{
    page_t pages[1024];
} page_table_t;

typedef struct
{
    page_table_t *tables[1024];
    uint32_t tables_physical[1024];
    uint32_t physical;
} page_directory_t;

extern page_directory_t *current_page_directory;

uint32_t get_physical_address(uint32_t virtual_address);
void switch_page_directory(page_table_t **dir);
void paging_init();
page_table_t *page_table_clone(page_table_t *table);
page_directory_t *page_directory_clone(page_directory_t *dir);
void paging_physcpy(uint32_t src, uint32_t dest);
void alloc_frame(page_t *page, int is_writable, int is_kernel);
page_t *get_page(uint32_t address, uint8_t init, page_directory_t *dir);

#endif