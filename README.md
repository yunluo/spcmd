# SPCMD - System Power Command Tool

SPCMD是一个功能强大的Windows系统命令行工具，提供了多种系统管理和用户交互功能。

## 功能特性

- **屏幕截图**: 捕获屏幕截图并保存为多种格式
- **快捷方式创建**: 创建桌面快捷方式
- **开机自启配置**: 配置程序开机自动启动
- **信息弹窗**: 显示各种类型的信息弹窗
- **自定义窗口**: 创建具有高级功能的自定义窗口
- **程序执行**: 执行应用程序并指定工作目录
- **系统服务**: 创建Windows系统服务
- **计划任务**: 创建Windows计划任务
- **进程重启**: 重启指定的进程

## 安装

```bash
# 克隆项目
git clone <repository-url>

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
spcmd screenshot [--save=path] [--fullscreen] [--active] [--format=png|bmp] [--base64=file] [--quality=value]
```

#### 创建快捷方式
```bash
spcmd shortcut --target=path [--name=name] [--args=args] [--workdir=dir] [--icon=path]
```

#### 开机自启配置
```bash
spcmd autorun --target=path [--name=name] [--args=args] [--workdir=dir] [--remove]
```

#### 信息弹窗
```bash
spcmd infobox "message" ["title"]
spcmd infoboxtop "message" ["title"]
spcmd qbox "message" ["title"] "command"
spcmd qboxtop "message" ["title"] "command"
```

#### 自定义窗口
```bash
spcmd window --text=message [--title=title] [--width=width] [--height=height] [--fontsize=size] [--bgcolor=color] [--textcolor=color] [--bold] [--modal] [--nodrag]
```

#### 程序执行
```bash
spcmd exec2 show|hide "workdir" "command" [args]
```

#### 系统服务
```bash
spcmd service --name=servicename --target=path [--args=args] [--workdir=dir] [--remove] [--start] [--stop]
```

#### 计划任务
```bash
spcmd task --name=taskname --target=path [--args=args] [--workdir=dir] [--remove] [--schedule=time]
```

#### 进程重启
```bash
spcmd restart processname
```

## 示例

```bash
# 创建屏幕截图
spcmd screenshot --save=screenshot.png --format=png

# 创建记事本快捷方式
spcmd shortcut --target=C:\Windows\notepad.exe --name=Notepad

# 显示信息弹窗
spcmd infobox "Hello World" "Greeting"

# 创建自定义窗口
spcmd window --text="Welcome to SPCMD!" --title="SPCMD Window" --bgcolor=lightblue --textcolor=blue --bold

# 执行程序
spcmd exec2 show "C:\temp" "notepad.exe" "test.txt"

# 创建系统服务
spcmd service --name=MyService --target=C:\MyApp\app.exe --start
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

[待定]