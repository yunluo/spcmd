<#
SPCMD PowerShell 示例脚本
保存为: scripts\spcmd_examples.ps1
说明: 本脚本演示常用命令调用、输出捕获与退出码处理
#>

# 设置可执行程序路径（根据实际情况调整）
$spcmd = Join-Path $PSScriptRoot '..\spcmd.exe'
if (-not (Test-Path $spcmd)) { $spcmd = 'spcmd' }

Write-Host "使用 spcmd 路径: $spcmd"

# 1) 屏幕截图: 保存为 PNG
& $spcmd screenshot --save="$PWD\screenshot_example.png" --format=png
Write-Host "screenshot -> exit:$LASTEXITCODE"

# 2) 屏幕截图: 输出 Base64 到文件
& $spcmd screenshot --base64="$PWD\screenshot_example.b64" --format=jpg --quality=85
Write-Host "screenshot(base64 file) -> exit:$LASTEXITCODE"

# 3) 快捷方式: 创建桌面快捷方式（需管理员或有写入权限）
& $spcmd shortcut --target="C:\Windows\notepad.exe" --name="SPCMD Notepad Demo" --desc="Open Notepad" --workdir="C:\Windows"
Write-Host "shortcut -> exit:$LASTEXITCODE"

# 4) 自启（autorun）: 添加与移除示例
& $spcmd autorun --target="C:\MyApp\myapp.exe" --name="MyAppAutoStart"
Write-Host "autorun add -> exit:$LASTEXITCODE"
# 移除
& $spcmd autorun --target="C:\MyApp\myapp.exe" --remove
Write-Host "autorun remove -> exit:$LASTEXITCODE"

# 5) 信息弹窗与确认后执行（qboxtop）
& $spcmd infoboxtop --message="Hello from SPCMD" --title="Demo Info"
Write-Host "infoboxtop -> exit:$LASTEXITCODE"

& $spcmd qboxtop --message="Open calc?" --program="calc.exe" --title="Demo QBox"
Write-Host "qboxtop -> exit:$LASTEXITCODE"

# 6) 自定义窗口并 onclick 执行
& $spcmd window --text="Click to open notepad" --title="Click Demo" --onclick="notepad.exe"
Write-Host "window(onclick) -> exit:$LASTEXITCODE"

# 7) 进程: 运行、检查、杀死
& $spcmd process --action=run --exec="notepad.exe"
Write-Host "process run -> exit:$LASTEXITCODE"
Start-Sleep -Seconds 2
& $spcmd process --action=check --name="notepad.exe"
Write-Host "process check -> exit:$LASTEXITCODE"
# 示例: 杀死所有 notepad 进程
& $spcmd process --action=kill --name="notepad.exe"
Write-Host "process kill -> exit:$LASTEXITCODE"

# 8) 计划任务: 创建（示例）
& $spcmd task --name="SPCMD_Demo_Task" --exec="C:\Windows\notepad.exe" --trigger=daily --starttime=23:00
Write-Host "task create -> exit:$LASTEXITCODE"

# 9) 重启进程
& $spcmd restart --path="C:\Windows\notepad.exe" --wait=5
Write-Host "restart -> exit:$LASTEXITCODE"

# 10) 通知
& $spcmd notify --title="SPCMD Example" --message="This is a sample notification" --icon=info --timeout=5
Write-Host "notify -> exit:$LASTEXITCODE"

# 11) 配置文件操作
& $spcmd config --file="$PWD\demo.ini" --action=set --section=User --key=Name --value=Alice
Write-Host "config set -> exit:$LASTEXITCODE"
& $spcmd config --file="$PWD\demo.ini" --action=get --section=User --key=Name
Write-Host "config get -> exit:$LASTEXITCODE"

# 12) 时间同步
& $spcmd timesync --server=time.windows.com
Write-Host "timesync -> exit:$LASTEXITCODE"

# 13) IPC (TCP) 发送数据
& $spcmd ipc --host=127.0.0.1 --port=9999 --value="hello from spcmd example"
Write-Host "ipc -> exit:$LASTEXITCODE"

Write-Host "示例脚本执行完成"
