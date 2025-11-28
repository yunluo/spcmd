@echo off
REM SPCMD Build Script for Windows XP compatibility

echo ===================
echo SPCMD Build Script
echo Ensuring Windows XP Compatibility
echo ===================
echo.

set PATH="D:\Program Files (x86)\i686-5.3.0-release-win32-dwarf-rt_v4-rev0\mingw32\bin";%PATH%
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
    echo Please ensure MinGW or similar tool is installed and added to system PATH
    pause
    exit /b 1
)

REM Check if windres (resource compiler) is available
echo Detecting resource compiler...
windres --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Warning: windres not found in system PATH
    echo Resource file compilation will be skipped
    echo Please ensure MinGW with windres is installed and added to system PATH
)

REM Compile source files - Ensure Windows XP compatibility
echo Compiling source files ...
echo Using Windows XP compatibility mode with size optimization...

REM Check if resource file exists and compile it if possible
set RESOBJ=
if exist spcmd.rc (
    echo Resource file found. Trying to compile...
    windres --version >nul 2>&1
    if %errorlevel% equ 0 (
        echo Compiling resource file spcmd.rc...
        windres -i spcmd.rc -o spcmd.res -O coff
        if %errorlevel% equ 0 (
            echo Resource compilation successful
            set RESOBJ=spcmd.res
        ) else (
            echo Warning: Resource compilation failed, proceeding without resources
        )
    ) else (
        echo Skipping resource compilation as windres is not available
    )
)

REM Main compilation command with or without resources - Optimized for Windows XP compatibility
REM Using -mwindows flag to create GUI application without console window
REM Added -static option to statically link all libraries, including msvCRT.dll, for Windows XP compatibility
gcc-xp -Wall -Wextra -std=c99 -m32 -mwindows -Os -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0500 -s -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--strip-all -static spcmd.c ini.c %RESOBJ% -o spcmd.exe -lgdi32 -luser32 -lshell32 -lole32 -lshlwapi

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
    echo Original executable size:
    for %%I in (spcmd.exe) do echo %%~zI bytes
    
) else (
    echo Warning: Executable file not found
)

REM Optional: Copy executable to VM directory if it exists
if exist "D:\VM\VM_PATH\" (
    copy spcmd.exe D:\VM\VM_PATH\ >nul 2>&1
    copy spcmd.exe D:\Python_Work\psg\ >nul 2>&1
    echo Copied executable to VM directory
) else (
    echo VM directory not found, skipping copy
)
echo.



REM Clean up temporary resource file
if exist spcmd.res (
    echo Cleaning up temporary resource file...
    del spcmd.res
)

REM All operations completed successfully
echo All operations completed successfully!
pause