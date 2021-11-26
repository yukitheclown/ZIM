#ifndef GRAPHICS_DEF
#define GRAPHICS_DEF

#include "config.h"
#include "utils.h"
#include "types.h"
#include "freetype.h"


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
	NCURSES_BG_SHADER,
	NUM_SHADERS
};

#define MAX_COLOR_PAIRS 32
#define MINFONTSIZE 6
#define MAXFONTSIZE 70

#define FONT_PATH "resources/font.png"
#define FONT_PATH_TTF "resources/Monoid-Regular.ttf"

#define POS_ATTRIB "pos"
#define UV_ATTRIB "uv"
#define COLORFG_ATTRIB "colorFG"
#define COLORBG_ATTRIB "colorBG"

#define FONT_SIZE_BITS 4
#define FONT_SIZE_MASK ((1 << FONT_SIZE_BITS)-1)
#define FONTSIZE 10


#define MAXTEXTHEIGHT 130
#define MAXTEXTWIDTH 130
#define MAX_TEXT_CHARS (int)(MAXTEXTHEIGHT*MAXTEXTWIDTH)
#define RENDER_VRAM_SIZE MAX_TEXT_CHARS*6// idk most text characters on screen possible

#define TAB_SPACING 4
#define SS_CHAR_SIZE 0.0625f

typedef struct {
	Config 		*cfg;

	u8           ncursesBgColors[MAX_TEXT_CHARS*6];
	u16          ncursesBgPos[MAX_TEXT_CHARS*6][2];
	u16          ncursespos[MAX_TEXT_CHARS*6][2];
	float        ncursesuv[MAX_TEXT_CHARS*6][2];
	u8           ncursesfgs[MAX_TEXT_CHARS*6];
	u8           ncursesbgs[MAX_TEXT_CHARS*6];
	u8           currentColorPair;
	u8          ncursesAttrPairs[MAX_COLOR_PAIRS][2];
	struct { u32 w; u32 h; } viewport;
	u8           fontSize;
// shaders
	Shader_t     shaders[NUM_SHADERS];
// textured/texturelesss vao/vbos
	u32          vao_g;
	u32          posVbo_g;
	u32          uvVbo_g;
// ncurses vao/vbos
	u32          stringOffset;
	u32          ncursesVao_g;
	u32          ncursesPosVbo_g;
	u32          ncursesUvVbo_g;
	u32          ncursesColorFGVbo_g;
	u32          ncursesColorBGVbo_g;
// ncurses bg vao/vbos
	u32          bgOffset;
	u32         ncursesBgVao_g;
	u32         ncursesBgPosVbo_g;
	u32         ncursesBgColorVbo_g;

// quad vao/vbo
	u32          quadVao_g;
	u32          quadVbo_g;
// framebuffer
	u32          fb_g;
	u32          fbTexture_g;
// textures
	Image_t     font_g;
	FontFace    fontTTF;
} Graphics;


u32 Graphics_TextCollumns(Graphics *g);
u32 Graphics_TextRows(Graphics *g);
u32 Graphics_FontWidth(Graphics *g);
u32 Graphics_FontHeight(Graphics *g);
void Graphics_mvprintw(Graphics *g, float x, float y, char *str, int strLen);
void Graphics_attron(Graphics *g, u32 attr);
void Graphics_init_pair(Graphics *g, u8 pair, u8 fg, u8 bg);
void Graphics_attroff(Graphics *g, u32 attr);
void Graphics_RenderNCurses(Graphics *g);
void Graphics_InitNCurses(Graphics *g);
void Graphics_Zoom(Graphics *g, int by);
void Graphics_Init(Graphics *g, Config *cfg);
void Graphics_Clear(Graphics *g);
void Graphics_Close(Graphics *g);
void Graphics_Resize(Graphics *g, int w, int h);
void Graphics_Render(Graphics *g);
void Graphics_RenderString(Graphics *graphics, char *str, u32 x, u32 y, u8 r, u8 g, u8 b);
void Graphics_RenderRect(Graphics *graphics, float x, float y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a);
void Graphics_RenderRectLines(Graphics *graphics, float x, float y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a);
void Graphics_UseShader(Graphics *graphics, int shader);
void Graphics_RenderSprite(Graphics *graphics, float x, float y, u16 w, u16 h, u16 tx, u16 ty, u16 tw, u16 th, Image_t tex);
void Graphics_RenderRotatedRect(Graphics *graphics, float x, float y, u16 w, u16 h, float rotation, s16 ox, s16 oy, u8 r, u8 g, u8 b, u8 a);
#endif