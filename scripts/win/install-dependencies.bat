@echo off

:: Should be in the build directory

:: G3LOGGER
git clone https://github.com/KjellKod/g3log.git
cd g3log
mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 12" ..
msbuild g3log.sln /p:Configuration=Release
