#ifndef WINDOW_DEF
#define WINDOW_DEF

#define WINDOW_TITLE "THOTH"
#define WINDOW_INIT_WIDTH 			960
#define WINDOW_INIT_HEIGHT 			540
// #define WINDOW_INIT_WIDTH 			300
// #define WINDOW_INIT_HEIGHT 			160
#ifdef LINUX_COMPILE
#define CONFIG_PATH "/.config/thoth/"
#define CONFIG_FILE Window_GetConfigPath(CONFIG_PATH "thothconfig.cfg")
#define LOGFILE Window_GetConfigPath(CONFIG_PATH "thothproject.god")
#define LOGCOMPILEFILE Window_GetConfigPath(CONFIG_PATH "thothlog.txt")
char *Window_GetConfigPath(char *rel);
#endif
#ifdef WINDOWS_COMPILE
#define CONFIG_FILE "thothconfig.cfg"
#define LOGFILE "thothproject.god"
#define LOGCOMPILEFILE "thothlog.txt"
#endif
#include <SDL2/SDL_events.h>

void Window_Swap();
void Window_Close();
int Window_GetTicks();
int Window_Open();
void Window_PollEvent(void (*callback)(SDL_Event ev));
#endif