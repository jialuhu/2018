/*************************************************************************
	> File Name: threadpool.h
	> Author:jialuhu 
	> Mail: 
	> Created Time: Tue 11 Sep 2018 02:19:48 PM CST
 ************************************************************************/

#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include<iostream>
#include<list>
#include<cstdio>
#include<semaphore.h>
#include<exception>
#include<pthread.h>
#include"myhttp_coon.h"
using namespace std;

/*封装信号量*/
class sem{
private:
    sem_t m_sem;
public:
    sem();
    ~sem();
    bool wait();//等待信号量
    bool post();//增加信号量
};
//创建信号量
sem :: sem()
{
    if(sem_init(&m_sem,0,0) != 0)
    {
        throw std ::exception();
    }
}
//销毁信号量
sem :: ~sem()
{
    sem_destroy(&m_sem);
}
//等待信号量
bool sem::wait()
{
    return sem_wait(&m_sem) == 0;
}
//增加信号量
bool sem::post()
{
    return sem_post(&m_sem) == 0;
}

/*封装互斥锁*/
class mylocker{
private:
    pthread_mutex_t m_mutex;
public:
    mylocker();
    ~mylocker();
    bool lock();
    bool unlock();
};

mylocker::mylocker()
{
    if(pthread_mutex_init(&m_mutex, NULL) != 0)
    {
        throw std::exception();
    }
}

mylocker::~mylocker()
{
    pthread_mutex_destroy(&m_mutex);
}
/*上锁*/
bool mylocker::lock()
{
    return pthread_mutex_lock(&m_mutex)==0;
}
/*解除锁*/
bool mylocker::unlock()
{
    return pthread_mutex_unlock(&m_mutex) == 0;
}

/*封装条件变量*/
class mycond{
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
public:
    mycond();
    ~mycond();
    bool wait();
    bool signal();
};

mycond::mycond()
{
    if(pthread_mutex_init(&m_mutex,NULL)!=0)
    {
        throw std::exception();
    }
    if(pthread_cond_init(&m_cond, NULL)!=0)
    {
        throw std::exception();
    }
}

mycond::~mycond()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

/*等待条件变量*/
bool mycond::wait()
{
    int ret;
    pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_wait(&m_cond,&m_mutex);
    pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

/*唤醒等待条件变量的线程*/
bool mycond::signal()
{
    return pthread_cond_signal(&m_cond) == 0;
}

template<typename T>
/*线程池的封装*/
class threadpool
{
private:
    int max_thread;//线程池中的最大线程总数
    int max_job;//工作队列的最大总数
    pthread_t *pthread_poll;//线程池数组
    std::list<T*> m_myworkqueue;//请求队列
    mylocker m_queuelocker;//保护请求队列的互斥锁
    sem m_queuestat;//由信号量来判断是否有任务需要处理
    bool m_stop;;//是否结束线程
public:
    threadpool();
    ~threadpool();
    bool addjob(T* request);
private:
    static void* worker(void *arg);
    void run();
};
/*线程池的创建*/
template <typename T>
threadpool<T> :: threadpool()
{
    max_thread = 8;
    max_job = 1000;
    m_stop = false;
    pthread_poll = new pthread_t[max_thread];//为线程池开辟空间
    if(!pthread_poll)
    {
        throw std::exception();
    }
    for(int i=0; i<max_thread; i++)
    {
        cout << "Create the pthread:" << i << endl;
        if(pthread_create(pthread_poll+i, NULL, worker, this)!=0)
        {
            delete [] pthread_poll;
            throw std::exception();
        }
        if(pthread_detach(pthread_poll[i]))//将线程分离
        {
            delete [] pthread_poll;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    delete[] pthread_poll;
    m_stop = true;
}

template <typename T>
bool threadpool<T>::addjob(T* request)
{
    m_queuelocker.lock();
    if(m_myworkqueue.size()> max_job)//如果请求队列大于了最大请求队列，则出错
    {
        m_queuelocker.unlock();
        return false;
    }
    m_myworkqueue.push_back(request);//将请求加入到请求队列中
    m_queuelocker.unlock();
    m_queuestat.post();//将信号量增加1
    return true;
}
template <typename T>
void* threadpool<T>::worker(void *arg)
{
    threadpool *pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T> :: run()
{
    while(!m_stop)
    {
        m_queuestat.wait();//信号量减1，直到为0的时候线程挂起等待
        m_queuelocker.lock();
        if(m_myworkqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_myworkqueue.front();
        m_myworkqueue.pop_front();
        m_queuelocker.unlock();
        if(!request)
        {
            continue;
        }
        request->doit();
    }
}


#endif
