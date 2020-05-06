@echo off

echo Elevating to Adminstrator privilages
if not "%1"=="am_admin" (powershell start -verb runas '%0' am_admin & exit /b)

echo.
echo Downloading components

set cmake_path=https://github.com/Kitware/CMake/releases/download/v3.17.2/
set cmake_file=cmake-3.17.2-win32-x86.msi

set gccarm_path=https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/
set gccarm_file=gcc-arm-none-eabi-9-2019-q4-major-win32-sha2.exe

set ninja_path=https://github.com/ninja-build/ninja/releases/download/v1.10.0/
set ninja_file=ninja-win.zip

if not exist "%TEMP%\%cmake_file%" (
    bitsadmin /transfer cmakejob /dynamic /download /priority FOREGROUND %cmake_path%%cmake_file% "%TEMP%\%cmake_file%"
)

if not exist "%TEMP%\%gccarm_file%" (
    bitsadmin /transfer gccarmjob /dynamic /download /priority FOREGROUND %gccarm_path%%gccarm_file% "%TEMP%\%gccarm_file%"
)

if not exist "%TEMP%\%ninja_file%" (
    bitsadmin /transfer ninjajob /dynamic /download /priority FOREGROUND %ninja_path%%ninja_file% "%TEMP%\%ninja_file%"
)

echo.
echo Installing CMake
"%TEMP%\%cmake_file%" ADD_CMAKE_TO_PATH=System /passive

echo.
echo Installing ARM GCC - Please don't close this window
"%TEMP%\%gccarm_file%" /S /P /R

echo.
echo Installing Ninja
mkdir "%ProgramFiles(x86)%\ninja"
"%~dp0\pathman.exe" /as "%ProgramFiles(x86)%\ninja"
powershell Expand-Archive -Force -Path "%TEMP%\%ninja_file%" -DestinationPath "%TEMP%"
copy "%TEMP%\ninja.exe" "%ProgramFiles(x86)%\ninja"

echo.
echo Installation complete
pause



