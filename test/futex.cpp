#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

// FUTEX_WAKE_PRIVATE 只对当前进程的线程有效
// FUTEX_WAIT 同一进程的线程之间共享该内存，其他进程也能看到锁的变化
// 当 uaddr 的值等于 val 时，线程会挂起等待，直到 futex 的值发生变化或者超时
#define futex_wait(uaddr, val) syscall(SYS_futex, uaddr, FUTEX_WAIT_PRIVATE, val, NULL, NULL, 0)
// val 参数指定了要唤醒的线程数
#define futex_wake(uaddr) syscall(SYS_futex, uaddr, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0)

volatile int futex_var = 0; // 共享的 futex 锁变量

typedef struct {
    int count;        // 当前等待的线程数
    int target;       // 目标线程数
    futex_t futex;    // Futex 变量
} custom_barrier;

void barrier_wait(custom_barrier *b) {
    int old = __atomic_add_fetch(&b->count, 1, __ATOMIC_SEQ_CST);
    if (old == b->target) {
        b->count = 0;
        futex_wake(&b->futex, INT_MAX); // 唤醒所有线程
    } else {
        futex_wait(&b->futex, old);     // 挂起等待
    }
}

void* thread_func(void *arg) {
    // Try to acquire the lock
    while (__sync_lock_test_and_set(&futex_var, 1)) {
        printf("Thread %ld wait\n", (long)arg);
        futex_wait(&futex_var, 1);  // Wait if lock is already acquired
    }

    // Critical section (locked)
    printf("Thread %ld acquired the lock\n", (long)arg);
    sleep(2);  // Simulate work

    // Release the lock
    futex_var = 0;
    futex_wake(&futex_var);  // Wake one waiting thread
    printf("Thread %ld released the lock\n", (long)arg);

    return NULL;
}

int main() {
    pthread_t threads[3];

    // Create multiple threads
    for (long i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_func, (void*)i);
    }

    // Wait for threads to finish
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
