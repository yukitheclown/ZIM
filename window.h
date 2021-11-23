#ifndef WINDOW_DEF
#define WINDOW_DEF

#define WINDOW_TITLE "KEK"
#define WINDOW_INIT_WIDTH 			960
#define WINDOW_INIT_HEIGHT 			540
// #define WINDOW_INIT_WIDTH 			300
// #define WINDOW_INIT_HEIGHT 			160

#include <SDL2/SDL_events.h>

void Window_Swap();
void Window_Close();
int Window_GetTicks();
int Window_Open();
void Window_PollEvent(void (*callback)(SDL_Event ev));

#endif