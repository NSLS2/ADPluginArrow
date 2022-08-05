#!/bin/bash


mkdir -p os/linux-x86_64

git clone https://github.com/apache/arrow
git checkout -q apache-arrow-9.0.0
cd arrow/cpp
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../.. ..
make -sj
make install

cd ../../..
cp lib64/*.a os/linux-x86_64/.
cp lib64/*.so os/linux-x86_64/.

rm -rf lib64 share arrow


