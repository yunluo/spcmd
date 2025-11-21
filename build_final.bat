@echo off
REM SPCMD Build Script for Windows XP compatibility

echo ===================
echo SPCMD Build Script
echo Ensuring Windows XP Compatibility
echo ===================
echo.

REM Check if source file exists
if not exist spcmd.c (
    echo Error: Source file spcmd.c not found
    echo Please make sure you are running this script in the correct directory
    pause
    exit /b 1
)

REM Try to use gcc compiler from system PATH
echo Detecting GCC compiler...
gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Warning: GCC compiler not found in system PATH
    echo Please ensure MinGW or similar tool is installed and added to system PATH
    pause
    exit /b 1
)

REM Compile source file - Ensure Windows XP compatibility
echo Compiling spcmd.c ...
echo Using Windows XP compatibility mode with size optimization...
gcc -Wall -Wextra -std=c99 -Os -D_WIN32_WINNT=0x0501 -s -fno-unwind-tables -fno-asynchronous-unwind-tables spcmd.c -o spcmd.exe -lgdi32 -luser32 -lshell32 -lole32

REM Check if compilation was successful
if %errorlevel% neq 0 (
    echo.
    echo Error: Compilation failed!
    pause
    exit /b %errorlevel%
)

echo.
echo Build successful! Generated executable: spcmd.exe
echo.

REM Check if executable was generated
if exist spcmd.exe (
    echo Build completed.
    echo Executable size:
    for %%I in (spcmd.exe) do echo %%~zI bytes
) else (
    echo Warning: Executable file not found
)

echo.
echo Press any key to exit...
pause >nul