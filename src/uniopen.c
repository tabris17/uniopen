/**
 * @file uniopen.c
 */

#include <stdarg.h>
#include <wchar.h>
#include <windows.h>
#include <magic.h>
#include "uniopen.h"
#include "messages.h"


/**
 * @brief Show error message
 */
static int ShowError(const PWSTR format, ...)
{
    WCHAR error[MAX_MSG];
    va_list args;
    va_start(args, format);
    vswprintf(error, MAX_MSG, format, args);
    va_end(args);

    WCHAR caption[MAX_MSG];
    swprintf(caption, MAX_MSG, L"%S %S", msg(IDS_APP_NAME), msg(IDS_ERROR));
    return MessageBoxW(NULL, error, caption, MB_ICONERROR);
}


/**
 * @brief Entry point of the program
 */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    //SetThreadUILanguage(MAKELANGID(LANG_CHINESE, SUBLANG_NEUTRAL));
    load_messages();
    const char *filename = "d:\\10000500\\Desktop\\test.7z";
    magic_t magic = magic_open(MAGIC_MIME | MAGIC_NO_CHECK_COMPRESS);
    if (magic == NULL) {
        ShowError(msg(IDS_ERROR_INIT_MAGIC));
        return 1;
    }

    if (magic_load(magic, "@") != 0) {
        ShowError(msg(IDS_ERROR_LOAD_MAGIC), magic_error(magic));
        goto ERROR_MAGIC_CLOSE;
    }

    const char *file_type = magic_file(magic, filename);
    if (file_type == NULL) {
        ShowError(msg(IDS_ERROR_FILE_TYPE), magic_error(magic));
        goto ERROR_MAGIC_CLOSE;
    }

    MessageBoxA(NULL, file_type, "FileType", MB_OK);
    //printf("File type: %s\n", file_type);
    magic_close(magic);
    return 0;

ERROR_MAGIC_CLOSE:
    magic_close(magic);
    return 1;
}
