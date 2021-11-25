#ifndef FILE_BROWSER_DEF
#define FILE_BROWSER_DEF
#include "types.h"
typedef struct {
    char path[MAX_PATH_LEN];
    char name[MAX_PATH_LEN];
    int dir;
} FileBrowser_File;

typedef struct {
    FileBrowser_File *files;
    int nFiles;
    char directory[MAX_PATH_LEN];
} FileBrowser;

void FileBrowser_Init(FileBrowser *fb);
void FileBrowser_ChangeDirectory(FileBrowser *fb);
void FileBrowser_Free(FileBrowser *fb);
#endif