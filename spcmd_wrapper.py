#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
SPCMD Python Wrapper
用于调用SPCMD工具的Python包装器
"""

import subprocess
import sys
import os

def generate_random_ids(id_type="uuid4", count=1):
    """
    调用SPCMD生成随机ID
    
    Args:
        id_type (str): ID类型 (uuid4, uuid7, snowflake, number)
        count (int): 生成数量
    
    Returns:
        list: 生成的ID列表
    """
    # 获取当前脚本所在目录
    current_dir = os.path.dirname(os.path.abspath(__file__))
    spcmd_path = os.path.join(current_dir, "spcmd.exe")
    
    # 检查SPCMD可执行文件是否存在
    if not os.path.exists(spcmd_path):
        raise FileNotFoundError(f"SPCMD executable not found at {spcmd_path}")
    
    # 构建命令
    cmd = [spcmd_path, "random", f"--type={id_type}", f"--count={count}"]
    
    try:
        # 执行命令并获取输出
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        # 分割输出为列表并去除空行
        ids = [line.strip() for line in result.stdout.strip().split('\n') if line.strip()]
        return ids
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Failed to generate random IDs: {e.stderr}")

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

# 命令行接口
def main():
    """命令行接口"""
    if len(sys.argv) < 2:
        print("Usage: python spcmd_wrapper.py <command> [options]")
        print("Commands:")
        print("  random [--type=TYPE] [--count=COUNT]")
        print("    TYPE: uuid4, uuid7, snowflake, number (default: uuid4)")
        print("    COUNT: number of IDs to generate (default: 1)")
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
    else:
        print(f"Unknown command: {command}")
        sys.exit(1)

if __name__ == "__main__":
    main()