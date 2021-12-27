#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>

#define STDOUT 1
#define STDIN 0

int32_t write(int32_t fd, const void *buffer, int32_t length);
int32_t exit();

#endif