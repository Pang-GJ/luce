#!/bin/bash

# 把头文件拷贝到 /usr/include/luce so库拷贝到/usr/lib PATH
if [ ! -d /usr/include/luce ]; then
  mkdir /usr/include/luce
fi

for header in `ls include/*.h`
do 
  cp $header /usr/include/luce
done

cp `pwd`/lib/libluce.so /usr/lib

ldconfig
