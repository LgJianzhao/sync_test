/*
 * spinlock.cpp
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

#include "helper.h"

#if defined (__APPLE__)
typedef int pthread_spinlock_t;
#endif

/* 全局变量 */
pthread_spinlock_t spinLock;
int64 targetPerThread = 32 * 1000 * 1000;
int64 targetShared = 0;
int criticalSectionType = 0;

void BigCriticalSection() {
    int j = 0;
    while (j++ < 10000) {
        int a = j;
        a *= 2;
    }
}

/* 线程函数 */
void* ThreadFunc(void *pArg) {
    int i = 0; // 每个线程累计加一定次数的锁
    while (i++ < targetPerThread) {
#if !defined (__APPLE__)
		pthread_spin_lock(&spinLock);
#endif
        targetShared++;
        if (criticalSectionType) {
            BigCriticalSection();
        }
#if !defined (__APPLE__)
        pthread_spin_unlock(&spinLock);
#endif
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: ./mutex [thread size] \n");
        return -1;
    }

    int size = atoi(argv[1]);
    if (argc > 2) {
        criticalSectionType = atoi(argv[2]);
    }

    targetPerThread = targetPerThread / size;

    pthread_t *threadId = new pthread_t[size];

#if !defined (__APPLE__)
    pthread_spin_init(&spinLock, 0);
#endif

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

    long double tps = targetShared * 1.00 / elapsedTs * 1000 * 1000;
    printf("%-10lld %-15lld %-8.2Lf\n", targetShared, elapsedTs / 1000, tps);

#if !defined (__APPLE__)
    pthread_spin_destroy(&spinLock);
#endif

    return 0;
}
