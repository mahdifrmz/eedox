#include <lock.h>

#define KRWLOCK_NONE 0
#define KRWLOCK_READ 1
#define KRWLOCK_WRITE 2

void ksemaphore_init(ksemaphore_t *sem, uint32_t initial)
{
    sem->value = initial;
    sem->sq = kqueue_new();
}
void ksemaphore_wait(ksemaphore_t *sem)
{
    if (sem->value-- <= 0)
    {
        kqueue_push(&sem->sq, (uint32_t)multsk_curtask());
        multsk_sleep();
    }
}
void ksemaphore_signal(ksemaphore_t *sem)
{
    sem->value++;
    if (sem->sq.size)
    {
        multsk_awake((task_t *)kqueue_pop(&sem->sq));
    }
}

void krwlock_read(krwlock *lock)
{
    if (lock->operation == KRWLOCK_WRITE ||
        (lock->operation == KRWLOCK_READ && lock->opq.size && kqueue_peek(&lock->opq) == KRWLOCK_WRITE))
    {
        kqueue_push(&lock->procq, (uint32_t)multsk_curtask());
        kqueue_push(&lock->opq, KRWLOCK_READ);
        multsk_sleep();
    }
    lock->readers++;
    lock->operation = KRWLOCK_READ;
    while (lock->procq.size > 0 && kqueue_peek(&lock->opq) == KRWLOCK_READ)
    {
        multsk_awake((task_t *)kqueue_pop(&lock->procq));
        kqueue_pop(&lock->opq);
    }
}
void krwlock_write(krwlock *lock)
{
    if (lock->operation != KRWLOCK_NONE)
    {
        kqueue_push(&lock->procq, (uint32_t)multsk_curtask());
        kqueue_push(&lock->opq, KRWLOCK_WRITE);
        multsk_sleep();
    }
    lock->operation = KRWLOCK_WRITE;
}
void krwlock_init(krwlock *lock)
{
    lock->procq = kqueue_new();
    lock->opq = kqueue_new();
    lock->operation = KRWLOCK_NONE;
    lock->readers = 0;
}
void krwlock_release(krwlock *lock)
{
    uint8_t flag = 0;
    if (lock->readers)
    {
        if (--lock->readers == 0)
        {
            flag = 1;
        }
    }
    else
    {
        flag = 1;
    }
    if (flag)
    {
        if (lock->procq.size)
        {
            multsk_awake((task_t *)kqueue_pop(&lock->procq));
            kqueue_pop(&lock->opq);
        }
        lock->operation = KRWLOCK_NONE;
    }
}