#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>

typedef struct
{
    uint32_t ptr;
    const char *name;
    const char *address;
} fnrec;

extern uint32_t symtable_count;
extern char symtable;

void trace(uint32_t eip, uint32_t ebp);
void trace_info();
void trace_init();

#endif