#ifndef FILE_BROWSER_DEF
#define FILE_BROWSER_DEF
#include "types.h"
#define THOTH_FILEBROWSER_MAX 100

typedef struct {
    int dir;
    char path[MAX_PATH_LEN];
    char name[MAX_PATH_LEN];
} Thoth_FileBrowserFile;

typedef struct {
    Thoth_FileBrowserFile files[THOTH_FILEBROWSER_MAX];
    int nFiles;
    char directory[MAX_PATH_LEN];
} Thoth_FileBrowser;

void Thoth_FileBrowser_Init(Thoth_FileBrowser *fb);
void Thoth_FileBrowser_ChangeDirectory(Thoth_FileBrowser *fb);
void Thoth_FileBrowser_Free(Thoth_FileBrowser *fb);
#endif