#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <SDL2/SDL.h>
#include "text_editor.h"
#include "window.h"
#include "memory.h"
#include "graphics.h"
enum {
    GODCODE_STATE_QUIT = 1,
    GODCODE_STATE_UPDATE,
    GODCODE_STATE_RUNNING,

};

typedef struct {
    int state;
    u32 key;
    TextEditor te;
} GodCode_t;

void Event(GodCode_t *gc){

    SDL_Event ev;
    if(!SDL_WaitEvent(&ev)) return;
    // while(SDL_PollEvent(&ev)){

        if(ev.type == SDL_QUIT){
            gc->state = GODCODE_STATE_QUIT;
        } 
        
        if(ev.type == SDL_KEYDOWN){
            
            int key = gc->key;

            if(ev.key.keysym.sym == SDLK_RETURN) key |= EDIT_ENTER_KEY;
            else if(ev.key.keysym.sym == SDLK_TAB) key = 9;
            else if(ev.key.keysym.sym == SDLK_ESCAPE) key = 27;
            else if(ev.key.keysym.sym == SDLK_BACKSPACE) key = 127;
            else if(ev.key.keysym.sym == SDLK_LSHIFT || ev.key.keysym.sym == SDLK_RSHIFT) key |= EDIT_SHIFT_KEY;
            else if(ev.key.keysym.sym == SDLK_LALT || ev.key.keysym.sym == SDLK_RALT) key |= EDIT_ALT_KEY;
            else if(ev.key.keysym.sym == SDLK_LCTRL || ev.key.keysym.sym == SDLK_RCTRL) key |= EDIT_CTRL_KEY;
            else if(ev.key.keysym.sym == SDLK_RIGHT) key |= EDIT_ARROW_RIGHT;
            else if(ev.key.keysym.sym == SDLK_UP) key |= EDIT_ARROW_UP;
            else if(ev.key.keysym.sym == SDLK_LEFT) key |= EDIT_ARROW_LEFT;
            else if(ev.key.keysym.sym == SDLK_DOWN) key |= EDIT_ARROW_DOWN;
            else if(key & EDIT_CTRL_KEY)
                key = (gc->key&0xFF00) | (ev.key.keysym.sym & 0xFF);

            gc->key = key;
            gc->state = GODCODE_STATE_UPDATE;


        } else if(ev.type == SDL_KEYUP) {

            if(ev.key.keysym.sym == SDLK_LSHIFT || ev.key.keysym.sym == SDLK_RSHIFT) gc->key ^= EDIT_SHIFT_KEY;
            else if(ev.key.keysym.sym == SDLK_LALT || ev.key.keysym.sym == SDLK_RALT) gc->key ^= EDIT_ALT_KEY;
            else if(ev.key.keysym.sym == SDLK_LCTRL || ev.key.keysym.sym == SDLK_RCTRL) gc->key ^= EDIT_CTRL_KEY;
            else if(ev.key.keysym.sym == SDLK_RETURN) gc->key ^= EDIT_ENTER_KEY;
            else if(ev.key.keysym.sym == SDLK_RIGHT) gc->key ^= EDIT_ARROW_RIGHT;
            else if(ev.key.keysym.sym == SDLK_UP) gc->key ^= EDIT_ARROW_UP;
            else if(ev.key.keysym.sym == SDLK_LEFT) gc->key ^= EDIT_ARROW_LEFT;
            else if(ev.key.keysym.sym == SDLK_DOWN) gc->key ^= EDIT_ARROW_DOWN;


        } else if(ev.type == SDL_TEXTINPUT){
            gc->key = (gc->key&0xFF00) | (ev.text.text[0] & 0xFF);
            gc->state = GODCODE_STATE_UPDATE;        

        } else if (ev.type == SDL_WINDOWEVENT){
            if(ev.window.event == SDL_WINDOWEVENT_RESIZED || ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
                Graphics_Resize(ev.window.data1, ev.window.data2);
                Graphics_Clear();
                TextEditor_Draw(&gc->te);        
                Graphics_Render();
                Window_Swap();
            }
        }
    // }
}

int main(int argc, char **argv){
    Window_Open();
    Graphics_Init();

    GodCode_t gc;
    memset(&gc,0,sizeof(GodCode_t));

    TextEditor_Init(&gc.te);

    if(argc > 1)
        TextEditor_LoadFile(&gc.te, argv[1]);
    else
        TextEditor_LoadFile(&gc.te, NULL);

    SDL_StartTextInput();
    u32 currTime;
    u32 frames = 0;
    u32 lastSecond = SDL_GetTicks();

   Graphics_Clear();
   TextEditor_Draw(&gc.te);        
   Graphics_Render();
   Window_Swap();

    while(gc.state != GODCODE_STATE_QUIT){

       // currTime = SDL_GetTicks();

       // int fps;
       // float frameTime;
       // if(currTime - lastSecond > 1000){
       //     fps = frames;
       //     frameTime = (currTime - lastSecond) / (float)frames;
       //     lastSecond = currTime;
       //     frames = 0;
           // printf("fps: %i | ms: %f\n", fps, frameTime);
       // }

       // ++frames;

       Event(&gc);

       if(gc.state == GODCODE_STATE_UPDATE){
           int key = gc.key;

           if((gc.key >> 8) == (EDIT_SHIFT_KEY >> 8)){
               key = (gc.key & 0xFF);
           } 

           TextEditor_Event(&gc.te, key );
            if(gc.te.quit) break;

           gc.key = gc.key & 0xff00;

           gc.state = GODCODE_STATE_RUNNING;
    
           Graphics_Clear();
           TextEditor_Draw(&gc.te);        
           Graphics_Render();
           Window_Swap();
       }
    }

    TextEditor_Destroy(&gc.te);
    Graphics_Close();
    Window_Close();
    return 0;
}