#include <fs.h>
#include <kutil.h>

const lba28_t fsstart = 1;
lba28_t balloc_ptr;
vec_t inodelist;
krwlock balloc_lock;

#define MAX_NODE_NAME_LENGTH 256
#define BALLOC_SECTOR 0

char _balloc_ptr_temp[SECTOR_SIZE];

void binit()
{

    balloc_fetch();
    if (balloc_ptr == 0)
    {
        balloc_ptr = fsstart;
        balloc_update();
    }
}

void balloc_update()
{
    krwlock_write(&balloc_lock);
    *(uint32_t *)_balloc_ptr_temp = balloc_ptr;
    ata_write(BALLOC_SECTOR, _balloc_ptr_temp);
    krwlock_release(&balloc_lock);
}
void balloc_fetch()
{
    krwlock_read(&balloc_lock);
    ata_read(BALLOC_SECTOR, _balloc_ptr_temp);
    balloc_ptr = *(uint32_t *)_balloc_ptr_temp;
    krwlock_release(&balloc_lock);
}

lba28_t balloc(lba28_t size)
{
    lba28_t ptr = balloc_ptr;
    balloc_ptr += size;
    balloc_update();
    return ptr;
}

void bfree(__attribute__((unused)) lba28_t address)
{
}

lba28_t brealloc(lba28_t address, lba28_t size)
{
    bfree(address);
    return balloc(size);
}

void inode_create(inode_type type, inode_t *parent, const char *name, inode_t *node, inode_t *gparent)
{
    if (strlen(name) > MAX_NODE_NAME_LENGTH)
    {
        kpanic("name %s is longer than %u characters and thus not suitable for a file name", name, MAX_NODE_NAME_LENGTH);
    }

    node->type = type;
    node->alloc = 0;
    node->size = 0;
    node->child_count = 0;
    node->index = balloc(1);
    node->parent = parent->index;
    node->isvalid = 1;

    inode_update(node);
    child_operation op;
    op.op = CHOP_ADD;
    op.name1 = name;
    op.index = node->index;
    inode_child_set(parent, op, gparent);
}
void inode_delete(inode_t *node, inode_t *parent)
{
    node->isvalid = 0;
    ata_write(node->index, node);
    child_operation op;
    op.op = CHOP_REM;
    op.name1 = (const char *)node->_pathbuf.fields.buffer[node->_pathbuf.fields.size];
    inode_child_set(parent, op, NULL);
}
void inode_write(inode_t *node, uint32_t from, const char *buffer, uint32_t count, inode_t *parent)
{
    operation_bounds op;
    op.bytes_from = from;
    op.bytes_count = count;
    inode_calculate_operation_bounds(node, &op);
    if (op.sec_overflow)
    {
        inode_realloc(node, op.sec_overflow + node->alloc, parent);
    }
    if (op.bytes_overflow)
    {
        node->size += op.bytes_overflow;
    }
    char *blocks = kmalloc(MAX_NODE_NAME_LENGTH * op.sec_count);
    uint32_t blocks_count = op.sec_count - op.sec_overflow;
    for (lba28_t i = 0; i < blocks_count; i++)
    {
        ata_read(node->index + 1 + op.sec_from + i, blocks + i * SECTOR_SIZE);
    }
    memcpy(blocks + from, buffer, count);
    for (lba28_t i = 0; i < blocks_count; i++)
    {
        ata_write(node->index + 1 + op.sec_from + i, blocks + i * SECTOR_SIZE);
    }
    kfree(blocks);
}
void inode_truncate(inode_t *node)
{
    node->alloc = 0;
    node->size = 0;
    inode_update(node);
}
uint32_t inode_read(inode_t *node, uint32_t from, char *buffer, uint32_t count)
{
    operation_bounds op;
    op.bytes_from = from;
    op.bytes_count = count;
    inode_calculate_operation_bounds(node, &op);

    uint32_t blocks_count = op.sec_count - op.sec_overflow;
    char *blocks = kmalloc(SECTOR_SIZE * blocks_count);
    for (lba28_t i = 0; i < blocks_count; i++)
    {
        ata_read(node->index + 1 + op.sec_from + i, blocks + i * SECTOR_SIZE);
    }
    memcpy(buffer, blocks, count - op.bytes_overflow);
    buffer += (count - op.bytes_overflow);
    kfree(blocks);
    return count - op.bytes_overflow;
}
uint32_t inode_readdir(inode_t *node, uint32_t from, char *buffer)
{
    if (from == node->size)
    {
        return 0;
    }
    char *dir = kmalloc(node->size - from);
    inode_read(node, from, dir, node->size - from);
    strcpy(buffer, dir);
    uint32_t len = strlen(dir) + 5;
    kfree(dir);
    return len;
}

void inode_child(inode_t *node, const char *name, inode_t *buffer)
{
    childtable_t table;
    table.ptr = kmalloc(node->alloc * SECTOR_SIZE);
    table.sectors = node->alloc;
    table.size = node->size;
    inode_read(node, 0, (char *)table.ptr, node->size);
    lba28_t child_index = childtable_get(&table, name);
    if (child_index != 0)
    {
        inode_fetch(child_index, buffer);
    }
    else
    {
        buffer->isvalid = 0;
    }
    kfree(table.ptr);
}
void inode_child_set(inode_t *node, child_operation op, inode_t *parent)
{
    childtable_t table;
    table.ptr = kmalloc(node->alloc * SECTOR_SIZE);
    table.size = node->size;
    table.sectors = node->alloc;

    inode_read(node, 0, (char *)table.ptr, node->size);

    if (op.op == CHOP_ADD)
    {
        childtable_add(&table, op.name1, op.index);
    }
    else if (op.op == CHOP_REM)
    {
        childtable_remove(&table, op.name1);
    }
    else if (op.op == CHOP_RENAME)
    {
        childtable_edit_name(&table, op.name1, op.name2);
    }
    else
    {
        childtable_remove(&table, op.name1);
    }

    node->alloc = table.sectors;
    node->size = table.size;

    inode_write(node, 0, (char *)table.ptr, node->size, parent);
    inode_update(node);
    kfree(table.ptr);
}
void inode_calculate_operation_bounds(inode_t *node, operation_bounds *operation)
{
    uint32_t bytes_to = operation->bytes_from + operation->bytes_count;
    operation->sec_from = node->index + operation->bytes_from / SECTOR_SIZE;
    lba28_t sec_to = node->index + ((bytes_to - 1) / SECTOR_SIZE) + 1;
    operation->sec_count = sec_to - operation->sec_from;

    if (sec_to > node->index + node->alloc)
    {
        operation->sec_overflow = sec_to - (node->index + node->alloc);
    }
    else
    {
        operation->sec_overflow = 0;
    }

    if (bytes_to > node->size)
    {
        operation->bytes_overflow = bytes_to - node->size;
    }
    else
    {
        operation->bytes_overflow = 0;
    }
}
void inode_realloc(inode_t *node, uint32_t sectors, inode_t *parent)
{
    lba28_t new_index = brealloc(node->index, sectors);
    child_operation op;
    op.op = CHOP_RELOC;
    op.name1 = (char *)pathbuf_name(&node->_pathbuf);
    op.index = new_index;
    inode_child_set(parent, op, NULL);
    node->index = new_index;
}

void inode_fetch(_unused lba28_t index, _unused inode_t *node)
{
    inode_t *buffer = kmalloc(SECTOR_SIZE);
    ata_read(index, buffer);
    node->isvalid = buffer->isvalid;
    node->alloc = buffer->alloc;
    node->size = buffer->size;
    node->child_count = buffer->child_count;
    node->parent = buffer->parent;
    node->type = buffer->type;
    node->index = buffer->index;
    kfree(buffer);
}
void inode_update(inode_t *node)
{
    ata_write(node->index, node);
}
inode_t *inode_new(pathbuf_t *pathbuf)
{
    inode_t *node = kmalloc(SECTOR_SIZE);
    node->_refs = 1;
    node->_pathbuf = pathbuf_copy(pathbuf);
    inodelist_add(node);
    krwlock_init(&node->_lock);
    return node;
}

inode_t *inodelist_get(pathbuf_t *pathbuf)
{
    for (uint32_t i = 0; i < inodelist.size; i++)
    {
        inode_t *node = (inode_t *)inodelist.buffer[i];
        if (pathbuf_cmp(&node->_pathbuf, pathbuf))
        {
            return node;
        }
    }
    return (inode_t *)inodelist.buffer[0];
}

void inodelist_add(inode_t *node)
{
    vec_push(&inodelist, (uint32_t)node);
}

void inodelist_remove(inode_t *node)
{
    for (uint32_t i = 0; i < inodelist.size; i++)
    {
        if ((inode_t *)inodelist.buffer[i] == node)
        {
            vec_erase(&inodelist, i, 1);
        }
    }
}

void *childtable_find(childtable_t *table, const char *name)
{
    char *ptr = table->ptr;
    while (strcmp(ptr, name) != 0 && ptr != table->ptr + table->size)
    {
        ptr += strlen(ptr) + 1 + sizeof(lba28_t);
    }
    if (ptr == table->ptr + table->size)
    {
        return NULL;
    }
    else
    {
        return ptr;
    }
}
void childtable_add(childtable_t *table, const char *name, lba28_t index)
{
    uint32_t name_len = strlen(name);
    uint32_t new_size = table->size + sizeof(lba28_t) + name_len + 1;
    if (new_size > table->sectors * SECTOR_SIZE)
    {
        table->sectors++;
        void *buffer = kmalloc(table->sectors * SECTOR_SIZE);
        memcpy(buffer, table->ptr, table->size);
        kfree(table->ptr);
        table->ptr = buffer;
    }
    char *end = table->ptr + table->size;
    strcpy(end, name);
    end[name_len] = 0;
    *(uint32_t *)(end + name_len + 1) = index;
    table->size = new_size;
}
void childtable_remove(childtable_t *table, const char *name)
{
    char *rec = childtable_find(table, name);
    if (rec == NULL)
    {
        return;
    }
    uint32_t name_len = strlen(name);
    char *next_rec = rec + name_len + 1 + sizeof(lba28_t);
    memcpy(rec, next_rec, (char *)table->ptr + table->size - next_rec);
    uint32_t new_size = table->size - name_len - 1 - sizeof(lba28_t);
    if (new_size <= (table->sectors - 1) * SECTOR_SIZE)
    {
        table->sectors--;
    }
    table->size = new_size;
}
void childtable_edit_index(childtable_t *table, const char *name, lba28_t new)
{
    char *rec = childtable_find(table, name);
    if (rec == NULL)
    {
        return;
    }
    lba28_t *index = (lba28_t *)(rec + strlen(name) + 1);
    *index = new;
}
void childtable_edit_name(childtable_t *table, const char *old, const char *new)
{
    lba28_t index = childtable_get(table, old);
    if (index == 0)
    {
        return;
    }
    childtable_remove(table, old);
    childtable_add(table, new, index);
}
lba28_t childtable_get(childtable_t *table, const char *name)
{
    char *rec = childtable_find(table, name);
    if (rec == NULL)
    {
        return 0;
    }
    lba28_t *index = (lba28_t *)(rec + strlen(name) + 1);
    return *index;
}

void fs_close(inode_t *node)
{
    if (--node->_refs)
    {
        inodelist_remove(node);
        pathbuf_free(&node->_pathbuf);
        kfree(node);
    }
}
inode_t *fs_node_parent(inode_t *node)
{
    pathbuf_t pathbuf = pathbuf_copy(&node->_pathbuf);
    vec_pop(&pathbuf.fields);
    inode_t *parent = inodelist_get(&pathbuf);
    if (pathbuf_cmp(&node->_pathbuf, &pathbuf))
    {
        return parent;
    }
    parent = inode_new(&pathbuf);
    krwlock_write(&parent->_lock);
    krwlock_read(&node->_lock);
    lba28_t address = node->parent;
    inode_fetch(address, parent);
    krwlock_release(&parent->_lock);
    krwlock_release(&node->_lock);
    return parent;
}

inode_t *fs_node_child(inode_t *node, const char *name)
{
    pathbuf_t pathbuf = pathbuf_copy(&node->_pathbuf);
    vec_push(&pathbuf.fields, (uint32_t)name);
    krwlock_read(&node->_lock);
    inode_t *child = inodelist_get(&pathbuf);
    if (pathbuf_cmp(&child->_pathbuf, &pathbuf))
    {
        return child;
    }
    krwlock_read(&node->_lock);
    child = inodelist_get(&pathbuf);
    if (pathbuf_cmp(&child->_pathbuf, &pathbuf))
    {
        return child;
    }
    child = inode_new(&pathbuf);
    krwlock_write(&node->_lock);
    inode_child(node, name, child);
    krwlock_release(&node->_lock);
    krwlock_release(&child->_lock);
    return child;
}

inode_t *fs_node_root()
{
    inode_t *node = (inode_t *)inodelist.buffer[0];
    node->_refs++;
    return node;
}

int8_t fs_node_open(pathbuf_t *pathbuf, inode_t **node, inode_t **parent, inode_t **gparent)
{
    inode_t *ptr = fs_node_root();
    vec_t loaded = vec_new();
    int8_t res = 0;

    while (pathbuf_cmp(&ptr->_pathbuf, pathbuf))
    {
        krwlock_read(&ptr->_lock);
        vec_push(&loaded, (uint32_t)ptr);
        if (!ptr->isvalid)
        {
            res = -1; // path not existing
        }
        if (ptr->type == inode_type_file)
        {
            res = -2; // was not dir
        }
        krwlock_release(&ptr->_lock);
        if (res != 0)
        {
            break;
        }
        ptr = fs_node_child(ptr, (char *)pathbuf_field(pathbuf, pathbuf_len(&ptr->_pathbuf)));
    }
    if (res == 0)
    {
        *node = ptr;
        if (loaded.size)
        {
            *parent = (inode_t *)vec_pop(&loaded);
        }
        if (loaded.size)
        {
            *gparent = (inode_t *)vec_pop(&loaded);
        }
    }
    for (uint32_t i = 0; i < loaded.size; i++)
    {
        fs_close((inode_t *)loaded.buffer[i]);
    }
    vec_free(&loaded);
    return res;
}

inode_t *fs_open(pathbuf_t *pathbuf, uint8_t create, uint8_t truncate)
{
    inode_t *node;
    inode_t *parent;
    inode_t *gparent;
    fs_node_open(pathbuf, &node, &parent, &gparent);

    krwlock_read(&node->_lock);
    uint8_t is_valid = node->isvalid;
    inode_type type = node->type;
    krwlock_release(&node->_lock);

    if (is_valid && type == inode_type_dir)
    {
        return NULL;
    }
    if (!is_valid)
    {
        if (create)
        {
            krwlock_write(&gparent->_lock);
            krwlock_write(&parent->_lock);
            krwlock_write(&node->_lock);
            if (!node->isvalid)
            {
                const char *name = (char *)pathbuf_name(&node->_pathbuf);
                inode_create(inode_type_file, parent, name, node, gparent);
            }
            krwlock_release(&gparent->_lock);
            krwlock_release(&parent->_lock);
            krwlock_release(&node->_lock);
        }
        else
        {
            return NULL;
        }
    }
    else if (truncate)
    {
        krwlock_write(&node->_lock);
        inode_truncate(node);
        krwlock_release(&node->_lock);
    }
    return node;
}

void fs_write(inode_t *node, const char *str, uint32_t from, uint32_t len)
{
    inode_t *parent = fs_node_parent(node);

    krwlock_write(&parent->_lock);
    krwlock_write(&node->_lock);
    inode_write(node, from, str, len, parent);
    krwlock_release(&parent->_lock);
    krwlock_release(&node->_lock);
    fs_close(parent);
}

void fs_read(inode_t *node, char *str, uint32_t from, uint32_t len)
{
    krwlock_read(&node->_lock);
    inode_read(node, from, str, len);
    krwlock_release(&node->_lock);
}

void fs_unlink(inode_t *node)
{
    inode_t *parent = fs_node_parent(node);

    krwlock_write(&parent->_lock);
    krwlock_write(&node->_lock);
    if (node->isvalid)
    {
        inode_delete(node, parent);
    }
    krwlock_release(&parent->_lock);
    krwlock_release(&node->_lock);
    fs_close(parent);
}

void fs_mkdir(pathbuf_t *pathbuf)
{
    inode_t *node;
    inode_t *parent;
    inode_t *gparent;
    fs_node_open(pathbuf, &node, &parent, &gparent);

    krwlock_read(&node->_lock);
    uint8_t is_valid = node->isvalid;
    inode_type type = node->type;
    krwlock_release(&node->_lock);

    if (is_valid || type != inode_type_dir)
    {
        return; // err
    }

    krwlock_write(&gparent->_lock);
    krwlock_write(&parent->_lock);
    krwlock_write(&node->_lock);
    if (node->isvalid)
    {
        return; // err
    }
    const char *name = (char *)pathbuf_name(&node->_pathbuf);
    inode_create(inode_type_dir, parent, name, node, gparent);
    fs_close(node);
    fs_close(parent);
    fs_close(gparent);
}

inode_t *fs_opendir(pathbuf_t *pathbuf)
{
    inode_t *node;
    inode_t *parent;
    inode_t *gparent;
    fs_node_open(pathbuf, &node, &parent, &gparent);

    krwlock_read(&node->_lock);
    uint8_t is_valid = node->isvalid;
    inode_type type = node->type;
    krwlock_release(&node->_lock);

    if (!is_valid || type != inode_type_dir)
    {
        return NULL;
    }
    fs_close(parent);
    fs_close(gparent);
    return node;
}

uint32_t fs_readdir(inode_t *node, char *buffer, uint32_t from)
{
    krwlock_read(&node->_lock);
    return inode_readdir(node, from, buffer);
    krwlock_release(&node->_lock);
}

int32_t fs_rmdir(pathbuf_t *pathbuf)
{
    inode_t *dir = fs_opendir(pathbuf);
    if (!dir || dir == fs_node_root() || dir->child_count != 0)
    {
        return -1;
    }
    inode_t *par = fs_node_parent(dir);
    krwlock_write(&par->_lock);
    krwlock_write(&dir->_lock);

    inode_delete(dir, par);

    krwlock_release(&par->_lock);
    krwlock_release(&dir->_lock);
    fs_close(dir);
    fs_close(par);
    return 0;
}

void fs_init()
{
    // the order of these calls should'nt be randomly changed
    krwlock_init(&balloc_lock);
    ata_init();
    binit();
    inodelist = vec_new();
    pathbuf_t root_path = pathbuf_root();
    inode_t *node_buffer = inode_new(&root_path);
    pathbuf_free(&root_path);
    inode_fetch(fsstart, node_buffer);
    if (!node_buffer->isvalid)
    {
        node_buffer->isvalid = 1;
        node_buffer->child_count = 0;
        node_buffer->index = fsstart;
        node_buffer->parent = node_buffer->index; // recursive parent
        node_buffer->size = 0;
        node_buffer->alloc = 0;
        node_buffer->type = inode_type_dir;
        inode_update(node_buffer);
    }
}