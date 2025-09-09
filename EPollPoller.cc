#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>


const int kNew = -1;  //表示一个新的Channel,Channel的成员index_ = -1
const int kAdded = 1; //表示已经添加到poller中
const int kDeleted = 2; //表示已经从poller中删除

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))     //创建epoll实例, EPOLL_CLOEXEC: 进程执行新程序时关闭该文件描述符
    , events_(kInitEventListSize)   //初始化epoll_event数组,vector<epoll_event>
{
    if (epollfd_ < 0){
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EPollPoller::~EPollPoller(){
    ::close(epollfd_);   //关闭epoll文件描述符
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels){      //启动epoll_wait
    //实际上应该用LOG_DEBUG输出日志更为合理
    LOG_INFO("func=%s => fd tatal count:%lu\n", __FUNCTION__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs); //调用epoll_wait
    int saveErrno = errno; //保存errno
    Timestamp now(Timestamp::now()); //获取当前时间

    if(numEvents > 0){
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels); //填充活跃的channel通道
        if(numEvents == events_.size() * 2){
            events_.resize(events_.size() * 2);  //如果返回的事件数等于数组大小，说明可能还有未处理的事件，扩大数组
        }
    }
    else if(numEvents == 0){
        LOG_DEBUG("%s timeiout", __FUNCTION__);
    }
    else{
        if(saveErrno != EINTR){
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now; //返回当前时间
}

//channel update remove => EventLoop updateChannel removeChannel=> Poller
/*
                EventLoop   => poller.poll
    ChannelList              Poller
                    ChannelMap    <fd, channel*>  epollfd

*/
void EPollPoller::updateChannel(Channel* channel){    //调用epoll_ctl
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events = %d index = %d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    if(index == kNew || index == kDeleted){
        if(index == kNew){
            int fd = channel->fd();
            channels_[fd] = channel;  //channels_是Poller的成员变量, map<fd, channel*>,channel是一个指针，保存fd到channel的映射
        }

        channel->set_index(kAdded);   //更新channel的状态为已经添加到poller中
        update(EPOLL_CTL_ADD, channel);   //封装了epoll_ctl函数
    }
    else{       //已经添加到poller中,并且更新了感兴趣的事件 
        int fd = channel->fd();
        if(channel->isNoneEvent()){   //如果channel没有感兴趣的事件
            update(EPOLL_CTL_DEL, channel);  //从epoll中删除该fd
            channel->set_index(kDeleted); //更新channel的状态为已经从poller中删除
        }
        else{
            update(EPOLL_CTL_MOD, channel);  //修改该fd对应的感兴趣事件
        }        
    }
} 

//从Poller中删除channel
void EPollPoller::removeChannel(Channel* channel){    //从poller的ChannelMap中删除该channel，如果是已经添加到epoll中（kAdded），还要调用epoll_ctl删除
    int fd = channel->fd();
    channels_.erase(fd);   //从Poller的成员变量ChannelMap中删除该channel

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int index = channel->index();
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel); //从epoll中删除该fd
    }
    channel->set_index(kNew);  //把channel的状态设置为新的channel,表示该channel还没有添加到poller中


}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const //填充活跃的channel通道
{
    for(int i =0; i < numEvents; ++i){
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);   //设置channel发生的具体事件
        activeChannels->push_back(channel); //添加到活跃的channel列表中,EventLoop就拿到了它返回的所有事件的channel列表了
    }
}
//更新Channel通道
void EPollPoller::update(int operation, Channel* channel)   //封装了epoll_ctl函数 
{
    epoll_event event;
    bzero(&event, sizeof(event));

    int fd = channel->fd();

    event.events = channel->events(); //channel感兴趣的事件
    event.data.fd = fd;
    event.data.ptr = channel; //把channel指针存储到epoll_event的data成员中
    

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0){   //调用epoll_ctl函数
        if(operation == EPOLL_CTL_DEL){
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else{
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}
