#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <vec.h>
#include <kstring.h>
#include <lock.h>
#include <pathbuf.h>
#include <ata.h>

#define CHOP_ADD 0
#define CHOP_REM 1
#define CHOP_RELOC 2
#define CHOP_RENAME 3

typedef uint32_t lba28_t;

typedef enum
{
    inode_type_file,
    inode_type_dir
} inode_type;

typedef struct inode_t inode_t;

struct /*__attribute__((packed))*/ inode_t
{
    uint8_t isvalid;
    uint32_t index; // lba
    uint32_t size;  // in bytes
    uint32_t alloc; // data sectors count
    uint32_t child_count;
    uint32_t parent; // lba
    inode_type type;

    uint32_t _refs;
    pathbuf_t _pathbuf;
    krwlock _lock;
};

typedef struct
{
    void *ptr;
    uint32_t size;
    uint32_t sectors;
} childtable_t;

typedef struct
{
    uint32_t bytes_from;
    uint32_t bytes_count;
    lba28_t sec_from;
    lba28_t sec_count;
    lba28_t sec_overflow;
    uint32_t bytes_overflow;

} operation_bounds;

typedef struct
{
    const char *name1;
    const char *name2;
    lba28_t index;
    uint8_t op;

} child_operation;

void binit();
lba28_t balloc(lba28_t sectors);
void bfree(lba28_t address);
lba28_t brealloc(lba28_t address, lba28_t size);
void balloc_update();
void balloc_fetch();

void childtable_add(childtable_t *table, const char *name, lba28_t index);
void childtable_remove(childtable_t *table, const char *name);
void childtable_edit_index(childtable_t *table, const char *name, lba28_t new);
void childtable_edit_name(childtable_t *table, const char *old, const char *new);
lba28_t childtable_get(childtable_t *table, const char *name);
void *childtable_find(childtable_t *table, const char *name);

void inode_child(inode_t *node, const char *name, inode_t *buffer);
void inode_child_set(inode_t *node, child_operation op, inode_t *parent);
void inode_fetch(lba28_t index, inode_t *node);
inode_t *inode_parent(inode_t *parent);
void inode_calculate_operation_bounds(inode_t *node, operation_bounds *operation);
void inode_realloc(inode_t *node, uint32_t sectors, inode_t *parent);
inode_t *inode_new(pathbuf_t *pathbuf);
uint32_t inode_read(inode_t *node, uint32_t from, char *buffer, uint32_t count);
uint32_t inode_readdir(inode_t *node, uint32_t from, char *buffer);
void inode_write(inode_t *node, uint32_t from, const char *buffer, uint32_t count, inode_t *parent);
void inode_truncate(inode_t *node);
void inode_delete(inode_t *node, inode_t *parent);
void inode_create(inode_type type, inode_t *parent, const char *name, inode_t *node, inode_t *gparent);
void inode_update(inode_t *node);

inode_t *fs_node_parent(inode_t *node);
inode_t *fs_node_child(inode_t *node, const char *name);
inode_t *fs_node_root();
int8_t fs_node_open(pathbuf_t *pathbuf, inode_t **node, inode_t **parent, inode_t **gparent);

inode_t *inodelist_get(pathbuf_t *pathbuf);
void inodelist_remove(inode_t *node);
void inodelist_add(inode_t *node);

inode_t *fs_open(pathbuf_t *pathbuf, uint8_t create, uint8_t truncate);
void fs_close(inode_t *node);
void fs_write(inode_t *node, const char *str, uint32_t from, uint32_t len);
void fs_read(inode_t *node, char *str, uint32_t from, uint32_t len);
void fs_unlink(inode_t *node);
void fs_mkdir(pathbuf_t *pathbuf);
inode_t *fs_opendir(pathbuf_t *pathbuf);
uint32_t fs_readdir(inode_t *node, char *buffer, uint32_t from);
int32_t fs_rmdir(pathbuf_t *pathbuf);
void fs_init();

#endif