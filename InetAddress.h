#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

//封装socket地址类型
class InetAddress
{
private:
    sockaddr_in addr_;   //ipv4的socket地址
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");     //ip可以不传，不传就是"127.0.0.1"
    explicit InetAddress(const sockaddr_in &addr)       //用一个已经存在的sockaddr_in来构造InetAddress
    :addr_(addr)
    {};  

    std::string toIp() const;    //返回点分十进制的ip地址
    std::string toIpPort() const; //返回点分十进制:端口号
    uint16_t toPort() const;     //返回主机字节序的端口号
    const sockaddr_in *getSockAddr() const{return &addr_;}  //返回sockaddr_in类型的地址
    void setSockAddr(const sockaddr_in& addr) { addr_ = addr;}
};


