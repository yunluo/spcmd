#include "spcmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <objbase.h>
#include <shobjidl.h>
#define COBJMACROS

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
        pShellLink->lpVtbl->SetPath(pShellLink, targetPath);
        
        // Set working directory
        if (strlen(workingDir) > 0) {
            pShellLink->lpVtbl->SetWorkingDirectory(pShellLink, workingDir);
        } else {
            // Use target file directory as working directory
            char targetDir[MAX_PATH];
            strncpy(targetDir, targetPath, MAX_PATH - 1);
            char* lastSlash = strrchr(targetDir, '\\');
            if (lastSlash != NULL) {
                *lastSlash = '\0';
                pShellLink->lpVtbl->SetWorkingDirectory(pShellLink, targetDir);
            }
        }
        
        // Set description
        if (strlen(description) > 0) {
            pShellLink->lpVtbl->SetDescription(pShellLink, description);
        }
        
        // Set icon
        if (strlen(iconPath) > 0) {
            pShellLink->lpVtbl->SetIconLocation(pShellLink, iconPath, 0);
        }
        
        // Save shortcut
        IPersistFile* pPersistFile = NULL;
        hres = pShellLink->lpVtbl->QueryInterface(pShellLink, &IID_IPersistFile, (LPVOID*)&pPersistFile);
        
        if (SUCCEEDED(hres)) {
            // Convert to wide character
            WCHAR wsz[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wsz, MAX_PATH);
            
            hres = pPersistFile->lpVtbl->Save(pPersistFile, wsz, TRUE);
            
            if (SUCCEEDED(hres)) {
                printf("Shortcut created: %s\n", shortcutPath);
            } else {
                printf("Error: Unable to save shortcut to %s\n", shortcutPath);
            }
            
            pPersistFile->lpVtbl->Release(pPersistFile);
        }
        
        pShellLink->lpVtbl->Release(pShellLink);
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