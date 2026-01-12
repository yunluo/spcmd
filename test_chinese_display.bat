@echo off
chcp 65001 >nul
echo 测试简体中文显示效果
echo ======================

echo 启动测试窗口...
spcmd.exe window --text="这是一个测试窗口，用于验证简体中文在不同系统上的显示效果。如果显示正常，则表示修改成功。" --title="简体中文测试窗口" --width=500 --height=300

echo.
echo 测试完成。
pause