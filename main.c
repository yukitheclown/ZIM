#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <SDL2/SDL.h>
#include "text_editor.h"
#include "window.h"
#include "memory.h"
#include "graphics.h"
#include "config.h"

enum {
    GODCODE_STATE_QUIT = 1,
    GODCODE_STATE_UPDATE,
    GODCODE_STATE_UPDATEDRAW,
    GODCODE_STATE_RUNNING,
};

#define MOUSEUPDATETIME 100

typedef struct {
    int state;
    u32 key;
    TextEditor te;
    Config cfg;
    Graphics graphics;
    int mousedown;
    int mousex;
    int mousey;
    int mousemotiontime;
} GodCode_t;

static void MouseMotionUpdate(GodCode_t *gc){
    int mousetime = SDL_GetTicks() - gc->mousemotiontime;
    if(mousetime > MOUSEUPDATETIME) {
        int mouseupdate = TextEditor_SetCursorPosSelection(&gc->te, gc->mousex, gc->mousey);
        if(mouseupdate){
            gc->state = GODCODE_STATE_UPDATEDRAW;
            gc->mousemotiontime = SDL_GetTicks(); //because timeout, we only wanna see if its been since the
        }
    }
}

void Event(GodCode_t *gc){

    SDL_Event ev;
    if(gc->te.logging == LOGMODE_CONSOLE){
        if(!SDL_WaitEventTimeout(&ev, 1000)){
            gc->state = GODCODE_STATE_UPDATEDRAW;
            return;
        }
    } else if(gc->mousedown){
        if(!SDL_WaitEventTimeout(&ev,MOUSEUPDATETIME)){
            MouseMotionUpdate(gc);
            return;
        }
    } else {
        if(!SDL_WaitEvent(&ev)) return;
    }

    if(ev.type == SDL_QUIT){
        gc->state = GODCODE_STATE_QUIT;
        return;
    }

    if(ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT){

        if(!gc->mousedown){
            int x = ev.button.x / Graphics_FontWidth(&gc->graphics);
            int y = ev.button.y / Graphics_FontHeight(&gc->graphics);
            if(ev.button.clicks >= 2) 
                TextEditor_SetCursorPosDoubleClick(&gc->te, x, y);
            else{
                TextEditor_SetCursorPos(&gc->te, x, y);
                gc->mousedown = 1;
                gc->mousex = x;
                gc->mousey = y;
            }
            gc->state = GODCODE_STATE_UPDATEDRAW;
            return;
        }
    }
    if(ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT){
        gc->mousedown = 0;
        return;
    }
    if(ev.type == SDL_MOUSEMOTION){
        if(gc->mousedown){
            int x = ev.motion.x / Graphics_FontWidth(&gc->graphics);
            int y = ev.motion.y / Graphics_FontHeight(&gc->graphics);
            gc->mousex = x;
            gc->mousey = y;
            MouseMotionUpdate(gc);
            return;
        }
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
            Graphics_Resize(&gc->graphics, ev.window.data1, ev.window.data2);
            Graphics_Clear(&gc->graphics);
            TextEditor_Draw(&gc->te);        
            Graphics_Render(&gc->graphics);
            Window_Swap();
        }
    }
}

int main(int argc, char **argv){
    Window_Open();

    GodCode_t gc;
    memset(&gc,0,sizeof(GodCode_t));

    Config_Read(&gc.cfg);
    Graphics_Init( &gc.graphics, &gc.cfg);


    TextEditor_Init(&gc.te, &gc.graphics, &gc.cfg);

    if(argc > 1){
        int k;
        for(k = 0; k < argc-1; k++)
            TextEditor_LoadFile(&gc.te, argv[1+k]);
    }
    else if(gc.te.nFiles == 0)
        TextEditor_LoadFile(&gc.te, NULL);


    SDL_StartTextInput();
    u32 currTime;
    u32 frames = 0;
    u32 lastSecond = SDL_GetTicks();

   Graphics_Clear(&gc.graphics);
   TextEditor_Draw(&gc.te);        
   Graphics_Render(&gc.graphics);
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

       if(gc.state == GODCODE_STATE_QUIT){
            if(!TextEditor_Destroy(&gc.te))
                gc.state = GODCODE_STATE_RUNNING;
       }

       if(gc.state == GODCODE_STATE_UPDATE || gc.state == GODCODE_STATE_UPDATEDRAW){

           if(gc.state == GODCODE_STATE_UPDATE){
               int key = gc.key;

               if((gc.key >> 8) == (EDIT_SHIFT_KEY >> 8)){
                   key = (gc.key & 0xFF);
               }
               TextEditor_Event(&gc.te, key);
                if(gc.te.quit){
                    if(TextEditor_Destroy(&gc.te) > 0){
                        break;
                    }
                    gc.te.quit = 0;
                }

               gc.key = gc.key & 0xff00;

            }    
           gc.state = GODCODE_STATE_RUNNING;

           Graphics_Clear(&gc.graphics);
           TextEditor_Draw(&gc.te);        
           Graphics_Render(&gc.graphics);
           Window_Swap();
       }
    }


    Graphics_Close(&gc.graphics);
    Window_Close();
    return 0;
}
