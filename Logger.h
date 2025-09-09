#pragma ONCE

#include <string>

#include "noncopyable.h"

//LOG_INFO(%s %d, arg1, arg2)
#define LOG_INFO(logmsgFormat, ...)\
    do\
    {\
        Logger &logger = Logger::instance();\
        logger.setLogLevel(INFO);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)    //先do 后while(0)，只执行一次
//logmsgFormat：宏的第一个参数，用来传入格式化字符串，比如 "%s %d"，...：表示可变参数宏，__VA_ARGS__：在宏里用来展开所有可变参数。
//snpritf: C 库函数，作用是把格式化字符串写到 buf 里，最大写入 1023 字节（最后留 \0）。相当于 printf 的安全版本，不会写出界。
//##_VA_ARGS_ 如果调用宏时没有传可变参数，就把前面多余的逗号去掉。例如：LOG_INFO("only one arg");展开后是：snprintf(buf, 1024, "only one arg");而不是：snprintf(buf, 1024, "only one arg", ); 错误
//“\”告诉编译器，宏定义还没有结束，下一行也是宏定义的一部分。

#define LOG_ERROR(logmsgFormat, ...)\
    do\
    {\
        Logger &logger = Logger::instance();\
        logger.setLogLevel(ERROR);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)  

#define LOG_FATAL(logmsgFormat, ...)\
    do\
    {\
        Logger &logger = Logger::instance();\
        logger.setLogLevel(FATAL);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf);\
        exit(1);\
    }while(0)  

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)\
    do\
    {\
        Logger &logger = Logger::instance();\
        logger.setLogLevel(DEBUG);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);\
        logger.log(buf);\
    }while(0)      
#else
    #define LOG_DEBUG(logmsgFormat, ...)
#endif

//定义日志的级别  INFO ERROR FATAL DEBUG
enum LogLevel{
    INFO,    //普通信息
    ERROR,   //错误信息
    FATAL,   //core信息
    DEBUG    //调试信息
};

//输出一个日志类
class Logger : noncopyable{
public:
    //获取日志唯一的实例对象
    static Logger& instance();  //单例模式
    //设置日志级别
    void setLogLevel(int level);
    //写日志
    void log(std::string msg);

private:
    int logLevel_; //日志级别
    Logger(){}
};