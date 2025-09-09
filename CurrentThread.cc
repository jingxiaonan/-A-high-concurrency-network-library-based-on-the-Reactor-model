#include "CurrentThread.h"

namespace CurrentThread{
    __thread int t_cachedTid = 0;

    void cacheTid()
    {
        if(t_cachedTid == 0)  //如果线程ID未缓存
        {
            //通过linux系统调用，获取当前线程的tid值
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }    
    }
}

/*
t_cachedTid：线程局部变量，初始为 0。

cacheTid()：如果没缓存过，就用系统调用 syscall(SYS_gettid) 获取线程 ID 并保存。

外部可以通过 CurrentThread::tid()（在 .h 文件里定义的 inline 函数）来安全获取线程 ID。*/