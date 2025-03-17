/**
 * @file uniopen.c
 */

#include <stdarg.h>
#include <stdio.h>
#include <io.h>
#include <wchar.h>
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <magic.h>
#include <lmdb.h>
#include "uniopen.h"
#include "messages.h"


/**
 * @brief Get filenames from command line
 */
static PFILENAMES parse_filenames(PWSTR cmdline)
{
    if (cmdline == NULL || *cmdline == L'\0') {
        return NULL;
    }

    int argc;
    LPWSTR *argv = CommandLineToArgvW(cmdline, &argc);
    if (argv == NULL) {
        return NULL;
    }

    PFILENAMES filenames = (PFILENAMES)malloc(sizeof(FILENAMES) + argc * sizeof(PSTR));
    if (filenames == NULL) {
        LocalFree(argv);
        return NULL;
    }

    for (int i = 0; i < argc; i++) {
        int len = WideCharToMultiByte(CP_ACP, 0, argv[i], -1, NULL, 0, NULL, NULL);
        filenames->paths[i] = (PSTR)malloc(len);
        if (filenames->paths[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(filenames->paths[j]);
            }
            free(filenames);
            LocalFree(argv);
            return NULL;
        }
        WideCharToMultiByte(CP_ACP, 0, argv[i], -1, filenames->paths[i], len, NULL, NULL);
    }
    LocalFree(argv);

    filenames->count = argc;
    return filenames;
}


static int mdb_init(MDB_txn *txn, MDB_dbi dbi)
{
    /**
     * @todo implement
     */
    return 0;
}


/**
 * @brief make app context
 */
static BOOL make_context(PAPPCONTEXT ctx, HINSTANCE inst, PWSTR cli)
{
    memset(ctx, 0, sizeof(APPCONTEXT));
    ctx->instance = inst;

    WCHAR data_path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, data_path) != S_OK ||
        PathAppendW(data_path, WSTR(APP_NAME)) == FALSE) {

        SetLastError(ERR_USERDATA);
        return FALSE;
    }
    CreateDirectoryW(data_path, NULL);
    ctx->data_path_w = wcsdup(data_path);
    int data_path_len = WideCharToMultiByte(CP_ACP, 0, data_path, -1, NULL, 0, NULL, NULL);
    ctx->data_path = malloc(data_path_len);
    WideCharToMultiByte(CP_ACP, 0, data_path, -1, ctx->data_path, data_path_len, NULL, NULL);

    ctx->filenames = parse_filenames(cli);
    if (ctx->filenames == NULL) {
        SetLastError(ERR_NO_FILENAMES);
        return FALSE;
    }

    ctx->magic = magic_open(MAGIC_NO_CHECK_COMPRESS | MAGIC_SYMLINK | MAGIC_RAW);
    if (magic_load(ctx->magic, MAGIC_FILE) != 0) {
        SetLastError(ERR_MAGIC_LOAD);
        return FALSE;
    }

    mdb_env_create(&ctx->mdb_env);
    mdb_env_set_maxdbs(ctx->mdb_env, 1);
    if (mdb_env_open(ctx->mdb_env, ctx->data_path, 0, 0) != 0) {
        SetLastError(ERR_MDB_ENV_OPEN);
        return FALSE;
    }

    if (mdb_txn_begin(ctx->mdb_env, NULL, 0, &ctx->mdb_txn) != 0) {
        SetLastError(ERR_MDB_TXN_BEGIN);
        return FALSE;
    }

    BOOL mdb_initialized = TRUE;
    int mdb_result = mdb_dbi_open(ctx->mdb_txn, APP_NAME, 0, &ctx->mdb_dbi);
    if (mdb_result == MDB_NOTFOUND) {
        mdb_result = mdb_dbi_open(ctx->mdb_txn, APP_NAME, MDB_CREATE, &ctx->mdb_dbi);
        mdb_initialized = FALSE;
    }
    if (mdb_result != 0) {
        SetLastError(ERR_MDB_DBI_OPEN);
        return FALSE;
    }

    if (!mdb_initialized) {
        if (mdb_init(ctx->mdb_txn, ctx->mdb_dbi) != 0 ||
            mdb_txn_commit(ctx->mdb_txn) !=0 ||
            mdb_txn_begin(ctx->mdb_env, NULL, 0, &ctx->mdb_txn) != 0) {

                SetLastError(ERR_MDB_INIT_FAILED);
                return FALSE;
            }
    }
    return TRUE;
}


/**
 * @brief release app context
 */
static void release_context(PAPPCONTEXT ctx)
{
    if (ctx->data_path_w) {
        free(ctx->data_path_w);
    }

    if (ctx->data_path) {
        free(ctx->data_path);
    }

    if (ctx->filenames) {
        for (int i = 0; i < ctx->filenames->count; i++) {
            free(ctx->filenames->paths[i]);
        }
        free(ctx->filenames);
    }

    if (ctx->magic) {
        magic_close(ctx->magic);
    }

    if (ctx->mdb_dbi) {
        mdb_dbi_close(ctx->mdb_env, ctx->mdb_dbi);
        mdb_env_close(ctx->mdb_env);
    } else if (ctx->mdb_env) {
        mdb_env_close(ctx->mdb_env);
    }
}


/**
 * @brief Query file association
 */
static HRESULT query_file_assoc(PCSTR file_ext, PWSTR command, DWORD *command_len)
{
    WCHAR dot_ext[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, file_ext, -1, dot_ext + 1, MAX_PATH - 1);
    dot_ext[0] = L'.';
    return AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_COMMAND, dot_ext, NULL, command, command_len);
}


/**
 * @brief replace substring
 */
static PWSTR wstr_replace(PWSTR new_str, size_t new_str_count, PCWSTR str, PCWSTR *searchs, int search_count, PCWSTR replace)
{
    size_t str_len = wcslen(str);
    size_t replace_len = wcslen(replace);
    size_t new_str_len = 0;

    for (size_t i = 0; i < str_len; i++) {
        BOOL replaced = FALSE;
        for (int j = 0; j < search_count; j++) {
            size_t search_len = wcslen(searchs[j]);
            if (wcsncmp(&str[i], searchs[j], search_len) == 0) {
                if (new_str_len + replace_len >= new_str_count) {
                    return NULL;
                }
                wcscpy(&new_str[new_str_len], replace);
                new_str_len += replace_len;
                i += search_len - 1;
                replaced = TRUE;
                break;
            }
        }
        if (!replaced) {
            if (new_str_len + 1 >= new_str_count) {
                return NULL;
            }
            new_str[new_str_len++] = str[i];
        }
    }
    new_str[new_str_len] = L'\0';
    return new_str;
}


/**
 * @brief shell open
 */
static BOOL shell_open(PCSTR filename, PCWSTR command)
{
    WCHAR filename_w[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, filename, -1, filename_w, MAX_PATH);
    const size_t MAX_CMD = 0x7FFF;
    WCHAR command_line[MAX_CMD];
    PCWSTR searchs[] = { L"%1", L"%L" };
    wstr_replace(command_line, MAX_CMD, command, searchs, ARRAYSIZE(searchs), filename_w);
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessW(NULL, command_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return TRUE;
    }
    return FALSE;
}


/**
 * @brief UniOpen file
 */
static BOOL uniopen(magic_t magic, PCSTR filename)
{
    BOOL ret = TRUE;
    if (access(filename, R_OK) != 0) {
        SetLastError(ERR_FILE_NO_ACCESS);
        return FALSE;
    }

    int magic_flags = magic_getflags(magic);
    magic_setflags(magic, magic_flags | MAGIC_MIME);

    PCSTR file_type = magic_file(magic, filename);
    if (NULL == file_type) {
        SetLastError(ERR_UNKNOWN_FILE_TYPE);
        ret = FALSE;
        goto RETURN;
    }

    #define _MAKECONTENTTYPEFORMAT(_len_) ("%" #_len_ "[^/]/%" #_len_ "[^;]; charset=%" #_len_ "s")
    #define MAKECONTENTTYPEFORMAT(_len_) _MAKECONTENTTYPEFORMAT(_len_)

    char type[MAX_PATH] = { 0 };
    char subtype[MAX_PATH] = { 0 };
    char charset[MAX_PATH] = { 0 };
    int result = sscanf(file_type, MAKECONTENTTYPEFORMAT(MAX_PATH), type, subtype, charset);
    if (result < 2) {
        SetLastError(ERR_INVALID_FILE_TYPE);
        ret = FALSE;
        goto RETURN;
    }

    /*if (strcasecmp(type, "text") != 0) {
        if (strncmp(subtype, "x-", 2) == 0) {
            goto UNHANDLED;
        } else if (strcasecmp(subtype, "xml") == 0) {
            goto UNHANDLED;
        } else if (strcasecmp(subtype, "json") == 0) {
            goto UNHANDLED;
        }
        goto UNHANDLED;
    } else if (strcasecmp(type, "audio") != 0) {
        goto UNHANDLED;
    } else if (strcasecmp(type, "video") != 0) {
        goto UNHANDLED;
    } else if (strcasecmp(type, "image") != 0) {
        goto UNHANDLED;
    }*/

    magic_setflags(magic, magic_flags | MAGIC_CONTINUE);
    PCSTR file_detail = magic_file(magic, filename);
    
    WCHAR command[MAX_PATH];
    DWORD command_len = MAX_PATH;
    PCSTR default_ext = strrchr(filename, '.');
    if (!default_ext || default_ext == filename || 
        strchr(default_ext, '/') || strchr(default_ext, '\\') || 
        *(default_ext - 1) == '/' || *(default_ext - 1) == '\\') {

        default_ext = NULL;
    } else if (query_file_assoc(default_ext, command, &command_len) == S_OK) {
        ShellExecuteA(NULL, "open", filename, NULL, NULL, SW_SHOWNORMAL);
    }
    magic_setflags(magic, magic_flags | MAGIC_EXTENSION);
    PCSTR file_exts = magic_file(magic, filename);
    if (file_exts != NULL && file_exts[0] != '?') {
        PSTR saveptr = NULL;
        PCSTR ext = strtok_r((char *)file_exts, "/", &saveptr);
        while (ext != NULL) {
            if (!default_ext || strcasecmp(default_ext, ext) != 0) {
                if (query_file_assoc(ext, command, &command_len) == S_OK) {
                    shell_open(filename, command);
                    break;
                }
            }
            ext = strtok_r(NULL, "/", &saveptr);
        }
    }

RETURN:
    magic_setflags(magic, magic_flags);
    return ret;

UNHANDLED:
    SetLastError(ERR_UNHANDLED_FILE_TYPE);
    magic_setflags(magic, magic_flags);
    return FALSE;
}


static PCWSTR uniopen_error(magic_t magic, PCSTR filename)
{
    WCHAR filename_w[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, filename, -1, filename_w, MAX_PATH);
    PCWSTR message = NULL;
    DWORD last_error = GetLastError();
    switch (last_error) {
    case ERR_FILE_NO_ACCESS:
        message = format_message(MSG(IDS_ERROR_UNREADABLE), filename_w);
        break;
    case ERR_UNKNOWN_FILE_TYPE:
    case ERR_INVALID_FILE_TYPE:
        message = format_message(MSG(IDS_ERROR_FILE_TYPE), filename_w);
        break;
    }
    return message;
}


/**
 * @brief Entry point of the program
 */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    InitCommonControls();
    load_messages();

    int ret = EXIT_SUCCESS;
    APPCONTEXT ctx;

    if (!make_context(&ctx, hInstance, pCmdLine)) {
        ret = EXIT_FAILURE;
        goto CLEANUP;
    }

    PCSTR filename = NULL;
    if (ctx.filenames->count == 1) {
        filename = ctx.filenames->paths[0];
        if (!uniopen(ctx.magic, filename)) {
            MessageBoxW(NULL, uniopen_error(ctx.magic, filename), MSG(IDS_APP_NAME), MB_ICONERROR | MB_OK);
        }
    } else {
        BOOL quiet = FALSE;
        for (int i = 0; i < ctx.filenames->count; i++) {
            filename = ctx.filenames->paths[i];
            if (!uniopen(ctx.magic, filename) && !quiet) {
                int button_id = 0;
                const int IDIGNOREALL = 100;
                TASKDIALOG_BUTTON buttons[] = {
                    { IDIGNORE, MSG(IDS_IGNORE) },
                    { IDIGNOREALL, MSG(IDS_IGNORE_ALL) },
                    { IDABORT, MSG(IDS_ABORT) },
                };
                TASKDIALOGCONFIG config = {
                    .cbSize = sizeof(TASKDIALOGCONFIG),
                    .hwndParent = NULL,
                    .hInstance = hInstance,
                    .dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_SIZE_TO_CONTENT,
                    .pszWindowTitle = MAKEINTRESOURCEW(IDS_APP_NAME),
                    .pszMainIcon = MAKEINTRESOURCEW(IDI_UNIOPEN),
                    .pszContent = uniopen_error(ctx.magic, filename),
                    .pszMainInstruction = MAKEINTRESOURCEW(IDS_ERROR_OCCURRED),
                    .cButtons = ARRAYSIZE(buttons),
                    .pButtons = buttons,
                    .nDefaultButton = IDIGNORE,
                };
                if (FAILED(TaskDialogIndirect(&config, &button_id, NULL, NULL))) {
                    continue;
                }
                if (button_id == IDIGNOREALL) {
                    quiet = TRUE;
                } else if (button_id == IDABORT) {
                    break;
                } 
            }
        }
    }

CLEANUP:
    release_context(&ctx);
    return ret;
}
