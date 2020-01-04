@echo off

:: Should be in the build directory

:: G3LOGGER
git clone https://github.com/KjellKod/g3log.git
cd g3log

echo "Creating a build directory"
mkdir build
echo "Moving into build directory for g3log"
cd build

echo "calling CMAKE for g3log dependency"
cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 15 2017" ..

echo "Building g3log"
msbuild g3log.sln /p:Configuration=Release /p:Platform=Win32

cd ..
cd ..
