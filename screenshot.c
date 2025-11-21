#include "spcmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>
#include <stdint.h>

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
    int quality = 80; // default quality
    
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