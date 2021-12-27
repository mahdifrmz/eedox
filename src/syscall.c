#include <syscall.h>
#include <terminal.h>
#include <kutil.h>
#include <kstring.h>
#include <lock.h>
#include <kb.h>
#include <fs.h>
#include <prog.h>

ksemaphore_t reader_lock;

int32_t syscall_translate_fs_err(int32_t err)
{
    switch (err)
    {
    case FS_ERR_DELETED:
        return SYSCALL_ERR_UNLINKED_FILE;
    case FS_ERR_DIR_HAS_CHILD:
        return SYSCALL_ERR_HAS_CHILD;
    case FS_ERR_NONEXISTING:
        return SYSCALL_ERR_NONEXISTING;
    case FS_ERR_INVALID_PATH:
        return SYSCALL_ERR_INVALID_PATH;
    default:
        return 0;
    }
}

void syscalls_handle(registers *regs)
{
    if (regs->eax == SYSCALL_EXIT)
    {
        regs->eax = syscall_exit();
    }
    if (regs->eax == SYSCALL_WRITE)
    {
        regs->eax = syscall_write(regs);
    }
    if (regs->eax == SYSCALL_READ)
    {
        regs->eax = syscall_read(regs);
    }
    else if (regs->eax == SYSCALL_FORK)
    {
        regs->eax = multsk_fork();
    }
    else if (regs->eax == SYSCALL_EXEC)
    {
        regs->eax = syscall_exec(regs);
    }
    else if (regs->eax == SYSCALL_CLOSE)
    {
        regs->eax = syscall_close(regs);
    }
    else if (regs->eax == SYSCALL_OPEN)
    {
        regs->eax = syscall_open(regs);
    }
    else if (regs->eax == 0xffffffff)
    {
        syscall_test();
    }
}

int32_t syscall_exit()
{
    multsk_terminate();
    return 0;
}

int32_t syscall_open(registers *regs)
{
    task_t *task = multsk_curtask();
    const char *path = (const char *)regs->ebx;
    pathbuf_t pathbuf = pathbuf_parse(path);
    uint32_t flags = regs->ecx;
    uint8_t flag_create = flags & 0x00000001;
    uint8_t flag_truncate = flags & 0x00000010;
    int8_t res;
    inode_t *node = fs_open(&pathbuf, flag_create, flag_truncate, 0, 0, &res);
    if (res != 0)
    {
        return syscall_translate_fs_err(res);
    }
    fd_t fd;
    fd.access = FD_ACCESS_WRITE | FD_ACCESS_READ;
    fd.pos = 0;
    fd.ptr = node;
    fd.kind = FD_KIND_DISK;
    fd.isopen = 1;
    fd_table_add(&task->table, fd);
    return 0;
}

int32_t syscall_close(registers *regs)
{
    task_t *task = multsk_curtask();
    uint32_t fd_id = regs->ebx;
    if (fd_id >= task->table.size)
    {
        return SYSCALL_ERR_INVALID_FD;
    }
    fd_t *fd = &task->table.records[fd_id];
    if (!fd->isopen)
    {
        return SYSCALL_ERR_INVALID_FD;
    }
    fd_table_rem(&task->table, fd_id);
    if (fd->kind == FD_KIND_DISK)
    {
        fs_close(fd->ptr);
    }
    return 0;
}

int32_t syscall_read_disk(fd_t *fd, char *ptr, int32_t len)
{
    inode_t *node = fd->ptr;
    int32_t ret = fs_read(node, ptr, fd->pos, len);
    if (ret == FS_ERR_DELETED)
    {
        return SYSCALL_ERR_UNLINKED_FILE;
    }
    fd->pos += ret;
    return ret;
}

int32_t syscall_read_stdin(char *ptr, int32_t len)
{
    ksemaphore_wait(&reader_lock);
    if (!input_list.size)
    {
        reader_task = multsk_curtask();
        multsk_sleep();
    }
    char *input = (char *)kqueue_peek(&input_list);
    uint32_t input_len = strlen(input);
    int32_t count = min(len, input_len);
    memcpy(ptr, input, count);
    if (len < (int32_t)input_len)
    {
        uint32_t extra_len = input_len - len;
        memcpy(input, input + len, extra_len);
        input[extra_len] = 0;
    }
    else
    {
        kqueue_pop(&input_list);
        kfree(input);
    }
    ksemaphore_signal(&reader_lock);
    return count;
}

int32_t syscall_read(registers *regs)
{
    int32_t len = regs->edx;
    char *ptr = (char *)regs->ecx;

    task_t *task = multsk_curtask();
    uint32_t fd_id = regs->ebx;
    if (fd_id >= task->table.size)
    {
        return SYSCALL_ERR_INVALID_FD;
    }
    fd_t *fd = &task->table.records[fd_id];
    if (!fd->isopen)
    {
        return SYSCALL_ERR_INVALID_FD;
    }
    if (!(fd->access | FD_ACCESS_READ))
    {
        return SYSCALL_ERR_WRITEONLY;
    }
    if (len <= 0)
    {
        return SYSCALL_ERR_INVALID_LENGTH;
    }
    if (fd->kind == FD_KIND_STDIN)
    {
        return syscall_read_stdin(ptr, len);
    }
    else
    {
        return syscall_read_disk(fd, ptr, len);
    }
}

int32_t syscall_write_disk(fd_t *fd, const char *ptr, int32_t len)
{
    inode_t *node = fd->ptr;
    int32_t ret = fs_write(node, ptr, fd->pos, len);
    if (ret == FS_ERR_DELETED)
    {
        return SYSCALL_ERR_UNLINKED_FILE;
    }
    fd->pos += ret;
    return ret;
}
int32_t syscall_write_stdout(const char *ptr, int32_t len)
{
    term_print_buffer(&glb_term, ptr, len);
    keyboard_input_size = 0;
    return len;
}

int32_t syscall_write(registers *regs)
{
    int32_t len = regs->edx;
    char *ptr = (char *)regs->ecx;

    task_t *task = multsk_curtask();
    uint32_t fd_id = regs->ebx;
    if (fd_id >= task->table.size)
    {
        return SYSCALL_ERR_INVALID_FD;
    }
    fd_t *fd = &task->table.records[fd_id];
    if (!fd->isopen)
    {
        return SYSCALL_ERR_INVALID_FD;
    }
    if (!(fd->access | FD_ACCESS_WRITE))
    {
        return SYSCALL_ERR_READONLY;
    }
    if (len <= 0)
    {
        return SYSCALL_ERR_INVALID_LENGTH;
    }
    if (fd->kind == FD_KIND_STDOUT)
    {
        return syscall_write_stdout(ptr, len);
    }
    else
    {
        return syscall_write_disk(fd, ptr, len);
    }
}

int32_t syscall_exec(registers *regs)
{
    pathbuf_t path = pathbuf_parse((char *)regs->ebx);
    int8_t rres;
    inode_t *binary = fs_open(&path, 0, 0, 0, 0, &rres);
    if (rres != 0)
    {
        return syscall_translate_fs_err(rres);
    }
    uint32_t entry;
    char *program = kmalloc(binary->size);
    int32_t rsl = fs_read(binary, program, 0, binary->size);
    if (rsl < 0)
    {
        return syscall_translate_fs_err(rres);
    }
    rsl = prog_load(program, kernel_memory_end, &entry);
    kfree(program);
    if (rsl != 0)
    {
        return SYSCALL_ERR_INVALID_LOAD_ADDRESS;
    }
    regs->eip = entry;
    regs->esp = user_stack_ptr + USER_STACK_SIZE;
    regs->ebp = regs->esp;
    return 0;
}

void syscalls_init()
{
    ksemaphore_init(&reader_lock, 1);
    load_int_handler(INTCODE_SYSCALL, syscalls_handle);
}