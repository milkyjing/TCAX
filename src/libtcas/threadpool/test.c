#include "threadpool.h"
#include <stdio.h>
#include <Windows.h>

void *pp(void *args) {
    int i;
    const char *str = (const char *)args;
    for (i = 0; i < 10; i++) {
        printf("%05i %s\n", i, str);
        Sleep(1000);
    }
    return (void *)0;
}

void *aw(void *args) {
    ThreadpoolPtr tp;
    ThreadpoolWorkPtr pwork;
    tp = (ThreadpoolPtr)((unsigned long *)args)[0];
    pwork = (ThreadpoolWorkPtr)((unsigned long *)args)[1];
    tp_assign_work(tp, pwork);
    return (void *)0;
}


int main() {
    pthread_t tid;
    char sz1[1024];
    char sz2[1024];
    unsigned long aa[2];
    ThreadpoolPtr tp;
    ThreadpoolWork work;
    tp = tp_create(NULL, 3);
    if (tp_initialize(tp) != 0) {
        tp_destroy(tp, 0);
        return 0;
    }
    work.execution_route = (tpw_func)pp;
    memcpy(sz1, "Nice to meet you!", 500);
    work.args = (tpw_args)sz1;
    tp_assign_work(tp, &work);
    memcpy(sz2, "Nice to meet you, too!", 500);
    work.args = (tpw_args)sz2;
    tp_assign_work(tp, &work);
    tp_assign_work(tp, &work);
    aa[0] = (unsigned long)tp;
    aa[1] = (unsigned long)&work;
    pthread_create(&tid, NULL, aw, (void *)aa);
    //tp_assign_work(tp, &work);
    tp_wait(tp);
    printf("Destroy: %i\n", tp_destroy(tp, 1));
    return 0;
}

