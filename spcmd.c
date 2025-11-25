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
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "ini.h"
#include <objbase.h>
#include <shellapi.h> // 添加这个头文件以支持图标提取
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <stdint.h> // 添加这个头文件以支持uint32_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // 添加这个头文件以支持时间函数
#include <tlhelp32.h> // 添加这个头文件以支持进程操作
#include <locale.h> // 添加这个头文件以支持本地化
#include <windows.h>


// 定义BOOL类型，如果尚未定义
#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// 函数声明
void show_help();
void handle_command(int argc, char *argv[]);
void cmd_screenshot(int argc, char *argv[]);
void cmd_shortcut(int argc, char *argv[]);
void cmd_autorun(int argc, char *argv[]);
void cmd_infoboxtop(int argc, char *argv[]);
void cmd_qboxtop(int argc, char *argv[]);
void cmd_exec2(int argc, char *argv[]);
void cmd_task(int argc, char *argv[]);
void cmd_restart(int argc, char *argv[]);
void cmd_window(int argc, char *argv[]);
void cmd_notify(int argc, char *argv[]);
void cmd_config(int argc, char *argv[]);
int cmd_process(int argc, char *argv[]);
char* cmd_random(int argc, char *argv[]);
void cmd_logrotate(int argc, char *argv[]);
void cmd_tray(int argc, char *argv[]); // 新增托盘图标命令
void save_as_base64_data(const char *bitmap_data, DWORD data_size, const char *filename);
int save_bitmap_as_format(HBITMAP hBitmap, HDC hScreenDC, const char *filename, const char *format, int quality);
// 系统变量解析函数声明
char *resolve_system_variables(const char *input);

// 添加权限检查和提升权限的函数声明
BOOL IsRunAsAdmin();
BOOL ElevatePrivileges(int argc, char *argv[]);

// 通用进程查找和终止函数声明
BOOL find_process_by_name(const char* processName, DWORD* processId);
BOOL kill_process_by_name(const char* processName);

// 通用文件操作函数声明
BOOL create_empty_file(const char* path);
void rotate_log_file(const char* path, ULONGLONG size_bytes);

// 自定义弹窗结构体，用于传递参数
typedef struct {
  char *text;
  int fontSize;
  COLORREF bgColor;
  COLORREF textColor; // 添加文字颜色
  BOOL modal;         // 是否为模态弹窗
  BOOL noDrag;        // 是否禁止拖拽
  BOOL bold;          // 是否粗体
} WindowParams;


int main(int argc, char *argv[]) {
  // 如果没有参数或第一个参数是帮助相关的，则显示帮助信息
  if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "--h") == 0) {
    show_help();
    return 0;
  }

  // 处理命令
  handle_command(argc, argv);

  return 0;
}

void show_help() {
  printf("                                                                               \n");
  printf("       ooooooooo oooooooooo      ooooooo   ooo        ooooo oooooooooo         \n");
  printf("     d8P'    `Y8 `888   `Y88   d8P'  `Y8b  `88         888' `888'   `Y8b       \n");
  printf("     Y88bo        888    d88' 888           888b     d'888   888      888      \n");
  printf("      `'Y8888o    888ooo88P'  888           8 Y88   P  888   888      888      \n");
  printf("          `'Y88b  888         888           8  `888'   888   888      888      \n");
  printf("     oo      d8P  888         `88b    ooo   8    Y     888   888     d88'      \n");
  printf("     8\"\"88888P'    o888o         `Y8bood8P'  o8o        o888o o888bood8P'        \n");
  printf("                                                                               \n");
  printf("==========================================================================\n");
  printf("SPCMD - System Power Command Tool\n");
  printf("Usage: spcmd <command> [parameters]\n\n");
  printf("Supported commands:\n");
  printf("  screenshot            - Capture screen screenshot\n");
  printf("  shortcut              - Create desktop shortcut\n");
  printf("  autorun               - Configure auto-start\n");
  printf("  infoboxtop            - Display top-most message box\n");
  printf("  qboxtop               - Display top-most question dialog box\n");
  printf("  window                - Display custom window with advanced features\n");
  printf("  exec2                 - Execute application with working folder\n");
  printf("  task                  - Create scheduled task\n");
  printf("  restart               - Restart specified process\n");
  printf("  notify                - Display system notification\n");
  printf("  config                - Manage INI configuration files\n");
  printf("  process               - Check and kill processes\n");
  printf("  random                - Generate random numbers and IDs\n");
  printf("  logrotate             - Rotate and cut log files\n");
  printf("  tray                  - System tray icon with menu\n");
  printf("\nType spcmd <command> --help for specific command help\n");
}

void handle_command(int argc, char *argv[]) {
  // 创建解析后的参数数组
  char **resolved_argv = (char **)malloc(argc * sizeof(char *));
  if (!resolved_argv) {
    printf("Error: Memory allocation failed\n");
    return;
  }

  // 解析所有参数中的系统变量
  for (int i = 0; i < argc; i++) {
    resolved_argv[i] = resolve_system_variables(argv[i]);
    if (!resolved_argv[i]) {
      resolved_argv[i] = _strdup(argv[i]);
    }
  }

  if (strcmp(resolved_argv[1], "screenshot") == 0) {
    cmd_screenshot(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "shortcut") == 0) {
    cmd_shortcut(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "autorun") == 0) {
    cmd_autorun(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "infoboxtop") == 0) {
    cmd_infoboxtop(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "qboxtop") == 0) {
    cmd_qboxtop(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "window") == 0) {
    cmd_window(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "exec2") == 0) {
    cmd_exec2(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "task") == 0) {
    cmd_task(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "restart") == 0) {
    cmd_restart(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "notify") == 0) {
    cmd_notify(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "config") == 0) {
    cmd_config(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "process") == 0) {
    int result = cmd_process(argc, resolved_argv);
    if (result != 0) {
      exit(result); // 如果check命令返回非0值，退出程序
    }
  } else if (strcmp(resolved_argv[1], "random") == 0) {
    char *result = cmd_random(argc, resolved_argv);
    if (result != NULL) {
      printf("%s", result);
      free(result);
    } else {
      // 如果random命令返回NULL，确保有适当的输出
      // 这可能发生在帮助请求或错误情况下
      // cmd_random函数在这些情况下已经输出了信息，所以这里不需要额外输出
    }
  } else if (strcmp(resolved_argv[1], "logrotate") == 0) {
    cmd_logrotate(argc, resolved_argv);
  } else if (strcmp(resolved_argv[1], "tray") == 0) {
    cmd_tray(argc, resolved_argv);
  } else {
    printf("Unknown command: %s\n", resolved_argv[1]);
    show_help();
  }

  // 释放解析后的参数数组
  for (int i = 0; i < argc; i++) {
    if (resolved_argv[i]) {
      free(resolved_argv[i]);
    }
  }
  free(resolved_argv);
}

void cmd_screenshot(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Screenshot command help:\n");
    printf("  spcmd screenshot [--save=path] [--fullscreen] "
           "[--format=png|bmp] [--base64=file] [--quality=value]\n\n");
    printf("Parameter description:\n");
    printf("  --save=path       - Save screenshot to specified path, default "
           "to current directory\n");
    printf("  --fullscreen      - Capture full screen (default)\n");
    printf("  --format=png|jpg|bmp  - Save format, default is bmp\n");
    printf("  --base64=file     - Save as Base64 encoded data to specified "
           "file\n");
    printf("  --quality=value   - Image quality for PNG (1-100), default is "
           "100\n\n");
    printf("Examples:\n");
    printf("  spcmd screenshot\n");
    printf("  spcmd screenshot --save=C:\\screenshots\\screen.png\n");
    printf("  spcmd screenshot --format=png\n");
    printf("  spcmd screenshot --format=jpg\n");
    printf("  spcmd screenshot --base64=screenshot.b64\n");
    printf("  spcmd screenshot --quality=80\n");
    return;
  }

  // Get screen DC
  HDC hScreenDC = GetDC(NULL);
  if (hScreenDC == NULL) {
    printf("Error: Unable to get screen device context\n");
    return;
  }

  // Get screen dimensions
  int screenWidth = GetDeviceCaps(hScreenDC, HORZRES);
  int screenHeight = GetDeviceCaps(hScreenDC, VERTRES);

  // Create compatible memory DC
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
  if (hMemoryDC == NULL) {
    printf("Error: Unable to create memory device context\n");
    ReleaseDC(NULL, hScreenDC);
    return;
  }

  // Create bitmap
  HBITMAP hBitmap =
      CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
  if (hBitmap == NULL) {
    printf("Error: Unable to create bitmap\n");
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    return;
  }

  // Select bitmap to memory DC
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

  // Capture full screen (default behavior)
  BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0,
         SRCCOPY);

  // Restore old bitmap
  SelectObject(hMemoryDC, hOldBitmap);

  // Generate filename and format
  char filename[MAX_PATH] = "screenshot.bmp";
  char format[10] = "bmp";              // default format
  char base64_filename[MAX_PATH] = {0}; // for base64 encoded data
  BOOL save_as_base64 = FALSE;
  int quality = 100; // default quality

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--save=", 7) == 0) {
      strncpy(filename, argv[i] + 7, MAX_PATH - 1);
      filename[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--format=", 9) == 0) {
      strncpy(format, argv[i] + 9, sizeof(format) - 1);
      format[sizeof(format) - 1] = '\0';
    } else if (strncmp(argv[i], "--base64=", 9) == 0) {
      strncpy(base64_filename, argv[i] + 9, MAX_PATH - 1);
      base64_filename[MAX_PATH - 1] = '\0';
      save_as_base64 = TRUE;
    // --active parameter is deprecated
    } else if (strncmp(argv[i], "--quality=", 10) == 0) {
      quality = atoi(argv[i] + 9);
      // Ensure quality is between 1 and 100
      if (quality < 1)
        quality = 1;
      if (quality > 100)
        quality = 100;
    }
  }

  // Handle base64 encoded data save
  if (save_as_base64) {
    // Save as base64 encoded data
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize =
        ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char *lpbitmap = (char *)GlobalLock(hDIB);

    GetDIBits(hScreenDC, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap,
              (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    // Save as base64 encoded data
    save_as_base64_data(lpbitmap, dwBmpSize, base64_filename);

    // Clean up resources
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
  } else {
    // Save bitmap in the specified format
    // Ensure filename has correct extension
    if (strcmp(format, "png") == 0 || strcmp(format, "PNG") == 0) {
      // Ensure filename has .png extension
      if (strstr(filename, ".bmp")) {
        // Replace .bmp with .png
        char *dot = strrchr(filename, '.');
        if (dot) {
          strcpy(dot, ".png");
        }
      } else if (!strstr(filename, ".png")) {
        // Add .png extension if no extension
        strcat(filename, ".png");
      }
    } else if (strcmp(format, "jpg") == 0 || strcmp(format, "JPG") == 0 || 
               strcmp(format, "jpeg") == 0 || strcmp(format, "JPEG") == 0) {
      // Ensure filename has .jpg extension
      if (strstr(filename, ".bmp")) {
        // Replace .bmp with .jpg
        char *dot = strrchr(filename, '.');
        if (dot) {
          strcpy(dot, ".jpg");
        }
      } else if (!strstr(filename, ".jpg") && !strstr(filename, ".jpeg")) {
        // Add .jpg extension if no extension
        strcat(filename, ".jpg");
      }
    } else {
      // Save as BMP (default)
      // Ensure filename has .bmp extension
      if (strstr(filename, ".png")) {
        // Replace .png with .bmp
        char *dot = strrchr(filename, '.');
        if (dot) {
          strcpy(dot, ".bmp");
        }
      } else if (strstr(filename, ".jpg") || strstr(filename, ".jpeg")) {
        // Replace .jpg/.jpeg with .bmp
        char *dot = strrchr(filename, '.');
        if (dot) {
          strcpy(dot, ".bmp");
        }
      } else if (!strstr(filename, ".bmp")) {
        // Add .bmp extension if no extension
        strcat(filename, ".bmp");
      }
      // Use "bmp" as format for the save function
      strcpy(format, "bmp");
    }
    
    // Use the new helper function to save the bitmap
    save_bitmap_as_format(hBitmap, hScreenDC, filename, format, quality);
  }

  // Clean up resources
  DeleteObject(hBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL, hScreenDC);
}

void cmd_shortcut(int argc, char *argv[]) {
  printf("Creating shortcut...\n");

  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Shortcut command help:\n");
    printf("  spcmd shortcut --target=path [--name=name] [--desc=description] "
           "[--icon=iconpath] [--workdir=dir]\n\n");
    printf("Parameter description:\n");
    printf("  --target=path       - Target program path (required)\n");
    printf("  --name=name         - Shortcut name, default is program name\n");
    printf("  --desc=description  - Shortcut description\n");
    printf("  --icon=iconpath     - Icon path\n");
    printf("  --workdir=dir       - Working directory\n\n");
    printf("Examples:\n");
    printf("  spcmd shortcut --target=C:\\Windows\\notepad.exe\n");
    printf("  spcmd shortcut --target=C:\\Windows\\notepad.exe --name=Notepad "
           "--desc=Open Notepad program\n");
    return;
  }

  // Check required parameters
  char targetPath[MAX_PATH] = {0};
  char shortcutName[MAX_PATH] = {0};
  char description[MAX_PATH] = {0};
  char iconPath[MAX_PATH] = {0};
  char workingDir[MAX_PATH] = {0};

  BOOL hasTarget = FALSE;

  // Parse parameters
  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--target=", 9) == 0) {
      strncpy(targetPath, argv[i] + 9, MAX_PATH - 1);
      targetPath[MAX_PATH - 1] = '\0';
      hasTarget = TRUE;
    } else if (strncmp(argv[i], "--name=", 7) == 0) {
      strncpy(shortcutName, argv[i] + 7, MAX_PATH - 1);
      shortcutName[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--desc=", 7) == 0) {
      strncpy(description, argv[i] + 7, MAX_PATH - 1);
      description[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--icon=", 7) == 0) {
      strncpy(iconPath, argv[i] + 7, MAX_PATH - 1);
      iconPath[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--workdir=", 10) == 0) {
      strncpy(workingDir, argv[i] + 10, MAX_PATH - 1);
      workingDir[MAX_PATH - 1] = '\0';
    }
  }

  // Check required parameters
  if (!hasTarget) {
    printf("Error: Target program path must be specified (--target=path)\n");
    printf("Use spcmd shortcut /help for help\n");
    return;
  }

  // If shortcut name is not specified, use target filename
  if (strlen(shortcutName) == 0) {
    char *fileName = strrchr(targetPath, '\\');
    if (fileName != NULL) {
      strncpy(shortcutName, fileName + 1, MAX_PATH - 1);
    } else {
      strncpy(shortcutName, targetPath, MAX_PATH - 1);
    }

    // Remove extension
    char *dot = strrchr(shortcutName, '.');
    if (dot != NULL) {
      *dot = '\0';
    }
  }

  // Add .lnk extension
  char finalName[MAX_PATH];
  snprintf(finalName, MAX_PATH, "%s.lnk", shortcutName);

  // Get desktop path
  char desktopPath[MAX_PATH];
  if (FAILED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL,
                              SHGFP_TYPE_CURRENT, desktopPath))) {
    printf("Error: Unable to get desktop path\n");
    return;
  }

  // Construct shortcut full path
  char shortcutPath[MAX_PATH];
  snprintf(shortcutPath, MAX_PATH, "%s\\%s", desktopPath, finalName);

  // Create shortcut
  CoInitialize(NULL);

  IShellLinkA *pShellLink = NULL;
  HRESULT hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                  &IID_IShellLinkA, (LPVOID *)&pShellLink);

  if (SUCCEEDED(hres)) {
    // Set target path
    IShellLinkA_SetPath(pShellLink, targetPath);

    // Set working directory
    if (strlen(workingDir) > 0) {
      IShellLinkA_SetWorkingDirectory(pShellLink, workingDir);
    } else {
      // Use target file directory as working directory
      char targetDir[MAX_PATH];
      strncpy(targetDir, targetPath, MAX_PATH - 1);
      char *lastSlash = strrchr(targetDir, '\\');
      if (lastSlash != NULL) {
        *lastSlash = '\0';
        IShellLinkA_SetWorkingDirectory(pShellLink, targetDir);
      }
    }

    // Set description
    if (strlen(description) > 0) {
      IShellLinkA_SetDescription(pShellLink, description);
    }

    // Set icon
    if (strlen(iconPath) > 0) {
      IShellLinkA_SetIconLocation(pShellLink, iconPath, 0);
    } else {
      IShellLinkA_SetIconLocation(pShellLink, targetPath, 0);
    }

    // Save shortcut
    IPersistFile *pPersistFile = NULL;
    hres = IShellLinkA_QueryInterface(pShellLink, &IID_IPersistFile,
                                      (LPVOID *)&pPersistFile);

    if (SUCCEEDED(hres)) {
      // Convert to wide character
      WCHAR wsz[MAX_PATH];
      MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wsz, MAX_PATH);

      hres = IPersistFile_Save(pPersistFile, wsz, TRUE);

      if (SUCCEEDED(hres)) {
        printf("Shortcut created: %s\n", shortcutPath);
      } else {
        printf("Error: Unable to save shortcut to %s\n", shortcutPath);
      }

      IPersistFile_Release(pPersistFile);
    }

    IShellLinkA_Release(pShellLink);
  } else {
    printf("Error: Unable to create shortcut object\n");
  }

  CoUninitialize();

  // Refresh desktop to show the new shortcut icon
  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

void cmd_autorun(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Autorun command help:\n");
    printf("  spcmd autorun --target=path [--name=name] [--args=args] "
           "[--workdir=dir] [--remove]\n\n");
    printf("Parameter description:\n");
    printf("  --target=path   - Target program path (required)\n");
    printf("  --name=name     - Autorun entry name, default is program name\n");
    printf("  --args=args     - Program startup arguments\n");
    printf("  --workdir=dir   - Working directory\n");
    printf("  --remove        - Remove autorun entry\n\n");
    printf("Examples:\n");
    printf("  spcmd autorun --target=C:\\Windows\\notepad.exe\n");
    printf("  spcmd autorun --target=C:\\Windows\\notepad.exe --name=Notepad "
           "--args=\"C:\\temp\\test.txt\"\n");
    printf("  spcmd autorun --target=C:\\Windows\\notepad.exe --remove\n");
    return;
  }

  // Check required parameters
  char targetPath[MAX_PATH] = {0};
  char entryName[MAX_PATH] = {0};
  char arguments[MAX_PATH] = {0};
  char workingDir[MAX_PATH] = {0};
  BOOL removeEntry = FALSE;

  BOOL hasTarget = FALSE;

  // Parse parameters
  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--target=", 9) == 0) {
      strncpy(targetPath, argv[i] + 9, MAX_PATH - 1);
      targetPath[MAX_PATH - 1] = '\0';
      hasTarget = TRUE;
    } else if (strncmp(argv[i], "--name=", 7) == 0) {
      strncpy(entryName, argv[i] + 7, MAX_PATH - 1);
      entryName[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--args=", 7) == 0) {
      strncpy(arguments, argv[i] + 7, MAX_PATH - 1);
      arguments[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--workdir=", 10) == 0) {
      strncpy(workingDir, argv[i] + 10, MAX_PATH - 1);
      workingDir[MAX_PATH - 1] = '\0';
    } else if (strcmp(argv[i], "--remove") == 0) {
      removeEntry = TRUE;
    }
  }

  // Check required parameters
  if (!hasTarget) {
    printf("Error: Target program path must be specified (--target=path)\n");
    printf("Use spcmd autorun --help for help\n");
    return;
  }

  // If entry name is not specified, use target filename
  if (strlen(entryName) == 0) {
    char *fileName = strrchr(targetPath, '\\');
    if (fileName != NULL) {
      strncpy(entryName, fileName + 1, MAX_PATH - 1);
    } else {
      strncpy(entryName, targetPath, MAX_PATH - 1);
    }

    // Remove extension
    char *dot = strrchr(entryName, '.');
    if (dot != NULL) {
      *dot = '\0';
    }
  }

  // Add .lnk extension
  char finalName[MAX_PATH];
  snprintf(finalName, MAX_PATH, "%s.lnk", entryName);

  // Try to use system startup directory first (requires admin rights)
  char startupPath[MAX_PATH];
  BOOL useSystemStartup = TRUE;

  // Get system startup directory
  if (FAILED(SHGetFolderPathA(NULL, CSIDL_COMMON_STARTUP, NULL,
                              SHGFP_TYPE_CURRENT, startupPath))) {
    // Fall back to user startup directory
    useSystemStartup = FALSE;
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, SHGFP_TYPE_CURRENT,
                                startupPath))) {
      printf("Error: Unable to get startup directory\n");
      return;
    }
  }

  // Construct shortcut full path
  char shortcutPath[MAX_PATH];
  snprintf(shortcutPath, MAX_PATH, "%s\\%s", startupPath, finalName);

  if (removeEntry) {
    // Remove autorun entry (shortcut)
    BOOL removed = FALSE;

    // Try to remove from the primary location
    if (DeleteFileA(shortcutPath)) {
      printf("Autorun entry removed: %s\n", entryName);
      if (useSystemStartup) {
        printf("From system startup directory\n");
      } else {
        printf("From user startup directory\n");
      }
      removed = TRUE;
    }

    // If not removed from primary location, try the other location
    if (!removed) {
      char alternativePath[MAX_PATH];
      if (useSystemStartup) {
        // Try user startup directory
        char userStartupPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL,
                                       SHGFP_TYPE_CURRENT, userStartupPath))) {
          snprintf(alternativePath, MAX_PATH, "%s\\%s", userStartupPath,
                   finalName);
          if (DeleteFileA(alternativePath)) {
            printf("Autorun entry removed: %s\n", entryName);
            printf("From user startup directory\n");
            removed = TRUE;
          }
        }
      } else {
        // Try system startup directory
        char systemStartupPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_STARTUP, NULL,
                                       SHGFP_TYPE_CURRENT,
                                       systemStartupPath))) {
          snprintf(alternativePath, MAX_PATH, "%s\\%s", systemStartupPath,
                   finalName);
          if (DeleteFileA(alternativePath)) {
            printf("Autorun entry removed: %s\n", entryName);
            printf("From system startup directory\n");
            removed = TRUE;
          }
        }
      }
    }

    // If still not removed, report error
    if (!removed) {
      DWORD error = GetLastError();
      if (error == ERROR_FILE_NOT_FOUND) {
        printf("Autorun entry not found: %s\n", entryName);
      } else {
        printf("Error: Unable to remove autorun entry: %s (Error code: %lu)\n",
               entryName, error);
      }
    }
  } else {
    // Add or update autorun entry (create shortcut)

    // 检查是否需要管理员权限来创建系统开机自启项
    BOOL needAdmin = useSystemStartup;

    // 如果需要管理员权限但当前没有管理员权限，则尝试提升权限
    if (needAdmin && !IsRunAsAdmin()) {
      printf("需要管理员权限来创建系统开机自启项，正在请求权限提升...\n");

      // 尝试提升权限
      if (ElevatePrivileges(argc, argv)) {
        // 如果提升成功，程序会以管理员权限重新运行，当前进程会退出
        return;
      } else {
        // 如果提升失败，回退到用户开机自启
        printf("权限提升失败，回退到用户开机自启...\n");
        useSystemStartup = FALSE;

        // 重新获取用户启动目录
        if (FAILED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL,
                                    SHGFP_TYPE_CURRENT, startupPath))) {
          printf("Error: Unable to get user startup directory\n");
          return;
        }

        // 重新构造快捷方式路径
        snprintf(shortcutPath, MAX_PATH, "%s\\%s", startupPath, finalName);
      }
    }

    CoInitialize(NULL);

    IShellLinkA *pShellLink = NULL;
    HRESULT hres =
        CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                         &IID_IShellLinkA, (LPVOID *)&pShellLink);

    if (SUCCEEDED(hres)) {
      // Set target path
      IShellLinkA_SetPath(pShellLink, targetPath);

      // Set working directory
      if (strlen(workingDir) > 0) {
        IShellLinkA_SetWorkingDirectory(pShellLink, workingDir);
      } else {
        // Use target file directory as working directory
        char targetDir[MAX_PATH];
        strncpy(targetDir, targetPath, MAX_PATH - 1);
        char *lastSlash = strrchr(targetDir, '\\');
        if (lastSlash != NULL) {
          *lastSlash = '\0';
          IShellLinkA_SetWorkingDirectory(pShellLink, targetDir);
        }
      }

      // Set arguments
      if (strlen(arguments) > 0) {
        IShellLinkA_SetArguments(pShellLink, arguments);
      }

      // Save shortcut
      IPersistFile *pPersistFile = NULL;
      hres = IShellLinkA_QueryInterface(pShellLink, &IID_IPersistFile,
                                        (LPVOID *)&pPersistFile);

      if (SUCCEEDED(hres)) {
        // Convert to wide character
        WCHAR wsz[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wsz, MAX_PATH);

        hres = IPersistFile_Save(pPersistFile, wsz, TRUE);

        if (SUCCEEDED(hres)) {
          printf("Autorun entry added/updated: %s\n", entryName);
          printf("Shortcut created: %s\n", shortcutPath);
          if (useSystemStartup) {
            printf("In system startup directory\n");
          } else {
            printf("In user startup directory\n");
          }

          if (strlen(arguments) > 0) {
            printf("Arguments: %s\n", arguments);
          }
        } else {
          // If failed to save to system startup directory, try user startup
          // directory
          if (useSystemStartup) {
            printf("Unable to save to system startup directory, trying user "
                   "startup directory...\n");

            // Get user startup directory
            char userStartupPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL,
                                           SHGFP_TYPE_CURRENT,
                                           userStartupPath))) {
              // Construct new shortcut path
              char userShortcutPath[MAX_PATH];
              snprintf(userShortcutPath, MAX_PATH, "%s\\%s", userStartupPath,
                       finalName);

              // Try to save to user startup directory
              hres = IPersistFile_Save(pPersistFile, (WCHAR *)userShortcutPath,
                                       TRUE);

              if (SUCCEEDED(hres)) {
                printf("Autorun entry added/updated: %s\n", entryName);
                printf("Shortcut created: %s\n", userShortcutPath);
                printf("In user startup directory\n");

                if (strlen(arguments) > 0) {
                  printf("Arguments: %s\n", arguments);
                }
              } else {
                printf("Error: Unable to save shortcut to %s\n",
                       userShortcutPath);
              }
            } else {
              printf("Error: Unable to get user startup directory\n");
            }
          } else {
            printf("Error: Unable to save shortcut to %s\n", shortcutPath);
          }
        }

        IPersistFile_Release(pPersistFile);
      }

      IShellLinkA_Release(pShellLink);
    } else {
      printf("Error: Unable to create shortcut object\n");
    }

    CoUninitialize();
  }
}

void cmd_task(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Task command help:\n");
    printf("  spcmd task --name=task_name --exec=program_path [--trigger=daily|weekly|monthly] [--starttime=HH:MM] [--startdate=YYYY-MM-DD]\n\n");
    printf("Parameter description:\n");
    printf("  --name=task_name      - Task name (required)\n");
    printf("  --exec=program_path   - Program to execute (required)\n");
    printf("  --trigger=schedule    - Trigger type: daily, weekly, monthly (default: daily)\n");
    printf("  --starttime=HH:MM     - Start time in 24-hour format (default: 09:00)\n");
    printf("  --startdate=YYYY-MM-DD - Start date (default: today) - Available on Windows Vista and later\n\n");
    printf("Examples:\n");
    printf("  spcmd task --name=\"Daily Backup\" --exec=\"C:\\Windows\\system32\\cmd.exe\" --trigger=daily\n");
    printf("  spcmd task --name=\"Weekly Cleanup\" --exec=\"C:\\temp\\cleanup.bat\" --trigger=weekly --starttime=22:00\n");
    printf("  spcmd task --name=\"Monthly Report\" --exec=\"C:\\reports\\generate.exe\" --trigger=monthly --starttime=08:00 --startdate=2025-12-01\n");
    return;
  }

  // Parse parameters
  char taskName[MAX_PATH] = {0};
  char programPath[MAX_PATH] = {0};
  char triggerType[20] = "daily";  // default trigger
  char startTime[10] = "09:00";   // default start time
  char startDate[15] = {0};        // default to today

  BOOL hasName = FALSE;
  BOOL hasExec = FALSE;

  // Get current date as default start date
  SYSTEMTIME st;
  GetLocalTime(&st);
  snprintf(startDate, sizeof(startDate), "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--name=", 7) == 0) {
      strncpy(taskName, argv[i] + 7, MAX_PATH - 1);
      taskName[MAX_PATH - 1] = '\0';
      hasName = TRUE;
    } else if (strncmp(argv[i], "--exec=", 7) == 0) {
      strncpy(programPath, argv[i] + 7, MAX_PATH - 1);
      programPath[MAX_PATH - 1] = '\0';
      hasExec = TRUE;
    } else if (strncmp(argv[i], "--trigger=", 10) == 0) {
      strncpy(triggerType, argv[i] + 10, sizeof(triggerType) - 1);
      triggerType[sizeof(triggerType) - 1] = '\0';
    } else if (strncmp(argv[i], "--starttime=", 12) == 0) {
      strncpy(startTime, argv[i] + 12, sizeof(startTime) - 1);
      startTime[sizeof(startTime) - 1] = '\0';
    } else if (strncmp(argv[i], "--startdate=", 12) == 0) {
      strncpy(startDate, argv[i] + 12, sizeof(startDate) - 1);
      startDate[sizeof(startDate) - 1] = '\0';
    }
  }

  // Check required parameters
  if (!hasName || !hasExec) {
    printf("Error: Task name and program path must be specified\n");
    printf("Use spcmd task --help for help\n");
    return;
  }

  // Check if file exists
  if (GetFileAttributesA(programPath) == INVALID_FILE_ATTRIBUTES) {
    printf("Warning: Program file does not exist: %s\n", programPath);
  }

  printf("Creating scheduled task: %s\n", taskName);
  printf("Program: %s\n", programPath);
  printf("Trigger: %s\n", triggerType);
  printf("Start time: %s\n", startTime);
  printf("Start date: %s\n", startDate);

  // 检查Windows版本以确定使用哪种方法
  OSVERSIONINFO osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osvi);
  
  BOOL isWindowsXP = (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1); // Windows XP是5.1版本
  
  char command[1024];
  
  if (isWindowsXP) {
    // Windows XP兼容实现
    printf("Using Windows XP compatible mode\n");
    char scheduleType[20];
    
    // 确定调度类型
    if (strcmp(triggerType, "weekly") == 0) {
      strncpy(scheduleType, "WEEKLY", sizeof(scheduleType) - 1);
    } else if (strcmp(triggerType, "monthly") == 0) {
      strncpy(scheduleType, "MONTHLY", sizeof(scheduleType) - 1);
    } else { // 默认每日任务
      strncpy(scheduleType, "DAILY", sizeof(scheduleType) - 1);
    }
    scheduleType[sizeof(scheduleType) - 1] = '\0';
    
    // 构建schtasks命令 - XP兼容模式，不使用/sd参数
    snprintf(command, sizeof(command), "schtasks /create /tn \"%s\" /tr \"%s\" /sc %s /st %s", taskName, programPath, scheduleType, startTime);
  } else {
    // 新版本Windows实现，支持更多参数
    printf("Using modern Windows mode\n");
    char scheduleType[20];
    
    // 确定调度类型
    if (strcmp(triggerType, "weekly") == 0) {
      strncpy(scheduleType, "WEEKLY", sizeof(scheduleType) - 1);
    } else if (strcmp(triggerType, "monthly") == 0) {
      strncpy(scheduleType, "MONTHLY", sizeof(scheduleType) - 1);
    } else { // 默认每日任务
      strncpy(scheduleType, "DAILY", sizeof(scheduleType) - 1);
    }
    scheduleType[sizeof(scheduleType) - 1] = '\0';
    
    // 构建schtasks命令 - 现代模式，包含开始日期
    snprintf(command, sizeof(command), "schtasks /create /tn \"%s\" /tr \"%s\" /sc %s /st %s /sd %s", taskName, programPath, scheduleType, startTime, startDate);
  }
  
  printf("Executing command: %s\n", command);
  
  // 执行命令
  int result = system(command);
  if (result == 0) {
    printf("Task '%s' created successfully\n", taskName);
  } else {
    printf("Error: Failed to create task '%s'\n", taskName);
  }
}

void cmd_restart(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Restart command help:\n");
    printf("  spcmd restart --path=process_path [--workdir=working_directory]\n\n");
    printf("Parameter description:\n");
    printf("  --path=process_path   - Path to the process executable to restart "
           "(required)\n");
    printf("  --workdir=working_dir - Working directory for the process "
           "(optional)\n\n");
    printf("Examples:\n");
    printf("  spcmd restart --path=\"C:\\Windows\\notepad.exe\"\n");
    printf("  spcmd restart --path=\"C:\\Program Files\\MyApp\\myapp.exe\"\n");
    printf("  spcmd restart --path=\"C:\\MyApp\\myapp.exe\" "
           "--workdir=\"C:\\MyApp\"\n");
    return;
  }

  // Parse parameters
  char processPath[MAX_PATH] = {0};
  char workingDir[MAX_PATH] = {0};
  BOOL hasPath = FALSE;
  BOOL hasWorkDir = FALSE;

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--path=", 7) == 0) {
      strncpy(processPath, argv[i] + 7, MAX_PATH - 1);
      processPath[MAX_PATH - 1] = '\0';
      hasPath = TRUE;
    } else if (strncmp(argv[i], "--workdir=", 10) == 0) {
      strncpy(workingDir, argv[i] + 10, MAX_PATH - 1);
      workingDir[MAX_PATH - 1] = '\0';
      hasWorkDir = TRUE;
    }
  }

  // Check required parameters
  if (!hasPath) {
    printf("Error: Process path must be specified (--path=process_path)\n");
    printf("Use spcmd restart /help for help\n");
    return;
  }

  // Check if file exists
  if (GetFileAttributesA(processPath) == INVALID_FILE_ATTRIBUTES) {
    printf("Error: Process file does not exist: %s\n", processPath);
    return;
  }

  // Check if working directory exists (if specified)
  if (hasWorkDir && GetFileAttributesA(workingDir) == INVALID_FILE_ATTRIBUTES) {
    printf("Warning: Working directory does not exist: %s\n", workingDir);
    hasWorkDir = FALSE; // Don't use invalid working directory
  }

  printf("Restarting process: %s\n", processPath);

  // Get filename from path
  char *fileName = strrchr(processPath, '\\');
  if (fileName == NULL) {
    fileName = processPath;
  } else {
    fileName++; // Skip the backslash
  }

  // Remove extension for process name comparison
  char processName[MAX_PATH];
  strncpy(processName, fileName, MAX_PATH - 1);
  processName[MAX_PATH - 1] = '\0';
  char *dot = strrchr(processName, '.');
  if (dot != NULL) {
    *dot = '\0';
  }

  // First, try to find and kill existing processes with the same name
  PROCESSENTRY32 pe32;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
      do {
        // Compare process name (without extension) with our target
        char *procName = pe32.szExeFile;
        char *procDot = strrchr(procName, '.');
        if (procDot != NULL) {
          *procDot = '\0';
        }

        if (_stricmp(procName, processName) == 0) {
          // Found matching process, kill it
          HANDLE hProcess =
              OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
          if (hProcess != NULL) {
            if (TerminateProcess(hProcess, 0)) {
              printf("Killed process: %s (PID: %lu)\n", pe32.szExeFile,
                     pe32.th32ProcessID);
            } else {
              printf("Failed to kill process: %s (PID: %lu)\n", pe32.szExeFile,
                     pe32.th32ProcessID);
            }
            CloseHandle(hProcess);
          } else {
            printf("Failed to open process: %s (PID: %lu)\n", pe32.szExeFile,
                   pe32.th32ProcessID);
          }
        }

        // Restore the dot if we removed it
        if (procDot != NULL) {
          *procDot = '.';
        }
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }

  // Wait a bit for processes to terminate
  Sleep(1000);

  // Now start the new process
  STARTUPINFOA si = {0};
  PROCESS_INFORMATION pi = {0};
  si.cb = sizeof(si);

  
  // 如果没有指定工作目录，则使用进程所在目录作为工作目录
  char* effectiveWorkDir = NULL;
  char processDir[MAX_PATH] = {0};
  
  if (hasWorkDir) {
    effectiveWorkDir = workingDir;
  } else {
    // 获取进程文件所在目录
    strncpy(processDir, processPath, MAX_PATH - 1);
    processDir[MAX_PATH - 1] = '\0';
    char* lastSlash = strrchr(processDir, '\\');
    if (lastSlash != NULL) {
      *lastSlash = '\0';
      effectiveWorkDir = processDir;
    }
  }
  
  // Create the process
  if (CreateProcessA(NULL, processPath, NULL, NULL, FALSE, 0, NULL, 
                     effectiveWorkDir, &si, &pi)) {
    printf("Started process: %s (PID: %lu)\n", processPath, pi.dwProcessId);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  } else {
    printf("Failed to start process: %s\n", processPath);
    printf("Error code: %lu\n", GetLastError());
  }
}

void cmd_exec2(int argc, char *argv[]) {
  // exec2 [show/hide/min/max] [working folder] [application + command-line]
  // Similar to exec command, but also provide another parameter, [working
  // folder], that specifies the default working folder for the application that
  // you run.

  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("exec2 command help:\n");
    printf("  spcmd exec2 [show/hide/min/max] [working folder] [application + "
           "command-line]\n\n");
    printf("Parameter description:\n");
    printf("  show/hide/min/max  - Window state for the application\n");
    printf("  working folder     - Working directory for the application\n");
    printf("  application        - Application to run with optional "
           "command-line arguments\n\n");
    printf("Examples:\n");
    printf("  spcmd exec2 show \"f:\\winnt\\system32\" "
           "\"f:\\winnt\\system32\\calc.exe\"\n");
    printf("  spcmd exec2 hide c:\\temp \"c:\\temp\\wul.exe\" /savelangfile\n");
    return;
  }

  // Check if required parameters are provided
  if (argc < 5) {
    printf(
        "Error: Window state, working folder, and application are required\n");
    printf("Usage: spcmd exec2 [show/hide/min/max] [working folder] "
           "[application + command-line]\n");
    return;
  }

  // Parse window state
  int windowState = SW_SHOW;
  if (strcmp(argv[2], "hide") == 0) {
    windowState = SW_HIDE;
  } else if (strcmp(argv[2], "min") == 0) {
    windowState = SW_MINIMIZE;
  } else if (strcmp(argv[2], "max") == 0) {
    windowState = SW_MAXIMIZE;
  }

  // Get working folder and application
  char *workingFolder = argv[3];
  char *application = argv[4];

  // Build full command line
  char commandLine[MAX_PATH * 2] = {0};
  strncpy(commandLine, application, MAX_PATH - 1);

  // Append additional arguments if any
  for (int i = 5; i < argc; i++) {
    strncat(commandLine, " ", sizeof(commandLine) - strlen(commandLine) - 1);
    strncat(commandLine, argv[i],
            sizeof(commandLine) - strlen(commandLine) - 1);
  }

  // Run the application with specified working folder
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = windowState;

  ZeroMemory(&pi, sizeof(pi));

  // Start the application
  if (CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL,
                     workingFolder, &si, &pi)) {
    printf("Application '%s' started successfully in folder '%s'\n",
           application, workingFolder);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  } else {
    printf("Error: Failed to start application '%s' in folder '%s'\n",
           application, workingFolder);
    printf("Error code: %lu\n", GetLastError());
  }
}

void cmd_infoboxtop(int argc, char *argv[]) {
  // infoboxtop [message text] [title]
  // Similar to infobox, but displays the message-box as top-most window.

  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("infoboxtop command help:\n");
    printf("  spcmd infoboxtop [message text] [title]\n\n");
    printf("Parameter description:\n");
    printf("  message text  - Message to display in the box\n");
    printf("  title         - Title of the message box\n\n");
    printf("Examples:\n");
    printf("  spcmd infoboxtop \"This is a top-most message box!\" \"Top-Most "
           "Message\"\n");
    return;
  }

  // Check if required parameters are provided
  if (argc < 4) {
    printf("Error: Message text and title are required\n");
    printf("Usage: spcmd infoboxtop [message text] [title]\n");
    return;
  }

  // Display top-most message box
  // 获取当前活跃窗口句柄，然后在该窗口上显示置顶消息框
  HWND hActiveWnd = GetForegroundWindow();
  if (hActiveWnd == NULL) {
    // 如果无法获取活跃窗口句柄，则在桌面上显示
    hActiveWnd = GetDesktopWindow();
  }

  // 显示置顶消息框
  MessageBoxA(hActiveWnd, argv[2], argv[3],
              MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
  printf("Top-most message box displayed\n");
}

void cmd_qboxtop(int argc, char *argv[]) {
  // qboxtop [message text] [title] [program to run]
  // Similar to qbox, but displays the message-box as top-most window.

  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("qboxtop command help:\n");
    printf("  spcmd qboxtop [message text] [title] [program to run]\n\n");
    printf("Parameter description:\n");
    printf("  message text  - Question to display in the box\n");
    printf("  title         - Title of the question box\n");
    printf("  program       - Program to run if user answers Yes\n\n");
    printf("Examples:\n");
    printf("  spcmd qboxtop \"Do you want to run the calculator?\" "
           "\"Question\" \"calc.exe\"\n");
    return;
  }

  // Check if required parameters are provided
  if (argc < 5) {
    printf("Error: Message text, title, and program are required\n");
    printf("Usage: spcmd qboxtop [message text] [title] [program to run]\n");
    return;
  }

  // Display top-most question box
  // 获取当前活跃窗口句柄，然后在该窗口上显示置顶消息框
  HWND hActiveWnd = GetForegroundWindow();
  if (hActiveWnd == NULL) {
    // 如果无法获取活跃窗口句柄，则在桌面上显示
    hActiveWnd = GetDesktopWindow();
  }

  // 显示置顶消息框
  int result = MessageBoxA(hActiveWnd, argv[2], argv[3],
                           MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL);

  if (result == IDYES) {
    // Run the specified program
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the program
    if (CreateProcessA(NULL, argv[4], NULL, NULL, FALSE, 0, NULL, NULL, &si,
                       &pi)) {
      printf("Program '%s' started successfully\n", argv[4]);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    } else {
      printf("Error: Failed to start program '%s'\n", argv[4]);
    }
  } else {
    printf("User chose not to run the program\n");
  }
}

// 添加PNG保存函数
// 已移除，直接在cmd_screenshot函数中使用stb_image_write.h的函数

// 保存为BMP格式
// 已移除，直接在cmd_screenshot函数中使用stb_image_write.h的函数

// 保存为JPEG格式
// 已移除，直接在cmd_screenshot函数中使用stb_image_write.h的函数

// 辅助函数：从HBITMAP获取图像数据并保存为指定格式
int save_bitmap_as_format(HBITMAP hBitmap, HDC hScreenDC, const char *filename, 
                          const char *format, int quality) {
  // 获取BITMAP信息
  BITMAP bmp;
  GetObject(hBitmap, sizeof(BITMAP), &bmp);
  
  // 根据质量设置调整尺寸以减小文件大小
  int width = bmp.bmWidth;
  int height = bmp.bmHeight;
  
  if (quality < 100) {
    double scale = quality / 100.0;
    width = (int)(width * scale);
    height = (int)(height * scale);
  }
  
  // 创建缩放后的bitmap（如果需要）
  HDC hMemoryDC = NULL;
  HBITMAP hScaledBitmap = NULL;
  HBITMAP hOldBitmap = NULL;
  HBITMAP hBitmapToSave = hBitmap; // 默认使用原始bitmap
  
  if (width != bmp.bmWidth || height != bmp.bmHeight) {
    // 需要缩放
    hMemoryDC = CreateCompatibleDC(hScreenDC);
    hScaledBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hScaledBitmap);
    
    // 缩放图像
    SetStretchBltMode(hMemoryDC, HALFTONE);
    StretchBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, bmp.bmWidth, 
               bmp.bmHeight, SRCCOPY);
    
    hBitmapToSave = hScaledBitmap;
  }
  
  // 从bitmap获取数据
  BITMAPINFOHEADER bi = {0};
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = width;
  bi.biHeight = -height; // 负值表示顶部到底部的扫描线顺序
  bi.biPlanes = 1;
  bi.biBitCount = 24;
  bi.biCompression = BI_RGB;

  DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;

  HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
  char *lpbitmap = (char *)GlobalLock(hDIB);

  GetDIBits(hMemoryDC ? hMemoryDC : hScreenDC, hBitmapToSave, 0, (UINT)height, lpbitmap,
            (BITMAPINFO *)&bi, DIB_RGB_COLORS);

  // 调整RGB顺序以修复颜色发黄问题
  for (int i = 0; i < width * height * 3; i += 3) {
    // 交换红色和蓝色通道 (BGR -> RGB)
    char temp = lpbitmap[i];
    lpbitmap[i] = lpbitmap[i + 2];
    lpbitmap[i + 2] = temp;
  }

  // 使用stb_image_write保存为指定格式
  int result = 0;
  if (strcmp(format, "png") == 0 || strcmp(format, "PNG") == 0) {
    result = stbi_write_png(filename, width, height, 3, lpbitmap, width * 3);
    if (result) {
      if (width != bmp.bmWidth || height != bmp.bmHeight) {
        printf("Screenshot saved to: %s (scaled to %dx%d, as PNG format)\n", 
               filename, width, height);
      } else {
        printf("Screenshot saved to: %s (as PNG format)\n", filename);
      }
    }
  } else if (strcmp(format, "jpg") == 0 || strcmp(format, "JPG") == 0 || 
             strcmp(format, "jpeg") == 0 || strcmp(format, "JPEG") == 0) {
    result = stbi_write_jpg(filename, width, height, 3, lpbitmap, quality);
    if (result) {
      if (width != bmp.bmWidth || height != bmp.bmHeight) {
        printf("Screenshot saved to: %s (scaled to %dx%d, as JPEG format)\n", 
               filename, width, height);
      } else {
        printf("Screenshot saved to: %s (as JPEG format)\n", filename);
      }
    }
  } else { // 默认保存为BMP格式
    result = stbi_write_bmp(filename, width, height, 3, lpbitmap);
    if (result) {
      if (width != bmp.bmWidth || height != bmp.bmHeight) {
        printf("Screenshot saved to: %s (scaled to %dx%d, as BMP format)\n", 
               filename, width, height);
      } else {
        printf("Screenshot saved to: %s (as BMP format)\n", filename);
      }
    }
  }
  
  if (!result) {
    printf("Error: Unable to save screenshot to %s\n", filename);
  }

  // Clean up resources
  GlobalUnlock(hDIB);
  GlobalFree(hDIB);
  
  if (hScaledBitmap) {
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hScaledBitmap);
    DeleteDC(hMemoryDC);
  }
  
  return result;
}

// 添加Base64编码函数
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "0123456789+/";

char *base64_encode(const unsigned char *data, size_t input_length,
                    size_t *output_length) {
  *output_length = 4 * ((input_length + 2) / 3);

  char *encoded_data = malloc(*output_length + 1);
  if (encoded_data == NULL)
    return NULL;

  for (size_t i = 0, j = 0; i < input_length;) {
    uint32_t octet_a = i < input_length ? data[i++] : 0;
    uint32_t octet_b = i < input_length ? data[i++] : 0;
    uint32_t octet_c = i < input_length ? data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
  }

  for (size_t i = 0; i < (3 - (input_length % 3)) % 3; i++) {
    encoded_data[*output_length - 1 - i] = '=';
  }

  encoded_data[*output_length] = '\0';
  return encoded_data;
}

// 系统变量解析函数实现
char *resolve_system_variables(const char *input) {
  if (!input)
    return NULL;

  // 计算输出缓冲区大小（预留一些额外空间）
  size_t input_len = strlen(input);
  char *output = (char *)malloc(input_len * 2 + 1024);
  if (!output)
    return NULL;

  size_t out_pos = 0;
  size_t in_pos = 0;

  while (in_pos < input_len) {
    // 查找变量开始标记 [%
    if (input[in_pos] == '[' && input[in_pos + 1] == '%') {
      // 查找变量结束标记 %]
      size_t var_start = in_pos + 2;
      size_t var_end = var_start;

      while (var_end + 1 < input_len &&
             (input[var_end] != '%' || input[var_end + 1] != ']')) {
        var_end++;
      }

      if (var_end + 1 < input_len && input[var_end] == '%' &&
          input[var_end + 1] == ']') {
        // 找到了完整的变量格式 ~$...$
        char var_name[256] = {0};
        size_t var_len = var_end - var_start;
        if (var_len < sizeof(var_name) - 1) {
          strncpy(var_name, &input[var_start], var_len);
          var_name[var_len] = '\0'; // 确保字符串结束

          // 解析变量
          char *var_value = NULL;

          // 处理不同类型的变量
          if (strncmp(var_name, "folder.", 7) == 0) {
            char *folder_type = var_name + 7;

            if (strcmp(folder_type, "desktop") == 0) {
              char path[MAX_PATH];
              if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL,
                                             SHGFP_TYPE_CURRENT, path))) {
                var_value = _strdup(path);
              }
            } else if (strcmp(folder_type, "programs") == 0) {
              char path[MAX_PATH];
              if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAMS, NULL,
                                             SHGFP_TYPE_CURRENT, path))) {
                var_value = _strdup(path);
              }
            } else if (strcmp(folder_type, "start_menu") == 0) {
              char path[MAX_PATH];
              if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTMENU, NULL,
                                             SHGFP_TYPE_CURRENT, path))) {
                var_value = _strdup(path);
              }
            } else if (strcmp(folder_type, "startup") == 0) {
              char path[MAX_PATH];
              if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL,
                                             SHGFP_TYPE_CURRENT, path))) {
                var_value = _strdup(path);
              }
            } else if (strcmp(folder_type, "appdata") == 0) {
              char path[MAX_PATH];
              if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL,
                                             SHGFP_TYPE_CURRENT, path))) {
                var_value = _strdup(path);
              }
            } else if (strcmp(folder_type, "mydocuments") == 0) {
              char path[MAX_PATH];
              if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL,
                                             SHGFP_TYPE_CURRENT, path))) {
                var_value = _strdup(path);
              }
            } else if (strcmp(folder_type, "windows") == 0) {
              char path[MAX_PATH];
              GetWindowsDirectoryA(path, MAX_PATH);
              var_value = _strdup(path);
            } else if (strcmp(folder_type, "system") == 0) {
              char path[MAX_PATH];
              GetSystemDirectoryA(path, MAX_PATH);
              var_value = _strdup(path);
            } else if (strcmp(folder_type, "programfiles") == 0) {
              char path[MAX_PATH];
              SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL,
                               SHGFP_TYPE_CURRENT, path);
              var_value = _strdup(path);
            }
          } else if (strncmp(var_name, "sys.", 4) == 0) {
            char *env_var = var_name + 4;
            char *env_value = getenv(env_var);
            if (env_value) {
              var_value = _strdup(env_value);
            }
          } else if (strcmp(var_name, "clipboard") == 0) {
            // 简单实现：返回固定文本（实际实现需要更复杂的剪贴板操作）
            var_value = _strdup("clipboard_content");
          }

          // 如果找到了变量值，则替换
          if (var_value) {
            size_t value_len = strlen(var_value);
            if (out_pos + value_len < input_len * 2 + 1024) {
              strcpy(&output[out_pos], var_value);
              out_pos += value_len;
            }
            free(var_value);

            // 跳过已处理的变量部分（包括结束标记%]）
            in_pos = var_end + 2;
          } else {
            // 如果未识别变量，保留原样
            if (out_pos + (var_end - in_pos + 2) < input_len * 2 + 1024) {
              strncpy(&output[out_pos], &input[in_pos], var_end - in_pos + 2);
              out_pos += var_end - in_pos + 2;
            }
            in_pos = var_end + 2;
          }
        } else {
          // 变量名太长，保留原样
          if (out_pos + (var_end - in_pos + 2) < input_len * 2 + 1024) {
            strncpy(&output[out_pos], &input[in_pos], var_end - in_pos + 2);
            out_pos += var_end - in_pos + 2;
          }
          in_pos = var_end + 2;
        }
      } else {
        // 未找到结束标记，保留[%
        if (out_pos + 2 < input_len * 2 + 1024) {
          output[out_pos++] = input[in_pos++];
          output[out_pos++] = input[in_pos++];
        } else {
          break; // 防止缓冲区溢出
        }
      }
    } else {
      // 普通字符，直接复制
      if (out_pos < input_len * 2 + 1023) { // 留一个字符的空间给结尾\0
        output[out_pos++] = input[in_pos++];
      } else {
        break; // 防止缓冲区溢出
      }
    }
  }

  // 确保字符串结尾
  if (out_pos < input_len * 2 + 1024) {
    output[out_pos] = '\0';
  } else {
    output[input_len * 2 + 1023] = '\0';
  }

  return output;
}

void save_as_base64_data(const char *bitmap_data, DWORD data_size,
                         const char *filename) {
  size_t encoded_length;
  char *base64_data = base64_encode((const unsigned char *)bitmap_data,
                                    data_size, &encoded_length);

  if (base64_data == NULL) {
    printf("Error: Unable to encode data as Base64\n");
    return;
  }

  // Save base64 encoded data to file
  HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    DWORD dwBytesWritten;
    WriteFile(hFile, base64_data, (DWORD)encoded_length, &dwBytesWritten, NULL);
    CloseHandle(hFile);
    printf("Screenshot Base64 data saved to: %s\n", filename);
  } else {
    printf("Error: Unable to save Base64 data to %s\n", filename);
  }

  free(base64_data);
}

// 枚举窗口回调函数
BOOL CALLBACK EnumWindowsProcDisable(HWND hwnd, LPARAM lParam) {
  (void)lParam; // 防止未使用参数警告
  if (IsWindowVisible(hwnd)) {
    EnableWindow(hwnd, FALSE);
  }
  return TRUE;
}

BOOL CALLBACK EnumWindowsProcEnable(HWND hwnd, LPARAM lParam) {
  (void)lParam; // 防止未使用参数警告
  EnableWindow(hwnd, TRUE);
  return TRUE;
}

// 根据颜色名称获取RGB值
COLORREF GetColorByName(const char *colorName) {
  if (_stricmp(colorName, "white") == 0)
    return RGB(255, 255, 255);
  if (_stricmp(colorName, "black") == 0)
    return RGB(0, 0, 0);
  if (_stricmp(colorName, "red") == 0)
    return RGB(255, 0, 0);
  if (_stricmp(colorName, "green") == 0)
    return RGB(0, 255, 0);
  if (_stricmp(colorName, "blue") == 0)
    return RGB(0, 0, 255);
  if (_stricmp(colorName, "yellow") == 0)
    return RGB(255, 255, 0);
  if (_stricmp(colorName, "cyan") == 0)
    return RGB(0, 255, 255);
  if (_stricmp(colorName, "magenta") == 0)
    return RGB(255, 0, 255);
  if (_stricmp(colorName, "gray") == 0)
    return RGB(128, 128, 128);
  if (_stricmp(colorName, "lightgray") == 0)
    return RGB(211, 211, 211);
  if (_stricmp(colorName, "darkgray") == 0)
    return RGB(169, 169, 169);
  if (_stricmp(colorName, "orange") == 0)
    return RGB(255, 165, 0);
  if (_stricmp(colorName, "purple") == 0)
    return RGB(128, 0, 128);
  if (_stricmp(colorName, "brown") == 0)
    return RGB(165, 42, 42);
  if (_stricmp(colorName, "pink") == 0)
    return RGB(255, 192, 203);
  if (_stricmp(colorName, "lime") == 0)
    return RGB(0, 255, 0);
  if (_stricmp(colorName, "navy") == 0)
    return RGB(0, 0, 128);
  if (_stricmp(colorName, "teal") == 0)
    return RGB(0, 128, 128);
  if (_stricmp(colorName, "olive") == 0)
    return RGB(128, 128, 0);

  // 默认返回白色
  return RGB(255, 255, 255);
}

// 自定义弹窗窗口过程函数
LRESULT CALLBACK WindowWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
  static WindowParams *params = NULL;
  static HFONT hFont = NULL;
  static HWND hButton = NULL;
  static BOOL flashTimerActive = FALSE; // 闪亮定时器状态
  static UINT_PTR flashTimerId = 0;     // 闪亮定时器ID

  switch (msg) {
  case WM_CREATE: {
    // 获取传递的参数
    CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
    params = (WindowParams *)pcs->lpCreateParams;

    // 调试输出
    if (params) {
      // 初始化文字颜色为黑色（如果未指定）
      if (params->textColor == 0) {
        params->textColor = RGB(0, 0, 0); // 默认黑色
      }
    }

    // 创建字体，使用系统默认字体以确保中文支持
    hFont = CreateFontA(params ? params->fontSize : 18, 0, 0, 0,
                        params && params->bold ? FW_BOLD : FW_NORMAL, FALSE,
                        FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                        DEFAULT_PITCH | FF_SWISS, "Microsoft Sans Serif");

    // 如果上面的字体不可用，使用系统默认GUI字体
    if (!hFont) {
      hFont = GetStockObject(DEFAULT_GUI_FONT);
    }

    // 加载系统信息图标
    HICON hIcon = LoadIcon(NULL, IDI_INFORMATION);
    if (hIcon) {
      // 设置大图标
      SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
      // 设置小图标
      SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    // 创建确认按钮，使用宽字符确保中文支持
    wchar_t buttonText[] = L"确定";
    hButton =
        CreateWindowW(L"BUTTON", buttonText,
                      WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0,
                      0, 80, 30, hwnd, (HMENU)1, pcs->hInstance, NULL);

    // 设置按钮字体
    if (hFont) {
      SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    // 如果是模态窗口，启动闪亮定时器（每2秒闪亮一次）
    if (params && params->modal) {
      flashTimerId = SetTimer(hwnd, 1, 2000, NULL);
      flashTimerActive = TRUE;

      // 立即闪亮一次以吸引用户注意
      FLASHWINFO fwi;
      fwi.cbSize = sizeof(FLASHWINFO);
      fwi.hwnd = hwnd;
      fwi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG; // 闪亮整个窗口，不抢占焦点
      fwi.uCount = 2;                              // 闪亮2次
      fwi.dwTimeout = 0;                           // 使用默认时间间隔
      FlashWindowEx(&fwi);
    }

    break;
  }

  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // 获取客户区大小
    RECT rect;
    GetClientRect(hwnd, &rect);

    // 创建画笔和画刷
    HBRUSH hBrush = CreateSolidBrush(params->bgColor);
    HPEN hPen = CreatePen(PS_SOLID, 1, params->bgColor);

    // 选择画笔和画刷到设备上下文
    HGDIOBJ oldBrush = SelectObject(hdc, hBrush);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);

    // 填充整个客户区背景
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

    // 恢复旧的画笔和画刷
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);

    // 删除画笔和画刷
    DeleteObject(hBrush);
    DeleteObject(hPen);

    // 设置文本颜色和背景模式
    SetTextColor(hdc, params->textColor);
    SetBkMode(hdc, TRANSPARENT);

    // 选择字体
    if (hFont) {
      SelectObject(hdc, hFont);
    }

    // 直接使用处理后的文本
    char *displayText = params->text;

    // 计算文本绘制区域以实现真正的垂直居中
    RECT textRect = rect;
    // 为按钮留出空间（按钮高度30像素 + 边距10像素）
    textRect.bottom -= 40;

    // 计算文本实际占用的矩形区域
    RECT calcRect = textRect;
    DrawTextA(hdc, displayText, -1, &calcRect,
              DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL | DT_CALCRECT);

    // 调整垂直位置以实现居中
    int textHeight = calcRect.bottom - calcRect.top;
    int availableHeight = textRect.bottom - textRect.top;
    if (textHeight < availableHeight) {
      int offset = (availableHeight - textHeight) / 2;
      textRect.top += offset;
      textRect.bottom = textRect.top + textHeight;
    }

    // 绘制文本
    DrawTextA(hdc, displayText, -1, &textRect,
              DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL);

    EndPaint(hwnd, &ps);
    break;
  }

  case WM_COMMAND: {
    if (LOWORD(wParam) == 1) { // 点击了确认按钮
      DestroyWindow(hwnd);
    }
    break;
  }

  case WM_SIZE: {
    // 调整按钮位置到窗口底部居中
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    // 计算按钮位置，使其在底部居中
    int buttonWidth = 80;
    int buttonHeight = 30;
    int buttonX = (rcClient.right - buttonWidth) / 2;  // 水平居中
    int buttonY = rcClient.bottom - buttonHeight - 10; // 底部留10像素边距

    // 移动按钮到计算出的位置
    if (hButton) {
      MoveWindow(hButton, buttonX, buttonY, buttonWidth, buttonHeight, TRUE);
    }
    break;
  }

  case WM_TIMER: {
    if (wParam == 1) { // 闪亮定时器
      // 使窗口闪亮以提醒用户注意
      FLASHWINFO fwi;
      fwi.cbSize = sizeof(FLASHWINFO);
      fwi.hwnd = hwnd;
      fwi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG; // 闪亮整个窗口，不抢占焦点
      fwi.uCount = 3;                              // 闪亮3次
      fwi.dwTimeout = 0;                           // 使用默认时间间隔
      FlashWindowEx(&fwi);
    }
    break;
  }

  case WM_NCHITTEST: {
    // 如果禁止拖拽，阻止窗口移动
    if (params && params->noDrag) {
      // 调试输出
      // printf("Debug: Blocking drag, returning HTCLIENT\n");
      // 直接返回HTCLIENT，阻止标题栏拖拽
      return HTCLIENT;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  case WM_DESTROY: {
    // 销毁按钮窗口
    if (hButton) {
      DestroyWindow(hButton);
      hButton = NULL;
    }

    // 清理字体资源
    if (hFont) {
      DeleteObject(hFont);
      hFont = NULL;
    }

    // 清理定时器
    if (flashTimerActive) {
      KillTimer(hwnd, flashTimerId);
      flashTimerActive = FALSE;
    }

    PostQuitMessage(0);
    break;
  }

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

void cmd_window(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Window command help:\n");
    printf("  spcmd window --text=message [--title=title] [--width=width] "
           "[--height=height] [--fontsize=size] [--bgcolor=color] "
           "[--textcolor=color] [--bold] [--modal] [--nodrag]\n\n");
    printf("Parameter description:\n");
    printf("  --text=message     - Window display text (required)\n");
    printf("  --title=title      - Window title, default is \"系统提示\"\n");
    printf("  --width=width      - Window width in pixels, default is 600\n");
    printf("  --height=height    - Window height in pixels, default is 400\n");
    printf("  --fontsize=size    - Font size, default is 18\n");
    printf("  --bgcolor=color    - Background color as name "
           "(white,black,red,green,blue,yellow,cyan,magenta,gray,orange,purple,"
           "pink,lightblue,lightgreen,lightgray) or RGB values (r,g,b), "
           "default is white\n");
    printf("  --textcolor=color  - Text color as name or RGB values, default "
           "is black\n");
    printf("  --bold             - Set text to bold\n");
    printf("  --modal            - Make window modal (blocks other windows "
           "until closed and enables forced interaction)\n");
    printf("  --nodrag           - Disable window dragging\n\n");
    printf("Examples:\n");
    printf("  spcmd window --text=\"Hello World\"\n");
    printf("  spcmd window --text=\"Line 1\\nLine 2\\nLine 3\" "
           "--title=\"Multi-line Text\" --width=500 --height=300\n");
    printf("  spcmd window --text=\"Red background window\" --bgcolor=red "
           "--fontsize=20\n");
    printf("  spcmd window --text=\"Blue text on yellow background\" "
           "--bgcolor=yellow --textcolor=blue\n");
    printf("  spcmd window --text=\"Bold text example\" --bold\n");
    printf("  spcmd window --text=\"Modal window with forced interaction\" "
           "--modal\n");
    return;
  }

  // Parse parameters
  char *message = NULL;
  char title[256] = "SYSTEM INFORMATION";
  int width = 600;
  int height = 400;
  int fontSize = 18;
  COLORREF bgColor = RGB(255, 255, 255); // 默认白色背景
  COLORREF textColor = RGB(0, 0, 0);     // 默认黑色文字
  BOOL modal = FALSE;
  BOOL noDrag = FALSE;
  BOOL bold = FALSE;
  BOOL hasText = FALSE;

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--text=", 7) == 0) {
      message = argv[i] + 7; // 跳过 "--text=" 前缀
      hasText = TRUE;
    } else if (strncmp(argv[i], "--title=", 8) == 0) {
      strncpy(title, argv[i] + 8, sizeof(title) - 1); // 跳过 "--title=" 前缀
      title[sizeof(title) - 1] = '\0';
    } else if (strncmp(argv[i], "--width=", 8) == 0) {
      width = atoi(argv[i] + 8); // 跳过 "--width=" 前缀
    } else if (strncmp(argv[i], "--height=", 9) == 0) {
      height = atoi(argv[i] + 9); // 跳过 "--height=" 前缀
    } else if (strncmp(argv[i], "--fontsize=", 11) == 0) {
      fontSize = atoi(argv[i] + 11); // 跳过 "--fontsize=" 前缀
    } else if (strncmp(argv[i], "--bgcolor=", 10) == 0) {
      char colorStr[256];
      strncpy(colorStr, argv[i] + 10,
              sizeof(colorStr) - 1); // 跳过 "--bgcolor=" 前缀
      colorStr[sizeof(colorStr) - 1] = '\0';

      // 检查是否为RGB格式 (r,g,b)
      if (strchr(colorStr, ',') != NULL) {
        int r, g, b;
        if (sscanf(colorStr, "%d,%d,%d", &r, &g, &b) == 3) {
          bgColor = RGB(r, g, b);
        }
      } else {
        // 使用颜色名称
        bgColor = GetColorByName(colorStr);
      }
    } else if (strncmp(argv[i], "--textcolor=", 12) == 0) {
      char colorStr[256];
      strncpy(colorStr, argv[i] + 12,
              sizeof(colorStr) - 1); // 跳过 "--textcolor=" 前缀
      colorStr[sizeof(colorStr) - 1] = '\0';

      // 检查是否为RGB格式 (r,g,b)
      if (strchr(colorStr, ',') != NULL) {
        int r, g, b;
        if (sscanf(colorStr, "%d,%d,%d", &r, &g, &b) == 3) {
          textColor = RGB(r, g, b);
        }
      } else {
        // 使用颜色名称
        textColor = GetColorByName(colorStr);
      }
    } else if (strcmp(argv[i], "--modal") == 0) {
      modal = TRUE;
    } else if (strcmp(argv[i], "--nodrag") == 0) {
      noDrag = TRUE;
    } else if (strcmp(argv[i], "--bold") == 0) {
      bold = TRUE;
    }
  } 

  // Check if required parameters are provided
  if (!hasText) {
    printf("Error: Text parameter is required.\n");
    printf("Use 'spcmd window /help' for usage information.\n");
    return;
  }


  // 处理命令行参数中的换行符（将\\n替换为\n）
  char *processedMessage = (char *)malloc(strlen(message) * 2 + 1);
  int j = 0;
  for (int i = 0; message[i] != '\0'; i++) {
    if (message[i] == '\\' && message[i + 1] == 'n') {
      processedMessage[j++] = '\n';
      i++; // 跳过'n'
    } else {
      processedMessage[j++] = message[i];
    }
  }
  processedMessage[j] = '\0';

  // 如果是模态弹窗，禁用所有其他窗口
  if (modal) {
    EnumWindows(EnumWindowsProcDisable, (LPARAM)NULL);
  }

  // 创建窗口参数结构
  WindowParams *params = (WindowParams *)malloc(sizeof(WindowParams));
  if (!params) {
    printf("Error: Memory allocation failed.\n");
    free(processedMessage);
    return;
  }

  // 初始化参数
  params->text = processedMessage;
  params->fontSize = fontSize;
  params->bgColor = bgColor;
  params->textColor = textColor;
  params->modal = modal;
  params->noDrag = noDrag;
  params->bold = bold;

  // 注册窗口类
  WNDCLASSA wc = {0};
  wc.lpfnWndProc = WindowWndProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = "WindowClass";
  wc.hbrBackground = CreateSolidBrush(bgColor); // 使用指定的背景色
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);

  RegisterClassA(&wc);

  // 计算窗口位置，使其居中显示
  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  int x = (screenWidth - width) / 2;
  int y = (screenHeight - height) / 2;

  // 创建窗口
  CreateWindowExA(WS_EX_TOPMOST | WS_EX_APPWINDOW, // 强制顶层显示
                              "WindowClass",
                              title, // 直接使用标题
                              noDrag ? (WS_POPUP | WS_SYSMENU | WS_VISIBLE)
                                     : // 禁止拖拽时使用弹出窗口样式
                                  (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                                   WS_VISIBLE), // 正常情况显示标题栏
                              x, y, width, height, NULL, NULL,
                              GetModuleHandle(NULL), params);



  // 消息循环
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (msg.message == WM_QUIT) {
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // 注销窗口类
  UnregisterClassA("WindowClass", GetModuleHandle(NULL));

  // 如果是模态弹窗，重新启用所有窗口
  if (modal) {
    EnumWindows(EnumWindowsProcEnable, (LPARAM)NULL);
  }

  // 清理资源
  free(params);
  free(processedMessage);
}

// 检查当前进程是否以管理员权限运行
BOOL IsRunAsAdmin() {
  BOOL fIsRunAsAdmin = FALSE;
  DWORD dwError = ERROR_SUCCESS;
  PSID pAdministratorsGroup = NULL;

  // 创建管理员组SID
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                &pAdministratorsGroup)) {
    dwError = GetLastError();
    goto Cleanup;
  }

  // 检查令牌是否包含管理员组
  if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin)) {
    dwError = GetLastError();
    goto Cleanup;
  }

Cleanup:
  // 清理资源
  if (pAdministratorsGroup) {
    FreeSid(pAdministratorsGroup);
    pAdministratorsGroup = NULL;
  }

  // 忽略ERROR_NO_TOKEN错误，这是正常情况
  if (ERROR_NO_TOKEN == dwError) {
    dwError = ERROR_SUCCESS;
  }

  return fIsRunAsAdmin;
}

// 提升权限并重新运行程序
BOOL ElevatePrivileges(int argc, char *argv[]) {
  wchar_t szPath[MAX_PATH];
  wchar_t szCmdLine[1024] = {0};

  // 获取当前程序路径
  if (!GetModuleFileNameW(NULL, szPath, MAX_PATH)) {
    return FALSE;
  }

  // 构建命令行参数
  wcscat(szCmdLine, L"\"");
  wcscat(szCmdLine, szPath);
  wcscat(szCmdLine, L"\" ");

  // 添加原始参数
  for (int i = 1; i < argc; i++) {
    wchar_t argW[512];
    MultiByteToWideChar(CP_ACP, 0, argv[i], -1, argW, 512);

    wcscat(szCmdLine, L"\"");
    wcscat(szCmdLine, argW);
    wcscat(szCmdLine, L"\" ");
  }

  // 初始化SHELLEXECUTEINFO结构
  SHELLEXECUTEINFOW sei = {0};
  sei.cbSize = sizeof(SHELLEXECUTEINFOW);
  sei.lpVerb = L"runas"; // 请求提升权限
  sei.lpFile = szPath;
  sei.lpParameters = szCmdLine + wcslen(szPath) + 3; // 跳过程序路径部分
  sei.hwnd = NULL;
  sei.nShow = SW_NORMAL;
  sei.fMask = SEE_MASK_NOCLOSEPROCESS;

  // 尝试以管理员权限运行
  if (!ShellExecuteExW(&sei)) {
    DWORD dwError = GetLastError();
    if (dwError == ERROR_CANCELLED) {
      // 用户拒绝了UAC提示
      printf("用户拒绝了权限提升请求\n");
    } else {
      printf("权限提升失败，错误代码: %lu\n", dwError);
    }
    return FALSE;
  }

  // 等待新进程完成
  WaitForSingleObject(sei.hProcess, INFINITE);
  // 获取退出代码
  DWORD exitCode;
  GetExitCodeProcess(sei.hProcess, &exitCode);

  // 关闭进程句柄
  CloseHandle(sei.hProcess);
  // 退出当前进程
  ExitProcess(exitCode);

  return TRUE;
}

void cmd_notify(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Notify command help:\n");
    printf("  spcmd notify --title=title --message=message [--icon=info|warning|error] [--timeout=seconds]\n\n");
    printf("Parameter description:\n");
    printf("  --title=title         - Notification title (required)\n");
    printf("  --message=message     - Notification message (required)\n");
    printf("  --icon=type           - Icon type: info, warning, error (default: info)\n");
    printf("  --timeout=seconds     - Notification timeout in seconds (default: 5)\n\n");
    printf("Examples:\n");
    printf("  spcmd notify --title=\"System Update\" --message=\"Your system has been updated successfully.\"\n");
    printf("  spcmd notify --title=\"Warning\" --message=\"Disk space is low.\" --icon=warning\n");
    printf("  spcmd notify --title=\"Error\" --message=\"Application failed to start.\" --icon=error --timeout=10\n");
    return;
  }

  // Parse parameters
  char title[MAX_PATH] = {0};
  char message[MAX_PATH * 2] = {0};
  char iconType[20] = "info";  // default icon
  int timeout = 5;  // default timeout in seconds

  BOOL hasTitle = FALSE;
  BOOL hasMessage = FALSE;

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--title=", 8) == 0) {
      strncpy(title, argv[i] + 8, MAX_PATH - 1);
      title[MAX_PATH - 1] = '\0';
      hasTitle = TRUE;
    } else if (strncmp(argv[i], "--message=", 10) == 0) {
      strncpy(message, argv[i] + 10, MAX_PATH * 2 - 1);
      message[MAX_PATH * 2 - 1] = '\0';
      hasMessage = TRUE;
    } else if (strncmp(argv[i], "--icon=", 7) == 0) {
      strncpy(iconType, argv[i] + 7, sizeof(iconType) - 1);
      iconType[sizeof(iconType) - 1] = '\0';
    } else if (strncmp(argv[i], "--timeout=", 10) == 0) {
      timeout = atoi(argv[i] + 10);
      // Ensure timeout is reasonable
      if (timeout < 1)
        timeout = 1;
      if (timeout > 60)
        timeout = 60;
    }
  }

  // Check required parameters
  if (!hasTitle || !hasMessage) {
    printf("Error: Title and message must be specified\n");
    printf("Use spcmd notify --help for help\n");
    return;
  }

  printf("Displaying notification: %s\n", title);

  // 检查Windows版本以确定使用哪种通知方法
  OSVERSIONINFO osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osvi);
  
  BOOL isWindowsXP = (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1); // Windows XP是5.1版本
  
  if (isWindowsXP) {
    // Windows XP兼容实现 - 使用MessageBox
    printf("Using Windows XP compatible mode\n");
    
    // 确定消息框类型
    UINT mbType = MB_OK | MB_TOPMOST;
    if (strcmp(iconType, "warning") == 0) {
      mbType |= MB_ICONWARNING;
    } else if (strcmp(iconType, "error") == 0) {
      mbType |= MB_ICONERROR;
    } else {
      mbType |= MB_ICONINFORMATION;
    }
    
    // 显示消息框
    MessageBoxA(NULL, message, title, mbType);
    
    // 注意：在XP系统上，MessageBox是模态的，无法实现超时自动关闭
    // 用户需要手动点击确定按钮
    printf("Note: On Windows XP, the notification requires user interaction to close\n");
  } else {
    // Windows 7/10/11实现 - 使用Shell_NotifyIcon
    printf("Using modern Windows notification\n");
    
    // 注册窗口类
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "NotifyWindowClass";
    
    if (RegisterClassA(&wc)) {
      // 创建隐藏窗口
      HWND hwnd = CreateWindowExA(0, "NotifyWindowClass", "NotifyWindow", 
                                 0, 0, 0, 0, 0, NULL, NULL, 
                                 GetModuleHandle(NULL), NULL);
      
      if (hwnd) {
        // 初始化NOTIFYICONDATA结构
        NOTIFYICONDATAA nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATAA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        
        // 设置图标
        if (strcmp(iconType, "warning") == 0) {
          nid.hIcon = LoadIcon(NULL, IDI_WARNING);
        } else if (strcmp(iconType, "error") == 0) {
          nid.hIcon = LoadIcon(NULL, IDI_ERROR);
        } else {
          nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
        }
        
        // 设置提示文本
        strncpy(nid.szTip, title, sizeof(nid.szTip) - 1);
        nid.szTip[sizeof(nid.szTip) - 1] = '\0';
        
        // 添加通知图标
        Shell_NotifyIconA(NIM_ADD, &nid);
        
        // 更新通知内容
        nid.uFlags = NIF_INFO;
        strncpy(nid.szInfoTitle, title, sizeof(nid.szInfoTitle) - 1);
        nid.szInfoTitle[sizeof(nid.szInfoTitle) - 1] = '\0';
        strncpy(nid.szInfo, message, sizeof(nid.szInfo) - 1);
        nid.szInfo[sizeof(nid.szInfo) - 1] = '\0';
        
        // 设置信息标志
        if (strcmp(iconType, "warning") == 0) {
          nid.dwInfoFlags = NIIF_WARNING;
        } else if (strcmp(iconType, "error") == 0) {
          nid.dwInfoFlags = NIIF_ERROR;
        } else {
          nid.dwInfoFlags = NIIF_INFO;
        }
        
        // 显示通知
        Shell_NotifyIconA(NIM_MODIFY, &nid);
        
        // 等待指定时间
        Sleep(timeout * 1000);
        
        // 删除通知图标
        Shell_NotifyIconA(NIM_DELETE, &nid);
        
        // 销毁窗口
        DestroyWindow(hwnd);
      }
      
      // 注销窗口类
      UnregisterClassA("NotifyWindowClass", GetModuleHandle(NULL));
    } else {
      // 如果无法注册窗口类，回退到MessageBox
      printf("Falling back to MessageBox\n");
      UINT mbType = MB_OK | MB_TOPMOST;
      if (strcmp(iconType, "warning") == 0) {
        mbType |= MB_ICONWARNING;
      } else if (strcmp(iconType, "error") == 0) {
        mbType |= MB_ICONERROR;
      } else {
        mbType |= MB_ICONINFORMATION;
      }
      MessageBoxA(NULL, message, title, mbType);
    }
  }
  
  printf("Notification displayed successfully\n");
}

// INI文件处理的回调函数
struct IniEntry {
  char section[256];
  char name[256];
  char value[256];
  struct IniEntry *next;
};

struct IniData {
  struct IniEntry *head;
  struct IniEntry *tail;
};

// INI解析回调函数
static int config_ini_handler(void* user, const char* section, const char* name, const char* value) {
  struct IniData *data = (struct IniData*)user;
  struct IniEntry *entry = (struct IniEntry*)malloc(sizeof(struct IniEntry));
  
  if (!entry) return 0;
  
  strncpy(entry->section, section, sizeof(entry->section) - 1);
  entry->section[sizeof(entry->section) - 1] = '\0';
  
  strncpy(entry->name, name, sizeof(entry->name) - 1);
  entry->name[sizeof(entry->name) - 1] = '\0';
  
  strncpy(entry->value, value, sizeof(entry->value) - 1);
  entry->value[sizeof(entry->value) - 1] = '\0';
  
  entry->next = NULL;
  
  if (data->tail) {
    data->tail->next = entry;
  } else {
    data->head = entry;
  }
  data->tail = entry;
  
  return 1;
}

// 释放INI数据
void free_ini_data(struct IniData *data) {
  struct IniEntry *entry = data->head;
  while (entry) {
    struct IniEntry *next = entry->next;
    free(entry);
    entry = next;
  }
  data->head = NULL;
  data->tail = NULL;
}

// 查找INI条目
struct IniEntry* find_ini_entry(struct IniData *data, const char *section, const char *name) {
  struct IniEntry *entry = data->head;
  while (entry) {
    if (strcmp(entry->section, section) == 0 && strcmp(entry->name, name) == 0) {
      return entry;
    }
    entry = entry->next;
  }
  return NULL;
}

// 显示所有INI条目
void display_ini_data(struct IniData *data) {
  struct IniEntry *entry = data->head;
  while (entry) {
    printf("[%s] %s = %s\n", entry->section, entry->name, entry->value);
    entry = entry->next;
  }
}

void cmd_config(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Config command help:\n");
    printf("  spcmd config --file=path [--action=read|write|delete] [--section=section] [--key=key] [--value=value]\n\n");
    printf("Parameter description:\n");
    printf("  --file=path           - INI file path (required)\n");
    printf("  --action=action       - Action: read, write, delete (default: read)\n");
    printf("  --section=section     - Section name (required for write/delete)\n");
    printf("  --key=key             - Key name (required for write/delete)\n");
    printf("  --value=value         - Value (required for write)\n\n");
    printf("Examples:\n");
    printf("  spcmd config --file=config.ini --action=read\n");
    printf("  spcmd config --file=config.ini --action=write --section=General --key=Username --value=John\n");
    printf("  spcmd config --file=config.ini --action=delete --section=General --key=Username\n");
    return;
  }

  // Parse parameters
  char filePath[MAX_PATH] = {0};
  char action[20] = "read";  // default action
  char section[256] = {0};
  char key[256] = {0};
  char value[256] = {0};

  BOOL hasFile = FALSE;
  BOOL hasSection = FALSE;
  BOOL hasKey = FALSE;
  BOOL hasValue = FALSE;

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--file=", 7) == 0) {
      strncpy(filePath, argv[i] + 7, MAX_PATH - 1);
      filePath[MAX_PATH - 1] = '\0';
      hasFile = TRUE;
    } else if (strncmp(argv[i], "--action=", 9) == 0) {
      strncpy(action, argv[i] + 9, sizeof(action) - 1);
      action[sizeof(action) - 1] = '\0';
    } else if (strncmp(argv[i], "--section=", 10) == 0) {
      strncpy(section, argv[i] + 10, sizeof(section) - 1);
      section[sizeof(section) - 1] = '\0';
      hasSection = TRUE;
    } else if (strncmp(argv[i], "--key=", 6) == 0) {
      strncpy(key, argv[i] + 6, sizeof(key) - 1);
      key[sizeof(key) - 1] = '\0';
      hasKey = TRUE;
    } else if (strncmp(argv[i], "--value=", 8) == 0) {
      strncpy(value, argv[i] + 8, sizeof(value) - 1);
      value[sizeof(value) - 1] = '\0';
      hasValue = TRUE;
    }
  }

  // Check required parameters
  if (!hasFile) {
    printf("Error: File path must be specified\n");
    printf("Use spcmd config --help for help\n");
    return;
  }

  printf("Config file: %s\n", filePath);
  printf("Action: %s\n", action);

  if (strcmp(action, "read") == 0) {
    // 读取INI文件
    struct IniData data = {0};
    int result = ini_parse(filePath, config_ini_handler, &data);
    
    if (result == 0) {
      printf("Reading configuration:\n");
      if (data.head) {
        display_ini_data(&data);
      } else {
        printf("No entries found\n");
      }
      free_ini_data(&data);
    } else if (result == -1) {
      printf("Error: Unable to open file %s\n", filePath);
    } else {
      printf("Error: Parse error on line %d\n", result);
      free_ini_data(&data);
    }
  } else if (strcmp(action, "write") == 0) {
    // 写入INI文件
    if (!hasSection || !hasKey || !hasValue) {
      printf("Error: Section, key, and value must be specified for write action\n");
      printf("Use spcmd config --help for help\n");
      return;
    }
    
    printf("Writing: [%s] %s = %s\n", section, key, value);
    
    // 读取现有配置
    struct IniData data = {0};
    ini_parse(filePath, config_ini_handler, &data);
    
    // 查找是否已存在该条目
    struct IniEntry *entry = find_ini_entry(&data, section, key);
    if (entry) {
      // 更新现有条目
      strncpy(entry->value, value, sizeof(entry->value) - 1);
      entry->value[sizeof(entry->value) - 1] = '\0';
      printf("Updated existing entry\n");
    } else {
      // 添加新条目
      entry = (struct IniEntry*)malloc(sizeof(struct IniEntry));
      if (entry) {
        strncpy(entry->section, section, sizeof(entry->section) - 1);
        entry->section[sizeof(entry->section) - 1] = '\0';
        strncpy(entry->name, key, sizeof(entry->name) - 1);
        entry->name[sizeof(entry->name) - 1] = '\0';
        strncpy(entry->value, value, sizeof(entry->value) - 1);
        entry->value[sizeof(entry->value) - 1] = '\0';
        entry->next = NULL;
        
        if (data.tail) {
          data.tail->next = entry;
        } else {
          data.head = entry;
        }
        data.tail = entry;
        printf("Added new entry\n");
      }
    }
    
    // 写入文件
    FILE *file = fopen(filePath, "w");
    if (file) {
      // 按section分组写入
      char current_section[256] = {0};
      struct IniEntry *entry = data.head;
      while (entry) {
        if (strcmp(current_section, entry->section) != 0) {
          // 新section
          strcpy(current_section, entry->section);
          fprintf(file, "[%s]\n", current_section);
        }
        fprintf(file, "%s = %s\n", entry->name, entry->value);
        entry = entry->next;
      }
      fclose(file);
      printf("Configuration saved successfully\n");
    } else {
      printf("Error: Unable to write to file %s\n", filePath);
    }
    
    free_ini_data(&data);
  } else if (strcmp(action, "delete") == 0) {
    // 删除INI条目
    if (!hasSection || !hasKey) {
      printf("Error: Section and key must be specified for delete action\n");
      printf("Use spcmd config --help for help\n");
      return;
    }
    
    printf("Deleting: [%s] %s\n", section, key);
    
    // 读取现有配置
    struct IniData data = {0};
    ini_parse(filePath, config_ini_handler, &data);
    
    // 查找并删除条目
    struct IniEntry *prev = NULL;
    struct IniEntry *entry = data.head;
    BOOL found = FALSE;
    
    while (entry) {
      if (strcmp(entry->section, section) == 0 && strcmp(entry->name, key) == 0) {
        // 找到要删除的条目
        if (prev) {
          prev->next = entry->next;
        } else {
          data.head = entry->next;
        }
        if (data.tail == entry) {
          data.tail = prev;
        }
        free(entry);
        found = TRUE;
        printf("Entry deleted successfully\n");
        break;
      }
      prev = entry;
      entry = entry->next;
    }
    
    if (!found) {
      printf("Entry not found\n");
      free_ini_data(&data);
      return;
    }
    
    // 写入文件
    FILE *file = fopen(filePath, "w");
    if (file) {
      // 按section分组写入
      char current_section[256] = {0};
      entry = data.head;
      while (entry) {
        if (strcmp(current_section, entry->section) != 0) {
          // 新section
          strcpy(current_section, entry->section);
          fprintf(file, "[%s]\n", current_section);
        }
        fprintf(file, "%s = %s\n", entry->name, entry->value);
        entry = entry->next;
      }
      fclose(file);
      printf("Configuration saved successfully\n");
    } else {
      printf("Error: Unable to write to file %s\n", filePath);
    }
    
    free_ini_data(&data);
  } else {
    printf("Error: Unknown action '%s'. Use read, write, or delete\n", action);
  }
}

int cmd_process(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Process command help:\n");
    printf("  spcmd process [--action=check|kill] [--name=process_name] [--pid=process_id]\n\n");
    printf("Parameter description:\n");
    printf("  --action=action       - Action to perform: check (default), kill\n");
    printf("  --name=process_name   - Process name to check or kill\n");
    printf("  --pid=process_id      - Process ID to check or kill\n\n");
    printf("Examples:\n");
    printf("  spcmd process --name=notepad.exe\n");
    printf("  spcmd process --pid=1234\n");
    printf("  spcmd process --action=kill --name=notepad.exe\n");
    return 0;
  }

  // Parse parameters
  char action[20] = "check";  // default action
  char processName[MAX_PATH] = {0};
  DWORD processId = 0;

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--action=", 9) == 0) {
      strncpy(action, argv[i] + 9, sizeof(action) - 1);
      action[sizeof(action) - 1] = '\0';
    } else if (strncmp(argv[i], "--name=", 7) == 0) {
      strncpy(processName, argv[i] + 7, MAX_PATH - 1);
      processName[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--pid=", 6) == 0) {
      processId = atoi(argv[i] + 6);
    }
  }

  // Validate parameters
  if (strlen(processName) == 0 && processId == 0) {
    printf("Error: Either process name or PID must be specified\n");
    printf("Use spcmd process --help for help\n");
    return 1;
  }

  if (strcmp(action, "check") == 0) {
    // Check if process exists
    if (processId > 0) {
      // Check by PID
      HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
      if (hProcess != NULL) {
        printf("Process with PID %lu exists\n", processId);
        CloseHandle(hProcess);
        return 0; // Process exists
      } else {
        printf("Process with PID %lu does not exist\n", processId);
        return 1; // Process does not exist
      }
    } else {
      // Check by name
      DWORD foundPid = 0;
      if (find_process_by_name(processName, &foundPid)) {
        printf("Process '%s' exists with PID %lu\n", processName, foundPid);
        return 0; // Process exists
      } else {
        printf("Process '%s' does not exist\n", processName);
        return 1; // Process does not exist
      }
    }
  } else if (strcmp(action, "kill") == 0) {
    // Kill process
    if (processId > 0) {
      // Kill by PID
      printf("Killing process with PID: %lu\n", processId);
      HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
      if (hProcess != NULL) {
        if (TerminateProcess(hProcess, 0)) {
          printf("Process terminated successfully\n");
          CloseHandle(hProcess);
          return 0;
        } else {
          printf("Error: Failed to terminate process (Error code: %lu)\n", GetLastError());
          CloseHandle(hProcess);
          return 1;
        }
      } else {
        printf("Error: Failed to open process (Error code: %lu)\n", GetLastError());
        return 1;
      }
    } else {
      // Kill by name
      printf("Killing process with name: %s\n", processName);
      
      if (kill_process_by_name(processName)) {
        return 0;
      } else {
        printf("Process '%s' not found\n", processName);
        return 1;
      }
    }
  } else {
    printf("Error: Unknown action '%s'\n", action);
    printf("Use spcmd process --help for help\n");
    return 1;
  }
  
  // Default return value (should never reach here due to the logic above)
  return 1;
}

// 添加权限检查和提升权限的函数声明
BOOL IsRunAsAdmin();
BOOL ElevatePrivileges(int argc, char *argv[]);

// 通用进程查找函数
BOOL find_process_by_name(const char* processName, DWORD* processId) {
  PROCESSENTRY32 pe32;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  BOOL found = FALSE;

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
      do {
        if (_stricmp(pe32.szExeFile, processName) == 0) {
          if (processId != NULL) {
            *processId = pe32.th32ProcessID;
          }
          found = TRUE;
          break;
        }
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }
  
  return found;
}

// 通用进程终止函数
BOOL kill_process_by_name(const char* processName) {
  PROCESSENTRY32 pe32;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  BOOL found = FALSE;

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
      do {
        if (_stricmp(pe32.szExeFile, processName) == 0) {
          // 找到匹配的进程，杀掉它
          HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
          if (hProcess != NULL) {
            if (TerminateProcess(hProcess, 0)) {
              printf("Process '%s' with PID %lu terminated successfully\n", processName, pe32.th32ProcessID);
            } else {
              printf("Error: Unable to terminate process '%s' with PID %lu\n", processName, pe32.th32ProcessID);
            }
            CloseHandle(hProcess);
            found = TRUE;
          } else {
            printf("Error: Unable to open process '%s' with PID %lu\n", processName, pe32.th32ProcessID);
          }
        }
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  } else {
    printf("Error: Unable to create process snapshot\n");
  }
  
  return found;
}



// 获取系统时间戳（毫秒）
uint64_t get_current_timestamp_ms() {
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  uint64_t timestamp = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
  // 转换为Unix时间戳（毫秒）
  timestamp /= 10000; // 100-nanosecond intervals to milliseconds
  timestamp -= 11644473600000ULL; // 转换为Unix时间戳（从1970年1月1日开始）
  return timestamp;
}

// 生成UUID v4
void generate_uuid_v4(char *uuid_str) {
  // 生成随机数据
  uint32_t data[4];
  for (int i = 0; i < 4; i++) {
    data[i] = rand() * RAND_MAX + rand();
  }
  
  // 设置UUID版本（第13位为0100）
  data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;
  
  // 设置变体（第17位为10）
  data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;
  
  // 格式化为UUID字符串
  sprintf(uuid_str, "%08x-%04x-%04x-%04x-%04x%08x",
          data[0],
          data[1] & 0xFFFF,
          (data[1] >> 16) & 0xFFFF,
          data[2] & 0xFFFF,
          (data[2] >> 16) & 0xFFFF,
          data[3]);
}

// 生成UUID v7
void generate_uuid_v7(char *uuid_str) {
  // 获取当前时间戳（毫秒）
  uint64_t timestamp = get_current_timestamp_ms();
  
  // 生成随机数据
  uint32_t rand_a = rand() * RAND_MAX + rand();
  uint32_t rand_b = rand() * RAND_MAX + rand();
  
  // UUID v7格式：
  // 时间戳（48位）+ 随机数（12位）+ 版本（4位）+ 变体（2位）+ 随机数（62位）
  
  // 时间戳高位（32位）
  uint32_t ts_high = (timestamp >> 16) & 0xFFFFFFFF;
  
  // 时间戳低位（16位）+ 随机数高位（12位）
  uint32_t ts_low_rand = ((timestamp & 0xFFFF) << 12) | ((rand_a >> 20) & 0xFFF);
  
  // 随机数中位（16位），设置版本为0111（v7）
  uint32_t rand_mid = (rand_a >> 4) & 0x0FFF; // 清除版本位
  rand_mid |= 0x7000; // 设置版本为7
  
  // 随机数低位（16位），设置变体为10
  uint32_t rand_low = rand_b & 0x3FFF; // 清除变体位
  rand_low |= 0x8000; // 设置变体为10
  
  // 格式化为UUID字符串
  sprintf(uuid_str, "%08x-%04x-%04x-%04x-%04x%08x",
          ts_high,
          ts_low_rand & 0xFFFF,
          (ts_low_rand >> 16) & 0xFFFF,
          rand_mid,
          rand_low,
          rand_b >> 16);
}

// 雪花ID生成器结构
typedef struct {
  uint64_t last_timestamp;
  uint64_t node_id; // 节点ID（这里简化为固定值）
  uint64_t sequence; // 序列号
} snowflake_generator;

// 初始化雪花ID生成器
snowflake_generator init_snowflake() {
  snowflake_generator sf;
  sf.last_timestamp = 0;
  sf.node_id = 1; // 固定节点ID
  sf.sequence = 0;
  return sf;
}

// 生成雪花ID
uint64_t generate_snowflake_id(snowflake_generator *sf) {
  uint64_t timestamp = get_current_timestamp_ms();
  
  // 如果时钟回拨，等待
  if (timestamp < sf->last_timestamp) {
    // 简单处理，实际应用中可能需要更好的策略
    timestamp = sf->last_timestamp;
  }
  
  // 如果同一毫秒内，序列号递增
  if (timestamp == sf->last_timestamp) {
    sf->sequence = (sf->sequence + 1) & 0xFFF; // 12位序列号
    // 如果序列号溢出，等待下一毫秒
    if (sf->sequence == 0) {
      while (timestamp <= sf->last_timestamp) {
        timestamp = get_current_timestamp_ms();
      }
    }
  } else {
    sf->sequence = 0;
  }
  
  sf->last_timestamp = timestamp;
  
  // 组装雪花ID
  // 时间戳（42位）+ 节点ID（10位）+ 序列号（12位）
  uint64_t id = (timestamp << 22) | (sf->node_id << 12) | sf->sequence;
  return id;
}

char* cmd_random(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Random command help:\n");
    printf("  spcmd random [--type=uuid4|uuid7|snowflake|number] [--count=n]\n\n");
    printf("Parameter description:\n");
    printf("  --type=type           - Type of random ID: uuid4, uuid7, snowflake, number (default: uuid4)\n");
    printf("  --count=n             - Number of IDs to generate (default: 1)\n\n");
    printf("Examples:\n");
    printf("  spcmd random\n");
    printf("  spcmd random --type=uuid7\n");
    printf("  spcmd random --type=snowflake --count=5\n");
    printf("  spcmd random --type=number\n");
    return NULL;
  }

  // Parse parameters
  char type[20] = "uuid4";  // default type
  int count = 1;  // default count

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--type=", 7) == 0) {
      strncpy(type, argv[i] + 7, sizeof(type) - 1);
      type[sizeof(type) - 1] = '\0';
    } else if (strncmp(argv[i], "--count=", 8) == 0) {
      count = atoi(argv[i] + 8);
      if (count < 1) count = 1;
      if (count > 100) count = 100; // 限制最大数量
    }
  }

  // 初始化随机数种子
  srand((unsigned int)time(NULL));

  // 为结果分配内存
  char *result = (char*)malloc(4096); // 分配足够大的缓冲区
  if (!result) {
    printf("Error: Memory allocation failed\n");
    return NULL;
  }
  result[0] = '\0'; // 初始化为空字符串

  char temp[256]; // 临时缓冲区
  
  if (strcmp(type, "uuid4") == 0) {
    char uuid_str[37]; // UUID字符串长度为36字符+1个结束符
    for (int i = 0; i < count; i++) {
      generate_uuid_v4(uuid_str);
      sprintf(temp, "%s\n", uuid_str);
      strcat(result, temp);
    }
  } else if (strcmp(type, "uuid7") == 0) {
    char uuid_str[37]; // UUID字符串长度为36字符+1个结束符
    for (int i = 0; i < count; i++) {
      generate_uuid_v7(uuid_str);
      sprintf(temp, "%s\n", uuid_str);
      strcat(result, temp);
    }
  } else if (strcmp(type, "snowflake") == 0) {
    snowflake_generator sf = init_snowflake();
    for (int i = 0; i < count; i++) {
      uint64_t id = generate_snowflake_id(&sf);
      sprintf(temp, "%I64u\n", id);
      strcat(result, temp);
    }
  } else if (strcmp(type, "number") == 0) {
    for (int i = 0; i < count; i++) {
      sprintf(temp, "%d\n", rand());
      strcat(result, temp);
    }
  } else {
    printf("Error: Unknown type '%s'. Use uuid4, uuid7, snowflake, or number\n", type);
    free(result);
    return NULL;
  }
  
  return result;
}



// 日志轮转切割命令
void cmd_logrotate(int argc, char *argv[]) {
  // logrotate [--path=path_to_log_file] [--maxsize=size_in_mb] [--daily]
  // Rotates log files based on size or daily schedule.

  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Logrotate command help:\n");
    printf("  spcmd logrotate [--path=path_to_log_file] [--maxsize=size_in_mb] [--daily]\n\n");
    printf("Parameter description:\n");
    printf("  --path=path_to_log_file  - Path to the log file to rotate\n");
    printf("  --maxsize=size_in_mb     - Maximum size in MB before rotation (default: 10)\n");
    printf("  --daily                  - Rotate daily (default: size-based rotation)\n\n");
    printf("Examples:\n");
    printf("  spcmd logrotate --path=\"C:\\Logs\\app.log\" --maxsize=50\n");
    printf("  spcmd logrotate --path=\"C:\\Logs\\app.log\" --daily\n");
    return;
  }

  // Parse parameters
  char path[MAX_PATH] = "app.log";  // default log file
  int max_size_mb = 10;             // default max size in MB
  BOOL daily = FALSE;               // default to size-based rotation

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--path=", 7) == 0) {
      strncpy(path, argv[i] + 7, MAX_PATH - 1);
      path[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--maxsize=", 10) == 0) {
      max_size_mb = atoi(argv[i] + 10);
      if (max_size_mb < 1)
        max_size_mb = 1;
    } else if (strcmp(argv[i], "--daily") == 0) {
      daily = TRUE;
    }
  }

  // Check if file exists
  WIN32_FILE_ATTRIBUTE_DATA fileAttr;
  if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fileAttr)) {
    printf("Error: Log file '%s' does not exist or cannot be accessed\n", path);
    return;
  }

  // Daily rotation
  if (daily) {
    // Get current date
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    // Create backup filename with current date
    char backup_path[MAX_PATH];
    sprintf(backup_path, "%s.%04d%02d%02d", path, st.wYear, st.wMonth, st.wDay);
    
    // Check if backup file already exists for today
    if (GetFileAttributesA(backup_path) != INVALID_FILE_ATTRIBUTES) {
      printf("Log file already rotated today. Backup exists as '%s'\n", backup_path);
      return;
    }
    
    // Rename current log file to backup
    if (MoveFileA(path, backup_path)) {
      printf("Log file rotated successfully. Backup created as '%s'\n", backup_path);
      
      // Create new empty log file
      if (create_empty_file(path)) {
        printf("New log file '%s' created\n", path);
      }
    } else {
      printf("Error: Failed to rotate log file. Error code: %lu\n", GetLastError());
    }
    
    return;
  }

  // Size-based rotation (original behavior)
  // Calculate file size in bytes
  LARGE_INTEGER fileSize;
  fileSize.LowPart = fileAttr.nFileSizeLow;
  fileSize.HighPart = fileAttr.nFileSizeHigh;
  unsigned long long size_bytes = ((unsigned long long)fileSize.HighPart << 32) | fileSize.LowPart;
  unsigned long long max_size_bytes = (unsigned long long)max_size_mb * 1024 * 1024;

  // Debug output
  printf("File: %s\n", path);
  printf("Current size: %I64u bytes\n", size_bytes);
  printf("Threshold: %I64u bytes (%d MB)\n", max_size_bytes, max_size_mb);

  // Check if file needs rotation
  if (size_bytes < max_size_bytes) {
    printf("Log file '%s' size (%I64u bytes) is below threshold (%I64u bytes). No rotation needed.\n", 
           path, size_bytes, max_size_bytes);
    return;
  }

  rotate_log_file(path, size_bytes);
}

// 通用创建空文件函数
BOOL create_empty_file(const char* path) {
  HANDLE hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    CloseHandle(hFile);
    return TRUE;
  } else {
    printf("Warning: Failed to create file '%s'\n", path);
    return FALSE;
  }
}

// 通用日志文件轮转函数
void rotate_log_file(const char* path, ULONGLONG size_bytes) {
  if (size_bytes < 1) {
    return;
  }

  printf("Rotating log file '%s' (size: %I64u bytes)\n", path, size_bytes);

  // Perform log rotation
  char backup_path[MAX_PATH];
  SYSTEMTIME st;
  GetLocalTime(&st);
  
  // Create backup filename with timestamp
  sprintf(backup_path, "%s.%04d%02d%02d_%02d%02d%02d", 
          path, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

  // Rename current log file to backup
  if (MoveFileA(path, backup_path)) {
    printf("Log file rotated successfully. Backup created as '%s'\n", backup_path);
    
    // Create new empty log file
    if (create_empty_file(path)) {
      printf("New log file '%s' created\n", path);
    }
  } else {
    printf("Error: Failed to rotate log file. Error code: %lu\n", GetLastError());
  }
}

// 托盘图标相关的常量定义
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_APP_ICON 1001
#define ID_TRAY_EXIT 1002
#define ID_TRAY_ABOUT 1003

// 托盘图标数据结构
typedef struct {
  HWND hwnd;
  HMENU hMenu;
  NOTIFYICONDATA nid;
  char process_name[MAX_PATH];
  char icon_path[MAX_PATH];
  BOOL process_running;
  HICON hIcon;  // 存储加载的图标
} TrayIconData;

// 托盘图标窗口过程函数声明
LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 检查进程是否运行的函数
BOOL is_process_running(const char* processName) {
  PROCESSENTRY32 pe32;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  BOOL found = FALSE;

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
      do {
        if (_stricmp(pe32.szExeFile, processName) == 0) {
          found = TRUE;
          break;
        }
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }
  
  return found;
}

// 从exe文件提取图标
HICON extract_icon_from_exe(const char* exePath) {
  HICON hIcon = NULL;
  
  // 首先尝试提取大图标
  ExtractIconExA(exePath, 0, &hIcon, NULL, 1);
  
  // 如果没有提取到图标，使用默认图标
  if (hIcon == NULL) {
    hIcon = LoadIcon(NULL, IDI_APPLICATION);
  }
  
  return hIcon;
}

// 更新托盘图标状态
void update_tray_icon(TrayIconData* trayData) {
  // 检查进程状态
  trayData->process_running = is_process_running(trayData->process_name);
  
  // 如果进程未运行，删除托盘图标并退出
  if (!trayData->process_running) {
    // 删除托盘图标
    Shell_NotifyIcon(NIM_DELETE, &trayData->nid);
    // 发送退出消息
    PostMessage(trayData->hwnd, WM_DESTROY, 0, 0);
    return;
  }
  
  // 进程运行时，确保托盘图标存在
  // 修复：检查nid.hWnd是否有效，而不是是否为NULL
  if (trayData->nid.hWnd == NULL || trayData->nid.hWnd != trayData->hwnd) {
    // 重新添加托盘图标
    trayData->nid.hWnd = trayData->hwnd;
    trayData->nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    Shell_NotifyIcon(NIM_ADD, &trayData->nid);
  }
  
  // 更新图标提示文本
  snprintf(trayData->nid.szTip, sizeof(trayData->nid.szTip), 
           "%s - Running", trayData->process_name);
  
  // 根据图标路径设置图标
  if (strlen(trayData->icon_path) > 0 && PathFileExistsA(trayData->icon_path)) {
    // 如果指定了图标路径且文件存在，使用指定图标
    HICON hNewIcon = extract_icon_from_exe(trayData->icon_path);
    if (hNewIcon != NULL) {
      // 删除旧图标（如果不是系统图标）
      if (trayData->hIcon != NULL && 
          trayData->hIcon != LoadIcon(NULL, IDI_APPLICATION) &&
          trayData->hIcon != LoadIcon(NULL, IDI_INFORMATION) &&
          trayData->hIcon != LoadIcon(NULL, IDI_ERROR)) {
        DestroyIcon(trayData->hIcon);
      }
      trayData->hIcon = hNewIcon;
      trayData->nid.hIcon = trayData->hIcon;
    }
  } else {
    // 进程运行时使用信息图标
    if (trayData->hIcon != NULL) {
      DestroyIcon(trayData->hIcon);
    }
    trayData->hIcon = LoadIcon(NULL, IDI_INFORMATION);
    trayData->nid.hIcon = trayData->hIcon;
  }
  
  // 更新托盘图标
  Shell_NotifyIcon(NIM_MODIFY, &trayData->nid);
}

// 创建托盘图标 - XP兼容版本
BOOL create_tray_icon(TrayIconData* trayData, HINSTANCE hInstance, const char* title, 
                      const char* processName, const char* iconPath) {
  // 注册窗口类
  WNDCLASS wc = {0};
  wc.lpfnWndProc = TrayWndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "SPCMDTrayIconClass";
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  
  if (!RegisterClass(&wc)) {
    return FALSE;
  }
  
  // 创建隐藏窗口
  trayData->hwnd = CreateWindowEx(0, "SPCMDTrayIconClass", "SPCMD Tray Icon Window",
                                  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                  CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
  
  if (!trayData->hwnd) {
    return FALSE;
  }
  
  // 保存窗口实例数据
  SetWindowLongPtr(trayData->hwnd, GWLP_USERDATA, (LONG_PTR)trayData);
  
  // 初始化NOTIFYICONDATA结构 - XP兼容版本
  memset(&trayData->nid, 0, sizeof(NOTIFYICONDATA));
  trayData->nid.cbSize = sizeof(NOTIFYICONDATA);
  trayData->nid.hWnd = trayData->hwnd;
  trayData->nid.uID = ID_TRAY_APP_ICON;
  trayData->nid.uCallbackMessage = WM_TRAYICON;
  
  // 设置提示文本
  strncpy(trayData->nid.szTip, title, sizeof(trayData->nid.szTip) - 1);
  trayData->nid.szTip[sizeof(trayData->nid.szTip) - 1] = '\0';
  
  // 保存进程名和图标路径
  strncpy(trayData->process_name, processName, sizeof(trayData->process_name) - 1);
  trayData->process_name[sizeof(trayData->process_name) - 1] = '\0';
  
  strncpy(trayData->icon_path, iconPath, sizeof(trayData->icon_path) - 1);
  trayData->icon_path[sizeof(trayData->icon_path) - 1] = '\0';
  
  // 初始化图标
  trayData->hIcon = NULL;
  
  // 检查进程是否正在运行
  trayData->process_running = is_process_running(processName);
  
  // 如果进程未运行，不创建托盘图标
  if (!trayData->process_running) {
    printf("Process '%s' is not running. Tray icon will not be displayed.\n", processName);
    return TRUE; // 返回TRUE表示函数执行成功，但不显示图标
  }
  
  // 设置初始图标
  if (strlen(iconPath) > 0 && PathFileExistsA(iconPath)) {
    // 如果指定了图标路径且文件存在，使用指定图标
    trayData->hIcon = extract_icon_from_exe(iconPath);
    trayData->nid.hIcon = trayData->hIcon;
    trayData->nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  } else {
    // 否则使用信息图标
    trayData->hIcon = LoadIcon(NULL, IDI_INFORMATION);
    trayData->nid.hIcon = trayData->hIcon;
    trayData->nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  }
  
  // 添加托盘图标 - XP兼容版本
  if (!Shell_NotifyIcon(NIM_ADD, &trayData->nid)) {
    // 清理资源
    if (trayData->hIcon != NULL && 
        trayData->hIcon != LoadIcon(NULL, IDI_APPLICATION)) {
      DestroyIcon(trayData->hIcon);
    }
    DestroyWindow(trayData->hwnd);
    return FALSE;
  }
  
  // 创建右键菜单（取消Exit菜单项）
  trayData->hMenu = CreatePopupMenu();
  AppendMenu(trayData->hMenu, MF_STRING, ID_TRAY_ABOUT, "About");
  
  return TRUE;
}

// 销毁托盘图标
void destroy_tray_icon(TrayIconData* trayData) {
  // 删除托盘图标
  Shell_NotifyIcon(NIM_DELETE, &trayData->nid);
  
  // 销毁图标资源
  if (trayData->hIcon != NULL && 
      trayData->hIcon != LoadIcon(NULL, IDI_APPLICATION) &&
      trayData->hIcon != LoadIcon(NULL, IDI_INFORMATION) &&
      trayData->hIcon != LoadIcon(NULL, IDI_ERROR)) {
    DestroyIcon(trayData->hIcon);
  }
  
  // 销毁菜单
  if (trayData->hMenu) {
    DestroyMenu(trayData->hMenu);
  }
  
  // 销毁窗口
  if (trayData->hwnd) {
    DestroyWindow(trayData->hwnd);
  }
}

// 托盘图标窗口过程函数
LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  TrayIconData* trayData = (TrayIconData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  
  switch (msg) {
    case WM_TRAYICON:
      if (wParam == ID_TRAY_APP_ICON) {
        if (lParam == WM_RBUTTONDOWN) {
          // 右键点击显示菜单
          POINT curPoint;
          GetCursorPos(&curPoint);
          
          // 设置菜单前景窗口以确保菜单正确消失
          SetForegroundWindow(hwnd);
          
          // 显示上下文菜单
          TrackPopupMenu(trayData->hMenu, TPM_RIGHTBUTTON, curPoint.x, curPoint.y, 0, hwnd, NULL);
        } else if (lParam == WM_LBUTTONDOWN) {
          // 左键点击更新状态
          update_tray_icon(trayData);
        }
      }
      break;
      
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case ID_TRAY_EXIT:
          // 移除Exit菜单项处理
          break;
          
        case ID_TRAY_ABOUT:
          MessageBox(hwnd, "SPCMD System Tray Icon\nMonitoring process status", 
                     "About", MB_OK | MB_ICONINFORMATION);
          break;
      }
      break;
      
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
      
    case WM_TIMER:
      // 定时更新托盘图标状态（每5秒检查一次）
      if (trayData) {
        update_tray_icon(trayData);
      }
      break;
      
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  
  return 0;
}

// 托盘图标命令
void cmd_tray(int argc, char *argv[]) {
  // Check if help is needed
  if (argc > 2 &&
      (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--h") == 0)) {
    printf("Tray command help:\n");
    printf("  spcmd tray [--process=name] [--title=title] [icon_path]\n");
    printf("  spcmd tray --path=process_path [--title=title]\n\n");
    printf("Parameter description:\n");
    printf("  --process=name        - Process name to monitor (optional)\n");
    printf("  --title=title         - Tray icon title (optional)\n");
    printf("  --path=process_path   - Process path (auto detect process name and icon)\n");
    printf("  icon_path             - Icon file path (optional, can be specified directly)\n\n");
    printf("Examples:\n");
    printf("  spcmd tray\n");
    printf("  spcmd tray --process=python.exe --title=\"Python Monitor\"\n");
    printf("  spcmd tray --process=notepad.exe C:\\Windows\\notepad.exe\n");
    printf("  spcmd tray C:\\Windows\\notepad.exe\n");
    printf("  spcmd tray --path=C:\\Windows\\notepad.exe\n");
    return;
  }

  // Parse parameters
  char process_name[MAX_PATH] = "python.exe";  // default process name
  char title[MAX_PATH] = "SPCMD Tray";  // default title
  char icon_path[MAX_PATH] = "";  // default icon path (use system default)
  char process_path[MAX_PATH] = "";  // process path for auto detection

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--process=", 10) == 0) {
      strncpy(process_name, argv[i] + 10, MAX_PATH - 1);
      process_name[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--title=", 8) == 0) {
      strncpy(title, argv[i] + 8, MAX_PATH - 1);
      title[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--path=", 7) == 0) {
      strncpy(process_path, argv[i] + 7, MAX_PATH - 1);
      process_path[MAX_PATH - 1] = '\0';
      
      // Auto detect process name from path
      char *fileName = strrchr(process_path, '\\');
      if (fileName == NULL) {
        fileName = process_path;
      } else {
        fileName++; // Skip the backslash
      }
      
      // Copy the file name (with extension) as process name
      strncpy(process_name, fileName, MAX_PATH - 1);
      process_name[MAX_PATH - 1] = '\0';
      
      // Also use the process path as icon path
      strncpy(icon_path, process_path, MAX_PATH - 1);
      icon_path[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--icon=", 7) == 0) {
      // 保留对--icon参数的支持以保持向后兼容性
      strncpy(icon_path, argv[i] + 7, MAX_PATH - 1);
      icon_path[MAX_PATH - 1] = '\0';
    } else if (argv[i][0] != '-') {
      // 如果参数不是以-开头，则认为是图标路径
      strncpy(icon_path, argv[i], MAX_PATH - 1);
      icon_path[MAX_PATH - 1] = '\0';
    }
  }

  printf("Starting system tray icon for process monitoring...\n");
  printf("Process to monitor: %s\n", process_name);
  printf("Title: %s\n", title);
  if (strlen(icon_path) > 0) {
    printf("Icon path: %s\n", icon_path);
  }
  
  // 检查进程是否正在运行
  if (!is_process_running(process_name)) {
    printf("Process '%s' is not running. Tray icon will not be displayed.\n", process_name);
    printf("Tray icon terminated.\n");
    return;
  }
  
  printf("Process is running. Creating tray icon...\n");
  
  // 初始化COM库
  CoInitialize(NULL);
  
  // 创建托盘图标数据结构
  TrayIconData trayData = {0};
  
  // 创建托盘图标
  if (!create_tray_icon(&trayData, GetModuleHandle(NULL), title, process_name, icon_path)) {
    printf("Error: Failed to create system tray icon\n");
    CoUninitialize();
    return;
  }
  
  // 设置定时器，每5秒更新一次状态
  SetTimer(trayData.hwnd, 1, 5000, NULL);
  
  // 初始更新图标状态
  update_tray_icon(&trayData);
  
  // 消息循环
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  
  // 清理资源
  KillTimer(trayData.hwnd, 1);
  destroy_tray_icon(&trayData);
  CoUninitialize();
  
  printf("System tray icon terminated\n");
}

