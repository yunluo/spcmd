@echo off
echo SPCMD Functionality Test
echo =======================

if not exist spcmd.exe (
    echo Error: spcmd.exe not found!
    exit /b 1
)

echo Testing basic functionality...
echo.

echo 1. Testing random command:
spcmd.exe random >nul
if %errorlevel% equ 0 (
    echo    PASS
) else (
    echo    FAIL
)

echo 2. Testing process command:
spcmd.exe process --name=explorer.exe >nul
if %errorlevel% equ 0 (
    echo    PASS
) else (
    echo    FAIL
)

echo 3. Testing infoboxtop command:
spcmd.exe infoboxtop "Test message" "Test title" >nul
if %errorlevel% equ 0 (
    echo    PASS
) else (
    echo    FAIL
)

echo 4. Testing window command with Simplified Chinese:
spcmd.exe window --text="简体中文测试" --title="中文测试"
if %errorlevel% equ 0 (
    echo    PASS
    timeout /t 1 >nul
) else (
    echo    FAIL
)

echo 5. Testing window command with Traditional Chinese:
spcmd.exe window --text="繁體中文測試" --title="繁體中文測試" --font="微軟雅黑"
if %errorlevel% equ 0 (
    echo    PASS
    timeout /t 1 >nul
) else (
    echo    FAIL
)

echo 6. Testing window command with multiple languages:
spcmd.exe window --text="Hello World! 你好，世界！" --title="多语言测试" --fontsize=18
if %errorlevel% equ 0 (
    echo    PASS
    timeout /t 1 >nul
) else (
    echo    FAIL
)

echo 7. Testing window command with special Chinese titles (dedicated title bar test):
spcmd.exe window --text="This is a test of Chinese title display" --title="特殊字符中文标题测试：你好，世界！中文标题栏显示验证" --fontsize=16
if %errorlevel% equ 0 (
    echo    PASS
    timeout /t 1 >nul
) else (
    echo    FAIL
)

echo.
echo Test completed. If all tests show PASS, the executable is working properly.
pause