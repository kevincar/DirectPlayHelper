#! /usr/bin/env sh

# This file should be run from the repository root directory

#function fail_and_exit {
	#echo $@
	#exit 1
#}

echo "Checking for build directory..."
if [ -d build ] ; then
	echo "build directory already exists. Remvoing..."
	rm -rf ./build
fi

echo "Creating build directory"
mkdir build #|| echo "Failed to make directory" && exit 1

echo "Moving into build directory"
cd ./build #|| echo "Failed to cd into build" && exit 1

echo "Generating build environment..."
cmake .. #|| echo "Failed to generate the build environment" && exit 1

echo "Making..."
make #|| echo "Failed to make" && exit 1
#make install || fail_and_exit "Failed to install"
