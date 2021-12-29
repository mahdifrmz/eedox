#ifndef SYSCALL_H
#define SYSCALL_H

#include <idt.h>
#include <multsk.h>

extern uint32_t keyboard_input_size;
extern uint32_t kernel_memory_end;

#define SYSCALL_EXIT 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_OPEN 4
#define SYSCALL_CLOSE 5
#define SYSCALL_EXEC 9
#define SYSCALL_FORK 10
#define SYSCALL_GETCWD 11
#define SYSCALL_SETCWD 12
#define SYSCALL_WAITPID 13
#define SYSCALL_WAIT 14

#define SYSCALL_ERR_INVALID_FD -1
#define SYSCALL_ERR_WRITEONLY -2
#define SYSCALL_ERR_READONLY -3
#define SYSCALL_ERR_INVALID_LENGTH -4
#define SYSCALL_ERR_UNLINKED_FILE -5
#define SYSCALL_ERR_INVALID_PATH -6
#define SYSCALL_ERR_NONEXISTING -7
#define SYSCALL_ERR_HAS_CHILD -8
#define SYSCALL_ERR_INVALID_LOAD_ADDRESS -9
#define SYSCALL_ERR_INVAL_CHILDPID -10

typedef int32_t (*syscall_handler_t)(registers *);

void syscall_test();
int32_t syscall_translate_fs_err(int32_t err);
void syscalls_handle(registers *regs);
int32_t syscall_open(registers *regs);
int32_t syscall_close(registers *regs);
int32_t syscall_read_disk(fd_t *fd, char *ptr, int32_t len);
int32_t syscall_read_stdin(char *ptr, int32_t len);
int32_t syscall_read(registers *regs);
int32_t syscall_write_disk(fd_t *fd, const char *ptr, int32_t len);
int32_t syscall_write_stdout(const char *ptr, int32_t len);
int32_t syscall_write(registers *regs);
int32_t syscall_getcwd(registers *regs);
int32_t syscall_setcwd(registers *regs);
int32_t syscall_exec(registers *regs);
int32_t syscall_fork(registers *regs);
int32_t syscall_exit(registers *regs);
int32_t syscall_wait(registers *regs);
int32_t syscall_waitpid(registers *regs);
void syscalls_init();

#endif