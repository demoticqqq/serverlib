#!/bin/bash

set -e

# 如果没有build目录 创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -fr `pwd`/build/*
cd `pwd`/build &&
    cmake .. &&
    make

# 回到项目根目录
cd ..

# 把头文件拷贝到 /usr/include/serverlib       .so库拷贝到 /usr/lib
if [ ! -d /usr/include/serverlib ]; then
    mkdir /usr/include/serverlib
fi

if [ ! -d /usr/include/serverlib/base ]; then
    mkdir /usr/include/serverlib/base
fi

if [ ! -d /usr/include/serverlib/net ]; then
    mkdir /usr/include/serverlib/net
fi


for header in `ls ./base/*.h`
do
    cp $header /usr/include/serverlib/base
done

for header in `ls ./net/*.h`
do
    cp $header /usr/include/serverlib/net
done


cp `pwd`/lib/libserverlib.so /usr/lib

ldconfig