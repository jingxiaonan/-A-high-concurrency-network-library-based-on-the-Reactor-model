#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;
//muduo库中多路事件分发器的核心IO复用模块
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;
    
    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    //给所有IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0; //纯虚函数 让子类实现,这里相当于启动了epoll_wait
    virtual void updateChannel(Channel* channel) = 0;    //这里相当于调用epoll_ctl
    virtual void removeChannel(Channel* channel) = 0;

    //判断参数channel是否在当前Poller中
    bool hasChannel(Channel* channel) const;

    //EventLoop 可以通过该接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop); //静态函数 根据不同平台返回不同的Poller子类对象

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;   //map中的key: sockfd value:sockfd所属的channel通道类型
    ChannelMap channels_;   //保存fd到channel的映射
private:
    EventLoop* ownerLoop_; //定义Poller所属的事件循环EventLoop

};