/*
 * rwlock.cpp
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
pthread_rwlock_t lock;
int64 targetPerThread = 32 * 1000 * 1000;
int64 targetShared = 0;
int criticalSectionType = 0;

void BigCriticalSection() {
    int j = 0;
    while(j++ < 10000) {
        int a = j;
        a *= 2;
    }
}

/* 线程函数 读 */
void *ReadThreadFunc(void *pArg) {
	int i = 0;
    while (i++ < targetPerThread) {
    	pthread_rwlock_rdlock(&lock);
    	// targetShared++; 读不需要，主要是模拟是跟原子操作做对比
        if (criticalSectionType) {
            BigCriticalSection();
        }
        pthread_rwlock_unlock(&lock);
    }
    return NULL;
}

/* 线程函数 写 */
void *WriteThreadFunc(void *pArg) {
	pthread_t tid = pthread_self();
 
	int i = 0;
    while (i++ < targetPerThread) {
    	pthread_rwlock_wrlock(&lock);
    	targetShared ++;
#if 1
        if (debug > 2) {
        	if (i % (100 * 1000) == 0) {
        		printf("tid[%ld], write count === %d\n", (long)tid, i);
	        }
        }
#endif
        pthread_rwlock_unlock(&lock);
    }
    return NULL;
}

 
int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("usage: ./rwlock [write thread size] [read thread size]\n");
		return -1;
	}

	int wsize 	= atoi(argv[1]);
	int rsize 	= atoi(argv[2]);
    if (argc > 3) {
        criticalSectionType = atoi(argv[3]);
    }
    
    int size = wsize + rsize;
    targetPerThread = targetPerThread / size;
	
    pthread_t *wThreadId =  new pthread_t[wsize];
    pthread_t *rThreadId =  new pthread_t[rsize];
    pthread_rwlock_init(&lock, NULL);
 
    int64 startTs = STime_GetMicrosecondsTime();
    
    for (int i = 0; i < wsize; i++) {
    	if (pthread_create(&wThreadId[i], NULL, WriteThreadFunc, NULL)) {
        	perror("pthread_create-w");
            break;
        }
        // printf("pthread_create-w[%d], %d\n", i, wThreadId[i]);
    }
    
    for (int i = 0; i < rsize; i++) {
		if (pthread_create(&rThreadId[i], NULL, ReadThreadFunc, NULL)) {
	    	perror("pthread_create-r");
	        break;
	    }
	    // printf("pthread_create-r[%d], %d\n", i, rThreadId[i]);
	}
    
    for (int i = 0; i < wsize; i ++) {
        pthread_join(wThreadId[i], NULL);	
    }
    
    for (int i = 0; i < rsize; i ++) {
        pthread_join(rThreadId[i], NULL);	
    }
    
    int64 elapsedTs = STime_GetMicrosecondsTime() - startTs;
    
    if (debug > 0) {
		printf("\n"
				"Test       Elapsed Time    TPS     \n"
				"count      ms.             per/s   \n");
	}
    
    int64 totalTarget = targetPerThread * (rsize + wsize);
    long double tps = totalTarget * 1.00 / elapsedTs  * 1000 * 1000;
    printf("%-10lld %-15lld %-8.2Lf\n", totalTarget, elapsedTs / 1000, tps);
    
    pthread_rwlock_destroy(&lock);
    return 0;
}
