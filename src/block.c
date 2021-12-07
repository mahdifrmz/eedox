#include <block.h>
#include <kutil.h>
#include <ide.h>

lba28_t fsstart = 1;
lba28_t balloc_ptr;

#define MAX_NODE_NAME_LENGTH 256

void binit()
{
    balloc_ptr = fsstart;
}

lba28_t balloc(lba28_t size)
{
    lba28_t ptr = balloc_ptr;
    balloc_ptr += size;
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

inode_t *inode_create(inode_type type, inode_t *parent, char *name)
{
    inode_t *node = kmalloc(SECTOR_SIZE);
    node->type = type;
    if (strlen(name) > MAX_NODE_NAME_LENGTH)
    {
        kpanic("name %s is longer than %u characters and thus not suitable for a file name", name, MAX_NODE_NAME_LENGTH);
    }
    memset(node->name, 0, MAX_NODE_NAME_LENGTH);
    strcpy(node->name, name);
    node->name[MAX_NODE_NAME_LENGTH] = 0;
    node->alloc = 0;
    node->size = 0;
    node->child_count = 0;
    node->index = balloc(1);
    node->parent = parent->index;
    node->isvalid = 1;
    // write
    inode_child_set(parent, 0, node->index);
    return node;
}
void inode_delete(inode_t *node, inode_t *parent)
{
    node->isvalid = 0;
    // write
    if (parent == NULL)
    {
        parent = inode_parent(node);
    }
    inode_child_set(parent, node->index, 0);
}
void inode_write(inode_t *node, uint32_t from, char *buffer, uint32_t count)
{
    operation_bounds op;
    op.bytes_from = from;
    op.bytes_count = count;
    inode_calculate_operation_bounds(node, &op);
    if (op.sec_overflow)
    {
        inode_realloc(node, op.sec_overflow + node->alloc);
    }
    if (op.bytes_overflow)
    {
        node->size += op.bytes_overflow;
    }
    char *blocks = kmalloc(MAX_NODE_NAME_LENGTH * op.sec_count);
    for (lba28_t i = 0; i < op.sec_count; i++)
    {
        // read
    }
    memcpy(blocks + from, buffer, count);
    for (lba28_t i = 0; i < op.sec_count; i++)
    {
        // write
    }
    kfree(blocks);
}
void inode_read(inode_t *node, uint32_t from, char *buffer, uint32_t count)
{
    operation_bounds op;
    op.bytes_from = from;
    op.bytes_count = count;
    inode_calculate_operation_bounds(node, &op);

    char *blocks = kmalloc(MAX_NODE_NAME_LENGTH * op.sec_count - op.sec_overflow);
    for (lba28_t i = 0; i < op.sec_count - op.sec_overflow; i++)
    {
        // read
    }
    memcpy(buffer, blocks, count - op.bytes_overflow);
    buffer += (count - op.bytes_overflow);
    for (uint32_t j = 0; j < op.bytes_overflow; j++)
    {
        buffer[j] = 0;
    }
    kfree(blocks);
}
inode_t *inode_children(inode_t *node)
{
    uint32_t *children_indexes = kmalloc(node->alloc * SECTOR_SIZE);
    inode_read(node, 0, (char *)children_indexes, node->size);
    inode_t *children = kmalloc(node->size / 4 * sizeof(inode_t));
    for (uint32_t i = 0; i < node->size; i++)
    {
        inode_fetch(children_indexes[i], &children + i);
    }
    return children;
}
void inode_child_set(inode_t *node, lba28_t rem, lba28_t add)
{
    uint32_t children_count = node->size / 4;
    uint32_t *children_indexes = kmalloc(node->alloc * SECTOR_SIZE);
    inode_read(node, 0, (char *)children_indexes, node->size);

    if (add && rem)
    {
        for (uint32_t i = 0; i < children_count; i++)
        {
            if (rem == children_indexes[i])
            {
                children_indexes[i] = add;
                break;
            }
        }
    }
    else if (add)
    {
        if ((--node->size) % (SECTOR_SIZE / 4) == 0)
        {
            node->alloc++;
            kfree(children_indexes);
            children_indexes = kmalloc(node->alloc * SECTOR_SIZE);
        }
        children_indexes[node->size] = add;
        node->size += 4;
    }
    else if (rem)
    {
        for (uint32_t i = 0; i < children_count; i++)
        {
            if (rem == children_indexes[i])
            {
                for (; i < children_count - 1; i++)
                {
                    children_indexes[i] = children_indexes[i + 1];
                }
                children_indexes[i - 1] = 0;
                break;
            }
        }
        if ((--node->size) % (SECTOR_SIZE / 4) == 0)
        {
            node->alloc--;
        }
        node->size -= 4;
    }

    inode_write(node, 0, (char *)children_indexes, node->size);
    kfree(children_indexes);
}
inode_t *inode_parent(inode_t *node)
{
    inode_t *parent = kmalloc(sizeof(inode_t));
    inode_fetch(node->parent, &parent);
    return parent;
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
void inode_realloc(inode_t *node, uint32_t sectors)
{
    inode_t *parent = inode_parent(node);
    lba28_t new_index = brealloc(node->index, sectors);
    inode_child_set(parent, node->index, new_index);
    node->index = new_index;
}

void inode_fetch(__attribute__((unused)) lba28_t index, __attribute__((unused)) inode_t **node)
{
    // read
}