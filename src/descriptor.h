#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <stdint.h>

#define FD_ACCESS_READ 1
#define FD_ACCESS_WRITE 2

#define FD_KIND_STDIN 1
#define FD_KIND_STDOUT 2
#define FD_KIND_DISK 4
#define FD_KIND_DIR 5
#define FD_KIND_PIPE 6

typedef struct
{
    uint32_t kind;
    uint32_t pos;
    uint8_t isopen;
    uint8_t access;
    void *ptr;
} fd_t;

typedef struct
{
    fd_t *records;
    uint32_t cap;
    uint32_t size;
} fd_table;

uint32_t fd_table_add(fd_table *table, fd_t fd);
fd_t fd_table_clone_entry(fd_t* fd);
uint32_t fd_table_dup(fd_table *table, uint32_t index);
void fd_table_rem(fd_table *table, uint32_t index);
fd_table fd_table_create(uint32_t inital_size);
fd_table fd_table_clone(fd_table *table);
void fd_table_close(fd_table* table, uint32_t fd_id);

#endif