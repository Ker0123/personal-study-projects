#include <iostream>
#include <pthread.h>

using namespace std;

int global_flag = 1;

pthread_cond_t cond_01;
pthread_cond_t cond_02;
pthread_mutex_t lock;

void *thread_func_01(void *arg)
{
    while (true)
    {
        pthread_mutex_lock(&lock);
        while (global_flag != 1)
        {
            pthread_cond_wait(&cond_01, &lock);
        }
        cout << "thread_func_01: global_flag = " << global_flag << endl;
        global_flag = 2;
        pthread_cond_signal(&cond_02);
        pthread_mutex_unlock(&lock);
    }
}

void *thread_func_02(void *arg)
{
    while (true)
    {
        pthread_mutex_lock(&lock);
        while (global_flag != 2)
        {
            pthread_cond_wait(&cond_02, &lock);
        }
        cout << "thread_func_02: global_flag = " << global_flag << endl;
        global_flag = 1;
        pthread_cond_signal(&cond_01);
        pthread_mutex_unlock(&lock);
    }
}

int main()
{
    pthread_t thread1, thread2;

    pthread_cond_init(&cond_01, NULL);
    pthread_cond_init(&cond_02, NULL);
    pthread_mutex_init(&lock, NULL);

    // 创建线程
    pthread_create(&thread1, NULL, thread_func_01, NULL);
    pthread_create(&thread2, NULL, thread_func_02, NULL);

    // 等待线程结束
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_cond_destroy(&cond_01);
    pthread_cond_destroy(&cond_02);
    pthread_mutex_destroy(&lock);

    return 0;
}
