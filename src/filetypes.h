/**
 * @file filetypes.h
 */

#ifndef FILETYPES_H
#define FILETYPES_H

#include <lmdb.h>

typedef struct _FILETYPE {
    MDB_val key;
    MDB_val value;
} FILETYPE, *PFILETYPE;

FILETYPE filetype = {
    .key = { .mv_size = 4, .mv_data = "type" },
    .value = { .mv_size = 4, .mv_data = "text" }
};

#endif /* FILETYPES_H */