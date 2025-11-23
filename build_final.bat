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
gcc-xp --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Warning: GCC compiler not found in system PATH
    echo Trying alternative compiler name...
    gcc-xp --version >nul 2>&1
    if %errorlevel% neq 0 (
        echo Error: GCC compiler not found in system PATH
        echo Please ensure MinGW or similar tool is installed and added to system PATH
        pause
        exit /b 1
    )
)

REM Compile source files - Ensure Windows XP compatibility
echo Compiling source files ...
echo Using Windows XP compatibility mode with size optimization...
gcc-xp -Wall -Wextra -std=c99 -m32 -Os -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0500 -s -fno-unwind-tables -fno-asynchronous-unwind-tables -mwindows -mconsole spcmd.c -o spcmd.exe -lgdi32 -luser32 -lshell32 -lole32 -lshlwapi -lcomctl32 || gcc -Wall -Wextra -std=c99 -m32 -Os -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0500 -s -fno-unwind-tables -fno-asynchronous-unwind-tables -mwindows -mconsole spcmd.c -o spcmd.exe -lgdi32 -luser32 -lshell32 -lole32 -lshlwapi -lcomctl32

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