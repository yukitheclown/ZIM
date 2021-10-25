#ifndef GRAPHICS_DEF
#define GRAPHICS_DEF

#include "math.h"
#include "utils.h"
#include "types.h"

enum {
	POS_LOC = 0,
	UV_LOC,
	COLORFG_LOC,
	COLORBG_LOC,
};

typedef struct {
	u32 	program;
	u32 	fShader;
	u32 	vShader;
	u32		uniColorLoc;
	u32		invViewportLoc;
	u32		ncursesColorsLoc;
} Shader_t;

typedef struct {
    u32 	x;
    u32 	y;
    u16 	u;
    u16 	v;
} PosUV_t;

enum {
	TEXTURELESS_SHADER=0,
	TEXTURED_SHADER,
	QUAD_SHADER,
	NCURSES_SHADER,
	NUM_SHADERS
};

#define MAX_COLOR_PAIRS 32

enum {
	COLOR_CYAN = 1,
	COLOR_RED,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_MAGENTA,
	COLOR_WHITE,
	COLOR_BLACK,
	COLOR_GREY,
	COLOR_BG,
	NUM_COLORS,
};


u32 Graphics_TextCollumns();
u32 Graphics_TextRows();
void Graphics_mvprintw(u32 x, u32 y, char *str, int strLen);
void Graphics_attron(u32 attr);
void Graphics_init_pair(u8 pair, u8 fg, u8 bg);
void Graphics_attroff(u32 attr);
void Graphics_RenderNCurses();
void Graphics_InitNCurses();

void Graphics_Init(void);
void Graphics_Clear(void);
void Graphics_Close(void);
void Graphics_Resize(int w, int h);
void Graphics_Render(void);
void Graphics_RenderString(char *str, u32 x, u32 y, u8 r, u8 g, u8 b);
void Graphics_RenderRect(float x, float y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a);
void Graphics_RenderRectLines(float x, float y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a);
void Graphics_UseShader(int shader);
void Graphics_RenderSprite(float x, float y, u16 w, u16 h, u16 tx, u16 ty, u16 tw, u16 th, Image_t tex);
void Graphics_RenderRotatedRect(float x, float y, u16 w, u16 h, float rotation, s16 ox, s16 oy, u8 r, u8 g, u8 b, u8 a);
#endif