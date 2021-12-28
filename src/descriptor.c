#include <descriptor.h>
#include <kutil.h>

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
    memcpy(new_table.records, table->records, table->size * sizeof(fd_t));
    return new_table;
}