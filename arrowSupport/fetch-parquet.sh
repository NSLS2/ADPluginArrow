#!/bin/bash

wget -nv http://archive.apache.org/dist/thrift/0.16.0/thrift-0.16.0.tar.gz
tar xzf thrift-0.16.0.tar.gz
cd thrift-0.16.0
chmod +x ./configure
./configure --disable-libs
