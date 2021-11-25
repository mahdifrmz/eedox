#ifndef KUTIL_H
#define KUTIL_H

#include <terminal.h>
#include <kheap.h>
#include <util.h>

void kpanic(const char *message, ...);

void *kmalloc(uint32_t size);
void *kmalloc_a(uint32_t size);
void kfree(void *ptr);

int32_t kprintf(const char *message, ...);

#endif