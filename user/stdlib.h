#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <alloc.h>
#include <../src/util.h>

#define STDOUT 1
#define STDIN 0

typedef struct
{
    uint32_t index;
    uint8_t isdir;
    uint32_t size;
    uint32_t blocks;
} stat_t;

int write(int fd, const void *buffer, int length);
int read(int fd, const void *buffer, int length);
int open(const char *path, int flags);
int stat(const char *path,stat_t* stat);
int close(int fd);
int opendir(const char *path);
int readdir(int fd, char* buffer);
int getcwd(char *buffer);
int setcwd(const char *buffer);
int exit(short int statuscode);
int exec(const char *path,char* argv []);
int wait(short int *statuscode);
int mkdir(const char *path);
int wait_pid(int pid, short int *statuscode);
void* sbrk(int offset);
int getpid();
int fork();

void* malloc(int size);
void free(void* ptr);
void* realloc(void* ptr,int size);

int _printf(const char *message, va_list args);
int printf(const char *message, ...);

#endif