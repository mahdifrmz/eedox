#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>

// typedef struct
// {
//     uint32_t ptr;
//     const char *name;
//     const char *address;
// } fnrec;

void trace(uint32_t eip, uint32_t ebp);

#endif