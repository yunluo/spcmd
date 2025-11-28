#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
SPCMD Python Wrapper
===================
用于调用SPCMD工具的Python包装器
"""

import subprocess
import sys
import os

class SPCMD:
    """
    SPCMD工具的面向对象包装器
    
    用于调用SPCMD工具的Python面向对象包装器，支持Python 3.4+。
    """
    
    def __init__(self, spcmd_path: str = None):
        """
        初始化SPCMD包装器
        
        :param spcmd_path: SPCMD可执行文件路径，默认为当前目录下的spcmd.exe
        """
        if spcmd_path:
            self.spcmd_path = spcmd_path
        else:
            # 获取当前脚本所在目录
            current_dir = os.path.dirname(os.path.abspath(__file__))
            self.spcmd_path = os.path.join(current_dir, "spcmd.exe")
        
        # 检查SPCMD可执行文件是否存在
        if not os.path.exists(self.spcmd_path):
            raise FileNotFoundError("SPCMD executable not found at {}".format(self.spcmd_path))
    
    def run_command(self, command_args: list, wait: bool = True) -> str:
        """
        运行SPCMD命令
        
        :param command_args: 命令参数列表
        :param wait: 是否等待命令执行完成
        :return: 命令输出（仅当wait=True时）
        """
        # 构建命令
        cmd = [self.spcmd_path] + command_args
        
        try:
            if wait:
                # 执行命令并获取输出（需要等待）
                process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=False, encoding='utf-8')
                stdout, stderr = process.communicate()
                
                if process.returncode != 0:
                    raise RuntimeError("Failed to execute command: {}".format(stderr))
                
                return stdout
            else:
                # 执行命令但不等待（异步执行）
                subprocess.Popen(cmd, shell=False, close_fds=True)
                return ""
        except Exception as e:
            raise RuntimeError("Failed to execute command: {}".format(e))
    
    def generate_random_ids(self, id_type: str = "uuid4", count: int = 1) -> list:
        """
        调用SPCMD生成随机ID
        
        :param id_type: ID类型 (uuid4, uuid7, snowflake, number)
        :param count: 生成数量
        :return: 生成的ID列表
        """
        # 构建命令
        cmd = ["random", "--type={}".format(id_type)]
        
        result = self.run_command(cmd)
        # 分割输出为列表并去除空行
        ids = [line.strip() for line in result.strip().split('\n') if line.strip()]
        return ids
    
    def generate_uuid4(self, count: int = 1) -> list:
        """
        生成UUID v4
        
        :param count: 生成数量
        :return: 生成的UUID v4列表
        """
        return self.generate_random_ids("uuid4")
    
    def generate_uuid7(self, count: int = 1) -> list:
        """
        生成UUID v7
        
        :param count: 生成数量
        :return: 生成的UUID v7列表
        """
        return self.generate_random_ids("uuid7")
    
    def generate_snowflake_ids(self, count: int = 1) -> list:
        """
        生成雪花ID
        
        :param count: 生成数量
        :return: 生成的雪花ID列表
        """
        return self.generate_random_ids("snowflake")
    
    def generate_random_numbers(self, count: int = 1) -> list:
        """
        生成随机数
        
        :param count: 生成数量
        :return: 生成的随机数列表
        """
        return self.generate_random_ids("number")
    
    def take_screenshot(self, save_path: str = None, format_type: str = "bmp", quality: int = 100) -> str:
        """
        截图命令
        
        :param save_path: 保存路径
        :param format_type: 图片格式 (bmp, png, jpg)
        :param quality: 图片质量 (1-100)
        :return: 截图结果
        """
        cmd = ["screenshot"]
        if save_path:
            cmd.append("--save={}".format(save_path))
        cmd.append("--format={}".format(format_type))
        cmd.append("--quality={}".format(quality))
        
        return self.run_command(cmd)
    
    def create_shortcut(self, target: str, name: str = None, description: str = None, icon: str = None, workdir: str = None) -> str:
        """
        创建快捷方式
        
        :param target: 目标程序路径
        :param name: 快捷方式名称
        :param description: 描述
        :param icon: 图标路径
        :param workdir: 工作目录
        :return: 创建结果
        """
        cmd = ["shortcut", "--target={}".format(target)]
        if name:
            cmd.append("--name={}".format(name))
        if description:
            cmd.append("--desc={}".format(description))
        if icon:
            cmd.append("--icon={}".format(icon))
        if workdir:
            cmd.append("--workdir={}".format(workdir))
        
        return self.run_command(cmd)
    
    def configure_autorun(self, target: str, name: str = None, args: str = None, workdir: str = None, remove: bool = False) -> str:
        """
        配置开机自启
        
        :param target: 目标程序路径
        :param name: 任务名称
        :param args: 程序参数
        :param workdir: 工作目录
        :param remove: 是否移除自启配置
        :return: 配置结果
        """
        cmd = ["autorun", "--target={}".format(target)]
        if name:
            cmd.append("--name={}".format(name))
        if args:
            cmd.append("--args={}".format(args))
        if workdir:
            cmd.append("--workdir={}".format(workdir))
        if remove:
            cmd.append("--remove")
        
        return self.run_command(cmd)
    
    def show_infobox(self, message: str, title: str = "信息") -> str:
        """
        显示信息框
        
        :param message: 消息内容
        :param title: 标题
        :return: 显示结果
        """
        cmd = ["infoboxtop", message, title]
        return self.run_command(cmd, wait=False)  # 异步执行，不需要等待
    
    def show_question_box(self, message: str, title: str = "问题", program: str = None) -> str:
        """
        显示问题对话框
        
        :param message: 问题内容
        :param title: 标题
        :param program: 确认后要执行的程序
        :return: 显示结果
        """
        cmd = ["qboxtop", message, title]
        if program:
            cmd.append(program)
        return self.run_command(cmd, wait=False)  # 异步执行，不需要等待
    
    def show_custom_window(self, text: str, title: str = "系统提示", width: int = 600, height: int = 400, fontsize: int = 18, 
                          bgcolor: str = "white", textcolor: str = "black", font: str = None, bold: bool = False, 
                          modal: bool = False, nodrag: bool = False) -> str:
        """
        显示自定义窗口
        
        :param text: 窗口文本
        :param title: 窗口标题
        :param width: 窗口宽度
        :param height: 窗口高度
        :param fontsize: 字体大小
        :param bgcolor: 背景颜色
        :param textcolor: 文字颜色
        :param font: 字体名称
        :param bold: 是否粗体
        :param modal: 是否模态窗口
        :param nodrag: 是否禁止拖拽
        :return: 显示结果
        """
        cmd = ["window", "--text={}".format(text), "--title={}".format(title), 
               "--width={}".format(width), "--height={}".format(height), "--fontsize={}".format(fontsize),
               "--bgcolor={}".format(bgcolor), "--textcolor={}".format(textcolor)]
        
        if font:
            cmd.append("--font={}".format(font))
        if bold:
            cmd.append("--bold")
        if modal:
            cmd.append("--modal")
        if nodrag:
            cmd.append("--nodrag")
        
        return self.run_command(cmd, wait=False)  # 异步执行，不需要等待
    
    def execute_program(self, window_state: str, working_folder: str, application: str, args: str = None) -> str:
        """
        执行程序
        
        :param window_state: 窗口状态 (show/hide/min/max)
        :param working_folder: 工作目录
        :param application: 应用程序路径
        :param args: 程序参数
        :return: 执行结果
        """
        cmd = ["exec2", window_state, working_folder, application]
        if args:
            cmd.append(args)
        return self.run_command(cmd, wait=False)  # 异步执行，不需要等待
    
    def create_task(self, name: str, exec_path: str, trigger: str = "daily", starttime: str = "09:00", startdate: str = None) -> str:
        """
        创建计划任务
        
        :param name: 任务名称
        :param exec_path: 执行程序路径
        :param trigger: 触发器类型 (daily, weekly, monthly)
        :param starttime: 开始时间 (HH:MM)
        :param startdate: 开始日期 (YYYY-MM-DD)
        :return: 创建结果
        """
        cmd = ["task", "--name={}".format(name), "--exec={}".format(exec_path), 
               "--trigger={}".format(trigger), "--starttime={}".format(starttime)]
        
        if startdate:
            cmd.append("--startdate={}".format(startdate))
        
        return self.run_command(cmd)
    
    def restart_process(self, path: str, workdir: str = None) -> str:
        """
        重启进程
        
        :param path: 进程可执行文件路径
        :param workdir: 工作目录
        :return: 重启结果
        """
        cmd = ["restart", "--path={}".format(path)]
        if workdir:
            cmd.append("--workdir={}".format(workdir))
        
        return self.run_command(cmd)
    
    def show_notification(self, title: str, message: str, icon: str = "info", timeout: int = 5) -> str:
        """
        显示系统通知
        
        :param title: 通知标题
        :param message: 通知内容
        :param icon: 图标类型 (info, warning, error)
        :param timeout: 超时时间(秒)
        :return: 显示结果
        """
        cmd = ["notify", "--title={}".format(title), "--message={}".format(message), 
               "--icon={}".format(icon), "--timeout={}".format(timeout)]
        
        return self.run_command(cmd, wait=False)  # 异步执行，不需要等待
    
    def manage_config(self, file_path: str, action: str = "get", section: str = None, key: str = None, value: str = None) -> str:
        """
        管理配置文件
        
        :param file_path: 配置文件路径
        :param action: 操作类型 (get, set, save, del)
        :param section: 配置节
        :param key: 配置键
        :param value: 配置值
        :return: 操作结果
        """
        cmd = ["config", "--file={}".format(file_path), "--action={}".format(action)]
        
        if section:
            cmd.append("--section={}".format(section))
        if key:
            cmd.append("--key={}".format(key))
        if value:
            cmd.append("--value={}".format(value))
        
        return self.run_command(cmd)  # 需要等待结果
    
    def check_process(self, name: str = None, pid: int = None) -> str:
        """
        检查进程
        
        :param name: 进程名称
        :param pid: 进程ID
        :return: 检查结果
        """
        cmd = ["process"]
        if name:
            cmd.append("--name={}".format(name))
        elif pid:
            cmd.append("--pid={}".format(pid))
        
        return self.run_command(cmd)  # 需要等待结果
    
    def kill_process(self, name: str = None, pid: int = None) -> str:
        """
        终止进程
        
        :param name: 进程名称
        :param pid: 进程ID
        :return: 终止结果
        """
        cmd = ["process", "--action=kill"]
        if name:
            cmd.append(f"--name={name}")
        elif pid:
            cmd.append(f"--pid={pid}")
        
        return self.run_command(cmd)  # 需要等待结果
    
    def rotate_log(self, file_path: str, maxsize: int = None, daily: bool = False) -> str:
        """
        日志轮转
        
        :param file_path: 日志文件路径
        :param maxsize: 最大大小(MB)
        :param daily: 是否按天轮转
        :return: 轮转结果
        """
        cmd = ["logrotate", "--path={}".format(file_path)]
        if maxsize:
            cmd.append("--maxsize={}".format(maxsize))
        if daily:
            cmd.append("--daily")
        
        return self.run_command(cmd)
    
    def create_tray_icon(self, process_name: str, title: str = None, icon_path: str = None) -> str:
        """
        创建系统托盘图标
        
        :param process_name: 进程名称
        :param title: 托盘图标标题
        :param icon_path: 图标路径
        :return: 创建结果
        """
        cmd = ["tray", "--process={}".format(process_name)]
        if title:
            cmd.append("--title={}".format(title))
        if icon_path:
            cmd.append("--icon={}".format(icon_path))
        
        return self.run_command(cmd)
    
    def create_floating_icon(self, process_name: str, title: str = None, icon_path: str = None, path: str = None) -> str:
        """
        创建浮动图标
        
        :param process_name: 进程名称
        :param title: 浮动图标标题
        :param icon_path: 图标路径
        :param path: 进程路径，用于自动检测进程名和图标路径
        :return: 创建结果
        """
        cmd = ["floating", "--process={}".format(process_name)]
        if title:
            cmd.append("--title={}".format(title))
        if icon_path:
            cmd.append("--icon={}".format(icon_path))
        if path:
            cmd.append("--path={}".format(path))
        
        return self.run_command(cmd)
