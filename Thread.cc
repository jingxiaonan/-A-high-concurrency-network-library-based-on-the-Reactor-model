#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>


std::atomic_int Thread::numCreated_{0};

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{    
    setDefaultName();
}
Thread::~Thread()
{
    if(started_ && !joined_)    //线程已经启动，但还没有join
    {
        thread_->detach();   //thread类提供的设置分离线程的方法
    }
}

void Thread::start()   //开启线程,执行线程函数,返回线程ID,一个Thread对象记录的就是一个新线程的详细信息
{
    started_ = true;
    sem_t sem;   //信号量,确保获取到线程的tid值
    sem_init(&sem, false, 0);
    //开启线程
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        //获取线程的tid值
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();   //开启一个新线程，专门执行该线程函数
    }));

    //这里必须等待获取上面新创建的线程tid值
    sem_wait(&sem);

}
void Thread::join(){
    joined_ = true;
    thread_->join();
}


void Thread::setDefaultName(){    //设置默认线程名
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;  
    }
}