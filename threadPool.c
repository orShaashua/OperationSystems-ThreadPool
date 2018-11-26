/*
 * Name: Or Shaashua
 * ID:311148811
 */
#include "threadPool.h"


void execute_thread(ThreadPool* tp);
static void* start_thread(void* arg);
void sendError();

ThreadPool* tpCreate(int numOfThreads){
    int i;
    ThreadPool* thread_pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    thread_pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * numOfThreads);
    if (thread_pool == NULL || thread_pool->threads == NULL){ sendError(); }
    thread_pool->tasks_queue = osCreateQueue();
    thread_pool->pool_size = numOfThreads;
    thread_pool->shouldWaitForTasks = true ;
    if (pthread_mutex_init(&thread_pool->mutex, NULL)){ sendError(); }
    if(pthread_cond_init(&thread_pool->cond , NULL)){ sendError(); }

    for (i = 0; i< numOfThreads ; i++){
        pthread_t tid;
        int ret = pthread_create(&tid, NULL, start_thread, thread_pool);
        if (ret != 0) { sendError(); }
        thread_pool->threads[i] = tid;
    }
    return thread_pool;
}

static void* start_thread(void* arg)
{
    ThreadPool* tp = (ThreadPool*) arg;
    execute_thread(tp);
    return NULL;
}

void execute_thread(ThreadPool* tp) {

    Task* task = NULL;
    while (tp->shouldWaitForTasks) {

        if(pthread_mutex_lock(&tp->mutex)){ sendError(); }
        if (osIsQueueEmpty(tp->tasks_queue)) {
            if(pthread_cond_wait(&tp->cond, &tp->mutex)){sendError();}; //instead of busy waiting!
            if (!tp->shouldWaitForTasks) {
                if(pthread_mutex_unlock(&tp->mutex)){ sendError(); }
                return;
            }
        }
        task = (Task*) osDequeue (tp->tasks_queue);
        if(pthread_mutex_unlock(&tp->mutex)){sendError();}
        (*task->fn_ptr)(task->arg);
        free(task);
    }
}

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks){
    int i;
    bool exitLoop = true;
    //We have to wait until the end of Tasks are already running.
    if(shouldWaitForTasks == 0) {
        threadPool->shouldWaitForTasks = false;
    }
    //We have to wait until all the tasks are over (even those who are already running
    //and those that are in the queue while reading the function. You can not enter new tasks
    //after reading the function.
    else {
        while (exitLoop) {
            if (osIsQueueEmpty(threadPool->tasks_queue)) {
                threadPool->shouldWaitForTasks = false;
                exitLoop = false;
            }
        }
    }
    if(pthread_cond_broadcast(&threadPool->cond)){sendError();}
    while(!(osIsQueueEmpty(threadPool->tasks_queue))){
        free(osDequeue (threadPool->tasks_queue));
    }
    for (i = 0; i < threadPool->pool_size; i++) {
        pthread_join(threadPool->threads[i], NULL);
    }
    free(threadPool->threads);
    osDestroyQueue(threadPool->tasks_queue);
    pthread_cond_destroy(&threadPool->cond);
    pthread_mutex_destroy(&threadPool->mutex);
    free(threadPool);
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    if(threadPool->shouldWaitForTasks) {
        Task *task = (Task *) malloc(sizeof(Task));
        if (task == NULL){ sendError();}
        task->fn_ptr = computeFunc;
        task->arg = param;
        if(osIsQueueEmpty(threadPool->tasks_queue)){
            if (pthread_cond_broadcast(&threadPool->cond)){sendError();}
        }
        osEnqueue(threadPool->tasks_queue, task);
        return SUCCESS;
    }
    return FAIL;
}

void sendError(){
    write(FILE_DESCRIPTORS_STDERR, ERROR, strlen(ERROR));
    exit(FAIL);
}