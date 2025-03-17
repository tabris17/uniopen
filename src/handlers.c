/**
 * @file handlers.c
 */

FILETYPEHANDLERENTRY application_handlers[] = {
    { "js", MATCHTYPE_EXACT, "javascript", NULL },
    { "json", MATCHTYPE_EXACT, "json", NULL },
    { "pdf", MATCHTYPE_EXACT, "pdf", NULL },
    { "lnk", MATCHTYPE_EXACT, "x-ms-shortcut", NULL },
    { "exe", MATCHTYPE_EXACT, "vnd.microsoft.portable-executable", NULL },
    { "azw", MATCHTYPE_EXACT, "vnd.amazon.ebook", NULL },

    // begin of document files
    { "doc", MATCHTYPE_EXACT, "msword", NULL },
    { "rtf", MATCHTYPE_EXACT, "rtf", NULL },
    { "xls", MATCHTYPE_EXACT, "vnd.ms-excel", NULL },
    { "ppt", MATCHTYPE_EXACT, "vnd.ms-powerpoint", NULL },
    { "docx", MATCHTYPE_EXACT, "vnd.openxmlformats-officedocument.wordprocessingml.document", NULL },
    { "xlsx", MATCHTYPE_EXACT, "vnd.openxmlformats-officedocument.spreadsheetml.sheet", NULL },
    { "pptx", MATCHTYPE_EXACT, "vnd.openxmlformats-officedocument.presentationml.presentation", NULL },
    // end of document files

    // begin of compressed files
    { "gzip", MATCHTYPE_EXACT, "gzip", NULL },
    { "cab", MATCHTYPE_EXACT, "vnd.ms-cab-compressed", NULL },
    { "rar", MATCHTYPE_EXACT, "vnd.rar", NULL },
    { "bzip", MATCHTYPE_EXACT, "x-bzip", NULL},
    { "bz2", MATCHTYPE_EXACT, "x-bzip2", NULL },
    { "vim", MATCHTYPE_EXACT, "x-ms-vim", NULL },
    { "rar", MATCHTYPE_EXACT, "x-rar-compressed", NULL },
    { "tar", MATCHTYPE_EXACT, "x-tar", NULL },
    { "xz", MATCHTYPE_EXACT, "x-xz", NULL },
    { "zip", MATCHTYPE_EXACT, "zip", NULL },
    // end of compressed files
};


FILETYPEHANDLERENTRY image_handlers[] = {
    { MATCHTYPE_EXACT, "vnd.microsoft.icon", NULL },
};

FILETYPEHANDLERENTRY text_handlers[] = {
    { MATCHTYPE_EXACT, "css", NULL },
    { MATCHTYPE_EXACT, "html", NULL },
    { MATCHTYPE_EXACT, "javascript", NULL },
    { MATCHTYPE_EXACT, "markdown", NULL },
    { MATCHTYPE_EXACT, "plain", NULL },
    { MATCHTYPE_EXACT, "xml", NULL },
    { MATCHTYPE_EXACT, "x-ms-regedit", NULL },
};

FILETYPEHANDLERENTRY video_handlers[];

FILETYPEHANDLERENTRY inode_handlers[] = {
    { MATCHTYPE_EXACT, "directory", NULL },
    { MATCHTYPE_EXACT, "symlink", NULL },
};

