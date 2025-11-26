#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
SPCMD Python Wrapper
用于调用SPCMD工具的Python包装器
"""

import subprocess
import sys
import os

def run_spcmd_command(command_args):
    """
    运行SPCMD命令
    
    Args:
        command_args (list): 命令参数列表
    
    Returns:
        str: 命令输出
    """
    # 获取当前脚本所在目录
    current_dir = os.path.dirname(os.path.abspath(__file__))
    spcmd_path = os.path.join(current_dir, "spcmd.exe")
    
    # 检查SPCMD可执行文件是否存在
    if not os.path.exists(spcmd_path):
        raise FileNotFoundError(f"SPCMD executable not found at {spcmd_path}")
    
    # 构建命令
    cmd = [spcmd_path] + command_args
    
    try:
        # 执行命令并获取输出
        result = subprocess.run(cmd, capture_output=True, text=True, check=True, encoding='utf-8')
        return result.stdout
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Failed to execute command: {e.stderr}")

def generate_random_ids(id_type="uuid4", count=1):
    """
    调用SPCMD生成随机ID
    
    Args:
        id_type (str): ID类型 (uuid4, uuid7, snowflake, number)
        count (int): 生成数量
    
    Returns:
        list: 生成的ID列表
    """
    # 构建命令
    cmd = ["random", f"--type={id_type}"]
    
    result = run_spcmd_command(cmd)
    # 分割输出为列表并去除空行
    ids = [line.strip() for line in result.strip().split('\n') if line.strip()]
    return ids

def generate_uuid4(count=1):
    """
    生成UUID v4
    
    Args:
        count (int): 生成数量
    
    Returns:
        list: 生成的UUID v4列表
    """
    return generate_random_ids("uuid4", count)

def generate_uuid7(count=1):
    """
    生成UUID v7
    
    Args:
        count (int): 生成数量
    
    Returns:
        list: 生成的UUID v7列表
    """
    return generate_random_ids("uuid7", count)

def generate_snowflake_ids(count=1):
    """
    生成雪花ID
    
    Args:
        count (int): 生成数量
    
    Returns:
        list: 生成的雪花ID列表
    """
    return generate_random_ids("snowflake", count)

def generate_random_numbers(count=1):
    """
    生成随机数
    
    Args:
        count (int): 生成数量
    
    Returns:
        list: 生成的随机数列表
    """
    return generate_random_ids("number", count)

def take_screenshot(save_path=None, format_type="bmp", quality=100):
    """
    截图命令
    
    Args:
        save_path (str): 保存路径
        format_type (str): 图片格式 (bmp, png, jpg)
        quality (int): 图片质量 (1-100)
    
    Returns:
        str: 截图结果
    """
    cmd = ["screenshot"]
    if save_path:
        cmd.append(f"--save={save_path}")
    cmd.append(f"--format={format_type}")
    cmd.append(f"--quality={quality}")
    
    return run_spcmd_command(cmd)

def create_shortcut(target, name=None, description=None, icon=None, workdir=None):
    """
    创建快捷方式
    
    Args:
        target (str): 目标程序路径
        name (str): 快捷方式名称
        description (str): 描述
        icon (str): 图标路径
        workdir (str): 工作目录
    
    Returns:
        str: 创建结果
    """
    cmd = ["shortcut", f"--target={target}"]
    if name:
        cmd.append(f"--name={name}")
    if description:
        cmd.append(f"--desc={description}")
    if icon:
        cmd.append(f"--icon={icon}")
    if workdir:
        cmd.append(f"--workdir={workdir}")
    
    return run_spcmd_command(cmd)

def configure_autorun(target, name=None, args=None, workdir=None, remove=False):
    """
    配置开机自启
    
    Args:
        target (str): 目标程序路径
        name (str): 任务名称
        args (str): 程序参数
        workdir (str): 工作目录
        remove (bool): 是否移除自启配置
    
    Returns:
        str: 配置结果
    """
    cmd = ["autorun", f"--target={target}"]
    if name:
        cmd.append(f"--name={name}")
    if args:
        cmd.append(f"--args={args}")
    if workdir:
        cmd.append(f"--workdir={workdir}")
    if remove:
        cmd.append("--remove")
    
    return run_spcmd_command(cmd)

def show_infobox(message, title="信息"):
    """
    显示信息框
    
    Args:
        message (str): 消息内容
        title (str): 标题
    
    Returns:
        str: 显示结果
    """
    cmd = ["infoboxtop", message, title]
    return run_spcmd_command(cmd)

def show_question_box(message, title="问题", program=None):
    """
    显示问题对话框
    
    Args:
        message (str): 问题内容
        title (str): 标题
        program (str): 确认后要执行的程序
    
    Returns:
        str: 显示结果
    """
    cmd = ["qboxtop", message, title]
    if program:
        cmd.append(program)
    return run_spcmd_command(cmd)

def show_custom_window(text, title="系统提示", width=600, height=400, fontsize=18, 
                      bgcolor="white", textcolor="black", font=None, bold=False, 
                      modal=False, nodrag=False):
    """
    显示自定义窗口
    
    Args:
        text (str): 窗口文本
        title (str): 窗口标题
        width (int): 窗口宽度
        height (int): 窗口高度
        fontsize (int): 字体大小
        bgcolor (str): 背景颜色
        textcolor (str): 文字颜色
        font (str): 字体名称
        bold (bool): 是否粗体
        modal (bool): 是否模态窗口
        nodrag (bool): 是否禁止拖拽
    
    Returns:
        str: 显示结果
    """
    cmd = ["window", f"--text={text}", f"--title={title}", 
           f"--width={width}", f"--height={height}", f"--fontsize={fontsize}",
           f"--bgcolor={bgcolor}", f"--textcolor={textcolor}"]
    
    if font:
        cmd.append(f"--font={font}")
    if bold:
        cmd.append("--bold")
    if modal:
        cmd.append("--modal")
    if nodrag:
        cmd.append("--nodrag")
    
    return run_spcmd_command(cmd)

def execute_program(window_state, working_folder, application, args=None):
    """
    执行程序
    
    Args:
        window_state (str): 窗口状态 (show/hide/min/max)
        working_folder (str): 工作目录
        application (str): 应用程序路径
        args (str): 程序参数
    
    Returns:
        str: 执行结果
    """
    cmd = ["exec2", window_state, working_folder, application]
    if args:
        cmd.append(args)
    return run_spcmd_command(cmd)

def create_task(name, exec_path, trigger="daily", starttime="09:00", startdate=None):
    """
    创建计划任务
    
    Args:
        name (str): 任务名称
        exec_path (str): 执行程序路径
        trigger (str): 触发器类型 (daily, weekly, monthly)
        starttime (str): 开始时间 (HH:MM)
        startdate (str): 开始日期 (YYYY-MM-DD)
    
    Returns:
        str: 创建结果
    """
    cmd = ["task", f"--name={name}", f"--exec={exec_path}", 
           f"--trigger={trigger}", f"--starttime={starttime}"]
    
    if startdate:
        cmd.append(f"--startdate={startdate}")
    
    return run_spcmd_command(cmd)

def restart_process(path, workdir=None):
    """
    重启进程
    
    Args:
        path (str): 进程可执行文件路径
        workdir (str): 工作目录
    
    Returns:
        str: 重启结果
    """
    cmd = ["restart", f"--path={path}"]
    if workdir:
        cmd.append(f"--workdir={workdir}")
    
    return run_spcmd_command(cmd)

def show_notification(title, message, icon="info", timeout=5):
    """
    显示系统通知
    
    Args:
        title (str): 通知标题
        message (str): 通知内容
        icon (str): 图标类型 (info, warning, error)
        timeout (int): 超时时间(秒)
    
    Returns:
        str: 显示结果
    """
    cmd = ["notify", f"--title={title}", f"--message={message}", 
           f"--icon={icon}", f"--timeout={timeout}"]
    
    return run_spcmd_command(cmd)

def manage_config(file_path, action="get", section=None, key=None, value=None):
    """
    管理配置文件
    
    Args:
        file_path (str): 配置文件路径
        action (str): 操作类型 (get, set, save, del)
        section (str): 配置节
        key (str): 配置键
        value (str): 配置值
    
    Returns:
        str: 操作结果
    """
    cmd = ["config", f"--file={file_path}", f"--action={action}"]
    
    if section:
        cmd.append(f"--section={section}")
    if key:
        cmd.append(f"--key={key}")
    if value:
        cmd.append(f"--value={value}")
    
    return run_spcmd_command(cmd)

def check_process(name=None, pid=None):
    """
    检查进程
    
    Args:
        name (str): 进程名称
        pid (int): 进程ID
    
    Returns:
        str: 检查结果
    """
    cmd = ["process"]
    if name:
        cmd.append(f"--name={name}")
    elif pid:
        cmd.append(f"--pid={pid}")
    
    return run_spcmd_command(cmd)

def kill_process(name=None, pid=None):
    """
    终止进程
    
    Args:
        name (str): 进程名称
        pid (int): 进程ID
    
    Returns:
        str: 终止结果
    """
    cmd = ["process", "--action=kill"]
    if name:
        cmd.append(f"--name={name}")
    elif pid:
        cmd.append(f"--pid={pid}")
    
    return run_spcmd_command(cmd)

def rotate_log(file_path, maxsize=None, daily=False):
    """
    日志轮转
    
    Args:
        file_path (str): 日志文件路径
        maxsize (int): 最大大小(MB)
        daily (bool): 是否按天轮转
    
    Returns:
        str: 轮转结果
    """
    cmd = ["logrotate", f"--path={file_path}"]
    if maxsize:
        cmd.append(f"--maxsize={maxsize}")
    if daily:
        cmd.append("--daily")
    
    return run_spcmd_command(cmd)

def create_tray_icon(process_name, title=None, icon_path=None):
    """
    创建系统托盘图标
    
    Args:
        process_name (str): 进程名称
        title (str): 托盘图标标题
        icon_path (str): 图标路径
    
    Returns:
        str: 创建结果
    """
    cmd = ["tray", f"--process={process_name}"]
    if title:
        cmd.append(f"--title={title}")
    if icon_path:
        cmd.append(f"--icon={icon_path}")
    
    return run_spcmd_command(cmd)

# 命令行接口
def main():
    """命令行接口"""
    if len(sys.argv) < 2:
        print("Usage: python spcmd_wrapper.py <command> [options]")
        print("Commands:")
        print("  random [--type=TYPE] [--count=COUNT]")
        print("    TYPE: uuid4, uuid7, snowflake, number (default: uuid4)")
        print("    COUNT: number of IDs to generate (default: 1)")
        print("")
        print("  screenshot [--save=PATH] [--format=FORMAT] [--quality=QUALITY]")
        print("    FORMAT: bmp, png, jpg (default: bmp)")
        print("    QUALITY: 1-100 (default: 100)")
        print("")
        print("  shortcut --target=PATH [--name=NAME] [--desc=DESC] [--icon=ICON] [--workdir=DIR]")
        print("")
        print("  autorun --target=PATH [--name=NAME] [--args=ARGS] [--workdir=DIR] [--remove]")
        print("")
        print("  infobox MESSAGE [TITLE]")
        print("")
        print("  qbox MESSAGE [TITLE] [PROGRAM]")
        print("")
        print("  window --text=TEXT [--title=TITLE] [--width=WIDTH] [--height=HEIGHT]")
        print("         [--fontsize=SIZE] [--bgcolor=COLOR] [--textcolor=COLOR]")
        print("         [--font=FONT] [--bold] [--modal] [--nodrag]")
        print("")
        print("  exec2 STATE FOLDER APPLICATION [ARGS]")
        print("    STATE: show, hide, min, max")
        print("")
        print("  task --name=NAME --exec=PATH [--trigger=TYPE] [--starttime=TIME] [--startdate=DATE]")
        print("    TYPE: daily, weekly, monthly (default: daily)")
        print("    TIME: HH:MM (default: 09:00)")
        print("")
        print("  restart --path=PATH [--workdir=DIR]")
        print("")
        print("  notify --title=TITLE --message=MESSAGE [--icon=TYPE] [--timeout=SECONDS]")
        print("    TYPE: info, warning, error (default: info)")
        print("    SECONDS: timeout in seconds (default: 5)")
        print("")
        print("  config --file=PATH --action=ACTION [--section=SECTION] [--key=KEY] [--value=VALUE]")
        print("    ACTION: get, set, save, del")
        print("")
        print("  process [--action=ACTION] [--name=NAME] [--pid=PID]")
        print("    ACTION: check, kill (default: check)")
        print("")
        print("  logrotate --path=PATH [--maxsize=SIZE] [--daily]")
        print("    SIZE: maximum size in MB")
        print("")
        print("  tray --process=NAME [--title=TITLE] [--icon=ICON]")
        return
    
    command = sys.argv[1]
    
    if command == "random":
        # 解析参数
        id_type = "uuid4"
        count = 1
        
        for arg in sys.argv[2:]:
            if arg.startswith("--type="):
                id_type = arg.split("=", 1)[1]
            elif arg.startswith("--count="):
                count = int(arg.split("=", 1)[1])
        
        try:
            ids = generate_random_ids(id_type, count)
            for id_val in ids:
                print(id_val)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "screenshot":
        # 解析参数
        save_path = None
        format_type = "bmp"
        quality = 100
        
        for arg in sys.argv[2:]:
            if arg.startswith("--save="):
                save_path = arg.split("=", 1)[1]
            elif arg.startswith("--format="):
                format_type = arg.split("=", 1)[1]
            elif arg.startswith("--quality="):
                quality = int(arg.split("=", 1)[1])
        
        try:
            result = take_screenshot(save_path, format_type, quality)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "shortcut":
        # 解析参数
        target = None
        name = None
        description = None
        icon = None
        workdir = None
        
        for arg in sys.argv[2:]:
            if arg.startswith("--target="):
                target = arg.split("=", 1)[1]
            elif arg.startswith("--name="):
                name = arg.split("=", 1)[1]
            elif arg.startswith("--desc="):
                description = arg.split("=", 1)[1]
            elif arg.startswith("--icon="):
                icon = arg.split("=", 1)[1]
            elif arg.startswith("--workdir="):
                workdir = arg.split("=", 1)[1]
        
        if not target:
            print("Error: --target is required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = create_shortcut(target, name, description, icon, workdir)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "autorun":
        # 解析参数
        target = None
        name = None
        args = None
        workdir = None
        remove = False
        
        for arg in sys.argv[2:]:
            if arg.startswith("--target="):
                target = arg.split("=", 1)[1]
            elif arg.startswith("--name="):
                name = arg.split("=", 1)[1]
            elif arg.startswith("--args="):
                args = arg.split("=", 1)[1]
            elif arg.startswith("--workdir="):
                workdir = arg.split("=", 1)[1]
            elif arg == "--remove":
                remove = True
        
        if not target:
            print("Error: --target is required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = configure_autorun(target, name, args, workdir, remove)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "infobox":
        if len(sys.argv) < 3:
            print("Error: message is required", file=sys.stderr)
            sys.exit(1)
        
        message = sys.argv[2]
        title = sys.argv[3] if len(sys.argv) > 3 else "信息"
        
        try:
            result = show_infobox(message, title)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "qbox":
        if len(sys.argv) < 3:
            print("Error: message is required", file=sys.stderr)
            sys.exit(1)
        
        message = sys.argv[2]
        title = sys.argv[3] if len(sys.argv) > 3 else "问题"
        program = sys.argv[4] if len(sys.argv) > 4 else None
        
        try:
            result = show_question_box(message, title, program)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "window":
        # 解析参数
        text = None
        title = "系统提示"
        width = 600
        height = 400
        fontsize = 18
        bgcolor = "white"
        textcolor = "black"
        font = None
        bold = False
        modal = False
        nodrag = False
        
        for arg in sys.argv[2:]:
            if arg.startswith("--text="):
                text = arg.split("=", 1)[1]
            elif arg.startswith("--title="):
                title = arg.split("=", 1)[1]
            elif arg.startswith("--width="):
                width = int(arg.split("=", 1)[1])
            elif arg.startswith("--height="):
                height = int(arg.split("=", 1)[1])
            elif arg.startswith("--fontsize="):
                fontsize = int(arg.split("=", 1)[1])
            elif arg.startswith("--bgcolor="):
                bgcolor = arg.split("=", 1)[1]
            elif arg.startswith("--textcolor="):
                textcolor = arg.split("=", 1)[1]
            elif arg.startswith("--font="):
                font = arg.split("=", 1)[1]
            elif arg == "--bold":
                bold = True
            elif arg == "--modal":
                modal = True
            elif arg == "--nodrag":
                nodrag = True
        
        if not text:
            print("Error: --text is required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = show_custom_window(text, title, width, height, fontsize, bgcolor, textcolor, font, bold, modal, nodrag)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "exec2":
        if len(sys.argv) < 5:
            print("Error: window_state, working_folder, and application are required", file=sys.stderr)
            sys.exit(1)
        
        window_state = sys.argv[2]
        working_folder = sys.argv[3]
        application = sys.argv[4]
        args = sys.argv[5] if len(sys.argv) > 5 else None
        
        try:
            result = execute_program(window_state, working_folder, application, args)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "task":
        # 解析参数
        name = None
        exec_path = None
        trigger = "daily"
        starttime = "09:00"
        startdate = None
        
        for arg in sys.argv[2:]:
            if arg.startswith("--name="):
                name = arg.split("=", 1)[1]
            elif arg.startswith("--exec="):
                exec_path = arg.split("=", 1)[1]
            elif arg.startswith("--trigger="):
                trigger = arg.split("=", 1)[1]
            elif arg.startswith("--starttime="):
                starttime = arg.split("=", 1)[1]
            elif arg.startswith("--startdate="):
                startdate = arg.split("=", 1)[1]
        
        if not name or not exec_path:
            print("Error: --name and --exec are required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = create_task(name, exec_path, trigger, starttime, startdate)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "restart":
        # 解析参数
        path = None
        workdir = None
        
        for arg in sys.argv[2:]:
            if arg.startswith("--path="):
                path = arg.split("=", 1)[1]
            elif arg.startswith("--workdir="):
                workdir = arg.split("=", 1)[1]
        
        if not path:
            print("Error: --path is required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = restart_process(path, workdir)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "notify":
        # 解析参数
        title = None
        message = None
        icon = "info"
        timeout = 5
        
        for arg in sys.argv[2:]:
            if arg.startswith("--title="):
                title = arg.split("=", 1)[1]
            elif arg.startswith("--message="):
                message = arg.split("=", 1)[1]
            elif arg.startswith("--icon="):
                icon = arg.split("=", 1)[1]
            elif arg.startswith("--timeout="):
                timeout = int(arg.split("=", 1)[1])
        
        if not title or not message:
            print("Error: --title and --message are required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = show_notification(title, message, icon, timeout)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "config":
        # 解析参数
        file_path = None
        action = "get"
        section = None
        key = None
        value = None
        
        for arg in sys.argv[2:]:
            if arg.startswith("--file="):
                file_path = arg.split("=", 1)[1]
            elif arg.startswith("--action="):
                action = arg.split("=", 1)[1]
            elif arg.startswith("--section="):
                section = arg.split("=", 1)[1]
            elif arg.startswith("--key="):
                key = arg.split("=", 1)[1]
            elif arg.startswith("--value="):
                value = arg.split("=", 1)[1]
        
        if not file_path:
            print("Error: --file is required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = manage_config(file_path, action, section, key, value)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "process":
        # 解析参数
        action = "check"
        name = None
        pid = None
        
        for arg in sys.argv[2:]:
            if arg.startswith("--action="):
                action = arg.split("=", 1)[1]
            elif arg.startswith("--name="):
                name = arg.split("=", 1)[1]
            elif arg.startswith("--pid="):
                pid = int(arg.split("=", 1)[1])
        
        try:
            if action == "check":
                result = check_process(name, pid)
                print(result)
            elif action == "kill":
                result = kill_process(name, pid)
                print(result)
            else:
                print(f"Error: Unknown action {action}", file=sys.stderr)
                sys.exit(1)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "logrotate":
        # 解析参数
        file_path = None
        maxsize = None
        daily = False
        
        for arg in sys.argv[2:]:
            if arg.startswith("--path="):
                file_path = arg.split("=", 1)[1]
            elif arg.startswith("--maxsize="):
                maxsize = int(arg.split("=", 1)[1])
            elif arg == "--daily":
                daily = True
        
        if not file_path:
            print("Error: --path is required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = rotate_log(file_path, maxsize, daily)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    elif command == "tray":
        # 解析参数
        process_name = None
        title = None
        icon_path = None
        
        for arg in sys.argv[2:]:
            if arg.startswith("--process="):
                process_name = arg.split("=", 1)[1]
            elif arg.startswith("--title="):
                title = arg.split("=", 1)[1]
            elif arg.startswith("--icon="):
                icon_path = arg.split("=", 1)[1]
        
        if not process_name:
            print("Error: --process is required", file=sys.stderr)
            sys.exit(1)
        
        try:
            result = create_tray_icon(process_name, title, icon_path)
            print(result)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        print(f"Unknown command: {command}")
        sys.exit(1)

if __name__ == "__main__":
    main()