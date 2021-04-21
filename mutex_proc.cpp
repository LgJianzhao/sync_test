#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "helper.h"

/* 全局变量 */
int64 targetPerThread = 1000000;
int64 targetShared = 0;

struct mt
{
    int num;
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexattr;
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: ./mutex_proc [process size] \n");
        return -1;
    }

    int size = atoi(argv[1]);
//    targetPerThread = targetPerThread / size;

    struct mt *mm = (mt*) mmap(NULL, sizeof(pthread_mutex_t),
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    if (&mm->mutex == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    memset(mm, 0x00, sizeof(*mm));

    pthread_mutexattr_init(&mm->mutexattr);
    pthread_mutexattr_setpshared(&mm->mutexattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mm->mutex, &mm->mutexattr);

    int pid = -1;
    for (int i = 0; i < size; ++i) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            break;
        }

        if (pid == 0) {
            if (debug > 2) {
                printf("this is child, break");
            }
            /* 保证创建子进程只在主进程中进行 */
            break;
        }
    }

    int64 startTs = STime_GetMicrosecondsTime();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid > 0) { // 父进程
        // do nothing
    } else { // 子进程
        pid_t pid = getpid();
        printf("child pid:%d, mutex:%ld \n", pid, mm->mutex);

        for (int i = 0; i < targetPerThread; ++i) {
            pthread_mutex_lock(&mm->mutex);
            mm->num++;
            pthread_mutex_unlock(&mm->mutex);
        }

        // 子进程执行后退出，仅主进程统计性能信息
        return 0;
    }

    // 等待全部子进程结束
    while ((pid = waitpid(-1, NULL, 0))) {
       if (errno == ECHILD) {
          break;
       }
    }

    int64 elapsedTs = STime_GetMicrosecondsTime() - startTs;
    long double tps = targetPerThread * size * 1.00 /
                    elapsedTs * 1000 * 1000;
    if (debug > 0) {
        printf("\n"
                "Test       Elapsed Time    TPS     \n"
                "count      ms.             per/s   \n");
    }
    printf("%-10lld %-15lld %-8.2Lf\n", mm->num, elapsedTs / 1000, tps);

    pthread_mutexattr_destroy(&mm->mutexattr);
    pthread_mutex_destroy(&mm->mutex);

    return 0;
}
