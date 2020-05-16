@echo off
:: Move into the projects directory
cd C:/Users/Kevin/Documents/Computer-Projects/C++-Projects

:: Check if the project directory exists
IF exist ".\DPServer\" (
	echo "YES"
	::echo "removing folder and recloning"
	RMDIR DPServer /S /Q
	git clone https://github.com/kevincar/DPServer.git
	) ELSE (
	:: NO DPServer directory...  cloning
	echo "NO"
	git clone https://github.com/kevincar/DPServer.git
	 )

:: Create build directory
cd .\DPServer
git checkout windev
echo "Creating build directory"
mkdir build
cd build

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"
:: call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\VsDevCmd.bat"

:: Install Dependencies
echo "Installing Dependencies"
call "../scripts/win/install-dependencies.bat"

:: CMAKE
echo "Running cmake"
cmake -Dg3logger_DIR="C:\Users\Kevin\Documents\Computer-Projects\C++-Projects\DPServer\build\g3log\build" ..
