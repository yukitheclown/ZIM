#ifndef LIBRARY_COMPILE


#ifdef WINDOWS_COMPILE
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "window.h"
#include "types.h"
#include "log.h"

static SDL_Window *window;
static SDL_GLContext context;

int Window_Open(){

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_INIT_WIDTH,
        WINDOW_INIT_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;

    if(glewInit() != GLEW_OK) {
        LOG(LOG_RED, "Glew Init Failed\n");
        return 0;
    }

    glDisable(GL_DITHER);
    glDisable(GL_DEPTH_TEST);
    // glEnable(GL_STENCIL_TEST);
    // glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
    glEnable(GL_CULL_FACE);
    glDisable(GL_LINE_SMOOTH);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);


    
    return 1;
}

void Window_Close(){
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int Window_GetTicks(){
    return SDL_GetTicks();
}

void Window_Swap(){

    SDL_GL_SwapWindow(window);
}

void Window_PollEvent(void (*callback)(SDL_Event ev)){

    SDL_Event ev;

    while(SDL_PollEvent(&ev))
        callback(ev);
}

#endif