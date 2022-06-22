#include <descriptor.h>
#include <kutil.h>
#include <pipe.h>
#include <fs.h>

fd_table fd_table_create(uint32_t inital_size)
{
    fd_table table;
    table.cap = inital_size;
    table.size = 0;
    table.records = kmalloc(table.cap * sizeof(fd_t));
    return table;
}

uint32_t fd_table_add(fd_table *table, fd_t fd)
{
    uint32_t index = 0;
    while (index < table->size && table->records[index].isopen)
        index++;
    if (index == table->size)
    {
        if (table->cap == table->size)
        {
            void *buffer = kmalloc((table->cap *= 2) * sizeof(fd_t));
            memcpy(buffer, table->records, table->size * sizeof(fd_t));
            kfree(table->records);
            table->records = buffer;
        }
        table->size++;
    }
    table->records[index] = fd;
    table->records[index].isopen = 1;
    return index;
}

uint32_t fd_table_dup(fd_table *table, uint32_t index)
{
    if(index >= table->size)
        return -1;
    return fd_table_add(table,fd_table_clone_entry(&table->records[index]));
}

void fd_table_rem(fd_table *table, uint32_t index)
{
    table->records[index].isopen = 0;
}

fd_table fd_table_clone(fd_table *table)
{
    fd_table new_table;
    new_table.cap = table->cap;
    new_table.size = table->size;
    new_table.records = kmalloc(new_table.cap * sizeof(fd_t));
    for(uint32_t i=0;i<table->size;i++)
    {
        if(table->records[i].isopen)
        {
            new_table.records[i] = fd_table_clone_entry(&table->records[i]);
        }
        else{
            new_table.records[i].isopen = 0;
        }
    }
    return new_table;
}

void fd_table_close(fd_table* table, uint32_t fd_id)
{
    if (fd_id >= table->size)
    {
        return;
    }
    fd_t *fd = &table->records[fd_id];
    if (!fd->isopen)
    {
        return;
    }
    fd_table_rem(table, fd_id);
    if (fd->kind == FD_KIND_DISK || fd->kind == FD_KIND_DIR)
    {
        fs_close(fd->ptr);
    }
    else if(fd->kind == FD_KIND_PIPE || fd->kind == FD_KIND_MQ)
    {
        if(fd->access == FD_ACCESS_READ)
        {
            if(pipe_close_rd(fd->ptr) && fd->kind != FD_KIND_MQ)
            {
                kfree(fd->ptr);
            }
        }
        else{
            if(pipe_close_wr(fd->ptr) && fd->kind != FD_KIND_MQ)
            {
                kfree(fd->ptr);
            }
        }
    }
}

fd_t fd_table_clone_entry(fd_t* fd)
{
    if(fd->kind == FD_KIND_PIPE || fd->kind == FD_KIND_MQ)
    {
        pipe_t* pipe = (pipe_t*)fd->ptr;
        (*(fd->access==FD_ACCESS_READ?&pipe->reader_count:&pipe->writer_count))++;
    }
    else if(fd->kind == FD_KIND_DISK || fd->kind == FD_KIND_DIR)
    {
        inode_t* node = (inode_t*)fd->ptr;
        if(node->_parent)
            node->_parent->_refs++;
        node->_refs++;
    }
    return *fd;
}