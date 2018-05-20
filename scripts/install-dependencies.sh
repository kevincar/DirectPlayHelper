#! /usr/bin/env sh

brew install g3log

#brew install gtest
cd ./tests
unzip googletest-release-1.8.0.zip -d gtest
cd ./gtest/googletest-release-1.8.0/
mkdir build
cd ./build
cmake ..
make
