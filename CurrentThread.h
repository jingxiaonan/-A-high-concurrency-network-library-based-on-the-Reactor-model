#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread{
    extern __thread int t_cachedTid;    //线程局部变量
    //extern 表示这个变量的声明而不是定义，告诉编译器这个变量在别的地方（通常是 .cpp 文件）有一个真正的定义。
    /*
    __thread是 GCC/Clang 提供的线程局部存储（Thread-Local Storage, TLS）修饰符。
    含义：每个线程都有独立的一份变量副本，互不干扰。
    举例：__thread int x = 0;
    void* func(void*) {
        x++;
        printf("Thread %ld: x=%d\n", pthread_self(), x);
        return nullptr;
    }
    如果你开两个线程，它们的 x 各自维护，不会互相影响。
    */

    void cacheTid();    //缓存线程ID

    inline int tid()   //获取线程ID
    {
        if(__builtin_expect(t_cachedTid ==0, 0)){    //如果线程ID未缓存
        /*
        这是 GCC/Clang 的一个编译器内建函数，用来给分支预测做提示。
        语法：long __builtin_expect(long exp, long c);
        表示：我（程序员）期望表达式 exp 的值大多数情况下等于 c。
        __builtin_expect(t_cachedTid == 0, 0)
        意思是：程序员预计 t_cachedTid == 0 这个条件大多数情况下是“假”的。因为线程 ID 一旦缓存下来，后续调用 tid() 时一般都不需要再走 cacheTid()。
        编译器会根据这个提示调整汇编代码的分支布局，让“常走的路径”更快。
        */
            cacheTid();        //缓存线程ID
        }
        return t_cachedTid;    //返回线程ID
    }
}

/*
t_cachedTid 是线程局部变量，保存当前线程的 ID。

第一次调用 tid() 时，t_cachedTid 还是 0，于是调用 cacheTid() 把真实的线程 ID 缓存下来。

之后再调用 tid()，直接返回缓存的 ID，不用每次都去 syscall(SYS_gettid)，效率更高。

__builtin_expect 提示编译器“通常情况下 t_cachedTid 已经缓存好了”，提高分支预测命中率。*/