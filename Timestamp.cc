#include "Timestamp.h"

#include <time.h>

Timestamp::Timestamp():microSecondsSinceEpoch_(0){};

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    :microSecondsSinceEpoch_(microSecondsSinceEpoch){}

Timestamp Timestamp::now(){
    //获取从1970.1.1到现在的秒数
    return Timestamp(time(NULL));
}

std::string Timestamp::toString() const{
    char buf[128] = {0};
    tm* tm_time = localtime(&microSecondsSinceEpoch_);
    //格式化时间 2019-06-19 12:01:01
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
            tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
            tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
    return buf;
}

// #include <iostream>

// int main(){
//     std::cout << Timestamp::now().toString() << std::endl;
// }