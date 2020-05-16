#! /usr/bin/env sh

# This file should be run from the repository root directory

echo "Checking for build directory..."
if [ -d build ] ; then
	echo "build directory already exists. Remvoing..."
	rm -rf ./build
fi

echo "Creating build directory"
mkdir build

echo "Moving into build directory"
cd ./build

echo "Generating build environment..."
cmake ..

echo "Making..."
cmake --build .
