/**
 * @file messages.c
 */

#include <windows.h>
#include <wchar.h>
#include "messages.h"

PCWSTR messages[MESSAGE_COUNT] = {};

__thread WCHAR message[MAX_MSG];

/**
 * @brief Load messages
 * 
 * @return int 
 */
int load_messages(void)
{
    //SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL));
    HINSTANCE instance = GetModuleHandle(NULL);
    for (int i = 0; i < MESSAGE_COUNT; i++) {
        LoadStringW(instance, MESSAGE_BASE + i, (LPWSTR)(messages + i), 0);
    }
    return 0;
}

PWSTR format_message(PCWSTR format, ...)
{
    va_list args;
    va_start(args, format);
    vswprintf(message, MAX_MSG, format, args);
    va_end(args);
    return message;
}
