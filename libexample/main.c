#include <stdio.h>
#include <GL/glew.h>
#include <unistd.h>
#include <fcntl.h>
#include <SDL2/SDL.h>
#include "thoth/thoth.h"
#include "window.h"

int main(int argc, char **argv){
    Window_Open();

    Thoth_t *thoth = Thoth_Create(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT );
    Thoth_Resize(thoth, 50, 50, WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT);
    Thoth_LoadFile(thoth, "main.c");

    int quit = 0;
    while(1){
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_Event ev;
        while(SDL_PollEvent(&ev)){
            Thoth_Event(thoth, ev);
            if(ev.type == SDL_QUIT) {quit = 1; break;}
            if(ev.window.event == SDL_WINDOWEVENT_RESIZED || 
                ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
                int w = ev.window.data1;
                int h = ev.window.data2;

                Thoth_Resize(thoth, w/4, 0, (w/2) + (w/4), h);
            }
        }
        if(quit) break;
        Thoth_Render(thoth); //stencil buffer/framebuffer access todo

        Window_Swap();
    }

    Thoth_Destroy(thoth);
    Window_Close();
    return 0;
}
