#include <syscall.h>
#include <terminal.h>
#include <kutil.h>
#include <kstring.h>
#include <lock.h>
#include <kb.h>
#include <fs.h>
#include <prog.h>

#define syscall_handlers_cap 64

ksemaphore_t stdin_lock;
syscall_handler_t syscall_handlers[syscall_handlers_cap];

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
    syscall_handler_t handler = syscall_handlers[regs->eax];
    if (!handler)
    {
        kpanic("KERNEL : unknown syscall %u\n", regs->eax);
    }
    else
    {
        regs->eax = handler(regs);
    }
}

pathbuf_t resolve_path(pathbuf_t path)
{
    if(path.is_absolute)
    {
        return path;
    }
    else
    {
        task_t* curtask = task_curtask();
        pathbuf_t full = pathbuf_join(&curtask->cwd,&path);
        pathbuf_free(&path);
        return full;
    }
}

int32_t syscall_wait(registers *regs)
{
    int16_t *stref = (int16_t *)regs->ebx;

    task_t *task = task_curtask();
    task_t *child = task_find_zombie(task);
    if (!child)
    {
        task->wait = TASK_WAIT_ALL;
        task_sleep();
    }
    child = task->chwait;
    *stref = child->exit_status;
    uint32_t child_pid = child->pid;
    task_killtask(child);
    return child_pid;
}

int32_t syscall_waitpid(registers *regs)
{
    uint32_t child_pid = regs->ebx;
    int16_t *stref = (int16_t *)regs->ecx;

    task_t *task = task_curtask();
    task_t *child = task_gettask(child_pid);
    if (!child || task->parent != task)
    {
        return SYSCALL_ERR_INVAL_CHILDPID;
    }
    if (child->exit_status == -1)
    {
        task->wait = TASK_WAIT_PID;
        task->chwait = child;
        task_sleep();
    }
    *stref = child->exit_status;
    task_killtask(child);
    return child_pid;
}

int32_t syscall_getcwd(registers *regs)
{
    task_t *task = task_curtask();
    char *cwd = pathbuf_stringify(&task->cwd);
    char *buffer = (char *)regs->ebx;
    strcpy(buffer, cwd);
    buffer[strlen(cwd)] = 0;
    kfree(cwd);
    return 0;
}

int32_t syscall_getpid(_unused registers *regs)
{
    return multk_getpid();
}

int32_t syscall_setcwd(registers *regs)
{
    task_t *task = task_curtask();
    pathbuf_t cwd = pathbuf_parse((const char *)regs->ebx);
    cwd = resolve_path(cwd);
    int8_t res;
    inode_t *dir = fs_open(&cwd, 0, 0, 1, 0, &res);
    fs_close(dir);
    if (res != 0)
    {
        return syscall_translate_fs_err(res);
    }
    task->cwd = cwd;
    return 0;
}

int32_t syscall_exit(registers *regs)
{
    int16_t statuscode = regs->ebx;
    task_t *task = task_curtask();
    task_t *parent = task->parent;
    task->exit_status = statuscode;
    task_close_all_fds();
    if ((parent->wait == TASK_WAIT_PID && parent->chwait == task) || parent->wait == TASK_WAIT_ALL)
    {
        parent->chwait = task;
        parent->wait = TASK_WAIT_NONE;
        task_awake(parent);
    }
    task_sleep();
    return 0;
}

int32_t syscall_open(registers *regs)
{
    task_t *task = task_curtask();
    const char *path = (const char *)regs->ebx;
    pathbuf_t pathbuf = pathbuf_parse(path);
    pathbuf = resolve_path(pathbuf);
    uint8_t flags = regs->ecx;
    uint8_t flag_create = flags & 0x01;
    uint8_t flag_truncate = (flags & 0x02) >> 1;
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
    int32_t fd_index = (int32_t)fd_table_add(&task->table, fd);
    return fd_index;
}

int32_t syscall_mkdir(registers *regs)
{
    const char *path = (const char *)regs->ebx;
    pathbuf_t pathbuf = pathbuf_parse(path);
    pathbuf = resolve_path(pathbuf);
    int8_t res;
    inode_t *node = fs_open(&pathbuf, 1, 0, 1, 0, &res);
    fs_close(node);
    if (res != 0)
    {
        return syscall_translate_fs_err(res);
    }
    return 0;
}

int32_t syscall_opendir(registers *regs)
{
    task_t *task = task_curtask();
    const char *path = (const char *)regs->ebx;
    pathbuf_t pathbuf = pathbuf_parse(path);
    pathbuf = resolve_path(pathbuf);
    int8_t res;
    inode_t *node = fs_open(&pathbuf, 0, 0, 1, 0, &res);
    if (res != 0)
    {
        return syscall_translate_fs_err(res);
    }
    fd_t fd;
    fd.access = FD_ACCESS_READ;
    fd.pos = 0;
    fd.ptr = node;
    fd.kind = FD_KIND_DIR;
    fd.isopen = 1;
    int32_t fd_index = (int32_t)fd_table_add(&task->table, fd);
    return fd_index;
}

int32_t syscall_readdir(registers *regs)
{
    task_t *task = task_curtask();
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
    if (fd->kind != FD_KIND_DIR)
    {
        return SYSCALL_ERR_INVALID_FD;
    }
    inode_t *node = fd->ptr;
    int32_t ret = fs_readdir(node, (char*)regs->ecx, fd->pos);
    if (ret == FS_ERR_DELETED)
    {
        return SYSCALL_ERR_UNLINKED_FILE;
    }
    fd->pos += ret;
    return ret;
}

int32_t syscall_stat(registers *regs)
{
    const char *path = (const char *)regs->ebx;
    stat_t* stat = (stat_t*) regs->ecx;
    pathbuf_t pathbuf = pathbuf_parse(path);
    pathbuf = resolve_path(pathbuf);
    int8_t res;
    int32_t status = 0;
    inode_t* node = fs_open(&pathbuf,0,0,2,0,&res);
    if(res != 0)
    {
        status = syscall_translate_fs_err(res);
    }
    else{
        stat->index = node->index;
        stat->isdir = node->type == inode_type_dir;
        stat->size = node->size;
        stat->blocks = node->alloc + 1;
    }
    pathbuf_free(&pathbuf);
    return status;
}

int32_t syscall_close(registers *regs)
{
    task_t *task = task_curtask();
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
    task_close_fd(fd_id);
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
    ksemaphore_wait(&stdin_lock);
    if (!input_list.size)
    {
        reader_task = task_curtask();
        task_sleep();
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
    ksemaphore_signal(&stdin_lock);
    return count;
}

int32_t syscall_read(registers *regs)
{
    int32_t len = regs->edx;
    char *ptr = (char *)regs->ecx;

    task_t *task = task_curtask();
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

    task_t *task = task_curtask();
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
    if (!(fd->access & FD_ACCESS_WRITE))
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

void place_args_vector(const char** argv,uint32_t* stack)
{
    uint32_t esp = *stack;
    vec_t arg_list = vec_new();
    uint32_t args_size = 0;
    while(*argv)
    {
        args_size += (1 + strlen(*argv));
        vec_push(&arg_list,(uint32_t)*(argv++));
    }
    const char** ptrs = (const char**)(esp - args_size); 
    char* args = (char*) esp;
    uint32_t new_esp = esp - 8 - 4 * (vec_size(&arg_list) + 1) - args_size;
    uint32_t argc = vec_size(&arg_list);
    while(vec_size(&arg_list))
    {
        const char* arg = (const char*)vec_pop(&arg_list);
        uint32_t arglen = strlen(arg);
        args -= arglen + 1;
        strcpy(args,arg);
        args[arglen] = 0;
        *(--ptrs) = args;
    }

    new_esp -= new_esp % 64;
    *(uint32_t*)(new_esp + 4) = argc;
    *(const char***) (new_esp + 8) = ptrs;
    *stack = new_esp;
    vec_free(&arg_list);
}

inode_t* exec_open(pathbuf_t* path,int8_t* rres)
{
    task_t* task = task_curtask();
    pathbuf_t respath;
    pathbuf_t bindir = pathbuf_parse("/bin/");
    inode_t *binary;
    if(!path->is_absolute)
    {
        respath = pathbuf_join(&task->cwd,path);
        binary = fs_open(&respath, 0, 0, 0, 0, rres);
        if (*rres != 0)
        {
            pathbuf_free(&respath);
            respath = pathbuf_join(&bindir,path);
            binary = fs_open(&respath, 0, 0, 0, 0, rres);
            if(*rres != 0)
            {
                binary = NULL;
            }
        }
        pathbuf_free(&respath);
    }
    else{
        binary = fs_open(path, 0, 0, 0, 0, rres);
        if (*rres != 0)
        {
            binary = NULL;   
        }
    }
    pathbuf_free(&bindir);
    return binary;
}

int32_t syscall_exec(registers *regs)
{
    pathbuf_t path = pathbuf_parse((char *)regs->ebx);
    int8_t rres;
    inode_t* binary = exec_open(&path,&rres);
    if(rres != 0)
    {
        return syscall_translate_fs_err(rres);
    }
    char *program = kmalloc(binary->size);
    int32_t rsl = fs_read(binary, program, 0, binary->size);
    if (rsl < 0)
    {
        return syscall_translate_fs_err(rsl);
    }

    uint32_t entry;
    rsl = prog_load(program, kernel_memory_end, &entry);
    kfree(program);
    if (rsl != 0)
    {
        return SYSCALL_ERR_NOT_EXECUTABLE;
    }
    uint32_t stack_ptr = regs->esp + USER_STACK_SIZE - 0x40;
    place_args_vector((const char**)regs->ecx,&stack_ptr);
    regs->eip = entry;
    regs->useresp = stack_ptr;
    pathbuf_free(&path);
    return 0;
}

int32_t syscall_sbrk(registers *regs)
{
    return task_curtask()->brk += regs->ebx;
}

int32_t syscall_fork(_unused registers *regs)
{
    return task_fork();
}

void syscalls_init()
{
    ksemaphore_init(&stdin_lock, 1);
    memset(syscall_handlers, 0, syscall_handlers_cap * sizeof(syscall_handler_t));
    syscall_handlers[SYSCALL_CLOSE] = syscall_close;
    syscall_handlers[SYSCALL_OPEN] = syscall_open;
    syscall_handlers[SYSCALL_WRITE] = syscall_write;
    syscall_handlers[SYSCALL_READ] = syscall_read;
    syscall_handlers[SYSCALL_READDIR] = syscall_readdir;
    syscall_handlers[SYSCALL_OPENDIR] = syscall_opendir;
    syscall_handlers[SYSCALL_STAT] = syscall_stat;
    syscall_handlers[SYSCALL_EXEC] = syscall_exec;
    syscall_handlers[SYSCALL_FORK] = syscall_fork;
    syscall_handlers[SYSCALL_GETCWD] = syscall_getcwd;
    syscall_handlers[SYSCALL_SETCWD] = syscall_setcwd;
    syscall_handlers[SYSCALL_WAIT] = syscall_wait;
    syscall_handlers[SYSCALL_WAITPID] = syscall_waitpid;
    syscall_handlers[SYSCALL_EXIT] = syscall_exit;
    syscall_handlers[SYSCALL_GETPID] = syscall_getpid;
    syscall_handlers[SYSCALL_MKDIR] = syscall_mkdir;
    syscall_handlers[SYSCALL_SBRK] = syscall_sbrk;
    load_int_handler(INTCODE_SYSCALL, syscalls_handle);
}