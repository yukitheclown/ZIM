#include <stdio.h>
#include <gtk-3.0/gtk/gtk.h>
#include <ncurses.h>
#include "text_editor.h"
#include <X11/Xutil.h>
#include <X11/Xlib.h>

#define X11_CONTROL_KEY KEY_MAX+1
#define X11_ALT_KEY KEY_MAX+2
#define X11_SUPER_KEY KEY_MAX+3
#define X11_NUM_OF_RET_KEYS 32*8


static Display *display;
static Window window, ourWindow, root, *children, parent;
static GC gc;
static XIM xim;
static XIC xic;

void X11_Init(){
    display = XOpenDisplay(0);
    char *id;
    int revert;
    unsigned int nchildren;

    if((id = getenv("WINDOWID")) != NULL){
        window = (Window)atoi(id);
    } else {
        XGetInputFocus(display, &window, &revert);
    }

    if(!window) return;

    XWindowAttributes attr;
    XGetWindowAttributes(display, window, &attr);

    int width = 0, height = 0;

    while(1){
        Window p_window;
        XQueryTree(display, window, &root, &parent, &children, &nchildren);
        p_window = window;
        int i;
        for(i = 0; i < nchildren; i++){
            XGetWindowAttributes(display, children[i], &attr);
            if(attr.width > width && attr.height > height){
                width = attr.width;
                height = attr.height;
                window = children[i];
            }
        }

        if(p_window == window) break;
    }

    if(width == 1 && height == 1)
        window = parent;

    unsigned long windowMask;
    XSetWindowAttributes winAttrib; 
            
    windowMask = CWBackPixel | CWBorderPixel;   
    winAttrib.border_pixel = BlackPixel (display, 0);
    winAttrib.background_pixel = BlackPixel (display, 0);
    winAttrib.override_redirect = 0;

    ourWindow = XCreateWindow(display, window, attr.x, attr.y, attr.width, attr.height, attr.border_width, attr.depth, attr.class, 
        attr.visual, windowMask, &winAttrib );

    gc = XCreateGC(display, ourWindow, 0, NULL);

    XSelectInput(display,ourWindow,KeyPressMask|KeyReleaseMask);
}

unsigned int X11_GetGlobalKeys(){
    char keys[32];
    XQueryKeymap(display, keys);
    int k;
    unsigned int ret = 0;
    for(k = 0; k < 32; k++){ 
        if(keys[k]) {
            int f;
            for(f = 0; f < 8; f++){
                if(keys[k] & 0x01 << f){
                    XKeyEvent keyEvent;
                    keyEvent.keycode = (k*8)+f;
                    keyEvent.display = display;
                    int sym = XLookupKeysym(&keyEvent, 0);
                    char *str = XKeysymToString(sym);



					  XKeyPressedEvent event;
					  event.type = KeyPress;
					  event.display = display;
					  event.state = 0;
					  event.keycode = keyEvent.keycode;
                    char symchrs[64];
                    int nchars = Xutf8LookupString(xic, &keyEvent,symchrs,64,NULL,NULL);
                    printw("%i %s\n",nchars, symchrs);
                    int j;
                    for(j = 0; j < nchars; j++) printw("%c\n",symchrs[j]);
                    // printw("%i %i %i %i\n", symchrs[0],symchrs[1],symchrs[2],symchrs[3]);
                    if(strlen(str) == 1) ret |= str[0];//ret[(k*8)+f] = (int)str[0];
                    else if(strcmp(str, "Up") == 0) ret |= EDIT_ARROW_UP;//ret[(k*8)+f] = EDIT_X11_CONTROL_KEY;
                    else if(strcmp(str, "Down") == 0) ret |= EDIT_ARROW_DOWN;//ret[(k*8)+f] = EDIT_X11_CONTROL_KEY;
                    else if(strcmp(str, "Left") == 0) ret |= EDIT_ARROW_LEFT;//ret[(k*8)+f] = EDIT_X11_CONTROL_KEY;
                    else if(strcmp(str, "Right") == 0) ret |= EDIT_ARROW_RIGHT;//ret[(k*8)+f] = EDIT_X11_CONTROL_KEY;
                    else if(strcmp(str, "Control_L") == 0) ret |= EDIT_CTRL_KEY;//ret[(k*8)+f] = EDIT_X11_CONTROL_KEY;
                    else if(strcmp(str, "Control_R") == 0) ret |= EDIT_CTRL_KEY;//ret[(k*8)+f] = EDIT_X11_CONTROL_KEY;
                    else if(strcmp(str, "Alt_L") == 0) ret |= EDIT_ALT_KEY;//ret[(k*8)+f] = EDIT_X11_ALT_KEY;
                    else if(strcmp(str, "Alt_R") == 0) ret |= EDIT_ALT_KEY;//ret[(k*8)+f] = EDIT_X11_ALT_KEY;
                    else if(strcmp(str, "Shift_L") == 0) ret |= EDIT_SHIFT_KEY;//ret[(k*8)+f] = EDIT_X11_SUPER_KEY;
                    else if(strcmp(str, "Shift_R") == 0) ret |= EDIT_SHIFT_KEY;//ret[(k*8)+f] = X11_SUPER_KEY;
                    else if(strcmp(str, "BackSpace") == 0) return 127;//ret[(k*8)+f] = X11_SUPER_KEY;
                    else if(strcmp(str, "Tab") == 0) return 9;//ret[(k*8)+f] = X11_SUPER_KEY;
                    else if(strcmp(str, "Return") == 0) return 10;//ret[(k*8)+f] = X11_SUPER_KEY;
                    else if(strcmp(str, "Escape") == 0) return 27;//ret[(k*8)+f] = X11_SUPER_KEY;
                }
            }
        }
    }
    return ret;
}

int main(int argc, char **argv){

	X11_Init();
	xim = XOpenIM(display, 0, 0, 0);
  	xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);

	TextEditor te;

    initscr();
    curs_set(0);
    raw();
    noecho();

    keypad(stdscr, TRUE);
    start_color();
    use_default_colors();
    // init_pair(TITLE_COLOR_PAIR,                 -1,            COLOR_BLUE);
	TextEditor_Init(&te);

	while(1){

 		int getchkey = getch();

	    unsigned int key = X11_GetGlobalKeys();

	    if((key & 0xFF00) == EDIT_SHIFT_KEY) {
		    key = (key&0xFF)-32; //capital
		}

		if(getchkey == 27 && key != 27){
			// want to use getch for blocking
			// but with some keys like alt getch fires several times to trigger an escape sequence 
			// this seems to work	
			continue; //hack
		}

		if(key == 'q') break;
		// TextEditor_Event(&te, key);
		if(te.quit) break;
		// TextEditor_Draw(&te);		
	}

	clear();
	TextEditor_Destroy(&te);

    XWithdrawWindow(display, ourWindow, DefaultScreen(display));
    XSync(display,False);
    XFreeGC(display,gc);
    XCloseDisplay(display);

	endwin();

	return 0;
}