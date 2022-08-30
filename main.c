#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>

#include "text_editor.h"
#include "window.h"
#include "memory.h"
#include "graphics.h"
#include "config.h"
#include "thoth.h"

enum {
	THOTH_STATE_QUIT = 1,
	THOTH_STATE_UPDATE,
	THOTH_STATE_UPDATEDRAW,
	THOTH_STATE_RUNNING,
};

#define MOUSEUPDATETIME 50



#ifndef LIBRARY_COMPILE
// #ifdef LINUX_COMPILE || WINDOWS_COMPILE

static char configpath_g[MAX_PATH_LEN];

char *Thoth_GetConfigPath(char *relpath){
#if LINUX_COMPILE
	char *path = getenv("HOME");
#elif WINDOWS_COMPILE
	char *path = getenv("APPDATA");
#endif
	if(relpath == NULL)
		sprintf(configpath_g,"%s%s",path,THOTH_CONFIG_PATH);
	else
		sprintf(configpath_g,"%s%s",path,relpath);
	// ignored if exists
	return configpath_g;
}
// #endif 
#endif


static void MouseMotionUpdate(Thoth_t *t){
	int mousetime = SDL_GetTicks() - t->mousemotiontime;
	if(mousetime > MOUSEUPDATETIME) {
		int mouseupdate = Thoth_Editor_SetCursorPosSelection(&t->te, t->mousex, t->mousey);
		if(mouseupdate){
			t->state = THOTH_STATE_UPDATEDRAW;
			t->mousemotiontime = SDL_GetTicks(); //because timeout, we only wanna see if its been since the
		}
	}
}

#ifdef LIBRARY_COMPILE
void Event(Thoth_t *t, SDL_Event ev){
#else
void Event(Thoth_t *t){
	SDL_Event ev;
	if(t->te.logging == THOTH_LOGMODE_CONSOLE){
		if(!SDL_WaitEventTimeout(&ev, 1000)){
			t->state = THOTH_STATE_UPDATEDRAW;
			return;
		}
	} else if(t->mousedown){
		if(!SDL_WaitEventTimeout(&ev,MOUSEUPDATETIME)){
			MouseMotionUpdate(t);
			return;
		}
	} else {
		if(!SDL_WaitEvent(&ev)) return;
	}
#endif

	if(ev.type == SDL_DROPFILE){
		char *drop = ev.drop.file;
		Thoth_Editor_LoadFile(&t->te, drop);
		t->state = THOTH_STATE_UPDATEDRAW;
		SDL_free(drop);
		return;
	}
	if(ev.type == SDL_QUIT){
		t->state = THOTH_STATE_QUIT;
		return;
	}

	if(ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT){

		if(!t->mousedown){
			int x = ev.button.x / Thoth_Graphics_FontWidth(&t->graphics);
			int y = ev.button.y / Thoth_Graphics_FontHeight(&t->graphics);
			// if(ev.button.clicks >= 2) 
				// Thoth_Editor_SetCursorPosDoubleClick(&t->te, x, y);
			// else{
				Thoth_Editor_SetCursorPos(&t->te, x, y);
				t->mousedown = 1;
				t->mousex = x;
				t->mousey = y;
			// }
			t->state = THOTH_STATE_UPDATEDRAW;
			return;
		}
	}

	if(ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT){
		t->mousedown = 0;
		return;
	}
	if(ev.type == SDL_MOUSEMOTION){
		if(t->mousedown){
			int x = ev.motion.x / Thoth_Graphics_FontWidth(&t->graphics);
			int y = ev.motion.y / Thoth_Graphics_FontHeight(&t->graphics);
			t->mousex = x;
			t->mousey = y;
			MouseMotionUpdate(t);
			return;
		}
	}
	
	if(ev.type == SDL_KEYDOWN){
		
		int key = t->key;

		if(ev.key.keysym.sym == SDLK_RETURN) key |= THOTH_ENTER_KEY;
		else if(ev.key.keysym.sym == SDLK_TAB) key = 9;
		else if(ev.key.keysym.sym == SDLK_ESCAPE) key = 27;
		else if(ev.key.keysym.sym == SDLK_BACKSPACE) key = 127;
		else if(ev.key.keysym.sym == SDLK_LSHIFT || ev.key.keysym.sym == SDLK_RSHIFT) key |= THOTH_SHIFT_KEY;
		else if(ev.key.keysym.sym == SDLK_LALT || ev.key.keysym.sym == SDLK_RALT) key |= THOTH_ALT_KEY;
		else if(ev.key.keysym.sym == SDLK_LCTRL || ev.key.keysym.sym == SDLK_RCTRL) key |= THOTH_CTRL_KEY;
		else if(ev.key.keysym.sym == SDLK_RIGHT) key |= THOTH_ARROW_RIGHT;
		else if(ev.key.keysym.sym == SDLK_UP) key |= THOTH_ARROW_UP;
		else if(ev.key.keysym.sym == SDLK_LEFT) key |= THOTH_ARROW_LEFT;
		else if(ev.key.keysym.sym == SDLK_DOWN) key |= THOTH_ARROW_DOWN;
		else if(key & THOTH_CTRL_KEY)
			key = (t->key&0xFF00) | (ev.key.keysym.sym & 0xFF);

		if(key != t->key || key & THOTH_ARROW_RIGHT || key & THOTH_ARROW_UP || key & THOTH_ARROW_DOWN || key & THOTH_ARROW_LEFT)
			t->state = THOTH_STATE_UPDATE;
			
		t->key = key;



	} else if(ev.type == SDL_KEYUP) {

		if(ev.key.keysym.sym == SDLK_LSHIFT || ev.key.keysym.sym == SDLK_RSHIFT) t->key ^= THOTH_SHIFT_KEY;
		else if(ev.key.keysym.sym == SDLK_LALT || ev.key.keysym.sym == SDLK_RALT) t->key ^= THOTH_ALT_KEY;
		else if(ev.key.keysym.sym == SDLK_LCTRL || ev.key.keysym.sym == SDLK_RCTRL) t->key ^= THOTH_CTRL_KEY;
		else if(ev.key.keysym.sym == SDLK_RETURN) t->key ^= THOTH_ENTER_KEY;
		else if(ev.key.keysym.sym == SDLK_RIGHT) t->key ^= THOTH_ARROW_RIGHT;
		else if(ev.key.keysym.sym == SDLK_UP) t->key ^= THOTH_ARROW_UP;
		else if(ev.key.keysym.sym == SDLK_LEFT) t->key ^= THOTH_ARROW_LEFT;
		else if(ev.key.keysym.sym == SDLK_DOWN) t->key ^= THOTH_ARROW_DOWN;


	} else if(ev.type == SDL_TEXTINPUT){
		t->key = (t->key&0xFF00) | (ev.text.text[0] & 0xFF);
		t->state = THOTH_STATE_UPDATE;        

#ifndef LIBRARY_COMPILE
	} else if (ev.type == SDL_WINDOWEVENT){
		if(ev.window.event == SDL_WINDOWEVENT_RESIZED || ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
			Thoth_Graphics_Resize(&t->graphics, ev.window.data1, ev.window.data2);
			Thoth_Graphics_Clear(&t->graphics);
			Thoth_Editor_Draw(&t->te);        
			Thoth_Graphics_Render(&t->graphics);
			Window_Swap();
		}
#endif
	}
}

#ifdef LIBRARY_COMPILE
void Thoth_Resize(Thoth_t *t, int x, int y, int w, int h){
	Thoth_Graphics_Resize(&t->graphics, w, h);
	Thoth_Graphics_ViewportXY(&t->graphics, x, y);
	Thoth_Graphics_Clear(&t->graphics);
	Thoth_Editor_Draw(&t->te);        
	Thoth_Graphics_Render(&t->graphics);
}

Thoth_t *Thoth_Create(int w, int h){

	Thoth_t *t = (Thoth_t *)malloc(sizeof(Thoth_t));

	memset(t,0,sizeof(Thoth_t));

	Thoth_Config_Read(&t->cfg);
	Thoth_Graphics_Init(&t->graphics, &t->cfg, w, h);


	Thoth_Editor_Init(&t->te, &t->graphics, &t->cfg);
	Thoth_Editor_LoadFile(&t->te, NULL);

	SDL_StartTextInput();

   Thoth_Graphics_Clear(&t->graphics);
   Thoth_Editor_Draw(&t->te);        
   Thoth_Graphics_Render(&t->graphics);

	return (void *)t;
}

int Thoth_Event(Thoth_t *t, SDL_Event ev){

	Thoth_Graphics_Clear(&t->graphics);
	Event(t, ev);

   if(t->state == THOTH_STATE_UPDATE || t->state == THOTH_STATE_UPDATEDRAW){
	   if(t->state == THOTH_STATE_UPDATE){
		   int key = t->key;

		   if((t->key >> 8) == (THOTH_SHIFT_KEY >> 8)){
			   key = (t->key & 0xFF);
		   }
		   Thoth_Editor_Event(&t->te, key);
		   t->key = t->key & 0xff00;

		}
	   t->state = THOTH_STATE_RUNNING;

	   Thoth_Graphics_Clear(&t->graphics);
	   Thoth_Editor_Draw(&t->te);        
	   Thoth_Graphics_Render(&t->graphics);
	   return 1;
   }
   return 0;
}
void Thoth_Render(Thoth_t *t){
	Thoth_Graphics_Clear(&t->graphics);
	Thoth_Editor_Draw(&t->te);        
	Thoth_Graphics_Render(&t->graphics);
}

void Thoth_LoadFile(Thoth_t *t, char *path){
	Thoth_Editor_LoadFile(&t->te, path);
   Thoth_Graphics_Clear(&t->graphics);
   Thoth_Editor_Draw(&t->te);        
   Thoth_Graphics_Render(&t->graphics);
}

void Thoth_Destroy(Thoth_t *t){
	Thoth_Graphics_Close(&t->graphics);
	free(t);
}
#endif

#ifndef LIBRARY_COMPILE
int main(int argc, char **argv){
	Window_Open();

	Thoth_t t;
	memset(&t,0,sizeof(Thoth_t));

#ifdef LINUX_COMPILE
	mkdir(Thoth_GetConfigPath(NULL), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

	Thoth_Config_Read(&t.cfg);
	Thoth_Graphics_Init( &t.graphics, &t.cfg,WINDOW_INIT_WIDTH,WINDOW_INIT_HEIGHT);


	Thoth_Editor_Init(&t.te, &t.graphics, &t.cfg);

	if(argc > 1){
		int k;
		for(k = 0; k < argc-1; k++)
			Thoth_Editor_LoadFile(&t.te, argv[1+k]);
	}
	else if(t.te.nFiles == 0)
		Thoth_Editor_LoadFile(&t.te, NULL);


	SDL_StartTextInput();
	// u32 currTime;
	// u32 frames = 0;
	// u32 lastSecond = SDL_GetTicks();

   Thoth_Graphics_Clear(&t.graphics);
   Thoth_Editor_Draw(&t.te);        
   Thoth_Graphics_Render(&t.graphics);
   Window_Swap();

	while(t.state != THOTH_STATE_QUIT){

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

	   Event(&t);

	   if(t.state == THOTH_STATE_QUIT){
			if(!Thoth_Editor_Destroy(&t.te))
				t.state = THOTH_STATE_UPDATEDRAW;
	   }

	   if(t.state == THOTH_STATE_UPDATE || t.state == THOTH_STATE_UPDATEDRAW){

		   if(t.state == THOTH_STATE_UPDATE){
			   int key = t.key;

			   if((t.key >> 8) == (THOTH_SHIFT_KEY >> 8)){
				   key = (t.key & 0xFF);
			   }
			   Thoth_Editor_Event(&t.te, key);
				if(t.te.quit){
					if(Thoth_Editor_Destroy(&t.te) > 0){
						break;
					}
					t.te.quit = 0;
				}

			   t.key = t.key & 0xff00;

			}    
		   t.state = THOTH_STATE_RUNNING;

		   Thoth_Graphics_Clear(&t.graphics);
		   Thoth_Editor_Draw(&t.te);        
		   Thoth_Graphics_Render(&t.graphics);
		   Window_Swap();
	   }
	}


	Thoth_Graphics_Close(&t.graphics);
	Window_Close();
	return 0;
}
#endif
