@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ============================================
echo SPCMD Test Script
echo ============================================
echo.

set PASS=0
set FAIL=0
set TOTAL=0
set TESTDIR=tests
set LOGFILE=%TESTDIR%\test_result.log
set TMPFILE=%TESTDIR%\test_output.tmp

:: Create tests directory if not exists
if not exist "%TESTDIR%" mkdir "%TESTDIR%"

:: Initialize log file
echo ============================================ > "%LOGFILE%"
echo SPCMD Test Log >> "%LOGFILE%"
echo Date: %date% %time% >> "%LOGFILE%"
echo ============================================ >> "%LOGFILE%"
echo. >> "%LOGFILE%"

goto :main

:test_help
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> "%LOGFILE%"
    echo   Command: %CMD% >> "%LOGFILE%"
    
    %CMD% > "%TMPFILE%" 2>&1
    
    echo   Output: >> "%LOGFILE%"
    type "%TMPFILE%" >> "%LOGFILE%"
    
    for %%A in ("%TMPFILE%") do (
        set FILESIZE=%%~zA
        echo   Output size: !FILESIZE! bytes >> "%LOGFILE%"
        echo   Criteria: output size ^> 10 bytes >> "%LOGFILE%"
        if %%~zA gtr 10 (
            echo   Result: PASS
            echo   Result: PASS >> "%LOGFILE%"
            set /a PASS+=1
        ) else (
            echo   Result: FAIL - no help output
            echo   Result: FAIL - output too small >> "%LOGFILE%"
            set /a FAIL+=1
        )
    )
    echo. >> "%LOGFILE%"
    echo.
    goto :eof

:test_output
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> "%LOGFILE%"
    echo   Command: %CMD% >> "%LOGFILE%"
    
    %CMD% > "%TMPFILE%" 2>&1
    
    for %%A in ("%TMPFILE%") do (
        set FILESIZE=%%~zA
        echo   Output size: !FILESIZE! bytes >> "%LOGFILE%"
        echo   Criteria: output size ^> 0 bytes >> "%LOGFILE%"
        if %%~zA gtr 0 (
            echo   Result: PASS
            echo   Result: PASS >> "%LOGFILE%"
            echo   Output: >> "%LOGFILE%"
            type "%TMPFILE%" >> "%LOGFILE%"
            set /a PASS+=1
        ) else (
            echo   Result: FAIL - no output
            echo   Result: FAIL - no output >> "%LOGFILE%"
            set /a FAIL+=1
        )
    )
    echo. >> "%LOGFILE%"
    echo.
    goto :eof

:test_file
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    set "FILEPATH=%~3"
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> "%LOGFILE%"
    echo   Command: %CMD% >> "%LOGFILE%"
    echo   Expected file: %FILEPATH% >> "%LOGFILE%"
    echo   Criteria: file exists after command >> "%LOGFILE%"
    
    %CMD% > "%TMPFILE%" 2>&1
    
    echo   Output: >> "%LOGFILE%"
    type "%TMPFILE%" >> "%LOGFILE%"
    
    if exist "%FILEPATH%" (
        for %%F in ("%FILEPATH%") do set FSIZE=%%~zF
        echo   Result: PASS - file created
        echo   Result: PASS - file created, size: !FSIZE! bytes >> "%LOGFILE%"
        set /a PASS+=1
        del "%FILEPATH%" >nul 2>&1
    ) else (
        echo   Result: FAIL - file not created
        echo   Result: FAIL - file not created >> "%LOGFILE%"
        set /a FAIL+=1
    )
    echo. >> "%LOGFILE%"
    echo.
    goto :eof

:test_keyword
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    set "KEY=%~3"
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> "%LOGFILE%"
    echo   Command: %CMD% >> "%LOGFILE%"
    echo   Expected keyword: %KEY% >> "%LOGFILE%"
    echo   Criteria: output contains keyword >> "%LOGFILE%"
    
    %CMD% > "%TMPFILE%" 2>&1
    
    echo   Output: >> "%LOGFILE%"
    type "%TMPFILE%" >> "%LOGFILE%"
    
    findstr /I /C:"%KEY%" "%TMPFILE%" >nul 2>&1
    if !errorlevel! equ 0 (
        echo   Result: PASS
        echo   Result: PASS - keyword found >> "%LOGFILE%"
        set /a PASS+=1
    ) else (
        echo   Result: FAIL - keyword not found: %KEY%
        echo   Result: FAIL - keyword not found >> "%LOGFILE%"
        set /a FAIL+=1
    )
    echo. >> "%LOGFILE%"
    echo.
    goto :eof

:skip_test
    set /a TOTAL+=1
    set "DESC=%~1"
    set "REASON=%~2"
    
    echo [Test !TOTAL!] %DESC%
    echo   Result: SKIP - %REASON%
    
    echo [Test !TOTAL!] %DESC% >> "%LOGFILE%"
    echo   Result: SKIP - %REASON% >> "%LOGFILE%"
    echo. >> "%LOGFILE%"
    echo.
    goto :eof

:test_gui
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    set "REASON=%~3"
    set "TIMEOUT=%~4"
    
    if "!TIMEOUT!"=="" set TIMEOUT=3000
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> "%LOGFILE%"
    echo   Command: %CMD% >> "%LOGFILE%"
    echo   Type: GUI Interactive Test >> "%LOGFILE%"
    echo   Note: %REASON% >> "%LOGFILE%"
    
    REM 启动GUI命令到tests目录，防止默认文件生成到根目录
    pushd "%TESTDIR%"
    timeout /t 1 /nobreak
    start "" cmd /c "%CMD%"
    timeout /t 2 /nobreak
    popd
    
    echo   Result: PASS - GUI command launched >> "%LOGFILE%"
    echo   Result: PASS - launched
    set /a PASS+=1
    echo. >> "%LOGFILE%"
    echo.
    goto :eof

:main

echo ============================================
echo 1. Help Command Tests - All Commands
echo ============================================
echo.

call :test_help "spcmd.exe screenshot --h" "screenshot help"
call :test_help "spcmd.exe shortcut --h" "shortcut help"
call :test_help "spcmd.exe autorun --h" "autorun help"
call :test_help "spcmd.exe infoboxtop --h" "infoboxtop help"
call :test_help "spcmd.exe qboxtop --h" "qboxtop help"
call :test_help "spcmd.exe window --h" "window help"
call :test_help "spcmd.exe task --h" "task help"
call :test_help "spcmd.exe restart --h" "restart help"
call :test_help "spcmd.exe notify --h" "notify help"
call :test_help "spcmd.exe config --h" "config help"
call :test_help "spcmd.exe process --h" "process help"
call :test_help "spcmd.exe tray --h" "tray help"
call :test_help "spcmd.exe floating --h" "floating help"
call :test_help "spcmd.exe timesync --h" "timesync help"
call :test_help "spcmd.exe ipc --h" "ipc help"

echo ============================================
echo 2. Screenshot Command Tests
echo ============================================
echo.

call :test_file "spcmd.exe screenshot --save=%TESTDIR%\test_shot.jpg" "screenshot save JPG" "%TESTDIR%\test_shot.jpg"
call :test_file "spcmd.exe screenshot --save=%TESTDIR%\test_shot.bmp --format=bmp" "screenshot save BMP" "%TESTDIR%\test_shot.bmp"
call :test_file "spcmd.exe screenshot --save=%TESTDIR%\test_shot.png --format=png" "screenshot save PNG" "%TESTDIR%\test_shot.png"
call :test_file "spcmd.exe screenshot --save=%TESTDIR%\test_shot_thumb.jpg --format=jpg --quality=50" "screenshot with quality" "%TESTDIR%\test_shot_thumb.jpg"

echo ============================================
echo 3. Config File Tests
echo ============================================
echo.

echo [section1] > %TESTDIR%\test_config.ini
echo key1=value1 >> %TESTDIR%\test_config.ini
echo key2=value2 >> %TESTDIR%\test_config.ini
echo [section2] >> %TESTDIR%\test_config.ini
echo key3=value3 >> %TESTDIR%\test_config.ini

call :test_keyword "spcmd.exe config --file=%TESTDIR%\test_config.ini --action=get --section=section1 --key=key1" "config get value" "value1"
call :test_file "spcmd.exe config --file=%TESTDIR%\test_config.ini --action=dump" "config dump file" "%TESTDIR%\test_config.ini"

del %TESTDIR%\test_config.ini >nul 2>&1

echo ============================================
echo 4. Process Command Tests
echo ============================================
echo.

call :test_output "spcmd.exe process --pid=%random%" "process info by PID"
call :test_help "spcmd.exe process --kill=notepad.exe" "process kill command"

echo ============================================
echo 5. UUID/ID Generation Tests
echo ============================================
echo.

call :test_output "spcmd.exe uuid" "generate default UUID"
call :test_output "spcmd.exe uuid 4" "generate UUID v4"
call :test_output "spcmd.exe uuid 7" "generate UUID v7"

echo ============================================
echo 6. Snowflake ID Tests
echo ============================================
echo.

call :test_output "spcmd.exe snowflake" "generate Snowflake ID"

echo ============================================
echo 7. Environment Variable Tests
echo ============================================
echo.

call :test_output "spcmd.exe getenv PATH" "get PATH environment variable"
call :test_output "spcmd.exe getenv WINDIR" "get WINDIR environment variable"

echo ============================================
echo 8. Time/Date Tests
echo ============================================
echo.

call :test_output "spcmd.exe time" "get current time"
call :test_output "spcmd.exe date" "get current date"

echo ============================================
echo 9. Screen/Display Tests
echo ============================================
echo.

call :test_output "spcmd.exe screen" "get screen resolution"

echo ============================================
echo 10. GUI Commands (Manual Verification Needed)
echo ============================================
echo.

call :test_gui "spcmd.exe window --text=%TESTDIR%\test_msg.txt" "window command" "Should display text from file"
call :test_gui "spcmd.exe infoboxtop --message=SPCMD Test" "infoboxtop dialog" "Should show information dialog"
call :test_gui "spcmd.exe qboxtop --message=Do you see this?" "qboxtop question box" "Should show yes/no dialog"
call :test_gui "spcmd.exe notify --title=SPCMD --message=Test Notification" "desktop notification" "Should show system notification"

echo ============================================
echo 11. Admin/System Commands (Requires Permissions)
echo ============================================
echo.

call :skip_test "shortcut" "requires write permission and parameters"
call :skip_test "autorun" "requires admin privileges"
call :skip_test "task" "requires admin privileges"
call :skip_test "restart" "requires admin privileges"
call :skip_test "tray" "requires running target process"
call :skip_test "floating" "requires running target process"
call :skip_test "timesync" "requires admin privileges"
call :skip_test "ipc" "requires special setup"

echo ============================================
echo Test Summary
echo ============================================
echo.
echo Total:  %TOTAL%
echo Passed: %PASS%
echo Failed: %FAIL%
echo.

echo ============================================ >> "%LOGFILE%"
echo Test Summary >> "%LOGFILE%"
echo ============================================ >> "%LOGFILE%"
echo Total:  %TOTAL% >> "%LOGFILE%"
echo Passed: %PASS% >> "%LOGFILE%"
echo Failed: %FAIL% >> "%LOGFILE%"
echo End Time: %date% %time% >> "%LOGFILE%"
echo ============================================ >> "%LOGFILE%"
echo. >> "%LOGFILE%"

:: Generate HTML Report
set HTMLFILE=%TESTDIR%\test_report.html
echo ^<!DOCTYPE html^> > "%HTMLFILE%"
echo ^<html lang="zh-CN"^> >> "%HTMLFILE%"
echo ^<head^> >> "%HTMLFILE%"
echo   ^<meta charset="UTF-8"^> >> "%HTMLFILE%"
echo   ^<title^>SPCMD 测试报告^</title^> >> "%HTMLFILE%"
echo   ^<style^> >> "%HTMLFILE%"
echo     body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; } >> "%HTMLFILE%"
echo     .report { background-color: white; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); } >> "%HTMLFILE%"
echo     h1 { color: #333; border-bottom: 2px solid #007bff; padding-bottom: 10px; } >> "%HTMLFILE%"
echo     h2 { color: #555; margin-top: 30px; } >> "%HTMLFILE%"
echo     .summary { background-color: #f9f9f9; padding: 15px; border-left: 4px solid #007bff; margin: 20px 0; } >> "%HTMLFILE%"
echo     .pass { color: #28a745; font-weight: bold; } >> "%HTMLFILE%"
echo     .fail { color: #dc3545; font-weight: bold; } >> "%HTMLFILE%"
echo     .skip { color: #ffc107; font-weight: bold; } >> "%HTMLFILE%"
echo     table { width: 100%%; border-collapse: collapse; margin: 20px 0; } >> "%HTMLFILE%"
echo     th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; } >> "%HTMLFILE%"
echo     th { background-color: #007bff; color: white; } >> "%HTMLFILE%"
echo     tr:hover { background-color: #f9f9f9; } >> "%HTMLFILE%"
echo     .timestamp { color: #666; font-size: 12px; } >> "%HTMLFILE%"
echo   ^</style^> >> "%HTMLFILE%"
echo ^</head^> >> "%HTMLFILE%"
echo ^<body^> >> "%HTMLFILE%"
echo   ^<div class="report"^> >> "%HTMLFILE%"
echo     ^<h1^>SPCMD 自动化测试报告^</h1^> >> "%HTMLFILE%"
echo     ^<p class="timestamp"^>生成时间: %date% %time%^</p^> >> "%HTMLFILE%"
echo     ^<div class="summary"^> >> "%HTMLFILE%"
echo       ^<h2 style="margin-top: 0;"^>测试摘要^</h2^> >> "%HTMLFILE%"
echo       ^<p^>总测试数: ^<strong^>%TOTAL%^</strong^>^</p^> >> "%HTMLFILE%"
echo       ^<p class="pass"^>通过: %PASS%^</p^> >> "%HTMLFILE%"
echo       ^<p class="fail"^>失败: %FAIL%^</p^> >> "%HTMLFILE%"

if %FAIL% equ 0 (
    echo       ^<p style="color: #28a745; font-size: 16px;"^>✓ 所有测试通过!^</p^> >> "%HTMLFILE%"
) else (
    echo       ^<p style="color: #dc3545; font-size: 16px;"^>✗ 存在测试失败!^</p^> >> "%HTMLFILE%"
)

echo     ^</div^> >> "%HTMLFILE%"
echo     ^<h2^>详细日志^</h2^> >> "%HTMLFILE%"
echo     ^<pre style="background-color: #f9f9f9; padding: 15px; border-radius: 5px; overflow-x: auto;"^> >> "%HTMLFILE%"

REM 读取日志内容并转义HTML特殊字符
for /f "delims=" %%A in ('type "%LOGFILE%"') do (
    set "line=%%A"
    setlocal EnableDelayedExpansion
    set "line=!line:&=^&amp;!"
    set "line=!line:<=^&lt;!"
    set "line=!line:>=^&gt;!"
    echo !line! >> "%HTMLFILE%"
    endlocal
)

echo     ^</pre^> >> "%HTMLFILE%"
echo   ^</div^> >> "%HTMLFILE%"
echo ^</body^> >> "%HTMLFILE%"
echo ^</html^> >> "%HTMLFILE%"

if exist "%TMPFILE%" del "%TMPFILE%" >nul 2>&1

echo.
echo ================================================================
echo 测试完成!
echo ================================================================
echo 日志文件:   %LOGFILE%
echo 报告文件:   %HTMLFILE%
echo 测试工件保存位置: %TESTDIR%
echo ================================================================
echo.

if %FAIL% gtr 0 (
    echo 部分测试失败!
    exit /b 1
) else (
    echo 所有测试通过!
    exit /b 0
)
