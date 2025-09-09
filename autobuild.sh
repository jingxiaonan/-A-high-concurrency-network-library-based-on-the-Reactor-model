#!/bin/bash

set -e   #让脚本在遇到非零退出码（出错）时立即退出

# 如果没有 build 目录 创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

# 如果没有 include 目录 创建该目录
if [ ! -d `pwd`/include ]; then
    mkdir `pwd`/include
fi

# 如果没有 lib 目录 创建该目录
if [ ! -d `pwd`/lib ]; then
    mkdir `pwd`/lib
fi


# 删除 build 目录下的所有文件,并执行 cmake 命令
rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

#回到项目根目录
cd ..

# 将头文件复制到 /usr/include,将动态库文件复制到/usr/lib
if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

for header in `ls *.h`
do
    cp $header /usr/include/mymuduo
done

cp `pwd`/lib/libmymuduo.so /usr/lib

# 使操作生效
ldconfig
