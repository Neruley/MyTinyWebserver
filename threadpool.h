#ifndef THREADPOOL_H
#define THREADPOOL_H
//线程池类

#include <pthread.h>
#include <list>
#include <exception>
#include "locker.h"
#include "connection_pool.h"

template<typename T>
class threadpool
{
private:
    int m_thread_number;        //线程数量
    pthread_t *m_threads;       //线程池数组，大小为m_thread_number
    int m_max_requests;         //请求队列允许的最大请求数
    std::list<T *> m_workqueue; //请求队列
    locker m_queuelocker;       //请求队列互斥锁
    sem m_queuestat;            //是否有任务需要处理
    bool m_stop;                //是否结束线程  
    connection_pool *m_connpool;//数据库
public:
    threadpool(connection_pool *conpool,int thread_number=8, int max_requests=10000)
    :m_thread_number(thread_number),m_max_requests(m_max_requests)
    ,m_threads(NULL),m_stop(false),m_connpool(conpool)
    {
        if (thread_number <= 0 || max_requests <= 0)
            throw std::exception();
        m_threads = new pthread_t[m_thread_number];
        if (!m_threads)
            throw std::exception();
        for (int i=0; i<thread_number; i++){
           // printf("create the %dth thread\n",i);
            if (pthread_create(m_threads + i, NULL, worker, this) != 0){
                delete[] m_threads;
                throw std::exception();
            }
            if (pthread_detach(m_threads[i])){
                delete[] m_threads;
                throw std::exception();
            }
        }
    }
    ~threadpool(){
        delete[] m_threads;
        m_stop = true;
    }
    bool append(T* request){
        m_queuelocker.lock();
        if (m_workqueue.size() > m_max_requests){
            m_queuelocker.unlock();
            return 0;
        }
        m_workqueue.push_back(request);
        m_queuelocker.unlock();
        m_queuestat.post();
        return 1;
    }
private:
    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void *worker(void *arg){
        threadpool *pool = (threadpool *) arg;
        pool->run();
        return pool;
    }
    void run(){
        while (!m_stop){
            m_queuestat.wait();
            m_queuelocker.lock();
            if (m_workqueue.empty()){
                m_queuelocker.unlock();
                continue;
            }
            T *request = m_workqueue.front();
            m_workqueue.pop_front();
            m_queuelocker.unlock();
            if (!request) continue;

            connectionRAII mysqlcon(&request->mysql, m_connpool);
            request->process();
        }
    }
};


#endif