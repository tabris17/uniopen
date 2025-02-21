/**
 * @file messages.c
 */

#include <windows.h>
#include "messages.h"

PWSTR messages[MESSAGE_COUNT] = {};

/**
 * @brief Load messages
 * 
 * @return int 
 */
int load_messages(void)
{
    HINSTANCE instance = GetModuleHandle(NULL);
    for (int i = 0; i < MESSAGE_COUNT; i++) {
        LoadStringW(instance, MESSAGE_BASE + i, (LPWSTR)(messages + i), 0);
    }
    return 0;
}
