#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

#include <sys/epoll.h>

//Channel类 主要封装了sockfd和其感兴趣的事件event,如EPOLIN、EPOLLOUT事件
//还绑定了poller返回的具体事件events

const int Channel::kNoneEvent = 0;     //没有感兴趣的事件，注意这里不用写static, 因为在类中已经声明为static了
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; //EPOLLPRI 高优先级数据可读
const int Channel::kWriteEvent = EPOLLOUT;          //EPOLLOUT  低优先级数据可写  

//EventLoop: 事件循环类 主要包含了两个大模块 Channel Poller(epoll的抽象)
Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false)
{
}

Channel::~Channel()
{ 
}

//channel的tie方法什么时候调用过？一个TcpConnection新链接创建的时候 TcpConnection => Channel
void Channel::tie(const std::shared_ptr<void>& obj){
    tie_ = obj;    //
    tied_ = true; //标记绑定
}

/*
当改变Channel所表示fd的events事件后，update负责在poller里面更改fd相应的事件状态（通过EventLoop调用epoll_ctl）
EventLoop => ChannelList   
*/
void Channel::update(){
    //通过Channel所属的EventLoop,调用Poller的相应方法，注册fd的events事件
    loop_->updateChannel(this);
}

//在Channel所属的EventLoop中，把当前的Channel删除掉
void Channel::remove(){
    loop_->removeChannel(this);
}


void Channel::handleEvent(Timestamp receiveTime){
    if(tied_){
        std::shared_ptr<void> guard = tie_.lock();  //尝试获取shared_ptr
        if(guard){  //如果成功，说明对象还存在
            handleEventWithGuard(receiveTime);     //调用实际的事件处理函数
        }
    }
    else{
        handleEventWithGuard(receiveTime);    //调用实际的事件处理函数
    }
}
/*
tied_ 是防止 Channel 所属对象被销毁后仍然调用回调
比如一个 TCP 连接对象持有 Channel，如果连接对象被 delete，Channel 的回调还在 EventLoop 的队列里
tie_ 保存了一个 std::weak_ptr 指向那个对象
tie_.lock() 尝试获取 shared_ptr，如果失败说明对象已经销毁
tied_ = true → 使用了这种保护机制
tied_ = false → 不保护，直接调用回调
换句话说：tied_ 不是控制流程的开关，而是Channel 是否“绑”了拥有它的对象，用于安全访问回调。
*/


//根据Poller通知的Channel发生的具体事件，由Channel负责调用具体的回调函数
void Channel::handleEventWithGuard(Timestamp receiveTime){
    LOG_INFO("Channel handleEvent revents:%d\n", revents_);
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){ //EPOLLHUP  对端关闭连接
        if(closeCallback_) closeCallback_(); //调用关闭回调函数
    }

    if(revents_ & EPOLLERR){ //EPOLLERR  错误事件
        if(errorCallback_) errorCallback_(); //调用错误回调函数
    }

    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){ //EPOLLRDHUP  对端关闭连接或者半关闭
        if(readCallback_) readCallback_(receiveTime); //调用读回调函数
    }

    if(revents_ & EPOLLOUT){ //EPOLLOUT  写事件
        if(writeCallback_) writeCallback_(); //调用写回调函数
    }
}