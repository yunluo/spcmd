#ifndef SPCMD_H
#define SPCMD_H

#include <windows.h>
#include <stdbool.h>

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
void cmd_window(int argc, char* argv[]);

// 工具函数声明
void save_as_png(HBITMAP hBitmap, HDC hScreenDC, const char* filename, int quality);
void save_as_base64_data(const char* bitmap_data, DWORD data_size, const char* filename);
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length);
COLORREF GetColorByName(const char* colorName);

// 窗口过程函数声明
LRESULT CALLBACK WindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 枚举窗口回调函数
BOOL CALLBACK EnumWindowsProcDisable(HWND hwnd, LPARAM lParam);
BOOL CALLBACK EnumWindowsProcEnable(HWND hwnd, LPARAM lParam);

#endif // SPCMD_H