@echo off
echo SPCMD Functionality Test
echo =======================

if not exist spcmd.exe (
    echo Error: spcmd.exe not found!
    exit /b 1
)

echo Testing basic functionality...
echo.

echo 1. Testing help command:
spcmd.exe --help >nul
if %errorlevel% equ 0 (
    echo    PASS
) else (
    echo    FAIL
)

echo 2. Testing --h command  (should contain "SPCMD - System Power Command Tool"):
spcmd.exe --h > help_output.txt
findstr /C:"SPCMD - System Power Command Tool" help_output.txt >nul
if %errorlevel% equ 0 (
    echo    PASS
) else (
    echo    FAIL
)
del help_output.txt

echo 3. Testing random command:
spcmd.exe random >nul
if %errorlevel% equ 0 (
    echo    PASS
) else (
    echo    FAIL
)

echo 4. Testing process command:
spcmd.exe process --name=explorer.exe >nul
if %errorlevel% equ 0 (
    echo    PASS
) else (
    echo    FAIL
)

echo 5. Testing infoboxtop command:
spcmd.exe infoboxtop "Test message" "Test title" >nul
if %errorlevel% equ 0 (
    echo    PASS
) else (
    echo    FAIL
)

echo.
echo Test completed. If all tests show PASS, the executable is working properly.
pause