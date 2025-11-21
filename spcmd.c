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

// 添加函数声明
void save_as_png(HBITMAP hBitmap, HDC hScreenDC, const char* filename, int quality);
void save_as_base64_data(const char* bitmap_data, DWORD data_size, const char* filename);

// 添加自定义弹窗函数声明


// 添加window命令函数声明
void cmd_window(int argc, char* argv[]); // 改为window命令

// 自定义弹窗结构体，用于传递参数
typedef struct {
    char* text;
    int fontSize;
    COLORREF bgColor;
    COLORREF textColor; // 添加文字颜色
    BOOL modal; // 是否为模态弹窗
    BOOL noDrag; // 是否禁止拖拽
} WindowParams;

// 函数声明
void show_help();
void handle_command(int argc, char* argv[]);
void cmd_screenshot(int argc, char* argv[]);
void cmd_shortcut(int argc, char* argv[]);
void cmd_autorun(int argc, char* argv[]);
void cmd_popup(int argc, char* argv[]);
void cmd_infobox(int argc, char* argv[]);
void cmd_infoboxtop(int argc, char* argv[]);
void cmd_qbox(int argc, char* argv[]);
void cmd_qboxtop(int argc, char* argv[]);
void cmd_exec2(int argc, char* argv[]);
void cmd_service(int argc, char* argv[]);
void cmd_task(int argc, char* argv[]);
void cmd_restart(int argc, char* argv[]);
void cmd_window(int argc, char* argv[]); // 改为window命令

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

void show_help() {
    printf("SPCMD - System Power Command Tool\n");
    printf("Usage: spcmd <command> [parameters]\n\n");
    printf("Supported commands:\n");
    printf("  screenshot            - Capture screen screenshot\n");
    printf("  shortcut              - Create desktop shortcut\n");
    printf("  autorun               - Configure auto-start\n");
    printf("  popup                 - Display information popup\n");
    printf("  infobox               - Display simple message box\n");
    printf("  infoboxtop            - Display top-most message box\n");
    printf("  qbox                  - Display question dialog box\n");
    printf("  qboxtop               - Display top-most question dialog box\n");
    printf("  window                - Display custom window with advanced features\n");
    printf("  exec2                 - Execute application with working folder\n");
    printf("  service               - Create system service\n");
    printf("  task                  - Create scheduled task\n");
    printf("  restart               - Restart specified process\n");
    printf("\nExamples:\n");
    printf("  spcmd screenshot\n");
    printf("  spcmd shortcut /target:C:\\Windows\\notepad.exe\n");
    printf("  spcmd infobox \"This is a message box!\" \"Message\"\n");
    printf("  spcmd infoboxtop \"This is a top-most message box!\" \"Top-Most Message\"\n");
    printf("  spcmd qbox \"Do you want to run calc?\" \"Question\" \"calc.exe\"\n");
    printf("  spcmd window /text:\"Hello World\" /title:\"Custom Window\"\n");
    printf("  spcmd exec2 show \"f:\\winnt\\system32\" \"f:\\winnt\\system32\\calc.exe\"\n");
    printf("  spcmd exec2 hide c:\\temp \"c:\\temp\\wul.exe\" /savelangfile\n");
    printf("\nType spcmd <command> /help for specific command help\n");
}

void handle_command(int argc, char* argv[]) {
    if (strcmp(argv[1], "screenshot") == 0) {
        cmd_screenshot(argc, argv);
    } else if (strcmp(argv[1], "shortcut") == 0) {
        cmd_shortcut(argc, argv);
    } else if (strcmp(argv[1], "autorun") == 0) {
        cmd_autorun(argc, argv);
    } else if (strcmp(argv[1], "popup") == 0) {
        cmd_popup(argc, argv);
    } else if (strcmp(argv[1], "infobox") == 0) {
        cmd_infobox(argc, argv);
    } else if (strcmp(argv[1], "infoboxtop") == 0) {
        cmd_infoboxtop(argc, argv);
    } else if (strcmp(argv[1], "qbox") == 0) {
        cmd_qbox(argc, argv);
    } else if (strcmp(argv[1], "qboxtop") == 0) {
        cmd_qboxtop(argc, argv);
    } else if (strcmp(argv[1], "window") == 0) { // 改为window命令
        cmd_window(argc, argv);
    } else if (strcmp(argv[1], "exec2") == 0) {
        cmd_exec2(argc, argv);
    } else if (strcmp(argv[1], "service") == 0) {
        cmd_service(argc, argv);
    } else if (strcmp(argv[1], "task") == 0) {
        cmd_task(argc, argv);
    } else if (strcmp(argv[1], "restart") == 0) {
        cmd_restart(argc, argv);
    } else {
        printf("Unknown command: %s\n", argv[1]);
        show_help();
    }
}

void cmd_screenshot(int argc, char* argv[]) {
    printf("Executing screenshot function...\n");
    
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("Screenshot command help:\n");
        printf("  spcmd screenshot [/save:path] [/fullscreen] [/active] [/format:png|bmp] [/base64:file] [/quality:value]\n\n");
        printf("Parameter description:\n");
        printf("  /save:path       - Save screenshot to specified path, default to current directory\n");
        printf("  /fullscreen      - Capture full screen (default)\n");
        printf("  /active          - Capture active window\n");
        printf("  /format:png|bmp  - Save format, default is bmp\n");
        printf("  /base64:file     - Save as Base64 encoded data to specified file\n");
        printf("  /quality:value   - Image quality for PNG (1-100), default is 100\n\n");
        printf("Examples:\n");
        printf("  spcmd screenshot\n");
        printf("  spcmd screenshot /save:C:\\screenshots\\screen.png\n");
        printf("  spcmd screenshot /active\n");
        printf("  spcmd screenshot /format:png\n");
        printf("  spcmd screenshot /base64:screenshot.b64\n");
        printf("  spcmd screenshot /format:png /quality:80\n");
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
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    if (hBitmap == NULL) {
        printf("Error: Unable to create bitmap\n");
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return;
    }
    
    // Select bitmap to memory DC
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
    
    // Check if capturing active window
    BOOL captureActiveWindow = FALSE;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "/active") == 0) {
            captureActiveWindow = TRUE;
            break;
        }
    }
    
    if (captureActiveWindow) {
        // Capture active window
        HWND hwnd = GetForegroundWindow();
        if (hwnd != NULL) {
            RECT rc;
            GetWindowRect(hwnd, &rc);
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;
            
            // Resize bitmap to active window size
            DeleteObject(hBitmap);
            hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
            SelectObject(hMemoryDC, hBitmap);
            
            // Copy active window to memory DC
            BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, rc.left, rc.top, SRCCOPY);
        }
    } else {
        // Capture full screen
        BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);
    }
    
    // Restore old bitmap
    SelectObject(hMemoryDC, hOldBitmap);
    
    // Generate filename and format
    char filename[MAX_PATH] = "screenshot.bmp";
    char format[10] = "bmp";  // default format
    char base64_filename[MAX_PATH] = {0};  // for base64 encoded data
    BOOL save_as_base64 = FALSE;
    int quality = 100; // default quality
    
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "/save:", 6) == 0) {
            strncpy(filename, argv[i] + 6, MAX_PATH - 1);
            filename[MAX_PATH - 1] = '\0';
        } else if (strncmp(argv[i], "/format:", 8) == 0) {
            strncpy(format, argv[i] + 8, sizeof(format) - 1);
            format[sizeof(format) - 1] = '\0';
        } else if (strncmp(argv[i], "/base64:", 8) == 0) {
            strncpy(base64_filename, argv[i] + 8, MAX_PATH - 1);
            base64_filename[MAX_PATH - 1] = '\0';
            save_as_base64 = TRUE;
        } else if (strncmp(argv[i], "/quality:", 9) == 0) {
            quality = atoi(argv[i] + 9);
            // Ensure quality is between 1 and 100
            if (quality < 1) quality = 1;
            if (quality > 100) quality = 100;
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
        
        DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
        
        HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        char* lpbitmap = (char*)GlobalLock(hDIB);
        
        GetDIBits(hScreenDC, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        
        // Save as base64 encoded data
        save_as_base64_data(lpbitmap, dwBmpSize, base64_filename);
        
        // Clean up resources
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
    } else {
        // Save bitmap in the specified format
        if (strcmp(format, "png") == 0 || strcmp(format, "PNG") == 0) {
            // Ensure filename has .png extension
            if (strstr(filename, ".bmp")) {
                // Replace .bmp with .png
                char* dot = strrchr(filename, '.');
                if (dot) {
                    strcpy(dot, ".png");
                }
            } else if (!strstr(filename, ".png")) {
                // Add .png extension if no extension
                strcat(filename, ".png");
            }
            save_as_png(hBitmap, hScreenDC, filename, quality);
        } else {
            // Save as BMP (default)
            // Save bitmap as BMP file
            BITMAP bmp;
            GetObject(hBitmap, sizeof(BITMAP), &bmp);
            
            BITMAPINFOHEADER bi = {0};
            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = bmp.bmWidth;
            bi.biHeight = bmp.bmHeight;
            bi.biPlanes = 1;
            bi.biBitCount = 24;
            bi.biCompression = BI_RGB;
            
            DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
            DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            
            BITMAPFILEHEADER bmfHeader = {0};
            bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
            bmfHeader.bfSize = dwSizeofDIB;
            bmfHeader.bfType = 0x4D42; // BM
            
            HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
            char* lpbitmap = (char*)GlobalLock(hDIB);
            
            GetDIBits(hScreenDC, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
            
            HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD dwBytesWritten;
                WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
                WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
                WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
                CloseHandle(hFile);
                printf("Screenshot saved to: %s\n", filename);
            } else {
                printf("Error: Unable to save screenshot to %s\n", filename);
            }
            
            // Clean up resources
            GlobalUnlock(hDIB);
            GlobalFree(hDIB);
        }
    }
    
    // Clean up resources
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}

void cmd_shortcut(int argc, char* argv[]) {
    printf("Creating shortcut...\n");
    
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("Shortcut command help:\n");
        printf("  spcmd shortcut /target:path [/name:name] [/desc:description] [/icon:iconpath] [/workdir:dir]\n\n");
        printf("Parameter description:\n");
        printf("  /target:path       - Target program path (required)\n");
        printf("  /name:name         - Shortcut name, default is program name\n");
        printf("  /desc:description  - Shortcut description\n");
        printf("  /icon:iconpath     - Icon path\n");
        printf("  /workdir:dir       - Working directory\n\n");
        printf("Examples:\n");
        printf("  spcmd shortcut /target:C:\\Windows\\notepad.exe\n");
        printf("  spcmd shortcut /target:C:\\Windows\\notepad.exe /name:Notepad /desc:Open Notepad program\n");
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
        if (strncmp(argv[i], "/target:", 8) == 0) {
            strncpy(targetPath, argv[i] + 8, MAX_PATH - 1);
            targetPath[MAX_PATH - 1] = '\0';
            hasTarget = TRUE;
        } else if (strncmp(argv[i], "/name:", 6) == 0) {
            strncpy(shortcutName, argv[i] + 6, MAX_PATH - 1);
            shortcutName[MAX_PATH - 1] = '\0';
        } else if (strncmp(argv[i], "/desc:", 6) == 0) {
            strncpy(description, argv[i] + 6, MAX_PATH - 1);
            description[MAX_PATH - 1] = '\0';
        } else if (strncmp(argv[i], "/icon:", 6) == 0) {
            strncpy(iconPath, argv[i] + 6, MAX_PATH - 1);
            iconPath[MAX_PATH - 1] = '\0';
        } else if (strncmp(argv[i], "/workdir:", 9) == 0) {
            strncpy(workingDir, argv[i] + 9, MAX_PATH - 1);
            workingDir[MAX_PATH - 1] = '\0';
        }
    }
    
    // Check required parameters
    if (!hasTarget) {
        printf("Error: Target program path must be specified (/target:path)\n");
        printf("Use spcmd shortcut /help for help\n");
        return;
    }
    
    // If shortcut name is not specified, use target filename
    if (strlen(shortcutName) == 0) {
        char* fileName = strrchr(targetPath, '\\');
        if (fileName != NULL) {
            strncpy(shortcutName, fileName + 1, MAX_PATH - 1);
        } else {
            strncpy(shortcutName, targetPath, MAX_PATH - 1);
        }
        
        // Remove extension
        char* dot = strrchr(shortcutName, '.');
        if (dot != NULL) {
            *dot = '\0';
        }
    }
    
    // Add .lnk extension
    char finalName[MAX_PATH];
    snprintf(finalName, MAX_PATH, "%s.lnk", shortcutName);
    
    // Get desktop path
    char desktopPath[MAX_PATH];
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, desktopPath))) {
        printf("Error: Unable to get desktop path\n");
        return;
    }
    
    // Construct shortcut full path
    char shortcutPath[MAX_PATH];
    snprintf(shortcutPath, MAX_PATH, "%s\\%s", desktopPath, finalName);
    
    // Create shortcut
    CoInitialize(NULL);
    
    IShellLinkA* pShellLink = NULL;
    HRESULT hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkA, (LPVOID*)&pShellLink);
    
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
            char* lastSlash = strrchr(targetDir, '\\');
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
        }
        
        // Save shortcut
        IPersistFile* pPersistFile = NULL;
        hres = IShellLinkA_QueryInterface(pShellLink, &IID_IPersistFile, (LPVOID*)&pPersistFile);
        
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
}

void cmd_autorun(int argc, char* argv[]) {
    (void)argc; // 消除未使用参数警告
    (void)argv; // 消除未使用参数警告
    printf("配置开机自启...\n");
    // TODO: 实现开机自启功能
    printf("[未实现] 开机自启配置功能将在后续版本中实现\n");
}

void cmd_popup(int argc, char* argv[]) {
    printf("Displaying popup...\n");
    
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("Popup command help:\n");
        printf("  spcmd popup /text:message [/title:title] [/type:type] [/timeout:seconds]\n\n");
        printf("Parameter description:\n");
        printf("  /text:message     - Popup display text (required)\n");
        printf("  /title:title      - Popup title, default is \"Information\"\n");
        printf("  /type:type        - Popup type: info(information), warning, error\n");
        printf("  /timeout:seconds  - Auto-close time (seconds), 0 means no auto-close\n\n");
        printf("Examples:\n");
        printf("  spcmd popup /text:\"Hello World\"\n");
        printf("  spcmd popup /text:\"Operation completed\" /title:\"Prompt\" /type:info\n");
        return;
    }
    
    // Parse parameters
    char message[MAX_PATH] = "Information";
    char title[MAX_PATH] = "Information";
    int type = MB_OK | MB_ICONINFORMATION;
    int timeout = 0;
    
    BOOL hasText = FALSE;
    
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "/text:", 6) == 0) {
            strncpy(message, argv[i] + 6, MAX_PATH - 1);
            message[MAX_PATH - 1] = '\0';
            hasText = TRUE;
        } else if (strncmp(argv[i], "/title:", 7) == 0) {
            strncpy(title, argv[i] + 7, MAX_PATH - 1);
            title[MAX_PATH - 1] = '\0';
        } else if (strncmp(argv[i], "/type:", 6) == 0) {
            char typeStr[MAX_PATH];
            strncpy(typeStr, argv[i] + 6, MAX_PATH - 1);
            typeStr[MAX_PATH - 1] = '\0';
            
            if (strcmp(typeStr, "warning") == 0) {
                type = MB_OK | MB_ICONWARNING;
            } else if (strcmp(typeStr, "error") == 0) {
                type = MB_OK | MB_ICONERROR;
            } else {
                type = MB_OK | MB_ICONINFORMATION;
            }
        } else if (strncmp(argv[i], "/timeout:", 9) == 0) {
            timeout = atoi(argv[i] + 9);
        }
    }
    
    // Check required parameters
    if (!hasText) {
        printf("Error: Popup text must be specified (/text:message)\n");
        printf("Use spcmd popup /help for help\n");
        return;
    }
    
    // Display popup
    if (timeout > 0) {
        // Create a thread to display popup and close after specified time
        MessageBox(NULL, message, title, type);
        Sleep(timeout * 1000);
    } else {
        // Directly display popup
        MessageBox(NULL, message, title, type);
    }
    
    printf("Popup displayed\n");
}

void cmd_service(int argc, char* argv[]) {
    (void)argc; // 消除未使用参数警告
    (void)argv; // 消除未使用参数警告
    printf("创建系统服务...\n");
    // TODO: 实现创建系统服务功能
    printf("[未实现] 系统服务创建功能将在后续版本中实现\n");
}

void cmd_task(int argc, char* argv[]) {
    (void)argc; // 消除未使用参数警告
    (void)argv; // 消除未使用参数警告
    printf("创建计划任务...\n");
    // TODO: 实现创建计划任务功能
    printf("[未实现] 计划任务创建功能将在后续版本中实现\n");
}

void cmd_restart(int argc, char* argv[]) {
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("Restart command help:\n");
        printf("  spcmd restart /path:process_path\n\n");
        printf("Parameter description:\n");
        printf("  /path:process_path  - Path to the process executable to restart (required)\n\n");
        printf("Examples:\n");
        printf("  spcmd restart /path:\"C:\\Windows\\notepad.exe\"\n");
        printf("  spcmd restart /path:\"C:\\Program Files\\MyApp\\myapp.exe\"\n");
        return;
    }
    
    // Parse parameters
    char processPath[MAX_PATH] = {0};
    BOOL hasPath = FALSE;
    
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "/path:", 6) == 0) {
            strncpy(processPath, argv[i] + 6, MAX_PATH - 1);
            processPath[MAX_PATH - 1] = '\0';
            hasPath = TRUE;
        }
    }
    
    // Check required parameters
    if (!hasPath) {
        printf("Error: Process path must be specified (/path:process_path)\n");
        printf("Use spcmd restart /help for help\n");
        return;
    }
    
    // Check if file exists
    if (GetFileAttributesA(processPath) == INVALID_FILE_ATTRIBUTES) {
        printf("Error: Process file does not exist: %s\n", processPath);
        return;
    }
    
    printf("Restarting process: %s\n", processPath);
    
    // Get filename from path
    char* fileName = strrchr(processPath, '\\');
    if (fileName == NULL) {
        fileName = processPath;
    } else {
        fileName++; // Skip the backslash
    }
    
    // Remove extension for process name comparison
    char processName[MAX_PATH];
    strncpy(processName, fileName, MAX_PATH - 1);
    processName[MAX_PATH - 1] = '\0';
    char* dot = strrchr(processName, '.');
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
                char* procName = pe32.szExeFile;
                char* procDot = strrchr(procName, '.');
                if (procDot != NULL) {
                    *procDot = '\0';
                }
                
                if (_stricmp(procName, processName) == 0) {
                    // Found matching process, kill it
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                    if (hProcess != NULL) {
                        if (TerminateProcess(hProcess, 0)) {
                            printf("Killed process: %s (PID: %lu)\n", pe32.szExeFile, pe32.th32ProcessID);
                        } else {
                            printf("Failed to kill process: %s (PID: %lu)\n", pe32.szExeFile, pe32.th32ProcessID);
                        }
                        CloseHandle(hProcess);
                    } else {
                        printf("Failed to open process: %s (PID: %lu)\n", pe32.szExeFile, pe32.th32ProcessID);
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
    
    // Create the process
    if (CreateProcessA(NULL, processPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("Started process: %s (PID: %lu)\n", processPath, pi.dwProcessId);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("Failed to start process: %s\n", processPath);
        printf("Error code: %lu\n", GetLastError());
    }
}

void cmd_exec2(int argc, char* argv[]) {
    // exec2 [show/hide/min/max] [working folder] [application + command-line]
    // Similar to exec command, but also provide another parameter, [working folder], 
    // that specifies the default working folder for the application that you run.
    
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("exec2 command help:\n");
        printf("  spcmd exec2 [show/hide/min/max] [working folder] [application + command-line]\n\n");
        printf("Parameter description:\n");
        printf("  show/hide/min/max  - Window state for the application\n");
        printf("  working folder     - Working directory for the application\n");
        printf("  application        - Application to run with optional command-line arguments\n\n");
        printf("Examples:\n");
        printf("  spcmd exec2 show \"f:\\winnt\\system32\" \"f:\\winnt\\system32\\calc.exe\"\n");
        printf("  spcmd exec2 hide c:\\temp \"c:\\temp\\wul.exe\" /savelangfile\n");
        return;
    }
    
    // Check if required parameters are provided
    if (argc < 5) {
        printf("Error: Window state, working folder, and application are required\n");
        printf("Usage: spcmd exec2 [show/hide/min/max] [working folder] [application + command-line]\n");
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
    char* workingFolder = argv[3];
    char* application = argv[4];
    
    // Build full command line
    char commandLine[MAX_PATH * 2] = {0};
    strncpy(commandLine, application, MAX_PATH - 1);
    
    // Append additional arguments if any
    for (int i = 5; i < argc; i++) {
        strncat(commandLine, " ", sizeof(commandLine) - strlen(commandLine) - 1);
        strncat(commandLine, argv[i], sizeof(commandLine) - strlen(commandLine) - 1);
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
    if (CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, workingFolder, &si, &pi)) {
        printf("Application '%s' started successfully in folder '%s'\n", application, workingFolder);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("Error: Failed to start application '%s' in folder '%s'\n", application, workingFolder);
        printf("Error code: %lu\n", GetLastError());
    }
}

void cmd_infobox(int argc, char* argv[]) {
    // infobox [message text] [title]
    // Displays a simple message box on the screen.
    
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("infobox command help:\n");
        printf("  spcmd infobox [message text] [title]\n\n");
        printf("Parameter description:\n");
        printf("  message text  - Message to display in the box\n");
        printf("  title         - Title of the message box\n\n");
        printf("Examples:\n");
        printf("  spcmd infobox \"This is a message box!\" \"Message\"\n");
        return;
    }
    
    // Check if required parameters are provided
    if (argc < 4) {
        printf("Error: Message text and title are required\n");
        printf("Usage: spcmd infobox [message text] [title]\n");
        return;
    }
    
    // Display message box
    MessageBoxA(NULL, argv[2], argv[3], MB_OK | MB_ICONINFORMATION);
    printf("Message box displayed\n");
}

void cmd_infoboxtop(int argc, char* argv[]) {
    // infoboxtop [message text] [title]
    // Similar to infobox, but displays the message-box as top-most window.
    
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("infoboxtop command help:\n");
        printf("  spcmd infoboxtop [message text] [title]\n\n");
        printf("Parameter description:\n");
        printf("  message text  - Message to display in the box\n");
        printf("  title         - Title of the message box\n\n");
        printf("Examples:\n");
        printf("  spcmd infoboxtop \"This is a top-most message box!\" \"Top-Most Message\"\n");
        return;
    }
    
    // Check if required parameters are provided
    if (argc < 4) {
        printf("Error: Message text and title are required\n");
        printf("Usage: spcmd infoboxtop [message text] [title]\n");
        return;
    }
    
    // Make the message box top-most
    HWND hwnd = GetForegroundWindow();
    if (hwnd != NULL) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    
    // Display top-most message box
    MessageBoxA(NULL, argv[2], argv[3], MB_OK | MB_ICONINFORMATION);
    printf("Top-most message box displayed\n");
}

void cmd_qbox(int argc, char* argv[]) {
    // qbox [message text] [title] [program to run]
    // Displays a question dialog-box on the screen. If the user answers "Yes", run a program
    
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("qbox command help:\n");
        printf("  spcmd qbox [message text] [title] [program to run]\n\n");
        printf("Parameter description:\n");
        printf("  message text  - Question to display in the box\n");
        printf("  title         - Title of the question box\n");
        printf("  program       - Program to run if user answers Yes\n\n");
        printf("Examples:\n");
        printf("  spcmd qbox \"Do you want to run the calculator?\" \"Question\" \"calc.exe\"\n");
        return;
    }
    
    // Check if required parameters are provided
    if (argc < 5) {
        printf("Error: Message text, title, and program are required\n");
        printf("Usage: spcmd qbox [message text] [title] [program to run]\n");
        return;
    }
    
    // Display question box
    int result = MessageBoxA(NULL, argv[2], argv[3], MB_YESNO | MB_ICONQUESTION);
    
    if (result == IDYES) {
        // Run the specified program
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        // Start the program
        if (CreateProcessA(NULL, argv[4], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
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

void cmd_qboxtop(int argc, char* argv[]) {
    // qboxtop [message text] [title] [program to run]
    // Similar to qbox, but displays the message-box as top-most window.
    
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("qboxtop command help:\n");
        printf("  spcmd qboxtop [message text] [title] [program to run]\n\n");
        printf("Parameter description:\n");
        printf("  message text  - Question to display in the box\n");
        printf("  title         - Title of the question box\n");
        printf("  program       - Program to run if user answers Yes\n\n");
        printf("Examples:\n");
        printf("  spcmd qboxtop \"Do you want to run the calculator?\" \"Question\" \"calc.exe\"\n");
        return;
    }
    
    // Check if required parameters are provided
    if (argc < 5) {
        printf("Error: Message text, title, and program are required\n");
        printf("Usage: spcmd qboxtop [message text] [title] [program to run]\n");
        return;
    }
    
    // Display top-most question box
    HWND hwnd = GetForegroundWindow();
    if (hwnd != NULL) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    
    int result = MessageBoxA(NULL, argv[2], argv[3], MB_YESNO | MB_ICONQUESTION);
    
    if (result == IDYES) {
        // Run the specified program
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        // Start the program
        if (CreateProcessA(NULL, argv[4], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
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
void save_as_png(HBITMAP hBitmap, HDC hScreenDC, const char* filename, int quality) {
    // 如果质量设置低于100，则调整图像尺寸以减小文件大小
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    
    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    
    // 根据质量设置调整尺寸
    if (quality < 100) {
        double scale = quality / 100.0;
        width = (int)(width * scale);
        height = (int)(height * scale);
        
        // 创建缩放后的bitmap
        HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
        HBITMAP hScaledBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hScaledBitmap);
        
        // 缩放图像
        SetStretchBltMode(hMemoryDC, HALFTONE);
        StretchBlt(hMemoryDC, 0, 0, width, height, 
                  hScreenDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
        
        // 从缩放后的bitmap获取数据
        BITMAPINFOHEADER bi = {0};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = height;
        bi.biPlanes = 1;
        bi.biBitCount = 24;
        bi.biCompression = BI_RGB;
        
        DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
        
        HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        char* lpbitmap = (char*)GlobalLock(hDIB);
        
        GetDIBits(hMemoryDC, hScaledBitmap, 0, (UINT)height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        
        // 保存为BMP格式（带.png扩展名）
        BITMAPFILEHEADER bmfHeader = {0};
        DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
        bmfHeader.bfSize = dwSizeofDIB;
        bmfHeader.bfType = 0x4D42; // BM
        
        HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwBytesWritten;
            WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
            WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
            WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
            CloseHandle(hFile);
            printf("Screenshot saved to: %s (scaled to %dx%d)\n", filename, width, height);
        } else {
            printf("Error: Unable to save screenshot to %s\n", filename);
        }
        
        // Clean up resources
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hScaledBitmap);
        DeleteDC(hMemoryDC);
    } else {
        // 保存为原始尺寸的BMP格式（带.png扩展名）
        printf("Note: Full PNG format support requires external libraries.\n");
        printf("Saving as BMP with .png extension: %s\n", filename);
        
        BITMAPINFOHEADER bi = {0};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmp.bmWidth;
        bi.biHeight = bmp.bmHeight;
        bi.biPlanes = 1;
        bi.biBitCount = 24;
        bi.biCompression = BI_RGB;
        
        DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
        DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        
        BITMAPFILEHEADER bmfHeader = {0};
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
        bmfHeader.bfSize = dwSizeofDIB;
        bmfHeader.bfType = 0x4D42; // BM
        
        HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
        char* lpbitmap = (char*)GlobalLock(hDIB);
        
        GetDIBits(hScreenDC, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        
        HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwBytesWritten;
            WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
            WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
            WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
            CloseHandle(hFile);
            printf("Screenshot saved to: %s (as BMP data with PNG extension)\n", filename);
        } else {
            printf("Error: Unable to save screenshot to %s\n", filename);
        }
        
        // Clean up resources
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
    }
}

// 添加Base64编码函数
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    
    char* encoded_data = malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;
    
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

void save_as_base64_data(const char* bitmap_data, DWORD data_size, const char* filename) {
    size_t encoded_length;
    char* base64_data = base64_encode((const unsigned char*)bitmap_data, data_size, &encoded_length);
    
    if (base64_data == NULL) {
        printf("Error: Unable to encode data as Base64\n");
        return;
    }
    
    // Save base64 encoded data to file
    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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

// 函数声明
void show_help();
void handle_command(int argc, char* argv[]);
void cmd_screenshot(int argc, char* argv[]);
void cmd_shortcut(int argc, char* argv[]);
void cmd_autorun(int argc, char* argv[]);
void cmd_popup(int argc, char* argv[]);
void cmd_infobox(int argc, char* argv[]);
void cmd_infoboxtop(int argc, char* argv[]);
void cmd_qbox(int argc, char* argv[]);
void cmd_qboxtop(int argc, char* argv[]);
void cmd_exec2(int argc, char* argv[]);
void cmd_service(int argc, char* argv[]);
void cmd_task(int argc, char* argv[]);
void cmd_restart(int argc, char* argv[]);
void cmd_window(int argc, char* argv[]); // 改为window命令


// 添加window命令函数声明
void cmd_window(int argc, char* argv[]); // 改为window命令



// 枚举窗口回调函数
BOOL CALLBACK EnumWindowsProcDisable(HWND hwnd, LPARAM lParam) {
    if (IsWindowVisible(hwnd)) {
        EnableWindow(hwnd, FALSE);
    }
    return TRUE;
}

BOOL CALLBACK EnumWindowsProcEnable(HWND hwnd, LPARAM lParam) {
    EnableWindow(hwnd, TRUE);
    return TRUE;
}

// 根据颜色名称获取RGB值
COLORREF GetColorByName(const char* colorName) {
    if (_stricmp(colorName, "white") == 0) return RGB(255, 255, 255);
    if (_stricmp(colorName, "black") == 0) return RGB(0, 0, 0);
    if (_stricmp(colorName, "red") == 0) return RGB(255, 0, 0);
    if (_stricmp(colorName, "green") == 0) return RGB(0, 255, 0);
    if (_stricmp(colorName, "blue") == 0) return RGB(0, 0, 255);
    if (_stricmp(colorName, "yellow") == 0) return RGB(255, 255, 0);
    if (_stricmp(colorName, "cyan") == 0) return RGB(0, 255, 255);
    if (_stricmp(colorName, "magenta") == 0) return RGB(255, 0, 255);
    if (_stricmp(colorName, "gray") == 0) return RGB(128, 128, 128);
    if (_stricmp(colorName, "lightgray") == 0) return RGB(211, 211, 211);
    if (_stricmp(colorName, "darkgray") == 0) return RGB(169, 169, 169);
    if (_stricmp(colorName, "orange") == 0) return RGB(255, 165, 0);
    if (_stricmp(colorName, "purple") == 0) return RGB(128, 0, 128);
    if (_stricmp(colorName, "brown") == 0) return RGB(165, 42, 42);
    if (_stricmp(colorName, "pink") == 0) return RGB(255, 192, 203);
    if (_stricmp(colorName, "lime") == 0) return RGB(0, 255, 0);
    if (_stricmp(colorName, "navy") == 0) return RGB(0, 0, 128);
    if (_stricmp(colorName, "teal") == 0) return RGB(0, 128, 128);
    if (_stricmp(colorName, "olive") == 0) return RGB(128, 128, 0);
    
    // 默认返回白色
    return RGB(255, 255, 255);
}

// 自定义弹窗窗口过程函数
LRESULT CALLBACK WindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static WindowParams* params = NULL;
    static HFONT hFont = NULL;
    static HWND hButton = NULL;
    
    switch (msg) {
        case WM_CREATE: {
            // 获取传递的参数
            CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
            params = (WindowParams*)pcs->lpCreateParams;
            
            // 初始化文字颜色为黑色（如果未指定）
            if (params->textColor == 0) {
                params->textColor = RGB(0, 0, 0); // 默认黑色
            }
            
            // 创建字体，使用系统默认字体以确保中文支持
            hFont = CreateFontA(
                params->fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Microsoft Sans Serif");
            
            // 如果上面的字体不可用，使用系统默认GUI字体
            if (!hFont) {
                hFont = GetStockObject(DEFAULT_GUI_FONT);
            }
            
            // 创建确认按钮，使用宽字符确保中文支持
            wchar_t buttonText[] = L"确定";
            hButton = CreateWindowW(
                L"BUTTON", buttonText,
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                0, 0, 80, 30,
                hwnd, (HMENU)1, pcs->hInstance, NULL);
        
            // 设置按钮字体
            if (hFont) {
                SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
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
            
            // 直接使用原始文本，不需要额外处理换行符
            char* displayText = params->text;
            
            // 计算文本绘制区域以实现真正的垂直居中
            RECT textRect = rect;
            // 为按钮留出空间（按钮高度30像素 + 边距10像素）
            textRect.bottom -= 40;
            
            // 计算文本实际占用的矩形区域
            RECT calcRect = textRect;
            DrawTextA(hdc, displayText, -1, &calcRect, DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL | DT_CALCRECT);
            
            // 调整垂直位置以实现居中
            int textHeight = calcRect.bottom - calcRect.top;
            int availableHeight = textRect.bottom - textRect.top;
            if (textHeight < availableHeight) {
                int offset = (availableHeight - textHeight) / 2;
                textRect.top += offset;
                textRect.bottom = textRect.top + textHeight;
            }
            
            // 绘制文本
            DrawTextA(hdc, displayText, -1, &textRect, DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL);
            
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
            int buttonX = (rcClient.right - buttonWidth) / 2; // 水平居中
            int buttonY = rcClient.bottom - buttonHeight - 10; // 底部留10像素边距
            
            // 移动按钮到计算出的位置
            if (hButton) {
                MoveWindow(hButton, buttonX, buttonY, buttonWidth, buttonHeight, TRUE);
            }
            break;
        }
        
        case WM_NCHITTEST: {
            // 如果禁止拖拽，阻止窗口移动
            if (params && params->noDrag) {
                LRESULT hit = DefWindowProc(hwnd, msg, wParam, lParam);
                if (hit == HTCAPTION) {
                    return HTCLIENT;
                }
                return hit;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        
        case WM_DESTROY: {
            // 清理字体资源
            if (hFont) {
                DeleteObject(hFont);
                hFont = NULL;
            }
            PostQuitMessage(0);
            break;
        }
        
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void cmd_window(int argc, char* argv[]) { // 改为window命令
    // Check if help is needed
    if (argc > 2 && (strcmp(argv[2], "/help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf("Window command help:\n");
        printf("  spcmd window /text:message [/title:title] [/width:width] [/height:height] [/fontsize:size] [/bgcolor:color] [/textcolor:color] [/modal] [/nodrag]\n\n");
        printf("Parameter description:\n");
        printf("  /text:message     - Window display text (required)\n");
        printf("  /title:title      - Window title, default is \"Custom Window\"\n");
        printf("  /width:width      - Window width in pixels, default is 600\n");
        printf("  /height:height    - Window height in pixels, default is 400\n");
        printf("  /fontsize:size    - Font size, default is 18\n");
        printf("  /bgcolor:color    - Background color as name (white,black,red,green,blue,yellow,cyan,magenta,gray,orange,purple,pink,lightblue,lightgreen,lightgray) or RGB values (r,g,b), default is white\n");
        printf("  /textcolor:color  - Text color as name or RGB values, default is black\n");
        printf("  /modal            - Make window modal (blocks other windows until closed)\n");
        printf("  /nodrag           - Disable window dragging\n\n");
        printf("Examples:\n");
        printf("  spcmd window /text:\"Hello World\"\n");
        printf("  spcmd window /text:\"Line 1\\nLine 2\\nLine 3\" /title:\"Multi-line Text\" /width:500 /height:300\n");
        printf("  spcmd window /text:\"Red background window\" /bgcolor:red /fontsize:20\n");
        printf("  spcmd window /text:\"Blue text on yellow background\" /bgcolor:yellow /textcolor:blue\n");
        printf("  spcmd window /text:\"Modal window\" /modal\n");
        return;
    }
    
    // Parse parameters
    char* message = NULL;
    char title[256] = "Custom Window";
    int width = 600;
    int height = 400;
    int fontSize = 18;
    COLORREF bgColor = RGB(255, 255, 255); // 默认白色背景
    COLORREF textColor = RGB(0, 0, 0); // 默认黑色文字
    BOOL modal = FALSE;
    BOOL noDrag = FALSE;
    BOOL hasText = FALSE;
    
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "/text:", 6) == 0) {
            message = argv[i] + 6;
            hasText = TRUE;
        } else if (strncmp(argv[i], "/title:", 7) == 0) {
            strncpy(title, argv[i] + 7, sizeof(title) - 1);
            title[sizeof(title) - 1] = '\0';
        } else if (strncmp(argv[i], "/width:", 7) == 0) {
            width = atoi(argv[i] + 7);
        } else if (strncmp(argv[i], "/height:", 8) == 0) {
            height = atoi(argv[i] + 8);
        } else if (strncmp(argv[i], "/fontsize:", 10) == 0) {
            fontSize = atoi(argv[i] + 10);
        } else if (strncmp(argv[i], "/bgcolor:", 9) == 0) {
            char colorStr[256];
            strncpy(colorStr, argv[i] + 9, sizeof(colorStr) - 1);
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
        } else if (strncmp(argv[i], "/textcolor:", 11) == 0) {
            char colorStr[256];
            strncpy(colorStr, argv[i] + 11, sizeof(colorStr) - 1);
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
        } else if (strcmp(argv[i], "/modal") == 0) {
            modal = TRUE;
        } else if (strcmp(argv[i], "/nodrag") == 0) {
            noDrag = TRUE;
        }
    }
    
    // Check if required parameters are provided
    if (!hasText) {
        printf("Error: Text parameter is required.\n");
        printf("Use 'spcmd window /help' for usage information.\n");
        return;
    }
    
    // 如果是模态弹窗，禁用所有其他窗口
    if (modal) {
        EnumWindows(EnumWindowsProcDisable, (LPARAM)NULL);
    }
    
    // 创建窗口参数结构
    WindowParams* params = (WindowParams*)malloc(sizeof(WindowParams));
    if (!params) {
        printf("Error: Memory allocation failed.\n");
        return;
    }
    
    // 初始化参数
    params->text = message;
    params->fontSize = fontSize;
    params->bgColor = bgColor;
    params->textColor = textColor;
    params->modal = modal;
    params->noDrag = noDrag;
    
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
    HWND hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_APPWINDOW, // 强制顶层显示
        "WindowClass",
        title, // 直接使用标题
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, width, height,
        NULL, NULL, GetModuleHandle(NULL), params);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
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
    
    printf("Custom window displayed\n");
}

