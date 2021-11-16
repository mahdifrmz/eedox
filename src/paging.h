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
} page_directory_t;

void switch_page_directory(page_table_t **dir);
void paging_init();

#endif