/**
 * @file uniopen.h
 */

#ifndef UNIOPEN_H
#define UNIOPEN_H

#include <lmdb.h>
#include <magic.h>

#define MAGIC_FILE              "magic.mgc"
#define APP_NAME                "uniopen"

#define IDI_UNIOPEN             101
#define IDR_MAGIC               102

#define IDS_APP_NAME            1000
#define IDS_ERROR_CORRUPTED     1001
#define IDS_ERROR_FILE_TYPE     1002
#define IDS_ERROR_UNREADABLE    1003
#define IDS_ERROR_OCCURRED      1004
#define IDS_IGNORE              1005
#define IDS_ABORT               1006
#define IDS_IGNORE_ALL          1007

#define ERRNO(_ERRNO_)          ((1 << 31) + _ERRNO_)   // user defined error code
#define ERR_FILE_NO_ACCESS      ERRNO(1)
#define ERR_UNKNOWN_FILE_TYPE   ERRNO(2)
#define ERR_INVALID_FILE_TYPE   ERRNO(3)
#define ERR_UNHANDLED_FILE_TYPE ERRNO(4)
#define ERR_NO_FILENAMES        ERRNO(5)
#define ERR_USERDATA            ERRNO(6)
#define ERR_MAGIC_LOAD          ERRNO(7)
#define ERR_MDB_ENV_OPEN        ERRNO(8)
#define ERR_MDB_DBI_OPEN        ERRNO(9)
#define ERR_MDB_TXN_BEGIN       ERRNO(10)
#define ERR_MDB_INIT_FAILED     ERRNO(11)

#define _WSTR(_str_)             L##_str_
#define WSTR(_str_)              _WSTR(_str_)

typedef struct _FILENAMES {
    int count;
    PSTR paths[];
} FILENAMES, *PFILENAMES;


typedef struct _APPCONTEXT {
    HINSTANCE instance;
    PFILENAMES filenames;
    magic_t magic;
    MDB_env *mdb_env;
    MDB_dbi mdb_dbi;
    MDB_txn *mdb_txn;
    PWSTR data_path_w;
    PSTR data_path;
} APPCONTEXT, *PAPPCONTEXT;

#endif /* UNIOPEN_H */
