@echo off

echo Installing prerequisites

net session >nul 2>&1
if NOT %errorLevel% == 0 (
    echo.
    echo Error: Unable to install, please execute bat file with Administrator privilages.
    echo.
    pause
    exit
) 

echo.
echo Downloading components...

set cmake_path=https://github.com/Kitware/CMake/releases/download/v3.17.2/
set cmake_file=cmake-3.17.2-win32-x86.msi

set gccarm_path=https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/
set gccarm_file=gcc-arm-none-eabi-9-2019-q4-major-win32-sha2.exe

set ninja_path=https://github.com/ninja-build/ninja/releases/download/v1.10.0/
set ninja_file=ninja-win.zip

if not exist "%TEMP%\%cmake_file%" (
    powershell Invoke-WebRequest -Uri "%cmake_path%%cmake_file%" -OutFile "'%TEMP%\%cmake_file%'"
)

if not exist "%TEMP%\%gccarm_file%" (
    powershell Invoke-WebRequest -Uri "%gccarm_path%%gccarm_file%" -OutFile "'%TEMP%\%gccarm_file%'"
)

if not exist "%TEMP%\%ninja_file%" (
    powershell Invoke-WebRequest -Uri "%ninja_path%%ninja_file%" -OutFile "'%TEMP%\%ninja_file%'"
)

echo.
echo Installing components, please don't close this window
echo.

echo Installing CMake...
"%TEMP%\%cmake_file%" ADD_CMAKE_TO_PATH=System /passive

echo Installing ARM GCC...
"%TEMP%\%gccarm_file%" /S /P /R

echo Installing Ninja...
if not exist "%ProgramFiles(x86)%\ninja" (
    mkdir "%ProgramFiles(x86)%\ninja"
)
"%~dp0\pathman.exe" /as "%ProgramFiles(x86)%\ninja"
powershell Expand-Archive -Force -Path "'%TEMP%\%ninja_file%'" -DestinationPath "'%ProgramFiles(x86)%\ninja'"

echo.
echo Installation complete! Successfully installed CMake, GCC-ARM and Ninja
echo.
pause



