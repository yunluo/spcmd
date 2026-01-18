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
set LOGFILE=test_result.log

:: Initialize log file
echo ============================================ > %LOGFILE%
echo SPCMD Test Log >> %LOGFILE%
echo Date: %date% %time% >> %LOGFILE%
echo ============================================ >> %LOGFILE%
echo. >> %LOGFILE%

goto :main

:test_help
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> %LOGFILE%
    echo   Command: %CMD% >> %LOGFILE%
    
    %CMD% > test_output.tmp 2>&1
    
    for %%A in (test_output.tmp) do (
        set FILESIZE=%%~zA
        echo   Output size: !FILESIZE! bytes >> %LOGFILE%
        echo   Criteria: output size ^> 10 bytes >> %LOGFILE%
        if %%~zA gtr 10 (
            echo   Result: PASS
            echo   Result: PASS >> %LOGFILE%
            set /a PASS+=1
        ) else (
            echo   Result: FAIL - no help output
            echo   Result: FAIL - output too small >> %LOGFILE%
            set /a FAIL+=1
        )
    )
    echo. >> %LOGFILE%
    echo.
    goto :eof

:test_output
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> %LOGFILE%
    echo   Command: %CMD% >> %LOGFILE%
    
    %CMD% > test_output.tmp 2>&1
    
    for %%A in (test_output.tmp) do (
        set FILESIZE=%%~zA
        echo   Output size: !FILESIZE! bytes >> %LOGFILE%
        echo   Criteria: output size ^> 0 bytes >> %LOGFILE%
        if %%~zA gtr 0 (
            echo   Result: PASS
            echo   Result: PASS >> %LOGFILE%
            echo   Output: >> %LOGFILE%
            type test_output.tmp >> %LOGFILE%
            set /a PASS+=1
        ) else (
            echo   Result: FAIL - no output
            echo   Result: FAIL - no output >> %LOGFILE%
            set /a FAIL+=1
        )
    )
    echo. >> %LOGFILE%
    echo.
    goto :eof

:test_file
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    set "FILE=%~3"
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> %LOGFILE%
    echo   Command: %CMD% >> %LOGFILE%
    echo   Expected file: %FILE% >> %LOGFILE%
    echo   Criteria: file exists after command >> %LOGFILE%
    
    %CMD% > test_output.tmp 2>&1
    
    if exist "%FILE%" (
        for %%F in ("%FILE%") do set FSIZE=%%~zF
        echo   Result: PASS - file created
        echo   Result: PASS - file created, size: !FSIZE! bytes >> %LOGFILE%
        set /a PASS+=1
        del "%FILE%" >nul 2>&1
    ) else (
        echo   Result: FAIL - file not created
        echo   Result: FAIL - file not created >> %LOGFILE%
        echo   Command output: >> %LOGFILE%
        type test_output.tmp >> %LOGFILE%
        set /a FAIL+=1
    )
    echo. >> %LOGFILE%
    echo.
    goto :eof

:test_keyword
    set /a TOTAL+=1
    set "CMD=%~1"
    set "DESC=%~2"
    set "KEY=%~3"
    
    echo [Test !TOTAL!] %DESC%
    echo   Command: %CMD%
    
    echo [Test !TOTAL!] %DESC% >> %LOGFILE%
    echo   Command: %CMD% >> %LOGFILE%
    echo   Expected keyword: %KEY% >> %LOGFILE%
    echo   Criteria: output contains keyword >> %LOGFILE%
    
    %CMD% > test_output.tmp 2>&1
    
    echo   Output: >> %LOGFILE%
    type test_output.tmp >> %LOGFILE%
    
    findstr /I /C:"%KEY%" test_output.tmp >nul 2>&1
    if !errorlevel! equ 0 (
        echo   Result: PASS
        echo   Result: PASS - keyword found >> %LOGFILE%
        set /a PASS+=1
    ) else (
        echo   Result: FAIL - keyword not found: %KEY%
        echo   Result: FAIL - keyword not found >> %LOGFILE%
        set /a FAIL+=1
    )
    echo. >> %LOGFILE%
    echo.
    goto :eof

:skip_test
    set /a TOTAL+=1
    echo [Test !TOTAL!] %~1
    echo   Result: SKIP - %~2
    
    echo [Test !TOTAL!] %~1 >> %LOGFILE%
    echo   Result: SKIP - %~2 >> %LOGFILE%
    echo. >> %LOGFILE%
    echo.
    goto :eof

:main

echo ============================================
echo 1. Help Command Tests
echo ============================================
echo.

call :test_help "spcmd.exe screenshot --h" "screenshot help"
call :test_help "spcmd.exe shortcut --h" "shortcut help"
call :test_help "spcmd.exe autorun --h" "autorun help"
call :test_help "spcmd.exe task --h" "task help"
call :test_help "spcmd.exe restart --h" "restart help"
call :test_help "spcmd.exe config --h" "config help"
call :test_help "spcmd.exe process --h" "process help"
call :test_help "spcmd.exe tray --h" "tray help"
call :test_help "spcmd.exe floating --h" "floating help"
call :test_help "spcmd.exe ipc --h" "ipc help"

echo ============================================
echo 2. Functional Tests
echo ============================================
echo.

call :test_file "spcmd.exe screenshot --save=test_shot.jpg" "screenshot save JPG" "test_shot.jpg"

echo [test] > test_cfg.ini
echo mykey=myvalue >> test_cfg.ini
call :test_keyword "spcmd.exe config --file=test_cfg.ini --action=get --section=test --key=mykey" "config read INI" "myvalue"
del test_cfg.ini >nul 2>&1

echo ============================================
echo 3. GUI Commands (Manual Test)
echo ============================================
echo.

call :skip_test "window popup" "GUI requires manual verification"
call :skip_test "notify notification" "GUI requires manual verification"
call :skip_test "infoboxtop popup" "GUI requires manual verification"
call :skip_test "qboxtop popup" "GUI requires manual verification"

echo ============================================
echo 4. Admin Commands (Skipped)
echo ============================================
echo.

call :skip_test "shortcut" "requires write permission"
call :skip_test "autorun" "requires admin"
call :skip_test "task" "requires admin"
call :skip_test "restart" "may require admin"
call :skip_test "tray" "requires target process"
call :skip_test "floating" "requires target process"
call :skip_test "timesync" "requires admin"

echo ============================================
echo Test Summary
echo ============================================
echo.
echo Total:  %TOTAL%
echo Passed: %PASS%
echo Failed: %FAIL%
echo.

echo ============================================ >> %LOGFILE%
echo Test Summary >> %LOGFILE%
echo ============================================ >> %LOGFILE%
echo Total:  %TOTAL% >> %LOGFILE%
echo Passed: %PASS% >> %LOGFILE%
echo Failed: %FAIL% >> %LOGFILE%
echo End Time: %date% %time% >> %LOGFILE%
echo ============================================ >> %LOGFILE%

if exist test_output.tmp del test_output.tmp >nul 2>&1

echo.
echo Log saved to: %LOGFILE%
echo.

if %FAIL% gtr 0 (
    echo Some tests failed!
    exit /b 1
) else (
    echo All tests passed!
    exit /b 0
)
