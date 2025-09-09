#pragma once

#include <iostream>
#include <string>

class Timestamp{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);     //explicit防止隐式转换,必须显示调用Timestamp(参数);
    static Timestamp now(); //获取当前时间
    std::string toString() const;   //转换为字符串，const放在后面表示常量成员函数，不会修改成员变量（在内部有修改成员变量的公式就会报错），可以被 const 对象 和 非 const 对象调用
private:
    int64_t microSecondsSinceEpoch_; //微秒级时间戳
};
