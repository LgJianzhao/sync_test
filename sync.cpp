/*
 * atomic.cpp
 *
 *  Created on: 2021年4月19日
 *      Author: jianzhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <atomic>

#include "helper.h"

/* 全局变量 */
int64 targetPerThread = 10 * 1000 * 1000;
int64 targetShared = 0;

/* 线程函数 */
void* ThreadFunc(void *pArg) {
    int i = 0;
    while (i++ < targetPerThread) {
        __sync_fetch_and_add(&targetShared, 1);
        // targetShared++;
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: ./mutex [thread size] \n");
        return -1;
    }

    int size = atoi(argv[1]);

    pthread_t *threadId = new pthread_t[size];
    int64 startTs = STime_GetMicrosecondsTime();

    for (int i = 0; i < size; i++) {
        if (pthread_create(&threadId[i], NULL, ThreadFunc, NULL)) {
            perror("pthread_create");
            break;
        }
    }

    for (int i = 0; i < size; i++) {
        pthread_join(threadId[i], NULL);
    }

    int64 elapsedTs = STime_GetMicrosecondsTime() - startTs;

    if (debug > 0) {
        printf("\n"
                "Test       Elapsed Time    TPS     \n"
                "count      ms.             per/s   \n");
    }

    long double tps = targetPerThread * size * 1.00 / elapsedTs * 1000 * 1000;
    printf("%-10lld %-15lld %-8.2Lf\n", targetPerThread, elapsedTs / 1000, tps);

    printf("targetShared : %lld\n", targetShared);

    return 0;
}
