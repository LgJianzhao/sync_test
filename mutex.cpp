/*
 * mutex.cpp
 *
 *  Created on: 2021年4月19日
 *      Author: jianzhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "helper.h"

/* 全局变量 */
pthread_mutex_t mutex, mutexTime;
int64 targetPerThread = 32 * 1000000;
int64 targetShared = 0;

/* 
 0: 模拟小临界区，eg: i++
 1: 模拟大临界区 eg: while(i++ < 1000)
 */
int criticalSectionType = 0;

/* 模拟耗时操作 */
void BigCriticalSection() {
    int j = 0;
    while (j++ < 10000) {
        int a = j;
        a *= 2;
    }
}

/* 线程函数 */
void* ThreadFunc(void *pArg) {
    int64 startTs = STime_GetMicrosecondsTime();
    int i = 0; // 每个线程累计加一定次数的锁
    while (i++ < targetPerThread) {
        pthread_mutex_lock(&mutex);
        targetShared++;
        if (criticalSectionType) {
            BigCriticalSection();
        }
        pthread_mutex_unlock(&mutex);
    }

    int64 perElap = STime_GetMicrosecondsTime() - startTs;

#if 1
    /* 记录每个线程耗时单独耗时 */
    pthread_mutex_lock(&mutexTime);
    printf("current elapsed:%lld\n", perElap);
    pthread_mutex_unlock(&mutexTime);
#endif	

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
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexTime, NULL);

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
    printf("%-10lld %-15lld %-8.2Lf\n", targetShared, elapsedTs / 1000, tps);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexTime);

    return 0;
}
