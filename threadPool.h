#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include <memory.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#define ERROR "Error in system call\n"
#define FILE_DESCRIPTORS_STDERR 2
#define FAIL (-1)
#define SUCCESS 0

typedef enum boolean {false = 0, true = 1}bool;
typedef struct thread_pool
{
    int pool_size;
    OSQueue* tasks_queue;
    pthread_t* threads;
    int shouldWaitForTasks;
    pthread_cond_t cond;
    pthread_mutex_t mutex;

}ThreadPool;

typedef struct task
{
    void (*fn_ptr)(void*);
    void* arg;
}Task;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
