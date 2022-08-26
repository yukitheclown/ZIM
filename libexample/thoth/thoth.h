#ifndef THOTH_DEF
#define THOTH_DEF 
#include "text_editor.h"
#include "graphics.h"
#include "config.h"
#include <SDL2/SDL.h>

typedef struct {
    int state;
    u32 key;
    Thoth_Editor te;
    Thoth_Config cfg;
    Thoth_Graphics graphics;
    int mousedown;
    int mousex;
    int mousey;
    int mousemotiontime;
} Thoth_t;

void Thoth_LoadFile(Thoth_t *t, char *path);
void Thoth_Destroy(Thoth_t *t);
int Thoth_Event(Thoth_t *t, SDL_Event ev);
Thoth_t *Thoth_Create(int w, int h);
void Thoth_Render(Thoth_t *t);
void Thoth_Resize(Thoth_t *t, int x, int y, int w, int h);
#ifdef LIBRARY_COMPILE
#define THOTH_CONFIG_FILE "thothconfig.cfg"
#define THOTH_LOGFILE "thothproject.god"
#define THOTH_LOGCOMPILEFILE "thothlog.txt"
#else
#ifdef LINUX_COMPILE
#define THOTH_CONFIG_PATH "/.config/thoth/"
#define THOTH_CONFIG_FILE Thoth_GetConfigPath(THOTH_CONFIG_PATH "thothconfig.cfg")
#define THOTH_LOGFILE Thoth_GetConfigPath(THOTH_CONFIG_PATH "thothproject.god")
#define THOTH_LOGCOMPILEFILE Thoth_GetConfigPath(THOTH_CONFIG_PATH "thothlog.txt")
char *Thoth_GetConfigPath(char *rel);
#endif
#endif
#ifdef WINDOWS_COMPILE
#define THOTH_CONFIG_FILE "thothconfig.cfg"
#define THOTH_LOGFILE "thothproject.god"
#define THOTH_LOGCOMPILEFILE "thothlog.txt"
#endif
#endif
