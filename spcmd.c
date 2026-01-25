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

//==============================================================================
// 版本信息
//==============================================================================
#define SPCMD_VERSION "7.6.0"

//==============================================================================
// 预处理宏定义
//==============================================================================
#define _WIN32_IE 0x0500
#define COBJMACROS
#define INITGUID
#define STB_IMAGE_WRITE_IMPLEMENTATION

//==============================================================================
// 头文件包含
//==============================================================================
#include "ini.h"
#include "stb_image_write.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <objbase.h>
#include <psapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <tlhelp32.h>

//==============================================================================
// 基本类型定义
//==============================================================================
#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// 资源ID定义
#define IDI_ICON1 101

//==============================================================================
// 常量定义
//==============================================================================
// 开机自启目录 GUID (Windows Vista+)
static const GUID FOLDERID_CommonStartup = {0xB97D20BB, 0xF46A, 0x4C97, {0xBA, 0x10, 0x5E, 0x36, 0x08, 0x43, 0xEC, 0xC3}};
static const GUID FOLDERID_Startup = {0xB97D20BB, 0xF46A, 0x4C97, {0xBA, 0x10, 0x5E, 0x36, 0x08, 0x42, 0xEC, 0xC3}};

//==============================================================================
// 结构体定义
//==============================================================================
// 参数定义结构体
typedef struct {
  const char *name;  // 参数名称
  char *value;       // 参数值
  BOOL is_required;  // 是否必填
  BOOL has_been_set; // 是否已设置
} ParamDefinition;

// 参数上下文结构体
typedef struct {
  ParamDefinition *params; // 参数定义数组
  int param_count;         // 参数数量
} ParamContext;

// 进程ID列表结构体
typedef struct {
  DWORD *pids;
  int count;
  int capacity;
} ProcessIdList;

// 自定义弹窗结构体
typedef struct {
  char *text;
  int fontSize;
  COLORREF bgColor;
  COLORREF textColor;
  BOOL modal;
  BOOL noDrag;
  BOOL bold;
  UINT codePage;
  char *onClickCommand;
} WindowParams;

// 命令结构定义
typedef struct {
  const char *name;
  void (*handler)(int argc, char *argv[]);
  int returns_value;
} Command;

//==============================================================================
// 函数声明 - 命令处理
//==============================================================================
void handle_command(int argc, char *argv[]);
void cmd_screenshot(int argc, char *argv[]);
void cmd_shortcut(int argc, char *argv[]);
void cmd_autorun(int argc, char *argv[]);
void cmd_infoboxtop(int argc, char *argv[]);
void cmd_qboxtop(int argc, char *argv[]);
void cmd_task(int argc, char *argv[]);
void cmd_restart(int argc, char *argv[]);
void cmd_window(int argc, char *argv[]);
void cmd_notify(int argc, char *argv[]);
void cmd_config(int argc, char *argv[]);
int cmd_process(int argc, char *argv[]);
void cmd_tray(int argc, char *argv[]);
void cmd_floating(int argc, char *argv[]);
void cmd_timesync(int argc, char *argv[]);
void cmd_ipc(int argc, char *argv[]);

//==============================================================================
// 函数声明 - 参数解析
//==============================================================================
ParamContext *create_param_context(ParamDefinition *param_defs, int count);
BOOL parse_parameters(ParamContext *context, int argc, char *argv[], int start_arg);
const char *get_param_value(ParamContext *context, const char *param_name);
int get_param_int_value(ParamContext *context, const char *param_name, int default_value);
BOOL is_param_set(ParamContext *context, const char *param_name);
BOOL check_required_params(ParamContext *context);
void free_param_context(ParamContext *context);

//==============================================================================
// 函数声明 - 系统功能
//==============================================================================
BOOL IsRunAsAdmin();
BOOL ElevatePrivileges(int argc, char *argv[]);
BOOL GetStartupPath(BOOL forAllUsers, char *path, int pathSize);
BOOL GetWindowsVersionSafe(DWORD *major, DWORD *minor);
char *resolve_system_variables(const char *input);

//==============================================================================
// 函数声明 - 进程操作
//==============================================================================
int find_process_by_name(const char *processName, DWORD *processId);
BOOL kill_process_by_name(const char *processName);
ProcessIdList *get_pids_by_exe_name(const char *exeName);

//==============================================================================
// 函数声明 - 文件操作
//==============================================================================
void save_as_base64_data(const char *bitmap_data, DWORD data_size, const char *filename);
int save_bitmap_as_format(HBITMAP hBitmap, HDC hScreenDC, const char *filename,
                          const char *format, int quality, BOOL quiet);
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);

//==============================================================================
// 命令表
//==============================================================================
Command command_table[] = {
    {"screenshot", (void (*)(int, char **))cmd_screenshot, 0},
    {"shortcut", (void (*)(int, char **))cmd_shortcut, 0},
    {"autorun", (void (*)(int, char **))cmd_autorun, 0},
    {"infoboxtop", (void (*)(int, char **))cmd_infoboxtop, 0},
    {"qboxtop", (void (*)(int, char **))cmd_qboxtop, 0},
    {"window", (void (*)(int, char **))cmd_window, 0},
    {"task", (void (*)(int, char **))cmd_task, 0},
    {"restart", (void (*)(int, char **))cmd_restart, 0},
    {"notify", (void (*)(int, char **))cmd_notify, 0},
    {"config", (void (*)(int, char **))cmd_config, 0},
    {"process", (void (*)(int, char **))cmd_process, 1},
    {"tray", (void (*)(int, char **))cmd_tray, 0},
    {"floating", (void (*)(int, char **))cmd_floating, 0},
    {"timesync", (void (*)(int, char **))cmd_timesync, 0},
    {"ipc", (void (*)(int, char **))cmd_ipc, 0},
    {NULL, NULL, 0}
};

//==============================================================================
// 函数实现 - 系统版本检测
//==============================================================================
typedef LONG (WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

BOOL GetWindowsVersionSafe(DWORD *major, DWORD *minor) {
  // 优先使用RtlGetVersion（不受manifest限制）
  HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
  if (hNtdll) {
    RtlGetVersionPtr pRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
    if (pRtlGetVersion) {
      RTL_OSVERSIONINFOW osvi = {0};
      osvi.dwOSVersionInfoSize = sizeof(osvi);
      if (pRtlGetVersion(&osvi) == 0) {
        *major = osvi.dwMajorVersion;
        *minor = osvi.dwMinorVersion;
        return TRUE;
      }
    }
  }
  // 回退到GetVersionEx（XP兼容）
  OSVERSIONINFO osvi = {0};
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  #if defined(_MSC_VER)
    #pragma warning(suppress: 4996)
  #elif defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  #endif
  if (GetVersionEx(&osvi)) {
  #if defined(__GNUC__)
    #pragma GCC diagnostic pop
  #endif
    *major = osvi.dwMajorVersion;
    *minor = osvi.dwMinorVersion;
    return TRUE;
  }
  return FALSE;
}

//==============================================================================
// 函数实现 - 程序入口
//==============================================================================
int WINAPI WinMain(HINSTANCE _hInstance, HINSTANCE _hPrevInstance,
                   LPSTR _lpCmdLine, int _nCmdShow) {
  // 初始化Winsock
  WSADATA wsaData;
  int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (wsaInit != 0) {
    printf("Error: Failed to initialize Winsock\n");
    return 1;
  }

  // 避免未使用参数警告
  (void)_hInstance;
  (void)_hPrevInstance;
  (void)_lpCmdLine;
  (void)_nCmdShow;
  // 声明变量
  int argc = 0;
  char **argv = NULL;
  LPWSTR *argvW = NULL;

  // 获取命令行参数
  argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argvW == NULL) {
    // 如果无法获取命令行参数，至少要有一个参数（程序名）
    argc = 1;
    argv = (char **)malloc(sizeof(char *));
    if (!argv) {
      printf("Error: Memory allocation failed\n");
      return 1;
    }
    argv[0] = (char *)"spcmd.exe";
  } else {
    // 转换宽字符参数为多字节字符（使用UTF-8编码，解决跨地区乱码问题）
    argv = (char **)malloc(argc * sizeof(char *));
    if (!argv) {
      printf("Error: Memory allocation failed\n");
      LocalFree(argvW);
      return 1;
    }
    for (int i = 0; i < argc; i++) {
      int len =
          WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
      argv[i] = (char *)malloc(len * sizeof(char));
      if (!argv[i]) {
        printf("Error: Memory allocation failed\n");
        // 清理已分配的内存
        for (int j = 0; j < i; j++) {
          free(argv[j]);
        }
        free(argv);
        LocalFree(argvW);
        return 1;
      }
      WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], len, NULL, NULL);
    }
    // 释放系统分配的宽字符参数
    LocalFree(argvW);
  }

  // 如果没有参数，直接返回
  if (argc < 2) {
    MessageBoxA(NULL,
                "SPCMD - System Power Command Tool\n"
                "Version: " SPCMD_VERSION "\n\n"
                "Usage: spcmd.exe [command] [options]",
                "INFO", MB_OK | MB_ICONINFORMATION);
    // 释放内存
    for (int i = 0; i < argc; i++) {
      free(argv[i]);
    }
    free(argv);

    return 0;
  }

  // 处理命令
  handle_command(argc, argv);

  // 释放内存
  for (int i = 0; i < argc; i++) {
    free(argv[i]);
  }
  free(argv);

  // 清理Winsock
  WSACleanup();

  return 0;
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
      if (!resolved_argv[i]) {
        // 内存分配失败，释放之前分配的内存并返回
        for (int j = 0; j < i; j++) {
          free(resolved_argv[j]);
        }
        free(resolved_argv);
        printf("Error: Memory allocation failed\n");
        return;
      }
    }
  }

  // 使用命令表查找并执行命令 - 优化的命令分派机制
  const char *command_name = resolved_argv[1];
  BOOL command_found = FALSE;

  for (int i = 0; command_table[i].name != NULL; i++) {
    if (strcmp(command_table[i].name, command_name) == 0) {
      command_found = TRUE;

      // 特殊处理有返回值的命令
      if (command_table[i].returns_value) {
        if (strcmp(command_name, "process") == 0) {
          int result = cmd_process(argc, resolved_argv);
          if (result != 0) {
            // 注意：在调用exit之前手动释放内存实际上是不必要的
            exit(result); // 如果check命令返回非0值，退出程序
          }
        }
      } else {
        // 常规命令调用
        command_table[i].handler(argc, resolved_argv);
      }

      break;
    }
  }

   // 处理未知命令
  if (!command_found) {
    printf("Unknown command: %s\n", command_name);
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
  BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

  // Restore old bitmap
  SelectObject(hMemoryDC, hOldBitmap);

  // Generate filename and format
  char filename[MAX_PATH] = "screenshot.jpg";
  char format[10] = "jpg";              // default format
  char base64_filename[MAX_PATH] = {0}; // for base64 encoded data
  BOOL save_as_base64 = FALSE;
  BOOL output_base64_to_console = FALSE; // Flag for --save=base64
  int quality = 100;                     // default quality

  // 使用统一的参数解析框架
  ParamDefinition param_defs[] = {{"save", NULL, FALSE, FALSE},
                                  {"format", NULL, FALSE, FALSE},
                                  {"base64", NULL, FALSE, FALSE},
                                  {"quality", NULL, FALSE, FALSE}};

  ParamContext *context = create_param_context(param_defs, 4);
  if (!context) {
    printf("Error: Failed to create parameter context\n");
    // 清理资源
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    return;
  }

  // 解析参数
  parse_parameters(context, argc, argv, 2);

  // 获取参数值
  const char *save_value = get_param_value(context, "save");
  if (save_value) {
    if (strcmp(save_value, "base64") == 0) {
      // Special case: output base64 to console
      output_base64_to_console = TRUE;
    } else {
      // Normal case: save to file
      strncpy(filename, save_value, MAX_PATH - 1);
      filename[MAX_PATH - 1] = '\0';
    }
  }

  const char *format_value = get_param_value(context, "format");
  if (format_value) {
    strncpy(format, format_value, sizeof(format) - 1);
    format[sizeof(format) - 1] = '\0';
  }

  const char *base64_value = get_param_value(context, "base64");
  if (base64_value) {
    strncpy(base64_filename, base64_value, MAX_PATH - 1);
    base64_filename[MAX_PATH - 1] = '\0';
    save_as_base64 = TRUE;
  }

  const char *quality_value = get_param_value(context, "quality");
  if (quality_value) {
    quality = atoi(quality_value);
    // Ensure quality is between 1 and 100
    if (quality < 1)
      quality = 1;
    if (quality > 100)
      quality = 100;
  }

  // 释放参数上下文
  free_param_context(context);

  // Handle base64 encoded data save to file
  if (save_as_base64) {
    // Save as base64 encoded data to file
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

    // 检查GetDIBits的返回值
    int getDIBitsResult =
        GetDIBits(hScreenDC, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap,
                  (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    if (getDIBitsResult == 0 || (DWORD)getDIBitsResult == GDI_ERROR) {
      printf("Error: Failed to get DIB bits\n");
      GlobalUnlock(hDIB);
      GlobalFree(hDIB);
      // 清理资源并返回
      DeleteObject(hBitmap);
      DeleteDC(hMemoryDC);
      ReleaseDC(NULL, hScreenDC);
      free_param_context(context);
      return;
    }

    // Save as base64 encoded data to file
    save_as_base64_data(lpbitmap, dwBmpSize, base64_filename);

    // Clean up resources
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
  }
  // Handle base64 encoded data output to console
  else if (output_base64_to_console) {
    // Save bitmap to memory first with the specified format
    char temp_filename[MAX_PATH] = "temp_screenshot.png";
    int result =
        save_bitmap_as_format(hBitmap, hScreenDC, temp_filename, format,
                              quality, TRUE); // 安静模式，不输出日志

    if (result) {
      // Read the saved file and encode it to base64
      HANDLE hFile = CreateFile(temp_filename, GENERIC_READ, 0, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hFile != INVALID_HANDLE_VALUE) {
        DWORD file_size = GetFileSize(hFile, NULL);
        if (file_size != INVALID_FILE_SIZE) {
          char *file_data = (char *)malloc(file_size);
          if (file_data != NULL) {
            DWORD bytes_read;
            if (ReadFile(hFile, file_data, file_size, &bytes_read, NULL) &&
                bytes_read == file_size) {
              // Encode the file data to base64
              size_t encoded_length;
              char *base64_data = base64_encode(
                  (const unsigned char *)file_data, file_size, &encoded_length);

              if (base64_data != NULL) {
                // Output the base64 data to console
                printf("%s", base64_data);
                free(base64_data);
              }
            }
            free(file_data);
          }
        }
        CloseHandle(hFile);
      } else {
        // 临时文件创建失败
        printf("Error: Failed to create temporary file %s\n", temp_filename);
      }

      // Delete the temporary file
      DeleteFile(temp_filename);
    } else {
      // save_bitmap_as_format失败
      printf("Error: Failed to save bitmap as format\n");
    }
  } else {
    // Save bitmap in the specified format
    // Ensure filename has correct extension
    if (strcmp(format, "png") == 0 || strcmp(format, "PNG") == 0) {
      // Ensure filename has .png extension
      if (strstr(filename, ".bmp")) {
        // Replace .bmp with .png
        char *dot = strrchr(filename, '.');
        if (dot) {
          strncpy(dot, ".png", MAX_PATH - (dot - filename) - 1);
          dot[MAX_PATH - (dot - filename) - 1] = '\0';
        }
      } else if (!strstr(filename, ".png")) {
        // Add .png extension if no extension
        strncat(filename, ".png", MAX_PATH - strlen(filename) - 1);
      }
    } else if (strcmp(format, "jpg") == 0 || strcmp(format, "JPG") == 0 ||
               strcmp(format, "jpeg") == 0 || strcmp(format, "JPEG") == 0) {
      // Ensure filename has .jpg extension
      if (strstr(filename, ".bmp")) {
        // Replace .bmp with .jpg
        char *dot = strrchr(filename, '.');
        if (dot) {
          strncpy(dot, ".jpg", MAX_PATH - (dot - filename) - 1);
          dot[MAX_PATH - (dot - filename) - 1] = '\0';
        }
      } else if (!strstr(filename, ".jpg") && !strstr(filename, ".jpeg")) {
        // Add .jpg extension if no extension
        strncat(filename, ".jpg", MAX_PATH - strlen(filename) - 1);
      }
    } else {
      // Save as BMP (default)
      // Ensure filename has .bmp extension
      if (strstr(filename, ".png")) {
        // Replace .png with .bmp
        char *dot = strrchr(filename, '.');
        if (dot) {
          strncpy(dot, ".bmp", MAX_PATH - (dot - filename) - 1);
          dot[MAX_PATH - (dot - filename) - 1] = '\0';
        }
      } else if (strstr(filename, ".jpg") || strstr(filename, ".jpeg")) {
        // Replace .jpg/.jpeg with .bmp
        char *dot = strrchr(filename, '.');
        if (dot) {
          strncpy(dot, ".bmp", MAX_PATH - (dot - filename) - 1);
          dot[MAX_PATH - (dot - filename) - 1] = '\0';
        }
      } else if (!strstr(filename, ".bmp")) {
        // Add .bmp extension if no extension
        strncat(filename, ".bmp", MAX_PATH - strlen(filename) - 1);
      }
      // Use "bmp" as format for the save function
      strncpy(format, "bmp", 4);
      format[3] = '\0';
    }

    // Use the new helper function to save the bitmap
    save_bitmap_as_format(hBitmap, hScreenDC, filename, format, quality,
                          FALSE); // 正常模式，输出日志
  }

  // Clean up resources
  DeleteObject(hBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL, hScreenDC);
}

void cmd_shortcut(int argc, char *argv[]) {
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
    return;
  }

  // If shortcut name is not specified, use target filename
  if (strlen(shortcutName) == 0) {
    char *fileName = strrchr(targetPath, '\\');
    if (fileName != NULL) {
      strncpy(shortcutName, fileName + 1, MAX_PATH - 1);
      shortcutName[MAX_PATH - 1] = '\0';
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
  HRESULT hr = SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL,
                                SHGFP_TYPE_CURRENT, desktopPath);
   if (FAILED(hr)) {
     printf("Error: Unable to get desktop path (hr=0x%08lX)\n", (unsigned long)hr);
     return;
   }

  // Construct shortcut full path
  char shortcutPath[MAX_PATH];
  snprintf(shortcutPath, MAX_PATH, "%s\\%s", desktopPath, finalName);

   // Create shortcut
  HRESULT hres = CoInitialize(NULL);
  if (FAILED(hres)) {
    printf("Error: Failed to initialize COM (hr=0x%08lX)\n", (unsigned long)hres);
    return;
  }

  IShellLinkA *pShellLink = NULL;
  hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IShellLinkA, (LPVOID *)&pShellLink);

   if (FAILED(hres)) {
     printf("Error: Unable to create ShellLink object (hr=0x%08lX)\n", (unsigned long)hres);
     CoUninitialize();
     return;
   }

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
      targetDir[MAX_PATH - 1] = '\0';
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
      // Convert to wide character for IPersistFile::Save
      WCHAR wsz[MAX_PATH];
      int wideLen = MultiByteToWideChar(CP_UTF8, 0, shortcutPath, -1, wsz, MAX_PATH);
      if (wideLen == 0) {
        printf("Error: Failed to convert path to Unicode (error=%lu)\n", GetLastError());
        IPersistFile_Release(pPersistFile);
        IShellLinkA_Release(pShellLink);
        CoUninitialize();
        return;
      }

      hres = IPersistFile_Save(pPersistFile, wsz, TRUE);

      if (SUCCEEDED(hres)) {
        printf("Shortcut created: %s\n", shortcutPath);
      } else {
        printf("Error: Unable to save shortcut to %s (hr=0x%08lX)\n", shortcutPath, (unsigned long)hres);
      }

      IPersistFile_Release(pPersistFile);
    } else {
      printf("Error: Unable to get IPersistFile interface (hr=0x%08lX)\n", (unsigned long)hres);
    }

    IShellLinkA_Release(pShellLink);
  } else {
    printf("Error: Unable to create shortcut object (hr=0x%08lX)\n", (unsigned long)hres);
  }

  CoUninitialize();

  // Refresh desktop to show the new shortcut icon
  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

void cmd_autorun(int argc, char *argv[]) {

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

    return;
  }

  // If entry name is not specified, use target filename
  if (strlen(entryName) == 0) {
    char *fileName = strrchr(targetPath, '\\');
    if (fileName != NULL) {
      strncpy(entryName, fileName + 1, MAX_PATH - 1);
      entryName[MAX_PATH - 1] = '\0';
    } else {
      strncpy(entryName, targetPath, MAX_PATH - 1);
      entryName[MAX_PATH - 1] = '\0';
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

  // 根据权限选择开机自启目录（兼容XP/Vista/7/10/11）
  char startupPath[MAX_PATH];
  BOOL useSystemStartup = IsRunAsAdmin();

  if (useSystemStartup) {
    // 管理员权限：使用系统开机自启目录
    if (!GetStartupPath(TRUE, startupPath, MAX_PATH)) {
      printf("无法获取系统开机自启目录，回退到用户开机自启\n");
      useSystemStartup = FALSE;
    }
  }

  // 非管理员权限或获取系统目录失败：使用用户开机自启目录
  if (!useSystemStartup) {
    if (!GetStartupPath(FALSE, startupPath, MAX_PATH)) {
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
        if (GetStartupPath(FALSE, userStartupPath, MAX_PATH)) {
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
        if (GetStartupPath(TRUE, systemStartupPath, MAX_PATH)) {
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
        targetDir[MAX_PATH - 1] = '\0';
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
        MultiByteToWideChar(CP_UTF8, 0, shortcutPath, -1, wsz, MAX_PATH);

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

  // Parse parameters
  char taskName[MAX_PATH] = {0};
  char programPath[MAX_PATH] = {0};
  char triggerType[20] = "daily"; // default trigger
  char startTime[10] = "09:00";   // default start time
  char startDate[15] = {0};       // default to today

  BOOL hasName = FALSE;
  BOOL hasExec = FALSE;

  // Get current date as default start date
  SYSTEMTIME st;
  GetLocalTime(&st);
  snprintf(startDate, sizeof(startDate), "%04d-%02d-%02d", st.wYear, st.wMonth,
           st.wDay);

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

    return;
  }

  // 尝试创建任务通常需要管理员权限
  BOOL needAdmin = TRUE;

  // 检查是否已经尝试过提权（防止无限循环）
  BOOL alreadyElevated = FALSE;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "--once") == 0) {
      alreadyElevated = TRUE;
      break;
    }
  }

  // 如果需要管理员权限但当前没有管理员权限，则尝试提升权限
  if (needAdmin && !IsRunAsAdmin() && !alreadyElevated) {
    printf("创建计划任务需要管理员权限，正在请求权限提升...\n");

    // 尝试提升权限
    if (ElevatePrivileges(argc, argv)) {
      // 如果提升成功，程序会以管理员权限重新运行，当前进程会退出
      return;
    } else {
      // 如果提升失败，询问用户是否继续以普通权限尝试
      printf("权限提升失败，将以普通权限尝试创建任务...\n");
    }
  }

  // 检查文件是否存在

  printf("Creating scheduled task: %s\n", taskName);
  printf("Program: %s\n", programPath);
  printf("Trigger: %s\n", triggerType);
  printf("Start time: %s\n", startTime);
  printf("Start date: %s\n", startDate);

  // 检查Windows版本以确定使用哪种方法
  DWORD dwMajorVersion = 0, dwMinorVersion = 0;
  GetWindowsVersionSafe(&dwMajorVersion, &dwMinorVersion);

  BOOL isWindowsXP = (dwMajorVersion == 5 &&
                      dwMinorVersion == 1); // Windows XP是5.1版本

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
    snprintf(command, sizeof(command),
             "schtasks /create /tn \"%s\" /tr \"%s\" /sc %s /st %s", taskName,
             programPath, scheduleType, startTime);
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
    snprintf(command, sizeof(command),
             "schtasks /create /tn \"%s\" /tr \"%s\" /sc %s /st %s /sd %s",
             taskName, programPath, scheduleType, startTime, startDate);
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
        // 创建进程名的副本以避免修改原始数据
        char procName[MAX_PATH];
        strncpy(procName, pe32.szExeFile, MAX_PATH - 1);
        procName[MAX_PATH - 1] = '\0';

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
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }

  // Wait for processes to terminate with a timeout
  BOOL processStillRunning = TRUE;
  DWORD waitCount = 0;
  const DWORD maxWaitCount = 10; // 最多等待5秒

  while (processStillRunning && waitCount < maxWaitCount) {
    Sleep(500); // 每次等待500毫秒
    waitCount++;

    processStillRunning = FALSE;
    HANDLE hSnapshot2 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot2 != INVALID_HANDLE_VALUE) {
      PROCESSENTRY32 pe32_2;
      pe32_2.dwSize = sizeof(PROCESSENTRY32);

      if (Process32First(hSnapshot2, &pe32_2)) {
        do {
          // 创建进程名的副本以避免修改原始数据
          char procName2[MAX_PATH];
          strncpy(procName2, pe32_2.szExeFile, MAX_PATH - 1);
          procName2[MAX_PATH - 1] = '\0';

          char *procDot2 = strrchr(procName2, '.');
          if (procDot2 != NULL) {
            *procDot2 = '\0';
          }

          if (_stricmp(procName2, processName) == 0) {
            processStillRunning = TRUE;
            printf("Process still running, waiting... (%lu/%lu)\n", waitCount,
                   maxWaitCount);
            break;
          }
        } while (Process32Next(hSnapshot2, &pe32_2));
      }
      CloseHandle(hSnapshot2);
    }
  }

  if (processStillRunning) {
    printf("Warning: Some processes may still be running after timeout\n");
  } else {
    printf("All processes terminated successfully\n");
  }

  // Now start the new process
  STARTUPINFOA si = {0};
  PROCESS_INFORMATION pi = {0};
  si.cb = sizeof(si);

  // 如果没有指定工作目录，则使用进程所在目录作为工作目录
  char *effectiveWorkDir = NULL;
  char processDir[MAX_PATH] = {0};

  if (hasWorkDir) {
    // 检查指定的工作目录是否存在
    if (GetFileAttributesA(workingDir) != INVALID_FILE_ATTRIBUTES) {
      effectiveWorkDir = workingDir;
    } else {
      printf("Warning: Specified working directory does not exist: %s\n",
             workingDir);
    }
  }

  // 如果没有有效的工作目录，使用进程所在目录
  if (effectiveWorkDir == NULL) {
    // 获取进程文件所在目录
    strncpy(processDir, processPath, MAX_PATH - 1);
    processDir[MAX_PATH - 1] = '\0';
    char *lastSlash = strrchr(processDir, '\\');
    if (lastSlash != NULL) {
      *lastSlash = '\0';
      // 检查进程目录是否存在
      if (GetFileAttributesA(processDir) != INVALID_FILE_ATTRIBUTES) {
        effectiveWorkDir = processDir;
      }
    }
  }

  // 如果仍然没有有效的工作目录，CreateProcessA将使用默认工作目录
  if (effectiveWorkDir == NULL) {
    printf("Warning: No valid working directory found, using system default\n");
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

void cmd_infoboxtop(int argc, char *argv[]) {
  // 使用统一的参数解析框架
  ParamDefinition param_defs[] = {{"message", NULL, TRUE, FALSE},
                                  {"title", NULL, TRUE, FALSE}};

  ParamContext *context = create_param_context(param_defs, 2);
  if (!context) {
    printf("Error: Failed to create parameter context\n");
    return;
  }

  // 解析参数
  parse_parameters(context, argc, argv, 2);

  // 获取参数值
  const char *message = get_param_value(context, "message");
  const char *title = get_param_value(context, "title");

  // 添加NULL检查 - 防止空指针解引用
  if (!message) {
    printf("Error: message parameter is required\n");
    free_param_context(context);
    return;
  }
  if (!title) {
    printf("Error: title parameter is required\n");
    free_param_context(context);
    return;
  }

  // 获取当前活跃窗口句柄，然后在该窗口上显示置顶消息框
  HWND hActiveWnd = GetForegroundWindow();
  if (hActiveWnd == NULL) {
    // 如果无法获取活跃窗口句柄，则在桌面上显示
    hActiveWnd = GetDesktopWindow();
  }

  // 显示置顶消息框
  MessageBoxA(hActiveWnd, message, title,
              MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
  printf("Top-most message box displayed\n");

  // 释放参数上下文
  free_param_context(context);
}

void cmd_qboxtop(int argc, char *argv[]) {
  // 使用统一的参数解析框架
  ParamDefinition param_defs[] = {{"message", NULL, TRUE, FALSE},
                                  {"title", NULL, TRUE, FALSE},
                                  {"program", NULL, TRUE, FALSE}};

  ParamContext *context = create_param_context(param_defs, 3);
  if (!context) {
    printf("Error: Failed to create parameter context\n");
    return;
  }

  // 解析参数
  parse_parameters(context, argc, argv, 2);

   // 获取参数值
  const char *message = get_param_value(context, "message");
  const char *title = get_param_value(context, "title");
  const char *program = get_param_value(context, "program");

  // 检查必填参数
  if (!message || !title || !program) {
    printf("Error: Missing required parameters\n");
    free_param_context(context);
    return;
  }

  // 获取当前活跃窗口句柄，然后在该窗口上显示置顶消息框
  HWND hActiveWnd = GetForegroundWindow();
  if (hActiveWnd == NULL) {
    // 如果无法获取活跃窗口句柄，则在桌面上显示
    hActiveWnd = GetDesktopWindow();
  }

  // 显示置顶消息框
  int result = MessageBoxA(hActiveWnd, message, title,
                           MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL);

  if (result == IDYES) {
    // Run the specified program
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the program
    if (CreateProcessA(NULL, (LPSTR)program, NULL, NULL, FALSE, 0, NULL, NULL,
                       &si, &pi)) {
      printf("Program '%s' started successfully\n", program);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    } else {
      printf("Error: Failed to start program '%s'\n", program);
    }
  } else {
    printf("User chose not to run the program\n");
  }

  // 释放参数上下文
  free_param_context(context);
}

// 辅助函数：从HBITMAP获取图像数据并保存为指定格式
int save_bitmap_as_format(HBITMAP hBitmap, HDC hScreenDC, const char *filename,
                          const char *format, int quality, BOOL quiet) {
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
    // 创建两个内存DC：一个用于原始图像，一个用于缩放后的图像
    HDC hSrcDC = CreateCompatibleDC(hScreenDC);
    HDC hDstDC = CreateCompatibleDC(hScreenDC);

    hScaledBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    HBITMAP hOldSrcBitmap = (HBITMAP)SelectObject(hSrcDC, hBitmap);
    HBITMAP hOldDstBitmap = (HBITMAP)SelectObject(hDstDC, hScaledBitmap);

    // 缩放图像
    SetStretchBltMode(hDstDC, HALFTONE);
    StretchBlt(hDstDC, 0, 0, width, height, hSrcDC, 0, 0, bmp.bmWidth,
               bmp.bmHeight, SRCCOPY);

    // 清理资源
    SelectObject(hSrcDC, hOldSrcBitmap);
    SelectObject(hDstDC, hOldDstBitmap);
    DeleteDC(hSrcDC);

    hMemoryDC = hDstDC; // 保存目标DC用于后续GetDIBits调用
    hBitmapToSave = hScaledBitmap;
  } else {
    // 不需要缩放，创建一个DC来选择原始bitmap
    hMemoryDC = CreateCompatibleDC(hScreenDC);
    hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
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

  // 检查GetDIBits的返回值
  int getDIBitsResult = GetDIBits(hMemoryDC, hBitmapToSave, 0, (UINT)height,
                                  lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

  // 检查GetDIBits的返回值
  if (getDIBitsResult == 0 || (DWORD)getDIBitsResult == GDI_ERROR) {
    if (!quiet) {
      printf("Error: Failed to get DIB bits in save_bitmap_as_format\n");
    }
    // 清理资源并返回错误
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    if (hMemoryDC) {
      if (hOldBitmap) {
        SelectObject(hMemoryDC, hOldBitmap);
      }
      if (hScaledBitmap) {
        DeleteObject(hScaledBitmap);
      }
      DeleteDC(hMemoryDC);
    }

    return 0; // 返回错误
  }

  // 调整RGB顺序以修复颜色发黄问题，确保不会越界访问
  DWORD pixelCount = width * height;
  if (pixelCount > 0 && lpbitmap != NULL) {
    for (DWORD i = 0; i < pixelCount * 3 && i + 2 < dwBmpSize; i += 3) {
      // 交换红色和蓝色通道 (BGR -> RGB)
      char temp = lpbitmap[i];
      lpbitmap[i] = lpbitmap[i + 2];
      lpbitmap[i + 2] = temp;
    }
  }

  // 使用stb_image_write保存为指定格式
  int result = 0;
  if (strcmp(format, "png") == 0 || strcmp(format, "PNG") == 0) {
    result = stbi_write_png(filename, width, height, 3, lpbitmap, width * 3);
    if (result) {
      if (!quiet) {
        if (width != bmp.bmWidth || height != bmp.bmHeight) {
          printf("Screenshot saved to: %s (scaled to %dx%d, as PNG format)\n",
                 filename, width, height);
        } else {
          printf("Screenshot saved to: %s (as PNG format)\n", filename);
        }
      }
    }
  } else if (strcmp(format, "jpg") == 0 || strcmp(format, "JPG") == 0 ||
             strcmp(format, "jpeg") == 0 || strcmp(format, "JPEG") == 0) {
    result = stbi_write_jpg(filename, width, height, 3, lpbitmap, quality);
    if (result) {
      if (!quiet) {
        if (width != bmp.bmWidth || height != bmp.bmHeight) {
          printf("Screenshot saved to: %s (scaled to %dx%d, as JPEG format)\n",
                 filename, width, height);
        } else {
          printf("Screenshot saved to: %s (as JPEG format)\n", filename);
        }
      }
    }
  } else { // 默认保存为BMP格式
    result = stbi_write_bmp(filename, width, height, 3, lpbitmap);
    if (result) {
      if (!quiet) {
        if (width != bmp.bmWidth || height != bmp.bmHeight) {
          printf("Screenshot saved to: %s (scaled to %dx%d, as BMP format)\n",
                 filename, width, height);
        } else {
          printf("Screenshot saved to: %s (as BMP format)\n", filename);
        }
      }
    }
  }

  if (!result && !quiet) {
    printf("Error: Unable to save screenshot to %s\n", filename);
  }

  // Clean up resources
  GlobalUnlock(hDIB);
  GlobalFree(hDIB);

  if (hMemoryDC) {
    if (hOldBitmap) {
      SelectObject(hMemoryDC, hOldBitmap);
    }
    if (hScaledBitmap) {
      DeleteObject(hScaledBitmap);
    }
    DeleteDC(hMemoryDC);
  }

  // 清理完成

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

  // 计算输出缓冲区大小（使用较小的初始缓冲区）
  size_t input_len = strlen(input);
  size_t buffer_size = input_len + 256; // 较小的初始缓冲区
  char *output = (char *)malloc(buffer_size);
  if (!output)
    return NULL;

  size_t out_pos = 0;
  size_t in_pos = 0;

   while (in_pos < input_len) {
    if (in_pos + 1 < input_len && input[in_pos] == '[' && input[in_pos + 1] == '%') {
      // 查找变量结束标记 %]
      size_t var_start = in_pos + 2;
      size_t var_end = var_start;

      while (var_end + 1 < input_len &&
             (input[var_end] != '%' || input[var_end + 1] != ']')) {
        var_end++;
      }

      if (var_end + 1 < input_len && input[var_end] == '%' &&
          input[var_end + 1] == ']') {
        // 找到了完整的变量格式
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
          }

          // 如果找到了变量值，则替换
          if (var_value) {
            size_t value_len = strlen(var_value);
            // 检查缓冲区大小，如果不够则重新分配
            if (out_pos + value_len >= buffer_size) {
              size_t new_size = buffer_size + value_len + 128;
              char *new_output = (char *)realloc(output, new_size);
              if (!new_output) {
                free(output);
                free(var_value);
                return NULL;
              }
              output = new_output;
              buffer_size = new_size;
            }
            strncpy(&output[out_pos], var_value, buffer_size - out_pos - 1);
            output[buffer_size - 1] = '\0';
            out_pos += value_len;
            free(var_value);

            // 跳过已处理的变量部分（包括结束标记%]）
            in_pos = var_end + 2;
          } else {
            // 如果未识别变量，保留原样
            size_t copy_len = var_end - in_pos + 2;
            // 检查缓冲区大小
            if (out_pos + copy_len >= buffer_size) {
              size_t new_size = buffer_size + copy_len + 128;
              char *new_output = (char *)realloc(output, new_size);
              if (!new_output) {
                free(output);
                return NULL;
              }
              output = new_output;
              buffer_size = new_size;
            }
            strncpy(&output[out_pos], &input[in_pos], copy_len);
            out_pos += copy_len;
            in_pos = var_end + 2;
          }
        } else {
          // 变量名太长，保留原样
          size_t copy_len = var_end - in_pos + 2;
          // 检查缓冲区大小
          if (out_pos + copy_len >= buffer_size) {
            size_t new_size = buffer_size + copy_len + 128;
            char *new_output = (char *)realloc(output, new_size);
            if (!new_output) {
              free(output);
              return NULL;
            }
            output = new_output;
            buffer_size = new_size;
          }
          strncpy(&output[out_pos], &input[in_pos], copy_len);
          out_pos += copy_len;
          in_pos = var_end + 2;
        }
      } else {
        // 未找到结束标记，保留[%
        // 检查缓冲区大小
        if (out_pos + 2 >= buffer_size) {
          size_t new_size = buffer_size + 256;
          char *new_output = (char *)realloc(output, new_size);
          if (!new_output) {
            free(output);
            return NULL;
          }
          output = new_output;
          buffer_size = new_size;
        }
        output[out_pos++] = input[in_pos++];
        output[out_pos++] = input[in_pos++];
      }
    } else {
      // 普通字符，直接复制
      // 检查缓冲区大小
      if (out_pos >= buffer_size - 1) { // 留一个字符的空间给结尾\0
        size_t new_size = buffer_size + 256;
        char *new_output = (char *)realloc(output, new_size);
        if (!new_output) {
          free(output);
          return NULL;
        }
        output = new_output;
        buffer_size = new_size;
      }
      output[out_pos++] = input[in_pos++];
    }
  }

  // 确保字符串结尾
  if (out_pos < buffer_size) {
    output[out_pos] = '\0';
  } else {
    // 这种情况不应该发生，因为我们一直在检查缓冲区大小
    free(output);
    return NULL;
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

    // 如果用户指定的字体不可用或未指定，尝试使用微软雅黑（使用Unicode版本确保中文支持）
    if (!hFont) {
      hFont = CreateFontW(params ? params->fontSize : 18, 0, 0, 0,
                          params && params->bold ? FW_BOLD : FW_NORMAL, FALSE,
                          FALSE, FALSE, GB2312_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                          DEFAULT_PITCH | FF_SWISS, L"微软雅黑");
    }

    // 如果微软雅黑不可用，尝试使用宋体
    if (!hFont) {
      hFont = CreateFontW(params ? params->fontSize : 18, 0, 0, 0,
                          params && params->bold ? FW_BOLD : FW_NORMAL, FALSE,
                          FALSE, FALSE, GB2312_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                          DEFAULT_PITCH | FF_SWISS, L"宋体");
    }

    // 如果宋体不可用，尝试使用黑体
    if (!hFont) {
      hFont = CreateFontW(params ? params->fontSize : 18, 0, 0, 0,
                          params && params->bold ? FW_BOLD : FW_NORMAL, FALSE,
                          FALSE, FALSE, GB2312_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                          DEFAULT_PITCH | FF_SWISS, L"黑体");
    }

    // 如果以上字体都不可用，使用系统默认GUI字体
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

    // 添加空指针检查
    if (!params) {
      EndPaint(hwnd, &ps);
      break;
    }

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

    // 将ANSI文本转换为宽字符以支持中文显示
    // 使用params中指定的代码页，支持UTF-8、GBK、BIG5等编码
    int wideCharLen = MultiByteToWideChar(params->codePage, 0, displayText, -1, NULL, 0);
    
    // 只有在成功获取到所需长度时才分配内存并进行转换
    if (wideCharLen > 0) {
      wchar_t *wideDisplayText = (wchar_t *)malloc(wideCharLen * sizeof(wchar_t));
      if (wideDisplayText) {
        // 执行实际的转换
        int result = MultiByteToWideChar(params->codePage, 0, displayText, -1, wideDisplayText, wideCharLen);
        
        // 只有转换成功才进行绘制
        if (result > 0) {
          // 使用Unicode版本的DrawTextW来计算文本区域
          DrawTextW(hdc, wideDisplayText, -1, &calcRect,
                    DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL | DT_CALCRECT);

          // 调整垂直位置以实现居中
          int textHeight = calcRect.bottom - calcRect.top;
          int availableHeight = textRect.bottom - textRect.top;
          if (textHeight < availableHeight) {
            int offset = (availableHeight - textHeight) / 2;
            textRect.top += offset;
            textRect.bottom = textRect.top + textHeight;
          }

          // 使用Unicode版本的DrawTextW来绘制文本
          DrawTextW(hdc, wideDisplayText, -1, &textRect,
                    DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL);
        }
        
        // 释放宽字符内存
        free(wideDisplayText);
      }
    }

    EndPaint(hwnd, &ps);
    break;
  }

   case WM_COMMAND: {
    if (LOWORD(wParam) == 1) { // 点击了确认按钮
       // 如果有onClickCommand，执行它
      if (params && params->onClickCommand && params->onClickCommand[0] != '\0') {
        printf("Executing on-click command: %s\n", params->onClickCommand);
        // 使用CreateProcess执行命令，不等待进程完成
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        // 构建命令行
        char cmdLine[1024];
        snprintf(cmdLine, sizeof(cmdLine), "cmd /c \"%s\"", params->onClickCommand);

        if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
          CloseHandle(pi.hProcess);
          CloseHandle(pi.hThread);
        }
      }
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
  BOOL hasOnClick = FALSE;
  const char *onClickCommand = NULL;
  UINT codePage = CP_UTF8; // 默认使用UTF-8编码，解决跨地区乱码问题

  // 解析所有参数
  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--text=", 7) == 0) {
      message = argv[i] + 7;
      hasText = TRUE;
    } else if (strncmp(argv[i], "--title=", 8) == 0) {
      strncpy(title, argv[i] + 8, sizeof(title) - 1);
      title[sizeof(title) - 1] = '\0';
    } else if (strncmp(argv[i], "--width=", 8) == 0) {
      width = atoi(argv[i] + 8);
      // 确保宽度在合理范围内
      if (width < 100)
        width = 100;
      if (width > 2000)
        width = 2000;
    } else if (strncmp(argv[i], "--height=", 9) == 0) {
      height = atoi(argv[i] + 9);
      // 确保高度在合理范围内
      if (height < 100)
        height = 100;
      if (height > 2000)
        height = 2000;
    } else if (strncmp(argv[i], "--fontsize=", 11) == 0) {
      fontSize = atoi(argv[i] + 11);
      // 确保字体大小在合理范围内
      if (fontSize < 8)
        fontSize = 8;
      if (fontSize > 72)
        fontSize = 72;
    } else if (strncmp(argv[i], "--bgcolor=", 10) == 0) {
      char *colorStr = argv[i] + 10;
      // 检查是否是RGB格式 (r,g,b)
      if (strchr(colorStr, ',') != NULL) {
        int r, g, b;
        if (sscanf(colorStr, "%d,%d,%d", &r, &g, &b) == 3) {
          // 确保RGB值在有效范围内
          if (r < 0)
            r = 0;
          if (r > 255)
            r = 255;
          if (g < 0)
            g = 0;
          if (g > 255)
            g = 255;
          if (b < 0)
            b = 0;
          if (b > 255)
            b = 255;
          bgColor = RGB(r, g, b);
        }
      } else {
        // 使用颜色名称
        bgColor = GetColorByName(colorStr);
      }
    } else if (strncmp(argv[i], "--textcolor=", 12) == 0) {
      char *colorStr = argv[i] + 12;
      // 检查是否是RGB格式 (r,g,b)
      if (strchr(colorStr, ',') != NULL) {
        int r, g, b;
        if (sscanf(colorStr, "%d,%d,%d", &r, &g, &b) == 3) {
          // 确保RGB值在有效范围内
          if (r < 0)
            r = 0;
          if (r > 255)
            r = 255;
          if (g < 0)
            g = 0;
          if (g > 255)
            g = 255;
          if (b < 0)
            b = 0;
          if (b > 255)
            b = 255;
          textColor = RGB(r, g, b);
        }
      } else {
        // 使用颜色名称
        textColor = GetColorByName(colorStr);
      }
    } else if (strcmp(argv[i], "--bold") == 0) {
      bold = TRUE;
    } else if (strcmp(argv[i], "--modal") == 0) {
      modal = TRUE;
    } else if (strcmp(argv[i], "--nodrag") == 0) {
      noDrag = TRUE;
     } else if (strncmp(argv[i], "--encoding=", 11) == 0) {
       char *encoding = argv[i] + 11;
       if (strcmp(encoding, "utf8") == 0 || strcmp(encoding, "utf-8") == 0) {
         codePage = CP_UTF8;
       } else if (strcmp(encoding, "gbk") == 0 || strcmp(encoding, "gb2312") == 0) {
         codePage = 936; // GBK
       } else if (strcmp(encoding, "big5") == 0) {
         codePage = 950; // BIG5
       } else if (strcmp(encoding, "auto") == 0) {
         codePage = CP_ACP; // 自动检测
       } else {
         // 未知编码，默认UTF-8
         codePage = CP_UTF8;
       }
     } else if (strncmp(argv[i], "--onclick=", 10) == 0) {
       hasOnClick = TRUE;
       onClickCommand = argv[i] + 10;
     }
   }

  // Check if required parameters are provided
  if (!hasText) {
    printf("Error: Text parameter is required.\n");
    return;
  }

  // 处理命令行参数中的换行符（将\\n替换为\n）
  char *processedMessage = (char *)malloc(strlen(message) * 2 + 1);
  if (!processedMessage) {
    printf("Error: Memory allocation failed\n");
    return;
  }
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
  params->codePage = codePage;
  params->onClickCommand = NULL;

  if (hasOnClick && onClickCommand) {
    size_t cmdLen = strlen(onClickCommand);
    params->onClickCommand = (char*)malloc(cmdLen + 1);
    if (params->onClickCommand) {
      strncpy(params->onClickCommand, onClickCommand, cmdLen);
      params->onClickCommand[cmdLen] = '\0';
    }
  }

  // 注册窗口类
  WNDCLASSA wc = {0};
  wc.lpfnWndProc = WindowWndProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = "WindowClass";
  wc.hbrBackground = CreateSolidBrush(bgColor); // 使用指定的背景色
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hIcon = LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_ICON1));

  RegisterClassA(&wc);

  // 计算窗口位置，使其居中显示
  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  int x = (screenWidth - width) / 2;
  int y = (screenHeight - height) / 2;

  // 将UTF-8标题转换为ANSI（用于CreateWindowExA）
  int titleLen = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
  wchar_t *wideTitle = (wchar_t *)malloc(titleLen * sizeof(wchar_t));
  if (!wideTitle) {
    printf("Error: Memory allocation failed\n");
    free_param_context(context);
    return;
  }
  MultiByteToWideChar(CP_UTF8, 0, title, -1, wideTitle, titleLen);
  int ansiLen = WideCharToMultiByte(CP_ACP, 0, wideTitle, -1, NULL, 0, NULL, NULL);
  char *ansiTitle = (char *)malloc(ansiLen);
  if (!ansiTitle) {
    printf("Error: Memory allocation failed\n");
    free(wideTitle);
    free_param_context(context);
    return;
  }
  WideCharToMultiByte(CP_ACP, 0, wideTitle, -1, ansiTitle, ansiLen, NULL, NULL);
  strncpy(title, ansiTitle, sizeof(title) - 1);
  title[sizeof(title) - 1] = '\0';
  free(ansiTitle);
  free(wideTitle);

   // 创建窗口
  HWND hwnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_APPWINDOW, // 强制顶层显示
                  "WindowClass",
                  title, // 使用转换后的ANSI标题
                  noDrag ? (WS_POPUP | WS_SYSMENU | WS_VISIBLE)
                         : // 禁止拖拽时使用弹出窗口样式
                      (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                       WS_VISIBLE), // 正常情况显示标题栏
                  x, y, width, height, NULL, NULL, GetModuleHandle(NULL),
                  params);

  if (!hwnd) {
    printf("Error: Failed to create window, error: %lu\n", GetLastError());
    DeleteObject((HBRUSH)wc.hbrBackground);
    UnregisterClassA("WindowClass", GetModuleHandle(NULL));
    free(params);
    free(processedMessage);
    if (modal) {
      EnumWindows(EnumWindowsProcEnable, (LPARAM)NULL);
    }
    return;
  }

  // 消息循环
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) { // NULL接收线程所有窗口消息，包括WM_QUIT
    if (msg.message == WM_QUIT) {
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

   // 注销窗口类前删除背景画刷
  DeleteObject((HBRUSH)wc.hbrBackground);
  UnregisterClassA("WindowClass", GetModuleHandle(NULL));

  // 如果是模态弹窗，重新启用所有窗口
  if (modal) {
    EnumWindows(EnumWindowsProcEnable, (LPARAM)NULL);
  }

   // 清理资源
  if (params->onClickCommand) {
    free(params->onClickCommand);
  }
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
    // 在SID操作失败时，确保返回FALSE而不是未定义的行为
    fIsRunAsAdmin = FALSE;
    goto Cleanup;
  }

  // 检查令牌是否包含管理员组
  if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin)) {
    dwError = GetLastError();
    // 在检查令牌失败时，确保返回FALSE而不是未定义的行为
    fIsRunAsAdmin = FALSE;
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

  // 如果有错误且不是ERROR_NO_TOKEN，则返回FALSE表示检查失败
  if (dwError != ERROR_SUCCESS) {
    return FALSE;
  }

  return fIsRunAsAdmin;
}

// 兼容Windows XP/Vista/7/10/11获取开机自启目录
BOOL GetStartupPath(BOOL forAllUsers, char *path, int pathSize) {
  DWORD dwMajorVersion = 0, dwMinorVersion = 0;
  GetWindowsVersionSafe(&dwMajorVersion, &dwMinorVersion);

  if (forAllUsers) {
    // Windows Vista+ 使用新的API
    if (dwMajorVersion >= 6) {
      typedef HRESULT(WINAPI * PFN_SHGetKnownFolderPath)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *);
      PFN_SHGetKnownFolderPath pfnSHGetKnownFolderPath = (PFN_SHGetKnownFolderPath)
        GetProcAddress(GetModuleHandleW(L"shell32.dll"), "SHGetKnownFolderPath");
      if (pfnSHGetKnownFolderPath) {
        PWSTR pwStr = NULL;
        HRESULT hr = pfnSHGetKnownFolderPath((REFKNOWNFOLDERID)&FOLDERID_CommonStartup, 0, NULL, &pwStr);
        if (SUCCEEDED(hr) && pwStr) {
          WideCharToMultiByte(CP_ACP, 0, pwStr, -1, path, pathSize, NULL, NULL);
          CoTaskMemFree(pwStr);
          return TRUE;
        }
      }
    }
    // Windows XP 使用旧API
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_STARTUP, NULL,
                                    SHGFP_TYPE_CURRENT, path))) {
      return TRUE;
    }
  } else {
    // Windows Vista+ 使用新的API
    if (dwMajorVersion >= 6) {
      typedef HRESULT(WINAPI * PFN_SHGetKnownFolderPath)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *);
      PFN_SHGetKnownFolderPath pfnSHGetKnownFolderPath = (PFN_SHGetKnownFolderPath)
        GetProcAddress(GetModuleHandleW(L"shell32.dll"), "SHGetKnownFolderPath");
      if (pfnSHGetKnownFolderPath) {
        PWSTR pwStr = NULL;
        HRESULT hr = pfnSHGetKnownFolderPath((REFKNOWNFOLDERID)&FOLDERID_Startup, 0, NULL, &pwStr);
        if (SUCCEEDED(hr) && pwStr) {
          WideCharToMultiByte(CP_ACP, 0, pwStr, -1, path, pathSize, NULL, NULL);
          CoTaskMemFree(pwStr);
          return TRUE;
        }
      }
    }
    // Windows XP 使用旧API
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL,
                                    SHGFP_TYPE_CURRENT, path))) {
      return TRUE;
    }
  }
  return FALSE;
}

// 提升权限并重新运行程序
BOOL ElevatePrivileges(int argc, char *argv[]) {
  wchar_t szPath[MAX_PATH];
  wchar_t szCmdLine[2048] = {0};

  // 获取当前程序路径
  if (!GetModuleFileNameW(NULL, szPath, MAX_PATH)) {
    return FALSE;
  }

  // 构建命令行参数
  wcscat(szCmdLine, L"\"");
  wcscat(szCmdLine, szPath);
  wcscat(szCmdLine, L"\" ");

  // 添加原始参数（使用UTF-8编码以支持中文路径）
  for (int i = 1; i < argc; i++) {
    wchar_t argW[512];
    MultiByteToWideChar(CP_UTF8, 0, argv[i], -1, argW, 512);

    wcscat(szCmdLine, L"\"");
    wcscat(szCmdLine, argW);
    wcscat(szCmdLine, L"\" ");
  }

  // 添加 --once 标记，防止重复提权
  wcscat(szCmdLine, L"--once ");

   // 初始化SHELLEXECUTEINFO结构
  SHELLEXECUTEINFOW sei = {0};
  sei.cbSize = sizeof(SHELLEXECUTEINFOW);
  sei.lpVerb = L"runas"; // 请求提升权限
  sei.lpFile = szPath;
  sei.lpParameters = szCmdLine + wcslen(szPath) + 3; // 跳过程序路径部分
  sei.hwnd = NULL;
  sei.nShow = SW_NORMAL;

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

  // 权限提升成功，新进程已在后台启动
  printf("管理员权限获取成功，任务正在后台执行...\n");
  printf("新进程已启动，原始进程将退出\n");

  return TRUE;
}

void cmd_notify(int argc, char *argv[]) {

  // 使用新的参数解析框架
  ParamDefinition param_defs[] = {{"title", NULL, TRUE, FALSE},
                                  {"message", NULL, TRUE, FALSE},
                                  {"icon", NULL, FALSE, FALSE},
                                  {"timeout", NULL, FALSE, FALSE}};

  ParamContext *context = create_param_context(param_defs, 4);
  if (!context) {
    printf("Error: Failed to create parameter context\n");
    return;
  }

  // 解析参数
  parse_parameters(context, argc, argv, 2);

  // 检查必填参数
  if (!check_required_params(context)) {

    free_param_context(context);
    return;
  }

  // 获取参数值
  const char *title = get_param_value(context, "title");
  const char *message = get_param_value(context, "message");
  const char *iconType_str = get_param_value(context, "icon");
  char iconType[20] = "info"; // 默认图标
  if (iconType_str) {
    strncpy(iconType, iconType_str, sizeof(iconType) - 1);
    iconType[sizeof(iconType) - 1] = '\0';
  }

  int timeout = get_param_int_value(context, "timeout", 5);
  // 确保timeout在合理范围内
  if (timeout < 1)
    timeout = 1;
  if (timeout > 60)
    timeout = 60;

  printf("Displaying notification: %s\n", title);

  // 检查Windows版本以确定使用哪种通知方法
  DWORD dwMajorVersion = 0, dwMinorVersion = 0;
  GetWindowsVersionSafe(&dwMajorVersion, &dwMinorVersion);

  BOOL isWindowsXP = (dwMajorVersion == 5 &&
                      dwMinorVersion == 1); // Windows XP是5.1版本

  // 为Unicode转换分配内存
  int title_len = MultiByteToWideChar(CP_ACP, 0, title, -1, NULL, 0);
  int message_len = MultiByteToWideChar(CP_ACP, 0, message, -1, NULL, 0);

  // 检查转换是否成功
  if (title_len <= 0 || message_len <= 0) {
    printf("Error: Failed to convert text to Unicode\n");
    free_param_context(context);
    return;
  }

  WCHAR *wtitle = (WCHAR *)malloc(title_len * sizeof(WCHAR));
  WCHAR *wmessage = (WCHAR *)malloc(message_len * sizeof(WCHAR));

  // 检查内存分配是否成功
  if (!wtitle || !wmessage) {
    printf("Error: Memory allocation failed\n");
    if (wtitle) free(wtitle);
    if (wmessage) free(wmessage);
    free_param_context(context);
    return;
  }

  // 转换为宽字符
  MultiByteToWideChar(CP_ACP, 0, title, -1, wtitle, title_len);
  MultiByteToWideChar(CP_ACP, 0, message, -1, wmessage, message_len);

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

    // 显示消息框 - 使用Unicode版本
    MessageBoxW(NULL, wmessage, wtitle, mbType);

    // 注意：在XP系统上，MessageBox是模态的，无法实现超时自动关闭
    // 用户需要手动点击确定按钮
    printf("Note: On Windows XP, the notification requires user interaction to "
           "close\n");
  } else {
    // Windows 7/10/11实现 - 使用Shell_NotifyIcon
    printf("Using modern Windows notification\n");

    // 注册窗口类 - 使用Unicode版本
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"NotifyWindowClass";
    wc.hIcon = LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(IDI_ICON1));

    if (RegisterClassW(&wc)) {
      // 创建隐藏窗口 - 使用Unicode版本
      HWND hwnd =
          CreateWindowExW(0, L"NotifyWindowClass", L"NotifyWindow", 0, 0, 0, 0,
                          0, NULL, NULL, GetModuleHandle(NULL), NULL);

      if (hwnd) {
        // 初始化NOTIFYICONDATA结构 - 使用Unicode版本
        NOTIFYICONDATAW nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATAW);
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

        // 设置提示文本 - 使用宽字符
        wcsncpy(nid.szTip, wtitle, ARRAYSIZE(nid.szTip) - 1);
        nid.szTip[ARRAYSIZE(nid.szTip) - 1] = L'\0';

        // 添加通知图标 - 使用Unicode版本
        Shell_NotifyIconW(NIM_ADD, &nid);

        // 更新通知内容
        nid.uFlags = NIF_INFO;
        wcsncpy(nid.szInfoTitle, wtitle, ARRAYSIZE(nid.szInfoTitle) - 1);
        nid.szInfoTitle[ARRAYSIZE(nid.szInfoTitle) - 1] = L'\0';
        wcsncpy(nid.szInfo, wmessage, ARRAYSIZE(nid.szInfo) - 1);
        nid.szInfo[ARRAYSIZE(nid.szInfo) - 1] = L'\0';

        // 设置信息标志
        if (strcmp(iconType, "warning") == 0) {
          nid.dwInfoFlags = NIIF_WARNING;
        } else if (strcmp(iconType, "error") == 0) {
          nid.dwInfoFlags = NIIF_ERROR;
        } else {
          nid.dwInfoFlags = NIIF_INFO;
        }

        // 显示通知 - 使用Unicode版本
        Shell_NotifyIconW(NIM_MODIFY, &nid);

        // 等待指定时间
        Sleep(timeout * 1000);

        // 删除通知图标 - 使用Unicode版本
        Shell_NotifyIconW(NIM_DELETE, &nid);

        // 销毁窗口
        DestroyWindow(hwnd);
      }

      // 注销窗口类 - 使用Unicode版本
      UnregisterClassW(L"NotifyWindowClass", GetModuleHandle(NULL));
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
      // 使用Unicode版本
      MessageBoxW(NULL, wmessage, wtitle, mbType);
    }
  }

  // 释放分配的内存
  free(wtitle);
  free(wmessage);
  // 释放参数上下文
  free_param_context(context);
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
static int config_ini_handler(void *user, const char *section, const char *name,
                              const char *value) {
  struct IniData *data = (struct IniData *)user;
  struct IniEntry *entry = (struct IniEntry *)malloc(sizeof(struct IniEntry));

  if (!entry)
    return 0;

  memset(entry, 0, sizeof(struct IniEntry));
  strncpy(entry->section, section, sizeof(entry->section) - 1);
  entry->section[sizeof(entry->section) - 1] = '\0';

  strncpy(entry->name, name, sizeof(entry->name) - 1);
  entry->name[sizeof(entry->name) - 1] = '\0';

  if (value) {
    strncpy(entry->value, value, sizeof(entry->value) - 1);
    entry->value[sizeof(entry->value) - 1] = '\0';
  }

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
struct IniEntry *find_ini_entry(struct IniData *data, const char *section,
                                const char *name) {
  if (!data || !section || !name) {
    return NULL;
  }
  struct IniEntry *entry = data->head;
  while (entry) {
    if (strcmp(entry->section, section) == 0 &&
        strcmp(entry->name, name) == 0) {
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

  // Parse parameters
  char filePath[MAX_PATH] = {0};
  char action[20] = "get"; // default action
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

    return;
  }

  printf("Config file: %s\n", filePath);
  printf("Action: %s\n", action);

  if (strcmp(action, "get") == 0) {
    // 获取单个配置项
    if (!hasSection || !hasKey) {
      printf("Error: Section and key must be specified for get action\n");

      return;
    }

    struct IniData data = {0};
    int result = ini_parse(filePath, config_ini_handler, &data);

    if (result == 0) {
      struct IniEntry *entry = find_ini_entry(&data, section, key);
      if (entry) {
        // 直接输出值，方便其他程序使用
        printf("%s\n", entry->value);
      } else {
        printf("Error: Entry not found\n");
      }
      free_ini_data(&data);
    } else if (result == -1) {
      printf("Error: Unable to open file %s\n", filePath);
    } else {
      printf("Error: Parse error on line %d\n", result);
      free_ini_data(&data);
    }
  } else if (strcmp(action, "set") == 0) {
    // 写入INI文件 - 尝试写入系统目录需要管理员权限
    if (!hasSection || !hasKey || !hasValue) {
      printf("Error: Section, key, and value must be specified for write "
             "action\n");

      return;
    }

    // 检查是否需要管理员权限（写入系统配置）
    char systemDir[MAX_PATH];
    if (GetSystemDirectoryA(systemDir, MAX_PATH) > 0) {
      if (strncmp(filePath, systemDir, strlen(systemDir)) == 0) {
        // 目标是系统目录，需要管理员权限
        // 检查是否已经尝试过提权（防止无限循环）
        BOOL alreadyElevated = FALSE;
        for (int i = 2; i < argc; i++) {
          if (strcmp(argv[i], "--once") == 0) {
            alreadyElevated = TRUE;
            break;
          }
        }
        if (!IsRunAsAdmin() && !alreadyElevated) {
          printf("写入系统配置文件需要管理员权限，正在请求权限提升...\n");
          if (ElevatePrivileges(argc, argv)) {
            return;
          } else {
            printf("权限提升失败，将尝试直接写入...\n");
          }
        }
      }
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
      entry = (struct IniEntry *)malloc(sizeof(struct IniEntry));
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

    // 写入文件（使用UTF-8 BOM编码）
    FILE *file = fopen(filePath, "wb");
    if (file) {
      // 写入UTF-8 BOM
      unsigned char bom[] = {0xEF, 0xBB, 0xBF};
      fwrite(bom, 1, 3, file);

      // 按section分组写入
      char line[1024];
      struct IniEntry *write_entry = data.head;
      while (write_entry) {
        if (write_entry->section[0] != '\0') {
          snprintf(line, sizeof(line), "[%s]\n", write_entry->section);
        } else {
          snprintf(line, sizeof(line), "\n");
        }
        fwrite(line, 1, strlen(line), file);

        if (write_entry->name[0] != '\0') {
          snprintf(line, sizeof(line), "%s = %s\n", write_entry->name, write_entry->value);
          fwrite(line, 1, strlen(line), file);
        }
        write_entry = write_entry->next;
      }
      fclose(file);
      printf("Configuration saved successfully\n");
    } else {
      printf("Error: Unable to write to file %s\n", filePath);
    }

    free_ini_data(&data);
  } else if (strcmp(action, "del") == 0) {
    // 删除INI条目
    if (!hasSection || !hasKey) {
      printf("Error: Section and key must be specified for delete action\n");

      return;
    }

    // 检查是否需要管理员权限（删除系统配置）
    char systemDir2[MAX_PATH];
    if (GetSystemDirectoryA(systemDir2, MAX_PATH) > 0) {
      if (strncmp(filePath, systemDir2, strlen(systemDir2)) == 0) {
        // 目标是系统目录，需要管理员权限
        // 检查是否已经尝试过提权（防止无限循环）
        BOOL alreadyElevated = FALSE;
        for (int i = 2; i < argc; i++) {
          if (strcmp(argv[i], "--once") == 0) {
            alreadyElevated = TRUE;
            break;
          }
        }
        if (!IsRunAsAdmin() && !alreadyElevated) {
          printf("删除系统配置文件需要管理员权限，正在请求权限提升...\n");
          if (ElevatePrivileges(argc, argv)) {
            return;
          } else {
            printf("权限提升失败，将尝试直接删除...\n");
          }
        }
      }
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
      if (strcmp(entry->section, section) == 0 &&
          strcmp(entry->name, key) == 0) {
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

     // 写入文件（使用UTF-8 BOM编码）
    FILE *file = fopen(filePath, "wb");
    if (file) {
      // 写入UTF-8 BOM
      unsigned char bom[] = {0xEF, 0xBB, 0xBF};
      fwrite(bom, 1, 3, file);

      // 按section分组写入
      char line[1024];
      struct IniEntry *write_entry = data.head;
      while (write_entry) {
        if (write_entry->section[0] != '\0') {
          snprintf(line, sizeof(line), "[%s]\n", write_entry->section);
        } else {
          snprintf(line, sizeof(line), "\n");
        }
        fwrite(line, 1, strlen(line), file);

        if (write_entry->name[0] != '\0') {
          snprintf(line, sizeof(line), "%s = %s\n", write_entry->name, write_entry->value);
          fwrite(line, 1, strlen(line), file);
        }
        write_entry = write_entry->next;
      }
      fclose(file);
      printf("Configuration saved successfully\n");
    } else {
      printf("Error: Unable to write to file %s\n", filePath);
    }

    free_ini_data(&data);
  } else {
    printf("Error: Unknown action '%s'. Use get, set, save, or del\n", action);
  }
}

int cmd_process(int argc, char *argv[]) {

  // Parse parameters
  char action[20] = "check"; // default action
  char processName[MAX_PATH] = {0};
  DWORD processId = 0;

  // Parameters for run action
  char workingFolder[MAX_PATH] = {0};
  char commandLine[MAX_PATH * 2] = {0};

  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--action=", 9) == 0) {
      strncpy(action, argv[i] + 9, sizeof(action) - 1);
      action[sizeof(action) - 1] = '\0';
    } else if (strncmp(argv[i], "--name=", 7) == 0) {
      strncpy(processName, argv[i] + 7, MAX_PATH - 1);
      processName[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--pid=", 6) == 0) {
      processId = atoi(argv[i] + 6);
    } else if (strncmp(argv[i], "--workdir=", 10) == 0) {
      strncpy(workingFolder, argv[i] + 10, MAX_PATH - 1);
      workingFolder[MAX_PATH - 1] = '\0';
    } else if (strncmp(argv[i], "--exec=", 7) == 0) {
      strncpy(commandLine, argv[i] + 7, sizeof(commandLine) - 1);
      commandLine[sizeof(commandLine) - 1] = '\0';
    }
  }

  // Validate parameters
  if (strcmp(action, "run") == 0) {
    // Run action specific validation
    if (strlen(commandLine) == 0) {
      printf("Error: --exec parameter is required for run action\n");
      return 1;
    }

    // Run the application with specified working folder
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    ZeroMemory(&pi, sizeof(pi));

    // Start the application
    if (CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL,
                       workingFolder, &si, &pi)) {
      printf("Application '%s' started successfully in folder '%s'\n",
             commandLine, workingFolder);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return 0;
    } else {
      printf("Error: Failed to start application '%s' in folder '%s'\n",
             commandLine, workingFolder);
      printf("Error code: %lu\n", GetLastError());
      return 1;
    }
  } else if (strlen(processName) == 0 && processId == 0) {
    printf("Error: Either process name or PID must be specified\n");
    return 1;
  } else if (strcmp(action, "check") == 0) {
    // Check if process exists
    if (processId > 0) {
      // Check by PID
      HANDLE hProcess =
          OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
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
      int processCount = find_process_by_name(processName, &foundPid);
      return processCount;
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
          printf("Error: Failed to terminate process (Error code: %lu)\n",
                 GetLastError());
          CloseHandle(hProcess);
          return 1;
        }
      } else {
        printf("Error: Failed to open process (Error code: %lu)\n",
               GetLastError());
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

    return 1;
  }

  // Default return value (should never reach here due to the logic above)
  return 1;
}

// 通用进程查找函数
int find_process_by_name(const char *processName, DWORD *processId) {
  PROCESSENTRY32 pe32;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  int count = 0;
  DWORD firstProcessId = 0;
  BOOL foundFirst = FALSE;

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
      do {
        if (_stricmp(pe32.szExeFile, processName) == 0) {
          count++;
          // 记录第一个找到的进程ID
          if (!foundFirst) {
            firstProcessId = pe32.th32ProcessID;
            foundFirst = TRUE;
          }
        }
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }

  // 如果提供了processId指针且至少找到了一个进程，返回第一个进程的ID
  if (processId != NULL && foundFirst) {
    *processId = firstProcessId;
  }

  // 返回找到的进程数量
  return count;
}

// 通过可执行文件名获取所有进程ID
ProcessIdList *get_pids_by_exe_name(const char *exeName) {
  PROCESSENTRY32 pe32;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  ProcessIdList *pidList = (ProcessIdList *)malloc(sizeof(ProcessIdList));

  if (pidList == NULL) {
    return NULL;
  }

  // 初始化列表
  pidList->count = 0;
  pidList->capacity = 10; // 初始容量
  pidList->pids = (DWORD *)malloc(sizeof(DWORD) * pidList->capacity);
  if (!pidList->pids) {
    free(pidList);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
      CloseHandle(hSnapshot);
    }
    return NULL;
  }

  if (pidList->pids == NULL) {
    free(pidList);
    return NULL;
  }

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
      do {
        if (_stricmp(pe32.szExeFile, exeName) == 0) {
          // 如果需要扩展容量
          if (pidList->count >= pidList->capacity) {
            pidList->capacity *= 2;
            DWORD *newPids = (DWORD *)realloc(
                pidList->pids, sizeof(DWORD) * pidList->capacity);
            if (newPids == NULL) {
              // 内存重新分配失败，清理并返回NULL
              free(pidList->pids);
              free(pidList);
              CloseHandle(hSnapshot);
              return NULL;
            }
            pidList->pids = newPids;
          }

          // 添加PID到列表
          pidList->pids[pidList->count] = pe32.th32ProcessID;
          pidList->count++;
        }
      } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
  }

  return pidList;
}

// 通用进程终止函数
BOOL kill_process_by_name(const char *processName) {
  PROCESSENTRY32 pe32;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  BOOL found = FALSE;

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
      do {
        if (_stricmp(pe32.szExeFile, processName) == 0) {
          // 找到匹配的进程，杀掉它
          HANDLE hProcess =
              OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
          if (hProcess != NULL) {
            if (TerminateProcess(hProcess, 0)) {
              printf("Process '%s' with PID %lu terminated successfully\n",
                     processName, pe32.th32ProcessID);
            } else {
              printf("Error: Unable to terminate process '%s' with PID %lu\n",
                     processName, pe32.th32ProcessID);
            }
            CloseHandle(hProcess);
            found = TRUE;
          } else {
            printf("Error: Unable to open process '%s' with PID %lu\n",
                   processName, pe32.th32ProcessID);
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
  timestamp /= 10000;             // 100-nanosecond intervals to milliseconds
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
  snprintf(uuid_str, 37, "%08x-%04x-%04x-%04x-%04x%08x", data[0], data[1] & 0xFFFF,
          (data[1] >> 16) & 0xFFFF, data[2] & 0xFFFF, (data[2] >> 16) & 0xFFFF,
          data[3]);
}

// 生成UUID v7
void generate_uuid_v7(char *uuid_str) {
  // 获取当前时间戳（毫秒）
  uint64_t timestamp = get_current_timestamp_ms();

  // 生成随机数据，使用更安全的方式避免整数溢出
  uint32_t rand_a = ((unsigned int)rand() << 16) | (unsigned int)rand();
  uint32_t rand_b = ((unsigned int)rand() << 16) | (unsigned int)rand();

  // UUID v7格式：
  // 时间戳（48位）+ 随机数（12位）+ 版本（4位）+ 变体（2位）+ 随机数（62位）

  // 时间戳高位（32位）
  uint32_t ts_high = (timestamp >> 16) & 0xFFFFFFFF;

  // 时间戳低位（16位）+ 随机数高位（12位）
  uint32_t ts_low_rand =
      ((timestamp & 0xFFFF) << 12) | ((rand_a >> 20) & 0xFFF);

  // 随机数中位（16位），设置版本为0111（v7）
  uint32_t rand_mid = (rand_a >> 4) & 0x0FFF; // 清除版本位
  rand_mid |= 0x7000;                         // 设置版本为7

  // 随机数低位（16位），设置变体为10
  uint32_t rand_low = rand_b & 0x3FFF; // 清除变体位
  rand_low |= 0x8000;                  // 设置变体为10

  // 格式化为UUID字符串
  // UUID格式：8-4-4-4-12
  snprintf(uuid_str, 37, "%08x-%04x-%04x-%04x-%04x%08x", ts_high,
          (ts_low_rand >> 16) & 0xFFFF, // 时间戳低位和随机数高位的高16位
          ts_low_rand & 0xFFFF,         // 时间戳低位和随机数高位的低16位
          rand_mid,                     // 版本字段
          rand_low,                     // 变体位
          rand_b >> 16);                // 剩余随机数
}

// 雪花ID生成器结构
typedef struct {
  uint64_t last_timestamp;
  uint64_t node_id;  // 节点ID（这里简化为固定值）
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

  // 如果时钟回拨，使用更严格的处理
  if (timestamp < sf->last_timestamp) {
    // 等待时钟赶上，最多等待100毫秒
    uint64_t wait_count = 0;
    while (timestamp < sf->last_timestamp) {
      Sleep(1); // 等待1毫秒
      timestamp = get_current_timestamp_ms();
      wait_count++;

      // 如果等待超过100毫秒，使用当前时间戳并增加序列号
      if (wait_count > 100) {
        timestamp = sf->last_timestamp;
        break;
      }
    }
  }

  // 如果同一毫秒内，序列号递增
  if (timestamp == sf->last_timestamp) {
    sf->sequence = (sf->sequence + 1) & 0xFFF; // 12位序列号
    if (sf->sequence == 0) {
      uint64_t wait_count = 0;
      while (timestamp <= sf->last_timestamp) {
        Sleep(1);
        timestamp = get_current_timestamp_ms();
        wait_count++;
        if (wait_count > 1000) {
          break;
        }
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

// 托盘图标相关的常量定义
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_APP_ICON 1001
#define ID_TRAY_EXIT 1002
#define ID_TRAY_ABOUT 1003

// 菜单命令结构
typedef struct {
  char name[128];    // 菜单项名称
  char command[256]; // 菜单项对应的命令
} MenuCommand;

// 托盘图标数据结构
typedef struct {
  HWND hwnd;
  HMENU hMenu;
  NOTIFYICONDATA nid;
  char process_name[MAX_PATH]; // 增加大小以支持完整路径
  char icon_path[MAX_PATH];    // 增加大小以支持完整路径
  BOOL process_running;
  HICON hIcon;                // 存储加载的图标
  MenuCommand *menu_commands; // 存储菜单命令的数组
  int menu_command_count;     // 菜单命令的数量
} TrayIconData;

// 浮动图标相关的常量定义
#define WM_FLOATING_ICON (WM_USER + 2)
#define ID_FLOATING_ABOUT 2001

// 浮动图标数据结构
typedef struct {
  HWND hwnd;
  HMENU hMenu;
  char process_name[MAX_PATH]; // 要监控的进程名
  char icon_path[MAX_PATH];    // 图标路径
  BOOL process_running;
  HICON hIcon;                // 存储加载的图标
  MenuCommand *menu_commands; // 存储菜单命令的数组
  int menu_command_count;     // 菜单命令的数量
  POINT drag_start;           // 拖动开始位置
  BOOL is_dragging;           // 是否正在拖动
} FloatingIconData;

// 托盘图标窗口过程函数声明
LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 浮动图标窗口过程函数声明
LRESULT CALLBACK FloatingWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                 LPARAM lParam);

// 检查进程是否运行的函数
BOOL is_process_running(const char *processName) {
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
HICON extract_icon_from_exe(const char *exePath) {
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
void update_tray_icon(TrayIconData *trayData) {
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
  snprintf(trayData->nid.szTip, sizeof(trayData->nid.szTip), "%s - Running",
           trayData->process_name);

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
BOOL create_tray_icon(TrayIconData *trayData, HINSTANCE hInstance,
                      const char *title, const char *processName,
                      const char *iconPath, MenuCommand *menu_commands,
                      int menu_command_count) {
  // 注册窗口类
  WNDCLASS wc = {0};
  wc.lpfnWndProc = TrayWndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "SPCMDTrayIconClass";
  wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON1));

  if (!RegisterClass(&wc)) {
    return FALSE;
  }

  // 创建隐藏窗口
  trayData->hwnd =
      CreateWindowEx(0, "SPCMDTrayIconClass", "SPCMD Tray Icon Window",
                     WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                     CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

   if (!trayData->hwnd) {
    UnregisterClass("SPCMDTrayIconClass", GetModuleHandle(NULL));
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
  strncpy(trayData->process_name, processName,
          sizeof(trayData->process_name) - 1);
  trayData->process_name[sizeof(trayData->process_name) - 1] = '\0';

  strncpy(trayData->icon_path, iconPath, sizeof(trayData->icon_path) - 1);
  trayData->icon_path[sizeof(trayData->icon_path) - 1] = '\0';

  // 初始化图标
  trayData->hIcon = NULL;

  // 检查进程是否正在运行
  trayData->process_running = is_process_running(processName);

  // 如果进程未运行，不创建托盘图标
  if (!trayData->process_running) {
    printf("Process '%s' is not running. Tray icon will not be displayed.\n",
           processName);
    // 注销窗口类 - 防止资源泄漏
    UnregisterClass("SPCMDTrayIconClass", GetModuleHandle(NULL));
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
    if (trayData->hIcon != NULL &&
        trayData->hIcon != LoadIcon(NULL, IDI_APPLICATION)) {
      DestroyIcon(trayData->hIcon);
    }
    DestroyWindow(trayData->hwnd);
    UnregisterClass("SPCMDTrayIconClass", GetModuleHandle(NULL));
    return FALSE;
   }

  // 保存菜单命令
  trayData->menu_commands = menu_commands;
  trayData->menu_command_count = menu_command_count;

  // 创建右键菜单
  trayData->hMenu = CreatePopupMenu();

  // 添加自定义菜单项
  for (int i = 0; i < menu_command_count; i++) {
    AppendMenu(trayData->hMenu, MF_STRING, ID_TRAY_ABOUT + 1 + i,
               menu_commands[i].name);
  }

  // 添加分隔线
  if (menu_command_count > 0) {
    AppendMenu(trayData->hMenu, MF_SEPARATOR, 0, NULL);
  }

  // 添加About菜单项
  AppendMenu(trayData->hMenu, MF_STRING, ID_TRAY_ABOUT, "About");

  return TRUE;
}

// 销毁托盘图标
void destroy_tray_icon(TrayIconData *trayData) {
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
LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                             LPARAM lParam) {
  TrayIconData *trayData =
      (TrayIconData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

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
        TrackPopupMenu(trayData->hMenu, TPM_RIGHTBUTTON, curPoint.x, curPoint.y,
                       0, hwnd, NULL);
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

    default: {
      // 处理自定义菜单项
      int menu_id = LOWORD(wParam);
      int custom_menu_index = menu_id - (ID_TRAY_ABOUT + 1);
      if (custom_menu_index >= 0 &&
          custom_menu_index < trayData->menu_command_count) {
        // 执行对应的命令
        char *command = trayData->menu_commands[custom_menu_index].command;
        if (strlen(command) > 0) {
          STARTUPINFO si = {0};
          PROCESS_INFORMATION pi = {0};
          si.cb = sizeof(si);

          CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si,
                        &pi);

          // 关闭进程和线程句柄
          CloseHandle(pi.hProcess);
          CloseHandle(pi.hThread);
        }
      }
      break;
    }
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

// 浮动图标窗口过程函数
LRESULT CALLBACK FloatingWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                 LPARAM lParam) {
  FloatingIconData *floatingData =
      (FloatingIconData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (msg) {
  case WM_CREATE: {
    // 获取传递的参数
    CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
    floatingData = (FloatingIconData *)pcs->lpCreateParams;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)floatingData);
    floatingData->hwnd = hwnd;
    floatingData->is_dragging = FALSE;

    // 加载图标
    if (strlen(floatingData->icon_path) > 0 &&
        PathFileExistsA(floatingData->icon_path)) {
      floatingData->hIcon = extract_icon_from_exe(floatingData->icon_path);
    } else {
      floatingData->hIcon = LoadIcon(NULL, IDI_INFORMATION);
    }

    // 设置窗口图标
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)floatingData->hIcon);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)floatingData->hIcon);

    // 创建右键菜单
    floatingData->hMenu = CreatePopupMenu();

    // 添加自定义菜单项
    for (int i = 0; i < floatingData->menu_command_count; i++) {
      AppendMenu(floatingData->hMenu, MF_STRING, ID_FLOATING_ABOUT + 1 + i,
                 floatingData->menu_commands[i].name);
    }

    // 添加分隔线
    if (floatingData->menu_command_count > 0) {
      AppendMenu(floatingData->hMenu, MF_SEPARATOR, 0, NULL);
    }

    // 添加About菜单项
    AppendMenu(floatingData->hMenu, MF_STRING, ID_FLOATING_ABOUT, "About");

    // 设置定时器，每1秒更新一次状态
    SetTimer(hwnd, 1, 1000, NULL);

    break;
  }

  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // 获取客户区大小
    RECT rect;
    GetClientRect(hwnd, &rect);

    // 绘制图标
    if (floatingData->hIcon) {
      DrawIcon(hdc, 0, 0, floatingData->hIcon);
    }

    EndPaint(hwnd, &ps);
    break;
  }

  case WM_LBUTTONDOWN: {
    // 开始拖动
    floatingData->is_dragging = TRUE;

    // 保存鼠标相对于窗口的初始位置
    floatingData->drag_start.x = LOWORD(lParam);
    floatingData->drag_start.y = HIWORD(lParam);

    SetCapture(hwnd);
    break;
  }

  case WM_MOUSEMOVE: {
    if (floatingData->is_dragging) {
      // 获取鼠标当前的屏幕坐标
      POINT cursor_pos;
      GetCursorPos(&cursor_pos);

      // 获取窗口当前位置和大小
      RECT window_rect;
      GetWindowRect(hwnd, &window_rect);

      // 计算新的窗口位置（鼠标位置减去初始偏移量）
      int new_x = cursor_pos.x - floatingData->drag_start.x;
      int new_y = cursor_pos.y - floatingData->drag_start.y;

      // 移动窗口
      MoveWindow(hwnd, new_x, new_y, window_rect.right - window_rect.left,
                 window_rect.bottom - window_rect.top, TRUE);
    }
    break;
  }

  case WM_LBUTTONUP: {
    // 结束拖动
    floatingData->is_dragging = FALSE;
    ReleaseCapture();
    break;
  }

  case WM_RBUTTONDOWN: {
    // 右键点击显示菜单
    POINT curPoint;
    GetCursorPos(&curPoint);

    // 设置菜单前景窗口以确保菜单正确消失
    SetForegroundWindow(hwnd);

    // 显示上下文菜单
    TrackPopupMenu(floatingData->hMenu, TPM_RIGHTBUTTON, curPoint.x, curPoint.y,
                   0, hwnd, NULL);
    break;
  }

  case WM_COMMAND: {
    switch (LOWORD(wParam)) {
    case ID_FLOATING_ABOUT:
      MessageBox(hwnd, "SPCMD Floating Icon\nMonitoring process status",
                 "About", MB_OK | MB_ICONINFORMATION);
      break;

    default: {
      // 处理自定义菜单项
      int menu_id = LOWORD(wParam);
      int custom_menu_index = menu_id - (ID_FLOATING_ABOUT + 1);
      if (custom_menu_index >= 0 &&
          custom_menu_index < floatingData->menu_command_count) {
        // 执行对应的命令
        char *command = floatingData->menu_commands[custom_menu_index].command;
        if (strlen(command) > 0) {
          STARTUPINFO si = {0};
          PROCESS_INFORMATION pi = {0};
          si.cb = sizeof(si);

          CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si,
                        &pi);

          // 关闭进程和线程句柄
          CloseHandle(pi.hProcess);
          CloseHandle(pi.hThread);
        }
      }
      break;
    }
    }
    break;
  }

   case WM_TIMER: {
    if (!floatingData) {
      break;
    }
    floatingData->process_running =
        is_process_running(floatingData->process_name);
    if (!floatingData->process_running) {
      PostMessage(hwnd, WM_DESTROY, 0, 0);
    }
    break;
   }

   case WM_DESTROY: {
    if (!floatingData) {
      PostQuitMessage(0);
      break;
    }
    // 清理资源
    KillTimer(hwnd, 1);

    // 销毁菜单
    if (floatingData->hMenu) {
      DestroyMenu(floatingData->hMenu);
      floatingData->hMenu = NULL;
    }

    // 销毁图标
    if (floatingData->hIcon != NULL &&
        floatingData->hIcon != LoadIcon(NULL, IDI_APPLICATION) &&
        floatingData->hIcon != LoadIcon(NULL, IDI_INFORMATION) &&
        floatingData->hIcon != LoadIcon(NULL, IDI_ERROR)) {
      DestroyIcon(floatingData->hIcon);
      floatingData->hIcon = NULL;
    }

    PostQuitMessage(0);
    break;
  }

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  return 0;
}

// 托盘图标命令
void cmd_tray(int argc, char *argv[]) {

  // Parse parameters
  char process_name[MAX_PATH] = "python.exe"; // default process name
  char title[MAX_PATH] = "SPCMD Tray";        // default title
  char icon_path[MAX_PATH] = "";    // default icon path (use system default)
  char process_path[MAX_PATH] = ""; // process path for auto detection

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

  // 处理menu参数
  MenuCommand *menu_commands = NULL;
  int menu_command_count = 0;

  // 使用新的参数解析框架处理menu参数
  ParamDefinition param_defs[] = {
      {"menu", NULL, FALSE, TRUE} // 菜单命令，支持多个，格式：name,command
  };

  ParamContext *context = create_param_context(param_defs, 1);
  if (context) {
    // 解析参数
    parse_parameters(context, argc, argv, 2);

    // 获取所有menu参数值
    for (int i = 0; i < context->param_count; i++) {
      if (strcmp(context->params[i].name, "menu") == 0 &&
          context->params[i].value) {
        menu_command_count++;
      }
    }

    if (menu_command_count > 0) {
      menu_commands =
          (MenuCommand *)malloc(sizeof(MenuCommand) * menu_command_count);
      if (menu_commands) {
        // 解析menu参数
        int current_index = 0;
        for (int i = 0; i < context->param_count; i++) {
          if (strcmp(context->params[i].name, "menu") == 0 &&
              context->params[i].value) {
            char *menu_value = context->params[i].value;
            char *comma_pos = strchr(menu_value, ',');
            if (comma_pos) {
              // 分离菜单项名称和命令
              *comma_pos = '\0';
              strncpy(menu_commands[current_index].name, menu_value,
                      sizeof(menu_commands[current_index].name) - 1);
              menu_commands[current_index]
                  .name[sizeof(menu_commands[current_index].name) - 1] = '\0';
              strncpy(menu_commands[current_index].command, comma_pos + 1,
                      sizeof(menu_commands[current_index].command) - 1);
              menu_commands[current_index]
                  .command[sizeof(menu_commands[current_index].command) - 1] =
                  '\0';
              current_index++;
            }
          }
        }
      }
    }

    // 释放参数上下文
    free_param_context(context);
  }

  // 检查进程是否正在运行
  if (!is_process_running(process_name)) {
    printf("Process '%s' is not running. Tray icon will not be displayed.\n",
           process_name);
    printf("Tray icon terminated.\n");
    return;
  }

  printf("Process is running. Creating tray icon...\n");

  // 初始化COM库
  CoInitialize(NULL);

  // 创建托盘图标数据结构
  TrayIconData trayData = {0};

  // 创建托盘图标
  if (!create_tray_icon(&trayData, GetModuleHandle(NULL), title, process_name,
                        icon_path, menu_commands, menu_command_count)) {
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

  // 释放菜单命令内存
  if (menu_commands) {
    free(menu_commands);
  }

  printf("System tray icon terminated\n");
}

// 核心功能函数 - 浮动图标

// 创建浮动图标
BOOL create_floating_icon(FloatingIconData *floatingData, HINSTANCE hInstance,
                          const char *title, const char *processName,
                          const char *iconPath, MenuCommand *menu_commands,
                          int menu_command_count) {
  // 注册窗口类
  WNDCLASS wc = {0};
  wc.lpfnWndProc = FloatingWndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "SPCMDFloatingIconClass";
  wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON1));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

  if (!RegisterClass(&wc)) {
    return FALSE;
  }

  // 保存进程名和图标路径
  strncpy(floatingData->process_name, processName,
          sizeof(floatingData->process_name) - 1);
  floatingData->process_name[sizeof(floatingData->process_name) - 1] = '\0';

  strncpy(floatingData->icon_path, iconPath,
          sizeof(floatingData->icon_path) - 1);
  floatingData->icon_path[sizeof(floatingData->icon_path) - 1] = '\0';

  // 保存菜单命令
  floatingData->menu_commands = menu_commands;
  floatingData->menu_command_count = menu_command_count;

  // 检查进程是否正在运行
  floatingData->process_running = is_process_running(processName);
  if (!floatingData->process_running) {
    // 注销窗口类 - 防止资源泄漏
    UnregisterClass("SPCMDFloatingIconClass", GetModuleHandle(NULL));
    return FALSE;
  }

  // 计算窗口大小（基于图标大小）
  int icon_size = GetSystemMetrics(SM_CXICON); // 使用大图标大小（通常32x32）
  int window_width = icon_size;                // 图标大小 + 边距
  int window_height = icon_size;

  // 计算窗口位置（屏幕居中）
  int screen_width = GetSystemMetrics(SM_CXSCREEN);
  int screen_height = GetSystemMetrics(SM_CYSCREEN);
  int x = (screen_width - window_width) / 2;   // 水平居中
  int y = (screen_height - window_height) / 2; // 垂直居中

  // 创建浮动窗口
  HWND hwnd = CreateWindowEx(
      WS_EX_TOPMOST | WS_EX_LAYERED |
          WS_EX_TOOLWINDOW, // 顶层、分层、工具窗口（不在任务栏显示）
      "SPCMDFloatingIconClass", title,
      WS_POPUP | WS_VISIBLE, // 弹出窗口，可见
      x, y, window_width, window_height, NULL, NULL, hInstance, floatingData);

  if (!hwnd) {
    UnregisterClass("SPCMDFloatingIconClass", hInstance);
    return FALSE;
  }

  // 设置窗口透明度
  SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

  return TRUE;
}

// 更新浮动图标状态
void update_floating_icon(FloatingIconData *floatingData) {
  // 检查进程状态
  floatingData->process_running =
      is_process_running(floatingData->process_name);
  if (!floatingData->process_running) {
    // 进程未运行，发送退出消息
    PostMessage(floatingData->hwnd, WM_DESTROY, 0, 0);
  }
}

// 销毁浮动图标
void destroy_floating_icon(FloatingIconData *floatingData) {
  // 销毁菜单
  if (floatingData->hMenu) {
    DestroyMenu(floatingData->hMenu);
    floatingData->hMenu = NULL;
  }

  // 销毁图标
  if (floatingData->hIcon != NULL &&
      floatingData->hIcon != LoadIcon(NULL, IDI_APPLICATION) &&
      floatingData->hIcon != LoadIcon(NULL, IDI_INFORMATION) &&
      floatingData->hIcon != LoadIcon(NULL, IDI_ERROR)) {
    DestroyIcon(floatingData->hIcon);
    floatingData->hIcon = NULL;
  }

  // 销毁窗口
  if (floatingData->hwnd) {
    DestroyWindow(floatingData->hwnd);
    floatingData->hwnd = NULL;
  }

  // 注销窗口类
  UnregisterClass("SPCMDFloatingIconClass", GetModuleHandle(NULL));
}

// 浮动图标命令
void cmd_floating(int argc, char *argv[]) {

  // 使用新的参数解析框架
  ParamDefinition param_defs[] = {
      {"process", NULL, FALSE, FALSE}, // 要监控的进程名
      {"title", NULL, FALSE, FALSE},   // 浮动图标的标题
      {"path", NULL, FALSE, FALSE},    // 进程路径，用于自动检测进程名和图标路径
      {"icon", NULL, FALSE, FALSE},    // 图标路径
      {"menu", NULL, FALSE, TRUE}      // 菜单命令，支持多个，格式：name,command
  };

  ParamContext *context = create_param_context(param_defs, 5);
  if (!context) {
    printf("Error: Failed to create parameter context\n");
    return;
  }

  // 解析参数
  parse_parameters(context, argc, argv, 2);

  // 获取参数值，使用默认值
  char process_name[MAX_PATH] = "python.exe"; // default process name
  char title[MAX_PATH] = "SPCMD Floating";    // default title
  char icon_path[MAX_PATH] = "";    // default icon path (use system default)
  char process_path[MAX_PATH] = ""; // process path for auto detection

  // 处理参数值
  const char *process_value = get_param_value(context, "process");
  if (process_value) {
    strncpy(process_name, process_value, MAX_PATH - 1);
    process_name[MAX_PATH - 1] = '\0';
  }

  const char *title_value = get_param_value(context, "title");
  if (title_value) {
    strncpy(title, title_value, MAX_PATH - 1);
    title[MAX_PATH - 1] = '\0';
  }

  const char *path_value = get_param_value(context, "path");
  if (path_value) {
    strncpy(process_path, path_value, MAX_PATH - 1);
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
  }

  const char *icon_value = get_param_value(context, "icon");
  if (icon_value) {
    strncpy(icon_path, icon_value, MAX_PATH - 1);
    icon_path[MAX_PATH - 1] = '\0';
  }

  // 处理menu参数
  MenuCommand *menu_commands = NULL;
  int menu_command_count = 0;

  // 获取所有menu参数值
  for (int i = 0; i < context->param_count; i++) {
    if (strcmp(context->params[i].name, "menu") == 0 &&
        context->params[i].value) {
      menu_command_count++;
    }
  }

  if (menu_command_count > 0) {
    menu_commands =
        (MenuCommand *)malloc(sizeof(MenuCommand) * menu_command_count);
    if (!menu_commands) {
      printf("Error: Memory allocation failed for menu commands\n");
      free_param_context(context);
      return;
    }

    // 解析menu参数
    int current_index = 0;
    for (int i = 0; i < context->param_count; i++) {
      if (strcmp(context->params[i].name, "menu") == 0 &&
          context->params[i].value) {
        char *menu_value = context->params[i].value;
        char *comma_pos = strchr(menu_value, ',');
        if (comma_pos) {
          // 分离菜单项名称和命令
          *comma_pos = '\0';
          strncpy(menu_commands[current_index].name, menu_value,
                  sizeof(menu_commands[current_index].name) - 1);
          menu_commands[current_index]
              .name[sizeof(menu_commands[current_index].name) - 1] = '\0';
          strncpy(menu_commands[current_index].command, comma_pos + 1,
                  sizeof(menu_commands[current_index].command) - 1);
          menu_commands[current_index]
              .command[sizeof(menu_commands[current_index].command) - 1] = '\0';
          current_index++;
        }
      }
    }
  }

  // 释放参数上下文
  free_param_context(context);

  printf("Starting floating icon for process monitoring...\n");
  printf("Process to monitor: %s\n", process_name);
  printf("Title: %s\n", title);
  if (strlen(icon_path) > 0) {
    printf("Icon path: %s\n", icon_path);
  }

  // 打印菜单命令
  if (menu_command_count > 0) {
    printf("Menu commands: %d\n", menu_command_count);
    for (int i = 0; i < menu_command_count; i++) {
      printf("  %s: %s\n", menu_commands[i].name, menu_commands[i].command);
    }
  }

  // 检查进程是否正在运行
  if (!is_process_running(process_name)) {
    printf(
        "Process '%s' is not running. Floating icon will not be displayed.\n",
        process_name);
    printf("Floating icon terminated.\n");
    if (menu_commands) {
      free(menu_commands);
    }
    return;
  }

  printf("Process is running. Creating floating icon...\n");

  // 初始化COM库
  CoInitialize(NULL);

  // 创建浮动图标数据结构
  FloatingIconData floatingData = {0};

  // 创建浮动图标
  if (!create_floating_icon(&floatingData, GetModuleHandle(NULL), title,
                            process_name, icon_path, menu_commands,
                            menu_command_count)) {
    printf("Error: Failed to create floating icon\n");
    CoUninitialize();
    if (menu_commands) {
      free(menu_commands);
    }
    return;
  }

  // 消息循环
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // 清理资源
  destroy_floating_icon(&floatingData);
  CoUninitialize();

  // 释放菜单命令内存
  if (menu_commands) {
    free(menu_commands);
  }

  printf("Floating icon terminated\n");
}

// 参数解析函数实现

// 创建参数上下文
ParamContext *create_param_context(ParamDefinition *param_defs, int count) {
  ParamContext *context = (ParamContext *)malloc(sizeof(ParamContext));
  if (!context)
    return NULL;

  context->params = (ParamDefinition *)malloc(sizeof(ParamDefinition) * count);
  if (!context->params) {
    free(context);
    return NULL;
  }

  context->param_count = count;

  // 复制参数定义
  for (int i = 0; i < count; i++) {
    context->params[i] = param_defs[i];
    context->params[i].value = NULL;
    context->params[i].has_been_set = FALSE;
  }

  return context;
}

// 解析命令行参数
BOOL parse_parameters(ParamContext *context, int argc, char *argv[],
                      int start_arg) {
  if (!context)
    return FALSE;

  for (int i = start_arg; i < argc; i++) {
    // 检查是否以 -- 开头的参数
    if (strncmp(argv[i], "--", 2) == 0) {
      char *param_name = argv[i] + 2;
      char *equals_pos = strchr(param_name, '=');

      if (equals_pos) {
        // 分离参数名和值
        *equals_pos = '\0'; // 将=替换为字符串结束符
        char *param_value = equals_pos + 1;

        // 查找对应的参数定义
        for (int j = 0; j < context->param_count; j++) {
          if (strcmp(context->params[j].name, param_name) == 0) {
            // 分配内存并复制参数值
            context->params[j].value = _strdup(param_value);
            context->params[j].has_been_set = TRUE;
            break;
          }
        }

        // 恢复=符号（可选，但为了不破坏原始参数）
        *equals_pos = '=';
      }
    }
  }

  return TRUE;
}

// 获取字符串参数
const char *get_param_value(ParamContext *context, const char *param_name) {
  if (!context || !param_name)
    return NULL;

  for (int i = 0; i < context->param_count; i++) {
    if (strcmp(context->params[i].name, param_name) == 0) {
      return context->params[i].value;
    }
  }

  return NULL;
}

// 获取整数参数
int get_param_int_value(ParamContext *context, const char *param_name,
                        int default_value) {
  const char *str_value = get_param_value(context, param_name);
  if (!str_value)
    return default_value;

  return atoi(str_value);
}

// 检查参数是否已设置
BOOL is_param_set(ParamContext *context, const char *param_name) {
  if (!context || !param_name)
    return FALSE;

  for (int i = 0; i < context->param_count; i++) {
    if (strcmp(context->params[i].name, param_name) == 0) {
      return context->params[i].has_been_set;
    }
  }

  return FALSE;
}

// 检查必填参数是否已设置
BOOL check_required_params(ParamContext *context) {
  if (!context)
    return FALSE;

  BOOL all_required_set = TRUE;

  for (int i = 0; i < context->param_count; i++) {
    if (context->params[i].is_required && !context->params[i].has_been_set) {
      printf("Error: Required parameter '--%s' is missing\n",
             context->params[i].name);
      all_required_set = FALSE;
    }
  }

  return all_required_set;
}

// 释放参数上下文
void free_param_context(ParamContext *context) {
  if (!context)
    return;

  if (context->params) {
    for (int i = 0; i < context->param_count; i++) {
      if (context->params[i].value) {
        free(context->params[i].value);
      }
    }
    free(context->params);
}

  free(context);
}

// TCP Socket通信命令
void cmd_ipc(int argc, char *argv[]) {
  char host[64] = "127.0.0.1";
  int port = 0;
  const char *value = "";

  // 解析参数
  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--host=", 7) == 0) {
       strncpy(host, argv[i] + 7, sizeof(host) - 1);
      host[sizeof(host) - 1] = '\0';
    } else if (strncmp(argv[i], "--port=", 7) == 0) {
      port = atoi(argv[i] + 7);
    } else if (strncmp(argv[i], "--value=", 8) == 0) {
      value = argv[i] + 8;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0) {
      printf("TCP Socket communication command usage:\n\n");
      printf("  spcmd ipc --host=IP --port=PORT --value=DATA  - Send data via TCP\n\n");
      printf("Parameters:\n");
      printf("  --host=IP       - Target IP address (default: 127.0.0.1)\n");
      printf("  --port=PORT     - Target port (required)\n");
      printf("  --value=DATA    - Data to send (required)\n\n");
      printf("Examples:\n");
      printf("  spcmd ipc --host=127.0.0.1 --port=9999 --value=hello\n");
      printf("  spcmd ipc --host=192.168.1.100 --port=8080 --value=test\n");
      return;
    }
  }

  // 检查必需参数
  if (port <= 0) {
    printf("Error: --port is required\n");
    return;
  }
  if (value == NULL || strlen(value) == 0) {
    printf("Error: --value is required\n");
    return;
  }

  // 初始化Winsock
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    printf("Error: Failed to initialize Winsock\n");
    return;
  }

  // 创建socket
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    printf("Error: Failed to create socket\n");
    WSACleanup();
    return;
  }

  // 连接服务器
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(host);
  server.sin_port = htons(port);

  if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
    printf("Error: Cannot connect to %s:%d, error: %d\n", host, port, WSAGetLastError());
    closesocket(sock);
    WSACleanup();
    return;
  }

  // 发送数据
  if (send(sock, value, strlen(value), 0) == SOCKET_ERROR) {
    printf("Error: Failed to send data, error: %d\n", WSAGetLastError());
  } else {
    printf("Message sent to %s:%d: %s\n", host, port, value);
  }

   closesocket(sock);
   WSACleanup();
}

// NTP时间同步命令
void cmd_timesync(int argc, char *argv[]) {
  // 默认NTP服务器
  char serverIP[64] = "time.windows.com";

  // 解析参数
  for (int i = 2; i < argc; i++) {
    if (strncmp(argv[i], "--server=", 9) == 0) {
      strncpy(serverIP, argv[i] + 9, sizeof(serverIP) - 1);
      serverIP[sizeof(serverIP) - 1] = '\0';
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0) {
      printf("Time synchronization command usage:\n");
      printf("  spcmd timesync                 - Sync time using default server\n");
      printf("  spcmd timesync --server=ip     - Sync time using specified NTP server\n\n");
      printf("Examples:\n");
      printf("  spcmd timesync\n");
      printf("  spcmd timesync --server=time.windows.com\n");
      printf("  spcmd timesync --server=192.168.1.1\n");
      return;
    }
  }

  printf("Synchronizing time from: %s\n", serverIP);

  // 创建UDP socket
  SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == INVALID_SOCKET) {
    printf("Error: Failed to create socket\n");
    return;
  }

  // 设置socket超时
  DWORD timeout = 5000; // 5秒超时
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

  // 解析服务器地址
  struct hostent *host = gethostbyname(serverIP);
  if (!host) {
    printf("Error: Cannot resolve server address: %s\n", serverIP);
    closesocket(sock);
    return;
  }

  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(123); // NTP端口
  memcpy(&serverAddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

  // 构建NTP请求包（48字节）
  char ntpRequest[48];
  memset(ntpRequest, 0, sizeof(ntpRequest));
  ntpRequest[0] = 0x1B; // LI=0, VN=3, Mode=3 (client)

  // 发送NTP请求
  int sendResult = sendto(sock, ntpRequest, sizeof(ntpRequest), 0,
                          (struct sockaddr *)&serverAddr, sizeof(serverAddr));
  if (sendResult == SOCKET_ERROR) {
    printf("Error: Failed to send NTP request\n");
    closesocket(sock);
    return;
  }

  // 接收NTP响应
  char ntpResponse[48];
  struct sockaddr_in fromAddr;
  int fromLen = sizeof(fromAddr);
  int recvResult = recvfrom(sock, ntpResponse, sizeof(ntpResponse), 0,
                            (struct sockaddr *)&fromAddr, &fromLen);
  closesocket(sock);

  if (recvResult == SOCKET_ERROR) {
    printf("Error: No response from NTP server (timeout or error)\n");
    return;
  }

  if (recvResult < 48) {
    printf("Error: Invalid NTP response\n");
    return;
  }

  // 解析NTP时间戳（第40-43字节是秒数部分，MSB first）
  // NTP时间从1900年1月1日开始，需要转换到1970年1月1日（Unix epoch）
  // 2208988800 = (70 * 365 + 17) * 24 * 60 * 60 (1900到1970的秒数差)
  unsigned int ntpSeconds = (unsigned char)ntpResponse[40];
  ntpSeconds = (ntpSeconds << 8) | (unsigned char)ntpResponse[41];
  ntpSeconds = (ntpSeconds << 8) | (unsigned char)ntpResponse[42];
  ntpSeconds = (ntpSeconds << 8) | (unsigned char)ntpResponse[43];

  // 转换为Unix时间戳
  time_t unixTime = ntpSeconds - 2208988800UL;

  // 显示当前时间和同步后的时间
  printf("Current system time: ");
  SYSTEMTIME st;
  GetLocalTime(&st);
  printf("%04d-%02d-%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay,
         st.wHour, st.wMinute, st.wSecond);

  struct tm *timeinfo = localtime(&unixTime);
  if (timeinfo) {
    printf("Target time (from %s): ", serverIP);
    printf("%04d-%02d-%02d %02d:%02d:%02d\n", timeinfo->tm_year + 1900,
           timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
           timeinfo->tm_min, timeinfo->tm_sec);

    // 设置系统时间
    SYSTEMTIME newTime;
    newTime.wYear = timeinfo->tm_year + 1900;
    newTime.wMonth = timeinfo->tm_mon + 1;
    newTime.wDayOfWeek = timeinfo->tm_wday;
    newTime.wDay = timeinfo->tm_mday;
    newTime.wHour = timeinfo->tm_hour;
    newTime.wMinute = timeinfo->tm_min;
    newTime.wSecond = timeinfo->tm_sec;
    newTime.wMilliseconds = 0;

    // 需要管理员权限来设置系统时间
    if (!IsRunAsAdmin()) {
      printf("\nNote: Setting system time requires administrator privileges.\n");
      printf("Please run as administrator to apply the time change.\n");
    } else {
      if (SetLocalTime(&newTime)) {
        printf("\nTime synchronized successfully!\n");
      } else {
        printf("\nError: Failed to set system time (error=%lu)\n", GetLastError());
      }
    }
  } else {
    printf("Error: Failed to parse NTP response\n");
  }
}
