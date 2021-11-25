#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "file_browser.h"

void FileBrowser_Init(FileBrowser *fb){
    memset(fb,0,sizeof(FileBrowser));
#ifdef LINUX_COMPILE
    getcwd(fb->directory, MAX_PATH_LEN);
#endif
#ifdef WINDOWS_COMPILE
    sprintf(fb->directory, "C:\\");
#endif

    FileBrowser_ChangeDirectory(fb);
}
void FileBrowser_Free(FileBrowser *fb){
    if(fb->files != NULL) free(fb->files);
    fb->files = NULL;
    fb->nFiles = 0;
}
void FileBrowser_ChangeDirectory(FileBrowser *fb){

    if(fb->files) free(fb->files);
    fb->files = NULL;
    fb->nFiles = 0;
    
    if(strlen(fb->directory) == 0) return;

    DIR *dir;
    struct dirent *dp;
    dir = opendir(fb->directory);
    if(dir == NULL) return;
    
    while((dp = readdir(dir)) != NULL){

        struct stat s;
        char temp[MAX_PATH_LEN];
        strcpy(temp,fb->directory);
        int len = strlen(temp);
#ifdef LINUX_COMPILE
        temp[len] = '/';
#endif
#ifdef WINDOWS_COMPILE
        temp[len] = '\\';
#endif

        temp[len+1] = 0;
        strcpy(&temp[strlen(temp)], dp->d_name);
        
        stat(temp, &s);

        if((s.st_mode & S_IFMT) == S_IFDIR){

            if(strcmp(dp->d_name, ".") == 0) continue;

            fb->files = (FileBrowser_File *)realloc(fb->files, sizeof(FileBrowser_File) * ++fb->nFiles);
            strcpy(fb->files[fb->nFiles-1].path, temp);
            strcpy(fb->files[fb->nFiles-1].name, dp->d_name);
            fb->files[fb->nFiles-1].dir = 1;

        } else {

            fb->files = (FileBrowser_File *)realloc(fb->files, sizeof(FileBrowser_File) * ++fb->nFiles);
            strcpy(fb->files[fb->nFiles-1].path, temp);
            strcpy(fb->files[fb->nFiles-1].name, dp->d_name);
            fb->files[fb->nFiles-1].dir = 0;
        }

    }
    closedir(dir);
}
