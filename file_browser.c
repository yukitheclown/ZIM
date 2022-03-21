#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "file_browser.h"

#define THOTH_FILEBROWSER_MAX 100

void Thoth_FileBrowser_Init(Thoth_FileBrowser *fb){
	memset(fb,0,sizeof(Thoth_FileBrowser));
#ifdef LINUX_COMPILE
	getcwd(fb->directory, MAX_PATH_LEN);
#endif
#ifdef WINDOWS_COMPILE
	sprintf(fb->directory, "C:\\");
#endif

	Thoth_FileBrowser_ChangeDirectory(fb);
}
void Thoth_FileBrowser_Free(Thoth_FileBrowser *fb){
	if(fb->files != NULL) free(fb->files);
	fb->files = NULL;
	fb->nFiles = 0;
}
void Thoth_FileBrowser_ChangeDirectory(Thoth_FileBrowser *fb){

	if(fb->files) free(fb->files);
	fb->files = NULL;
	fb->nFiles = 0;

	int len = strlen(fb->directory);

	if(len == 0 || (fb->directory[len-1] != '/' && fb->directory[len-1] != '\\')){
#ifdef LINUX_COMPILE
		fb->directory[len++] = '/';
#endif
#ifdef WINDOWS_COMPILE
		fb->directory[len++] = '\\';
#endif
	}

	fb->directory[len] = 0;
	DIR *dir;
	struct dirent *dp;
	dir = opendir(fb->directory);
	if(dir == NULL){
		int m;
		// -2 for trailing '/' and 1 for 0+indexing
		for(m = len-2; m >= 0; m--){
			if(fb->directory[m] == '/' || fb->directory[m] == '\\'){
				break;
			}
		}

		fb->directory[m+1] = 0;
		Thoth_FileBrowser_ChangeDirectory(fb);
		return;
	}

	int max = 0;

	while((dp = readdir(dir)) != NULL){
		
		if(max++ > THOTH_FILEBROWSER_MAX) break;

		struct stat s;
		char temp[MAX_PATH_LEN];
		strcpy(temp,fb->directory);
		strcpy(&temp[strlen(temp)], dp->d_name);
		
		stat(temp, &s);

		if((s.st_mode & S_IFMT) == S_IFDIR){

			if(strcmp(dp->d_name, ".") == 0) continue;
			
			fb->files = (Thoth_FileBrowserFile *)realloc(fb->files, sizeof(Thoth_FileBrowserFile) * ++fb->nFiles);
			fb->files[fb->nFiles-1].dir = 1;
			strcpy(fb->files[fb->nFiles-1].name, dp->d_name);

			if(strcmp(dp->d_name, "..") == 0) {
				int m;
				for(m = (strlen(temp)-strlen("/.."))-1; m >= 0; m--){
					if(temp[m] == '/' || temp[m] == '\\'){
						break;
					}
				}
				if(m <= 0){
					fb->files[fb->nFiles-1].path[m++] = '/';
				} else {
					strncpy(fb->files[fb->nFiles-1].path, temp, m);
				}
				fb->files[fb->nFiles-1].path[m] = 0;


				continue;
			}

			strcpy(fb->files[fb->nFiles-1].path, temp);

		} else {

			fb->files = (Thoth_FileBrowserFile *)realloc(fb->files, sizeof(Thoth_FileBrowserFile) * ++fb->nFiles);
			strcpy(fb->files[fb->nFiles-1].path, temp);
			strcpy(fb->files[fb->nFiles-1].name, dp->d_name);
			fb->files[fb->nFiles-1].dir = 0;
		}

	}
	closedir(dir);
}
