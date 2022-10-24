#!/bin/bash


# Create our output dir if it doesn't exist
mkdir -p os/linux-x86_64

# Clone arrow project and checkout version 91
git clone https://github.com/apache/arrow
git checkout -q apache-arrow-9.0.0
cd arrow/cpp

# Make a build directory
mkdir build
cd build

# Cmake w/ options
cmake .. \
    -DCMAKE_INSTALL_PREFIX=../../.. \
    -DARROW_CSV=ON \
    -DARROW_JEMALLOC=OFF \
    -DARROW_DATASET=ON \
    $ARROW_CMAKE_OPTIONS

# Build and install arrow
make -sj
make install

# Copy arrow libraries to epics location
cd ../../..
cp lib64/*.a os/linux-x86_64/.
cp lib64/*.so* os/linux-x86_64/.

# Remove unused dirs
rm -rf lib64 share arrow


