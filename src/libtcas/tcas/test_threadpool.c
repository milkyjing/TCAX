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
    printf("Destroy: %i\n", tp_destroy(tp, 1));
    return 0;
}

