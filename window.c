#include "spcmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>

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

// 自定义弹窗窗口过程函数
LRESULT CALLBACK WindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static WindowParams* params = NULL;
    static HFONT hFont = NULL;
    static HWND hButton = NULL;
    // 添加静态画笔和画刷以避免重复创建
    static HBRUSH hBrush = NULL;
    static HPEN hPen = NULL;
    
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
                params->fontSize, 0, 0, 0, params->bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Microsoft Sans Serif");
            
            // 如果上面的字体不可用，使用系统默认GUI字体
            if (!hFont) {
                hFont = GetStockObject(DEFAULT_GUI_FONT);
            }
            
            // 创建画笔和画刷（只创建一次）
            hBrush = CreateSolidBrush(params->bgColor);
            hPen = CreatePen(PS_SOLID, 1, params->bgColor);
            
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
            
            // 选择画笔和画刷到设备上下文（使用预先创建的）
            HGDIOBJ oldBrush = SelectObject(hdc, hBrush);
            HGDIOBJ oldPen = SelectObject(hdc, hPen);
            
            // 填充整个客户区背景
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
            
            // 恢复旧的画笔和画刷
            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);
            
            // 设置文本颜色和背景模式
            SetTextColor(hdc, params->textColor);
            SetBkMode(hdc, TRANSPARENT);
            
            // 选择字体
            if (hFont) {
                SelectObject(hdc, hFont);
            }
            
            // 直接使用处理后的文本
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
                // 直接返回HTCLIENT，阻止标题栏拖拽
                return HTCLIENT;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        
        case WM_DESTROY: {
            // 清理字体资源
            if (hFont) {
                DeleteObject(hFont);
                hFont = NULL;
            }
            // 清理画笔和画刷资源
            if (hBrush) {
                DeleteObject(hBrush);
                hBrush = NULL;
            }
            if (hPen) {
                DeleteObject(hPen);
                hPen = NULL;
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
    if (argc > 2 && (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "--help") == 0)) {
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
        printf("  /bold             - Set text to bold\n");
        printf("  /modal            - Make window modal (blocks other windows until closed)\n");
        printf("  /nodrag           - Disable window dragging\n\n");
        printf("Examples:\n");
        printf("  spcmd window /text:\"Hello World\"\n");
        printf("  spcmd window /text:\"Line 1\\nLine 2\\nLine 3\" /title:\"Multi-line Text\" /width:500 /height:300\n");
        printf("  spcmd window /text:\"Red background window\" /bgcolor:red /fontsize:20\n");
        printf("  spcmd window /text:\"Blue text on yellow background\" /bgcolor:yellow /textcolor:blue\n");
        printf("  spcmd window /text:\"Bold text example\" /bold\n");
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
    BOOL bold = FALSE;
    BOOL hasText = FALSE;
    
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "--text=", 7) == 0) {
            message = argv[i] + 7;
            hasText = TRUE;
        } else if (strncmp(argv[i], "--title=", 8) == 0) {
            strncpy(title, argv[i] + 8, sizeof(title) - 1);
            title[sizeof(title) - 1] = '\0';
        } else if (strncmp(argv[i], "--width=", 8) == 0) {
            width = atoi(argv[i] + 8);
        } else if (strncmp(argv[i], "--height=", 9) == 0) {
            height = atoi(argv[i] + 9);
        } else if (strncmp(argv[i], "--fontsize=", 11) == 0) {
            fontSize = atoi(argv[i] + 11);
        } else if (strncmp(argv[i], "--bgcolor=", 10) == 0) {
            char colorStr[256];
            strncpy(colorStr, argv[i] + 10, sizeof(colorStr) - 1);
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
            strncpy(colorStr, argv[i] + 12, sizeof(colorStr) - 1);
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
    char* processedMessage = (char*)malloc(strlen(message) * 2 + 1);
    int j = 0;
    for (int i = 0; message[i] != '\0'; i++) {
        if (message[i] == '\\' && message[i+1] == 'n') {
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
    WindowParams* params = (WindowParams*)malloc(sizeof(WindowParams));
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
    HWND hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_APPWINDOW, // 强制顶层显示
        "WindowClass",
        title, // 直接使用标题
        noDrag ? 
            (WS_POPUP | WS_SYSMENU | WS_VISIBLE) :  // 禁止拖拽时使用弹出窗口样式
            (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE),  // 正常情况显示标题栏
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
    free(processedMessage);
    
    printf("Custom window displayed\n");
}