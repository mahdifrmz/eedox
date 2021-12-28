#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <../src/util.h>

#define STDOUT 1
#define STDIN 0

int32_t write(int32_t fd, const void *buffer, int32_t length);
int32_t getcwd(char *buffer);
int32_t setcwd(const char *buffer);
int32_t exit(int16_t statuscode);
int32_t exec(const char *path);
int32_t wait(int16_t *statuscode);
int32_t wait_pid(int32_t pid, int16_t *statuscode);
int32_t fork();

int32_t _printf(const char *message, va_list args);
int32_t printf(const char *message, ...);

#endif