#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include <sys/epoll.h>

/*
epoll的使用：
1.创建监听的套接字 int lfd = socket(AF_NET, SOCK_STREM, 0);
2.设置端口复用（可选） int opt = 1; setsockopt(lfd, SOLK_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
3.使用本地IP与端口套接字进行绑定 int ret = bind(lfd, struct sockaddr*)&serv_addr, sizeof(servaddr));
4.给监听的套接字设置监听   listen(lfd, 128);
5.创建epoll实例对象 int epfd = epoll_create(100)
6.将用于监听的套接字添加到epoll实例中  struct epoll_event ev; ev.data.fd = lfd; ev.events = EPOLLIN; epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
7.检测添加到epoll实例中的文件描述符是否已经就绪，并将这些已就绪的文件描述符进行处理   int num = epoll_wait(epfd, EPOLL_CTRL_ADD,lfd, &ev);
    如果是监听的文件描述符，和新客户建立连接，将得到的文件描述符添加到epoll实例中 int cfd = accept(curfd,NULL, NULL); ev.events = EPOLLIN; ev.data.fd = cfd; epoll_ctrl(epfd, EPOLL_CTRL_ADD< cfd, &ev);
    如果是通信的描述符，和对应客户端通信，如果连接已断开，将该文件描述符从epoll实例中删除 https://subingwen.cn/linux/epoll/#2-%E6%93%8D%E4%BD%9C%E5%87%BD%E6%95%B0
8.重复第7步
*/


class Channel;

//epoll_ctl  add/mod/del
class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;
    
    //重写基类Poller的的抽象方法
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override; //启动epoll_wait
    void updateChannel(Channel* channel) override; //调用epoll_ctl
    void removeChannel(Channel* channel) override;
private:
    static const int kInitEventListSize = 16;   //epoll_event数组的初始大小

    //填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;  //填充活跃的channel通道
    //更新Channel通道
    void update(int operation, Channel* channel);   //封装了epoll_ctl函数 
    using EventList = std::vector<epoll_event>;

    int epollfd_;    //epoll_create返回的文件描述符
    EventList events_;   //epoll_event的数组, 用于存储epoll_wait返回的就绪事件
};