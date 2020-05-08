ECHO "Checking for build directory..."

IF exist ".\build\" (
	ECHO "build directory already exists. Removing..."
	RMDIR /S /Q build
)

ECHO "Creating build directory"
MKDIR build

ECHO "Moving into build directory"
CD build

ECHO "Enabling MSVC Dev Tools"
REM CALL "%MSVCTools%/VsDevCmd.bat"

ECHO "Generating build environment..."
REM cmake -DCMAKE_PREFIX_PATH="C:\g3log" ..
cmake ..

ECHO "Making..."
cmake --build . --config Release
