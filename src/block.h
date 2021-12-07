#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

typedef uint32_t lba28_t;

typedef enum
{
    inode_type_file,
    inode_type_dir
} inode_type;

extern lba28_t fsstart;

void binit();
lba28_t balloc(lba28_t sectors);
void bfree(lba28_t address);
lba28_t brealloc(lba28_t address, lba28_t size);

typedef struct
{
    uint32_t isvalid;
    char name[257];
    uint32_t index; // lba
    uint32_t size;  // in bytes
    uint32_t alloc; // data sectors count
    uint32_t child_count;
    uint32_t parent; // lba
    inode_type type;
} __attribute__((packed)) inode_t;

typedef struct
{
    uint32_t bytes_from;
    uint32_t bytes_count;
    lba28_t sec_from;
    lba28_t sec_count;
    lba28_t sec_overflow;
    uint32_t bytes_overflow;

} operation_bounds;

inode_t *inode_create(inode_type type, inode_t *parent, char *name);
void inode_delete(inode_t *node, inode_t *parent);
void inode_write(inode_t *node, uint32_t from, char *buffer, uint32_t count);
void inode_read(inode_t *node, uint32_t from, char *buffer, uint32_t count);
inode_t *inode_children(inode_t *node);
void inode_child_set(inode_t *node, lba28_t rem, lba28_t add);
void inode_fetch(lba28_t index, inode_t **parent);
inode_t *inode_parent(inode_t *parent);
void inode_calculate_operation_bounds(inode_t *node, operation_bounds *operation);
void inode_realloc(inode_t *node, uint32_t sectors);

#endif