#! /bin/sh

# This file should be run from the repository root directory

function fail_and_exit() {
	echo $@
	exit 1
}

mkdir build || fail_and_exit "Failed to make directory"
cd ./build || fail_and_exit "Failed to cd into build"
cmake .. || fail_and_exit "Failed to generate the build environment"
make || fail_and_exit "Failed to make"
#make install || fail_and_exit "Failed to install"
