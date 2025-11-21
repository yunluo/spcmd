/*
 * SPCMD - System Power Command Tool
 * A command-line tool developed in C, compatible with Windows XP/7/10/11
 * Reference NirCmd: http://www.nirsoft.net/utils/nircmd.html
 *
 * Features:
 * - System screenshot
 * - Create desktop shortcuts
 * - Configure auto-start (system/user level)
 * - Information popups (normal/topmost/custom large popups)
 * - Create system services
 * - Create scheduled tasks
 * - Restart specified processes
 */

#define _WIN32_IE 0x0500
#define COBJMACROS
#define INITGUID
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <locale.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <shobjidl.h>
#include <tlhelp32.h>  // 添加这个头文件以支持进程操作
#include <stdint.h>  // 添加这个头文件以支持uint32_t

#include "spcmd.h"

int main(int argc, char* argv[]) {
    // Set console code page for UTF-8 support
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // 如果没有参数或第一个参数是帮助相关的，则显示帮助信息
    if (argc < 2 || strcmp(argv[1], "/help") == 0 || strcmp(argv[1], "-h") == 0) {
        show_help();
        return 0;
    }
    
    // 处理命令
    handle_command(argc, argv);
    
    return 0;
}