# SPCMD - System Power Command Tool

SPCMD是一个功能强大的Windows系统命令行工具，提供了多种系统管理和用户交互功能。

## 版本

**当前版本: 7.5.0.0**

## 功能特性

- **屏幕截图**: 捕获屏幕截图并保存为多种格式
- **快捷方式创建**: 创建桌面快捷方式
- **开机自启配置**: 配置程序开机自动启动
- **信息弹窗**: 显示各种类型的信息弹窗
- **自定义窗口**: 创建具有高级功能的自定义窗口（支持点击确认后执行命令）
- **程序执行**: 执行应用程序并指定工作目录
- **计划任务**: 创建Windows计划任务
- **进程重启**: 重启指定的进程
- **系统通知**: 显示系统通知
- **配置文件管理**: 管理INI配置文件（支持UTF-8 BOM编码）
- **进程管理**: 检查和终止系统进程
- **随机数生成**: 生成各种类型的随机ID
- **日志轮转**: 日志文件轮转切割
- **系统托盘图标**: 创建系统托盘图标
- **浮动图标**: 创建浮动图标监控进程
- **时间同步**: 通过NTP同步系统时间
- **TCP通信**: 通过TCP Socket发送数据

## 安装

```bash
# 克隆项目
git clone git@gitee.com:yunluo/spcmd.git

# 使用构建脚本编译
build_final.bat
```

## 使用方法

### 基本语法
```bash
spcmd <command> [parameters]
```

### 主要命令

#### 屏幕截图
```bash
spcmd screenshot [--save=path|base64] [--format=png|bmp|jpg] [--base64=file] [--quality=value]
```

参数说明:
- `--save=path` - 保存截图到指定路径，默认保存为当前目录的 screenshot.bmp
- `--save=base64` - 将截图以 Base64 编码文本输出到控制台
- `--format=png|bmp|jpg` - 保存格式，默认为 bmp
- `--base64=file` - 保存为 Base64 编码数据到指定文件
- `--quality=value` - 图片质量（1-100），默认为 100

示例:
```bash
# 默认截图，保存为 screenshot.bmp
spcmd screenshot

# 保存到指定路径
spcmd screenshot --save=C:\screenshots\screen.png

# 输出 Base64 到控制台
spcmd screenshot --save=base64

# 指定格式为 png
spcmd screenshot --format=png

# 指定格式为 jpg
spcmd screenshot --format=jpg

# 保存为 Base64 文件
spcmd screenshot --base64=screenshot.b64

# 指定图片质量
spcmd screenshot --quality=80

# 组合使用参数
spcmd screenshot --save=screen.jpg --format=jpg --quality=90
spcmd screenshot --save=base64 --format=jpg --quality=90
```
> 需要自己加：data:image/jpeg;base64,
#### 创建快捷方式
```bash
spcmd shortcut --target=path [--name=name] [--desc=description] [--icon=iconpath] [--workdir=dir]
```

参数说明:
- `--target=path` - 目标程序路径（必需）
- `--name=name` - 快捷方式名称，默认为程序名称
- `--desc=description` - 快捷方式描述
- `--icon=iconpath` - 图标路径
- `--workdir=dir` - 工作目录

示例:
```bash
spcmd shortcut --target=C:\Windows\notepad.exe
spcmd shortcut --target=C:\Windows\notepad.exe --name=Notepad --desc=Open Notepad program
```

#### 开机自启配置
```bash
spcmd autorun --target=path [--name=name] [--args=args] [--workdir=dir] [--remove]
```

参数说明:
- `--target=path` - 目标程序路径（必需）
- `--name=name` - 任务名称，默认为程序名称
- `--args=args` - 程序参数
- `--workdir=dir` - 工作目录
- `--remove` - 移除自启配置

示例:
```bash
spcmd autorun --target=C:\MyApp\myapp.exe
spcmd autorun --target=C:\MyApp\myapp.exe --name=MyApp --args="/silent" --workdir=C:\MyApp
spcmd autorun --target=C:\MyApp\myapp.exe --remove
```

#### 信息弹窗
```bash
spcmd infoboxtop --message=message --title=title
spcmd qboxtop --message=message --title=title --program=command
```

参数说明:
- `--message=message` - 要显示的消息文本（必需）
- `--title=title` - 弹窗标题，默认为"信息"
- `--program=command` - 确认后要执行的程序（qboxtop必需）

示例:
```bash
spcmd infoboxtop --message="Hello World" --title="Greeting"
spcmd qboxtop --message="Do you want to run the calculator?" --title="Question" --program="calc.exe"
```

#### 自定义窗口
```bash
spcmd window --text=message [--title=title] [--width=width] [--height=height] [--fontsize=size] [--bgcolor=color] [--textcolor=color] [--bold] [--modal] [--nodrag] [--onclick=command]
```

参数说明:
- `--text=message` - 窗口显示文本（必需）
- `--title=title` - 窗口标题，默认为"SYSTEM INFORMATION"
- `--width=width` - 窗口宽度（像素），默认为600，范围100-2000
- `--height=height` - 窗口高度（像素），默认为400，范围100-2000
- `--fontsize=size` - 字体大小，默认为18，范围8-72
- `--bgcolor=color` - 背景颜色，可以是颜色名称（white,black,red,green,blue,yellow,cyan,magenta,gray,orange,purple,pink,lightblue,lightgreen,lightgray）或RGB值（r,g,b），默认为白色
- `--textcolor=color` - 文字颜色，可以是颜色名称或RGB值，默认为黑色
- `--bold` - 设置文本为粗体
- `--modal` - 设置窗口为模态窗口（阻塞其他窗口直到关闭并启用强制交互）
- `--nodrag` - 禁止窗口拖拽
- `--onclick=command` - 点击确认按钮后执行的命令

示例:
```bash
spcmd window --text="Hello World"
spcmd window --text="Line 1\nLine 2\nLine 3" --title="Multi-line Text" --width=500 --height=300
spcmd window --text="Red background window" --bgcolor=red --fontsize=20
spcmd window --text="Blue text on yellow background" --bgcolor=yellow --textcolor=blue
spcmd window --text="Bold text example" --bold
spcmd window --text="Modal window with forced interaction" --modal
spcmd window --text="你好，世界！" --fontsize=24
spcmd window --text="你好，世界！" --textcolor=blue
spcmd window --text="确认后打开计算器" --onclick="calc.exe"
spcmd window --text="确认后执行备份" --onclick="backup.bat"
```

#### 程序执行
```bash
spcmd process --action=run --exec=command [--workdir=working_directory]
```

参数说明:
- `--action=run` - 执行程序的操作
- `--exec=command` - 要运行的应用程序及可选的命令行参数（必需）
- `--workdir=working_directory` - 应用程序的工作目录

示例:
```bash
spcmd process --action=run --exec=calc.exe --workdir="C:\Windows\System32"
spcmd process --action=run --exec="notepad.exe test.txt" --workdir=C:\temp
```

#### 计划任务
```bash
spcmd task --name=task_name --exec=program_path [--trigger=daily|weekly|monthly] [--starttime=HH:MM] [--startdate=YYYY-MM-DD]
```

参数说明:
- `--name=task_name` - 任务名称（必需）
- `--exec=program_path` - 要执行的程序（必需）
- `--trigger=schedule` - 触发器类型：daily, weekly, monthly（默认：daily）
- `--starttime=HH:MM` - 开始时间（24小时格式，默认：09:00）
- `--startdate=YYYY-MM-DD` - 开始日期（默认：今天）- 仅在Windows Vista及更高版本上可用

示例:
```bash
spcmd task --name="Daily Backup" --exec="C:\Windows\system32\cmd.exe" --trigger=daily
spcmd task --name="Weekly Cleanup" --exec="C:\temp\cleanup.bat" --trigger=weekly --starttime=22:00
spcmd task --name="Monthly Report" --exec="C:\reports\generate.exe" --trigger=monthly --starttime=08:00 --startdate=2025-12-01
```

#### 进程重启
```bash
spcmd restart --path=process_path [--workdir=working_directory]
```

参数说明:
- `--path=process_path` - 进程可执行文件路径（必需）
- `--workdir=working_dir` - 进程的工作目录（可选）

示例:
```bash
spcmd restart --path="C:\Windows\notepad.exe"
spcmd restart --path="C:\Program Files\MyApp\myapp.exe"
spcmd restart --path="C:\MyApp\myapp.exe" --workdir="C:\MyApp"
```

#### 系统通知
```bash
spcmd notify --title=title --message=message [--icon=info|warning|error] [--timeout=seconds]
```

参数说明:
- `--title=title` - 通知标题（必需）
- `--message=message` - 通知内容（必需）
- `--icon=type` - 图标类型：info, warning, error（默认：info）
- `--timeout=seconds` - 通知超时时间（秒，默认：5）

示例:
```bash
spcmd notify --title="System Update" --message="Your system has been updated successfully."
spcmd notify --title="Warning" --message="Disk space is low." --icon=warning
spcmd notify --title="Error" --message="Application failed to start." --icon=error --timeout=10
```

#### 配置文件管理
```bash
spcmd config --file=path [--action=get|set|save|del] [--section=section] [--key=key] [--value=value]
```

参数说明:
- `--file=path` - INI文件路径（必需）
- `--action=action` - 操作：get, set, save, del（默认：get）
- `--section=section` - 配置节（get/set/del操作必需）
- `--key=key` - 配置键（get/set/del操作必需）
- `--value=value` - 配置值（set操作必需）

示例:
```bash
spcmd config --file="C:\config.ini" --action=get --section="Settings" --key="Username"
spcmd config --file="C:\config.ini" --action=set --section="Settings" --key="Username" --value="JohnDoe"
spcmd config --file="C:\config.ini" --action=del --section="Settings" --key="TempKey"
spcmd config --file="C:\config.ini" --action=save
```

#### 进程管理
```bash
spcmd process [--action=check|kill] [--name=process_name] [--pid=process_id]
```

参数说明:
- `--action=action` - 操作：check, kill（默认：check）
- `--name=process_name` - 进程名称
- `--pid=process_id` - 进程ID

示例:
```bash
spcmd process --name=notepad.exe
spcmd process --pid=1234
spcmd process --action=kill --name=notepad.exe
```

注意：当使用 check 操作时，如果指定进程存在，程序将返回 0 并输出进程信息；如果进程不存在，将返回 1。对于同名的多个进程，将返回找到的进程数量。

#### 随机数生成
```bash
spcmd random [--type=uuid4|uuid7|snowflake|number]
```

参数说明:
- `--type=type` - 随机ID类型：uuid4, uuid7, snowflake, number（默认：uuid4）
  - uuid4: 生成UUID版本4（随机UUID）
  - uuid7: 生成UUID版本7（时间戳UUID）
  - snowflake: 生成雪花ID（分布式ID）
  - number: 生成随机整数

示例:
```bash
spcmd random
spcmd random --type=uuid7
spcmd random --type=snowflake
spcmd random --type=number
```

#### 日志轮转
```bash
spcmd logrotate [--path=path_to_log_file] [--maxsize=size_in_mb] [--daily]
```

参数说明:
- `--path=path_to_log_file` - 日志文件路径，默认为"app.log"
- `--maxsize=size_in_mb` - 轮转前的最大大小（MB，默认：10，最小值：1）
- `--daily` - 按天轮转（默认：按大小轮转）

说明:
- 按大小轮转：当日志文件大小超过指定阈值时进行轮转
- 按天轮转：每天创建一个新的日志备份文件

示例:
```bash
spcmd logrotate --path="C:\Logs\app.log" --maxsize=50
spcmd logrotate --path="C:\Logs\app.log" --daily
```

#### 系统托盘图标
```bash
spcmd tray --process=process_name [--title=title] [--icon=icon_path] [--path=process_path] [--menu="name,command"]
```

参数说明:
- `--process=process_name` - 要监控的进程名称（必需）
- `--title=title` - 托盘图标标题，默认为"SPCMD Tray"
- `--icon=icon_path` - 图标文件路径
- `--path=process_path` - 进程路径（可自动检测进程名和图标）
- `--menu="name,command"` - 菜单项定义，格式为"名称,命令"，可多次使用以添加多个菜单项

说明:
- 如果指定了`--path`参数，程序将自动从路径中提取进程名称和图标
- 菜单项将在右键点击托盘图标时显示
- 当监控的进程退出时，托盘图标也会自动消失

示例:
```bash
spcmd tray --process=notepad.exe
spcmd tray --process=myapp.exe --title="My Application" --icon="C:\MyApp\icon.ico"
spcmd tray --path="C:\MyApp\myapp.exe"
spcmd tray --process=python.exe --menu="Open Notepad,notepad.exe" --menu="Open Calculator,calc.exe"
spcmd tray --process=explorer.exe --menu="Open Logs,explorer.exe .\logs" --menu="Open Command Prompt,cmd.exe"
```

#### 浮动图标
```bash
spcmd floating --process=process_name [--title=title] [--icon=icon_path] [--path=process_path] [--menu="name,command"]
```

参数说明:
- `--process=process_name` - 要监控的进程名称（必需）
- `--title=title` - 浮动图标标题，默认为"SPCMD Floating"
- `--icon=icon_path` - 图标文件路径
- `--path=process_path` - 进程路径（可自动检测进程名和图标）
- `--menu="name,command"` - 菜单项定义，格式为"名称,命令"，可多次使用以添加多个菜单项

说明:
- 浮动图标会在屏幕中央显示一个图标，实时监控指定进程的状态
- 如果指定了`--path`参数，程序将自动从路径中提取进程名称和图标
- 菜单项将在右键点击浮动图标时显示
- 当监控的进程退出时，浮动图标也会自动消失

示例:
```bash
spcmd floating --process=notepad.exe
spcmd floating --process=myapp.exe --title="My Application" --icon="C:\MyApp\icon.ico"
spcmd floating --path="C:\MyApp\myapp.exe"
spcmd floating --process=python.exe --menu="Open Notepad,notepad.exe" --menu="Open Calculator,calc.exe"
spcmd floating --process=explorer.exe --menu="Open Logs,explorer.exe .\logs" --menu="Open Command Prompt,cmd.exe"
```

#### 时间同步
```bash
spcmd timesync [--server=ntp_server]
```

参数说明:
- `--server=ntp_server` - NTP服务器地址（默认：time.windows.com）

示例:
```bash
spcmd timesync
spcmd timesync --server=time.windows.com
spcmd timesync --server=192.168.1.1
```

#### TCP Socket通信
```bash
spcmd ipc --host=ip --port=port --value=data
```

参数说明:
- `--host=ip` - 目标IP地址（默认：127.0.0.1）
- `--port=port` - 目标端口（必需）
- `--value=data` - 要发送的数据（必需）

示例:
```bash
spcmd ipc --host=127.0.0.1 --port=9999 --value=hello
spcmd ipc --host=192.168.1.100 --port=8080 --value=test
```

## 示例

```bash
# 创建屏幕截图
spcmd screenshot --save=screenshot.png --format=png

# 创建记事本快捷方式
spcmd shortcut --target=C:\Windows\notepad.exe --name=Notepad

# 显示信息弹窗
spcmd infoboxtop --message="Hello World" --title="Greeting"

# 创建自定义窗口
spcmd window --text="Welcome to SPCMD!" --title="SPCMD Window" --bgcolor=lightblue --textcolor=blue --bold

# 创建窗口并点击确认后执行命令
spcmd window --text="确认后打开计算器" --onclick="calc.exe"

# 执行程序
spcmd process --action=run --exec="notepad.exe test.txt" --workdir=C:\temp

# 生成随机UUID
spcmd random --type=uuid4

# 检查进程是否存在
spcmd process --name=notepad.exe

# 杀死进程
spcmd process --action=kill --name=notepad.exe

# 创建系统托盘图标
spcmd tray --process=notepad.exe --title="Notepad Monitor"

# 创建浮动图标
spcmd floating --process=notepad.exe --title="Notepad Monitor"

# 时间同步
spcmd timesync --server=time.windows.com

# TCP通信
spcmd ipc --host=127.0.0.1 --port=9999 --value=hello
```

## 构建

使用提供的构建脚本确保兼容性：

```bash
build_final.bat
```

## 系统要求

- Windows XP或更高版本
- 支持32位和64位系统

## 许可证

MIT License

Copyright (c) 2025 SPCMD Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## 更新日志

### v7.5.0.0 (2026-01)
- **代码精简**
  - 完全移除剪贴板功能及相关代码
  - 清理所有剪贴板相关引用

### v7.4.0.0 (2026-01)
- **新增IPC进程间通信功能**
  - 环境变量方式（setenv/getenv）
  - 命名管道方式（pipe）
  - TCP Socket方式（send）
- **自定义窗口新增--onclick参数**
  - 点击确认按钮后执行自定义命令
  - 命令静默执行，不显示黑窗口
- **INI配置文件支持UTF-8 BOM编码**
- **修复多项Bug**
  - 修复开机自启权限提升卡死问题
  - 修复窗口创建失败资源泄漏
  - 修复各种潜在内存泄漏
- **代码优化**
  - 清理死代码
  - 修复编译警告
- **文档更新**
- **功能精简**
  - 移除剪贴板功能（避免安全软件误报）
  - IPC通信简化为纯TCP方式

### v7.3.0.0
- 初始版本发布
- 基础功能完善