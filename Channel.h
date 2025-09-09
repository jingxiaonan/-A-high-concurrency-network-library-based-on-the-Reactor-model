#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>  //智能指针

class EventLoop;
/*这叫 前向声明（forward declaration）目的是告诉编译器：“有一个叫 EventLoop 的类存在”，但不展开定义
为什么用？因为你在 Channel 内部只 使用指针：EventLoop* loop_;指针大小是已知的，不需要完整类型
优点：减少头文件依赖，提高编译速度，避免循环包含；你说“这个类不应该出现在这里”，其实是必须的 如果你不想 #include "EventLoop.h"
*/

/*
理清楚 EventLoop、Channel、Poller 之间的关系，    《=Reactor模型上对应多路事件分发器 Demultiplex
Channel 理解为通道， 封装了sockfd 和其感兴趣的事件event,如EPOLIN、EPOLLOUT事件
还绑定了Poller返回的具体事件events
*/
class Channel: noncopyable{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    //fd得到poller通知以后，处理事件的函数
    void handleEvent(Timestamp receiveTime);

    //设置回调函数对象，在handleEvent里面调用
    //Channel 完全解耦了底层 I/O 和业务逻辑：底层只负责告诉你事件发生了，具体要做什么由用户传入的回调决定
    void setReadCallback(ReadEventCallback cb){
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback cb){
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback cb){
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback cb){
        errorCallback_ = std::move(cb);
    }


    //防止channel被手动remove掉后，channel还在执行回调函数
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }  //返回fd
    int events() const { return events_; } //channel感兴趣的事件
    void set_revents(int revt) { revents_ = revt; } //poller返回的具体发生的事件

    void enableReading() { events_ |= kReadEvent; update(); } //设置fd相应的事件状态，设置感兴趣读事件,不难猜出，update()在调用epoll_ctl
    void disableReading() { events_ &= ~kReadEvent; update();}    //取消关注读事件
    void enableWriting() { events_ |= kWriteEvent; update();}     //设置fd相应的事件状态，设置感兴趣写事件
    void disableWriting() { events_ &= ~kWriteEvent; update();}  //取消关注写事件
    void disableAll() { events_ = kNoneEvent; update();}          //设置fd相应的事件状态，设置没有感兴趣事件

    //返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; } //是否没有感兴趣的事件
    bool isWriting() const { return events_ & kWriteEvent; } //是否感兴趣写事件
    bool isReading() const { return events_ & kReadEvent;}    //是否感兴趣读事件

    int index() {return index_;}   //在poller中的状态
    void set_index(int idx) { index_ = idx;}   //在poller中的状态
    
    // one loop per thread
    EventLoop* ownerLoop() { return loop_;}    //返回channel所属的事件循环
    void remove();  //从poller中删除该channel
private:

    void update(); //更新channel所感兴趣的事件，由EventLoop调用Poller的相应方法
    void handleEventWithGuard(Timestamp receiveTime); //防止channel被手动remove掉后，channel还在执行回调函数

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_; //事件循环
    const int fd_; //sockfd，Poller监听的对象
    int events_; //注册fd感兴趣的事件
    int revents_; //poller返回的具体发生的事件
    int index_; //在poller中的状态

    std::weak_ptr<void> tie_;  //防止channel被手动remove掉后，channel还在执行回调函数
    bool tied_;

    //因为channel通道里面能够获知fd最发生的具体的事件revents，所以它负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};