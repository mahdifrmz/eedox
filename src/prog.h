#ifndef PROG_H
#define PROG_H

#include <stdint.h>

int32_t prog_load(const char *file, uint32_t laddr, uint32_t *entry);

#endif