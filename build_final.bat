@echo off
setlocal
REM SPCMD Build Script
REM Builds two binaries:
REM   spcmd.exe   - 32-bit, Windows XP compatible (gcc-xp / MinGW GCC 5.3)
REM   spcmd64.exe - 64-bit x86_64 (llvm-mingw)

echo ===================
echo SPCMD Build Script
echo Building x86 (XP compatible) and x64 binaries
echo ===================
echo.

set "X86_TOOLCHAIN=D:\Program Files (x86)\i686-5.3.0-release-win32-dwarf-rt_v4-rev0\mingw32\bin"
set "X64_TOOLCHAIN=D:\Program Files (x86)\llvm-mingw-20251021-msvcrt-x86_64\bin"

REM Check if source file exists
if not exist spcmd.c (
    echo Error: Source file spcmd.c not found
    echo Please make sure you are running this script in the correct directory
    pause
    exit /b 1
)

REM ==================== Phase 1: 32-bit (Windows XP compatible) ====================
echo [1/2] Building 32-bit spcmd.exe (Windows XP compatible)...
set "PATH=%X86_TOOLCHAIN%;%PATH%"

gcc-xp --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: gcc-xp not found in system PATH
    pause
    exit /b 1
)

set RESOBJ=
if not exist spcmd.rc goto build_x86
windres -i spcmd.rc -o spcmd.res -O coff
if errorlevel 1 (
    echo Warning: Resource compilation failed, proceeding without resources
    goto build_x86
)
set RESOBJ=spcmd.res
:build_x86

gcc-xp -Wall -Wextra -std=c99 -m32 -mwindows -Os -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0500 -s -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--strip-all -static spcmd.c ini.c %RESOBJ% -o spcmd.exe -lgdi32 -luser32 -lshell32 -lole32 -lshlwapi -lws2_32
if %errorlevel% neq 0 (
    echo.
    echo Error: 32-bit compilation failed!
    pause
    exit /b %errorlevel%
)
echo 32-bit build successful: spcmd.exe
echo.

REM ==================== Phase 2: 64-bit x86_64 ====================
echo [2/2] Building 64-bit spcmd64.exe...
set "PATH=%X64_TOOLCHAIN%;%PATH%"

x86_64-w64-mingw32-gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: x86_64-w64-mingw32-gcc not found in system PATH
    pause
    exit /b 1
)

set RESOBJ64=
if not exist spcmd.rc goto build_x64
x86_64-w64-mingw32-windres -i spcmd.rc -o spcmd64.res -O coff
if errorlevel 1 (
    echo Warning: Resource compilation failed, proceeding without resources
    goto build_x64
)
set RESOBJ64=spcmd64.res
:build_x64

x86_64-w64-mingw32-gcc -Wall -Wextra -std=c99 -mwindows -Os -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0500 -s -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--strip-all -static spcmd.c ini.c %RESOBJ64% -o spcmd64.exe -lgdi32 -luser32 -lshell32 -lole32 -lshlwapi -lws2_32
if %errorlevel% neq 0 (
    echo.
    echo Error: 64-bit compilation failed!
    pause
    exit /b %errorlevel%
)
echo 64-bit build successful: spcmd64.exe
echo.

REM Optional: Copy executables to VM directory if it exists
if exist "D:\VM\VM_PATH\" (
    copy spcmd.exe D:\VM\VM_PATH\ >nul 2>&1
    copy spcmd64.exe D:\VM\VM_PATH\ >nul 2>&1
    copy spcmd.exe D:\Python_Work\psg\ >nul 2>&1
    echo Copied executables to VM directory
) else (
    echo VM directory not found, skipping copy
)
echo.

REM Clean up temporary resource files
if exist spcmd.res (
    echo Cleaning up temporary resource files...
    del spcmd.res
)
if exist spcmd64.res del spcmd64.res

REM All operations completed successfully
echo All operations completed successfully!
pause
