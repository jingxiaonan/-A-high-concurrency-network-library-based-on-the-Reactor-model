#pragma once

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include "Channel.h"

#include <functional>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>

class Channel;
class Poller;

//事件循环类 主要包含了两个大模块 Channel Poller(epoll的抽象)
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    Timestamp pollReturnTime() const {return pollReturnTime_;}

    //在当前loop中执行cb
    void runInLoop(Functor cb);
    //把cb放入队列，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);

    void wakeup(); //唤醒loop所在的线程

    // EventLoop的方法，调用Poller相应的方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    //判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid();}


private:
    void handleRead(); //wakeupFd_的读回调函数，wake up
    void doPendingFunctors(); //执行回调操作

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;   //原子变量 事件循环是否启动
    std::atomic_bool quit_;      //原子变量 标识事件循环是否退出
    std::atomic_bool callingPendingFunctors_;  //原子变量 标识当前loop是否有需要执行的回调操作
    const pid_t threadId_;      //记录当前loop所在线程的id  
    Timestamp pollReturnTime_;  //poller返回发生事件的channels时间点
    std::unique_ptr<Poller> poller_;    //事件循环的核心IO复用模块, 封装了epoll

    int wakeupFd_;   //主要作用： eventfd 用于线程间的事件通知，当mainLoop获取一个新用户的channel,通过轮询算法选择一个subloop，通过给成员唤醒subloop，subLoop是reactor模型，subloop通过epoll_wait等待新用户的channel到来
    std::unique_ptr<Channel> wakeupChannel_;  //封装了wakeupFd_的channel，主要作用是当wakeupFd_可读时，调用其读回调函数handleRead

    ChannelList activeChannels_;  //保存poller返回的活跃的channel

    //std::atomic_bool callingPendingFunctors_; //标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;  //保存loop需要执行的所有回调操作
    std::mutex mutex_;  //互斥锁，保护pendingFunctors_的线程安全
};  
