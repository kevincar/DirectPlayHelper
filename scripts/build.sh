#! /usr/bin/env sh

# This file should be run from the repository root directory

function fail_and_exit() {
	echo $@
	exit 1
}

echo "Checking for build directory..."
if [ -d build ] ; then
	echo "build directory already exists. Remvoing..."
	rm -rf ./build
fi

echo "Creating build directory"
mkdir build || fail_and_exit "Failed to make directory"

echo "Moving into build directory"
cd ./build || fail_and_exit "Failed to cd into build"

echo "Generating build environment..."
cmake .. || fail_and_exit "Failed to generate the build environment"

echo "Making..."
make || fail_and_exit "Failed to make"
#make install || fail_and_exit "Failed to install"
