#ifndef XP_COMPAT_H
#define XP_COMPAT_H

// Windows XP 兼容性定义
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501
#define _WIN32_IE 0x0500

// 确保使用兼容的API
#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif

// 禁用可能导致XP不兼容的特性
#ifdef UNICODE
#undef UNICODE
#endif

#ifdef _UNICODE
#undef _UNICODE
#endif

#endif // XP_COMPAT_H