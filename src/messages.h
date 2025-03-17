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
#define MSG(MESSAGE_ID)         (messages[MESSAGE_ID - MESSAGE_BASE])

extern PCWSTR messages[MESSAGE_COUNT];

int load_messages(void);
PWSTR format_message(PCWSTR format, ...);

#endif // MESSAGES_H
