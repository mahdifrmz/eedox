#ifndef UTIL_H
#define UTIL_H

#define PARTIAL_PRINT_BUFFER_SIZE 512

#include <stdint.h>
#include <stdarg.h>

void memcpy(void *dest, const void *src, uint32_t size);
void memset(void *dest, uint8_t val, uint32_t size);
void memshift(void *dest, int32_t offset, uint32_t size);

void tostr_u(char *buffer, uint32_t i);
void tostr_x(char *buffer, uint32_t h);
char dec_to_hex(uint8_t d);

uint32_t strlen(const char *str);
void strcpy(char *dest, const char *src);
int8_t strcmp(const char *str1, const char *str2);

int32_t partial_print(char *buffer, int32_t index, const char *message, va_list args);
int32_t sprintf(char *buffer, const char *message, ...);

#endif