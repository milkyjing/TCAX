/*
 * Threadpool implementation using pthreads
 * Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * milkyjing
 *
 */

#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__
#pragma once

#include <stdlib.h>
#include <memory.h>
#include <semaphore.h>
#include <pthread.h>

#pragma comment(lib, "pthreadVC2.lib")


/* Threadpool status */
#define TP_STAT_INIT 0
#define TP_STAT_RUNNING 1
#define TP_STAT_DOWN 2

typedef void *(*tpw_func)(void *);
typedef void *tpw_args;

/**
 * ThreadpoolWork Structure
 * a work in the threadpool
 */
typedef struct _thread_pool_work {
    tpw_func execution_route;  /**< route function of the work */
    tpw_args args;             /**< parameters of the function */
    int args_deepcopy;         /**< indicates whether we should free args after we finishing the work or not */
} ThreadpoolWork, *ThreadpoolWorkPtr;

/**
 * ThreadpoolWorkQueue Structure
 * Work queue that holds works which are being dealt in the thread pool
 * queue of circular array implementation
 */
typedef struct _thread_pool_work_queue {
    ThreadpoolWorkPtr buf;   /**< pointer to the address of the queue (same as the address of the head element) */
    int size;                /**< size of the maximum works the queue can hold, should be equal to `threads' of the threadpool */
    int head_index;          /**< head of the queue */
    int rear_index;          /**< rear of the queue */
} ThreadpoolWorkQueue, *ThreadpoolWorkQueuePtr;

typedef tpw_func tp_thread_init_func;  /**< thread entrance function */

/**
 * Threadpool Structure
 * A threadpool hosts many threads, such threads are also named workers
 */
typedef struct _thread_pool {
    int threads;                           /**< number of threads in the thread pool, i.e. number of workers */
    pthread_t *ptids;                      /**< IDs of threads (workers) in the thread pool */
    pthread_mutex_t sync_mutex;            /**< mutex for synchronization */
    sem_t work_sem;                        /**< semaphore that indicates works which is under going */
    sem_t thread_sem;                      /**< semaphore that indicates available threads (idle workers) in the thread pool */
    tp_thread_init_func thread_entry;      /**< entry point for each thread, (the workers share the same door, so there is only one entry point) */
    ThreadpoolWorkQueue work_queue;        /**< work queue */
    int incompleted;                       /**< incompleted works in the thread pool */
    int tp_stat;                           /**< thread pool status, init, running, down */
    int shutdown;                          /**< shutdown = 1 indicates that the thread pool is about to shutting down */
    int completion;                        /**< set completion = 1 to check if the threadpool has completed all of the tasks assigned to it */
    pthread_cond_t completion_cond;        /**< condition variable for checking completion of the thread pool */
} Threadpool, *ThreadpoolPtr;

/* Inhibit C++ name-mangling for threadpool functions but not for system calls. */
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

/**
 * Create a new threadpool
 * @param ptp pointer to ThreadpoolPtr to hold the return value, can be NULL
 * @param size number of threads the threadpool maintains
 * @return ThreadpoolPtr
 */
extern ThreadpoolPtr tp_create(ThreadpoolPtr *ptp, int size);

/**
 * Initialize the threadpool
 * @param tp pointer to the Threadpool which is going to be initialized
 * @return int 0 on success
 */
extern int tp_initialize(ThreadpoolPtr tp);

/**
 * Assign a work to the threadpool
 * @param tp pointer to the Threadpool
 * @param pwork pointer to the work that is going to be assigned to the threadpool
 * @return int 0 on success
 */
extern int tp_assign_work(ThreadpoolPtr tp, const ThreadpoolWorkPtr pwork);

/**
 * Wait for completion of all tasks assigned to the threadpool
 * @param tp pointer to the Threadpool
 */
extern void tp_wait(ThreadpoolPtr tp);

/**
 * Destroy a threadpool and free the occupied resource
 * @param tp pointer to the Threadpool
 * @param max_waiting_works indicates the maximum possible works in the waiting queue, i.e., the works that haven't entered the threadpool
 * @return int 0 on success
 */
extern int tp_destroy(ThreadpoolPtr tp, int max_waiting_works);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif  /* __THREADPOOL_H__ */

