#ifndef MUTEX_H
#define MUTEX_H

#include <kqueue.h>
#include <task.h>

typedef struct
{
    kqueue_t sq;
    int32_t value;
} ksemaphore_t;

typedef struct
{
    kqueue_t procq;
    kqueue_t opq;
    uint32_t readers;
    int8_t operation;
} krwlock;

void ksemaphore_init(ksemaphore_t *sem, uint32_t initial);
void ksemaphore_wait(ksemaphore_t *sem);
void ksemaphore_signal(ksemaphore_t *sem);

void krwlock_read(krwlock *lock);
void krwlock_write(krwlock *lock);
void krwlock_init(krwlock *lock);
void krwlock_release(krwlock *lock);

#endif