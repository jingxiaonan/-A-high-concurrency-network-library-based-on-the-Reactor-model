#include "InetAddress.h"
#include "Logger.h"

#include <string.h> //bzero

InetAddress::InetAddress(uint16_t port, std::string ip){
    bzero(&addr_, sizeof(addr_)); //清零,void bzero(void *s, size_t n);把指针 s 指向的内存块的前 n 个字节全部置为 0。
    addr_.sin_family = AF_INET;   //ipv4
    addr_.sin_port = htons(port); //主机字节序转网络字节序
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); //点分十进制转网络字节序,.c_str()把string转为const char*
}


std::string InetAddress::toIp() const{   //返回点分十进制的ip地址
    //这里将经典写法注释掉了
    //char buf[64] = {0};  //std::string有一个构造函数，可以用const char*来构造string对象
    //::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)); //将网络字节序的ip地址转换为点分十进制
    //return buf

    return std::string(inet_ntoa(addr_.sin_addr));//网络字节序转点分十进制    
}    
std::string InetAddress::toIpPort() const{      //返回点分十进制:端口号
    //ip:port
    char buf[64] = {0};  //std::string有一个构造函数，可以用const char*来构造string对象
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));   //::强制调用全局命名空间的函数
    size_t end = strlen(buf);    //strlen计算字符串长度(只计算有效长度)，不包括\0
    uint16_t port = ntohs(addr_.sin_port); //网络字节序转主机字节序
    sprintf(buf+end, ":%u", port); //从buf的end位置开始写入端口号,buf这里退化为指针，代表数组的首地址，指针 + 偏移量写法
    return buf; 
} 
uint16_t InetAddress::toPort() const{    //返回主机字节序的端口号
    return ntohs(addr_.sin_port);
}    

#include <iostream>
int main(){
    InetAddress in_addr(8080);
    std::cout << in_addr.toIpPort() << std::endl;

}