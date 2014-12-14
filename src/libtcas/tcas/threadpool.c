/*
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
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

#include "threadpool.h"


static int _tp_fetch_work(ThreadpoolPtr tp, ThreadpoolWorkPtr pwork) {
    sem_wait(&tp->work_sem);  /* wait for available work, if no works need to be dealt, it will be suspended */
    if (TP_STAT_DOWN == tp->tp_stat) return 0;  /* the threadpool is down, no work fetched */
    pthread_mutex_lock(&tp->sync_mutex);  /* we are going to modify a shared value, so we should lock it */
    memcpy(pwork, &tp->work_queue.buf[tp->work_queue.head_index], sizeof(ThreadpoolWork));  /* must be a copy of work in the work_queue, since work_queue may be changed right out of the lock */
    tp->work_queue.head_index = (++tp->work_queue.head_index) % tp->work_queue.size;  /* remove the fetched work */
    pthread_mutex_unlock(&tp->sync_mutex);  /* unlock the work_queue */
    return 1;
}

/* this function (may have many forks) will be executed right after the tp_initialize function, and stays idle (wait for available work) */
static void *_tp_thread_execution_route(void *args) {
    ThreadpoolPtr tp;
    ThreadpoolWork work;
    tp = (ThreadpoolPtr)args;
    while (_tp_fetch_work(tp, &work)) {  /* wait until a work comes, if no work comes, suspends */
        work.execution_route(work.args);  /* do the work */
        if (work.args_deepcopy)
            free(work.args);  /* free the memory occupied by args */
        pthread_mutex_lock(&tp->sync_mutex);
        if (!tp->shutdown)  /* if we are shutting down the threadpool, then we musn't wake up the thread to accept new works */
            sem_post(&tp->thread_sem);  /* if we are not shutting down the threadpool then, after finishing the work, we must restore the thread (worker) to idle */
        tp->incompleted--;
        if ((tp->completion || tp->shutdown) && 0 == tp->incompleted)
            pthread_cond_signal(&tp->completion_cond);
        pthread_mutex_unlock(&tp->sync_mutex);
    }
    return (void *)0;
}

ThreadpoolPtr tp_create(ThreadpoolPtr *ptp, int size) {
    ThreadpoolPtr tp;
    tp = (ThreadpoolPtr)malloc(sizeof(Threadpool));
    tp->threads = (size < 1) ? 1 : size;
    tp->ptids = (pthread_t *)malloc(tp->threads * sizeof(pthread_t));
    pthread_mutex_init(&tp->sync_mutex, NULL);
    sem_init(&tp->work_sem, 0, 0);  /* initially there is no work to be delt */
    sem_init(&tp->thread_sem, 0, tp->threads);  /* and all workers stay idle */
    tp->thread_entry = (tp_thread_init_func)_tp_thread_execution_route;  /* the same entrance (start point) for each worker */
    tp->work_queue.size = tp->threads;
    tp->work_queue.buf = (ThreadpoolWorkPtr)malloc(tp->work_queue.size * sizeof(ThreadpoolWork));
    tp->work_queue.head_index = 0;
    tp->work_queue.rear_index = 0;
    tp->incompleted = 0;
    tp->tp_stat = TP_STAT_INIT;
    tp->shutdown = 0;
    tp->completion = 0;
    pthread_cond_init(&tp->completion_cond, NULL);
    if (ptp)
        *ptp = tp;
    return tp;
}

int tp_initialize(ThreadpoolPtr tp) {
    int i, j;
    if (TP_STAT_INIT != tp->tp_stat) return -1;
    for (i = 0; i < tp->threads; i++) {
        if (pthread_create(&tp->ptids[i], NULL, tp->thread_entry, tp) != 0) {  /* now _tp_thread_execution_route function will have many forks */
            tp->tp_stat = TP_STAT_DOWN;  /* an error occurs */
            for (j = 0; j < i; j++)
                sem_post(&tp->work_sem);
            return -1;
        }
    }
    tp->tp_stat = TP_STAT_RUNNING;
    return 0;
}

int tp_assign_work(ThreadpoolPtr tp, const ThreadpoolWorkPtr pwork) {
    if (TP_STAT_RUNNING != tp->tp_stat) return -1;
    sem_wait(&tp->thread_sem);  /* wait for available thread (worker) of the thread pool */
    if (TP_STAT_DOWN == tp->tp_stat) return -1;  /* if the thread pool is down before the work being assigned to the thread pool */
    pthread_mutex_lock(&tp->sync_mutex);
    memcpy(&tp->work_queue.buf[tp->work_queue.rear_index], pwork, sizeof(ThreadpoolWork));
    tp->work_queue.rear_index = (++tp->work_queue.rear_index) % tp->work_queue.size;
    sem_post(&tp->work_sem);  /* inform some thread that a work has come */
    tp->incompleted++;
    pthread_mutex_unlock(&tp->sync_mutex);
    return 0;
}

void tp_wait(ThreadpoolPtr tp) {
    tp->completion = 1;
    pthread_mutex_lock(&tp->sync_mutex);
    if (tp->incompleted > 0)  /* if there are still some incompleted works in the thread pool */
        pthread_cond_wait(&tp->completion_cond, &tp->sync_mutex);
    tp->completion = 0;
    pthread_mutex_unlock(&tp->sync_mutex);
}

int tp_destroy(ThreadpoolPtr tp, int max_waiting_works) {
    int i;
    if (TP_STAT_DOWN == tp->tp_stat) return 0;
    else if (TP_STAT_RUNNING == tp->tp_stat) {
        tp->shutdown = 1;  /* shutting down the threadpool */
        pthread_mutex_lock(&tp->sync_mutex);
        if (tp->incompleted > 0)  /* if there are still some incompleted works in the thread pool */
            pthread_cond_wait(&tp->completion_cond, &tp->sync_mutex);
        tp->tp_stat = TP_STAT_DOWN;  /* only after we finish all the under working tasks can we set the status of the threadpool to DOWN */
        for (i = 0; i < max_waiting_works; i++)
            sem_post(&tp->thread_sem);  /* cancel the waiting works (note that now shutdown == true) */
        for (i = 0; i < tp->threads; i++)
            sem_post(&tp->work_sem);  /* wake up the idle threads */
        for (i = 0; i < tp->threads; i++)
            pthread_join(tp->ptids[i], NULL);  /* wait for completion of the works in the thread pool */
        pthread_mutex_unlock(&tp->sync_mutex);
    }
    pthread_mutex_destroy(&tp->sync_mutex);
    sem_destroy(&tp->work_sem);
    sem_destroy(&tp->thread_sem);
    pthread_cond_destroy(&tp->completion_cond);
    free(tp->ptids);
    free(tp->work_queue.buf);
    free(tp);
    return 0;
}

