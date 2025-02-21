/**
 * @file messages.h
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include <windows.h>

#define MESSAGE_BASE            1000
#define MESSAGE_MAX             1100
#define MESSAGE_COUNT           (MESSAGE_MAX - MESSAGE_BASE)
#define MAX_MSG                 256 // Maximum message length

extern PWSTR messages[MESSAGE_COUNT];

int load_messages(void);
static inline PWSTR msg(const UINT id) { return messages[id - MESSAGE_BASE]; }

#endif // MESSAGES_H
