#ifndef GRAPHICS_DEF
#define GRAPHICS_DEF

#include "config.h"
#include "types.h"
#include "freetype.h"


typedef struct {
	u32 	program;
	u32 	fShader;
	u32 	vShader;
	u32		uniColorLoc;
	u32		invViewportLoc;
	u32		ncursesColorsLoc;
} Thoth_Shader;


enum {
	THOTH_TEXTURELESS_SHADER=0,
	THOTH_TEXTURED_SHADER,
	THOTH_QUAD_SHADER,
	THOTH_NCURSES_SHADER,
	THOTH_NCURSES_BG_SHADER,
	THOTH_NUM_SHADERS
};

#define THOTH_MAX_COLOR_PAIRS 32

#ifdef LINUX_INSTALL
// #define THOTH_FONT_PATH "/usr/local/share/thoth/font.png"
#define THOTH_FONT_PATH_TTF "/usr/local/share/thoth/Monoid-Regular.ttf"
#else
// #define THOTH_FONT_PATH "resources/font.png"
#define THOTH_FONT_PATH_TTF "resources/Monoid-Regular.ttf"
#endif


// font size 10, 1920x1080, max chars on screen is 11505
// but nobody programs like that lol
#define THOTH_MAX_TEXT_CHARS 10000

typedef struct {
	Thoth_Config 		*cfg;

	u8           ncursesBgColors[THOTH_MAX_TEXT_CHARS*6];
	u16          ncursesBgPos[THOTH_MAX_TEXT_CHARS*6][2];
	u16          ncursespos[THOTH_MAX_TEXT_CHARS*6][2];
	float        ncursesuv[THOTH_MAX_TEXT_CHARS*6][2];
	u8           ncursesfgs[THOTH_MAX_TEXT_CHARS*6];
	u8           ncursesbgs[THOTH_MAX_TEXT_CHARS*6];
	u8           currentColorPair;
	u8          ncursesAttrPairs[THOTH_MAX_COLOR_PAIRS][2];
	struct { u32 x; u32 y; u32 w; u32 h; } viewport;
	u8           fontSize;
// shaders
	Thoth_Shader     shaders[THOTH_NUM_SHADERS];
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
	Thoth_FontFace    fontTTF;
} Thoth_Graphics;


u32 Thoth_Graphics_TextCollumns(Thoth_Graphics *g);
u32 Thoth_Graphics_TextRows(Thoth_Graphics *g);
u32 Thoth_Graphics_FontWidth(Thoth_Graphics *g);
u32 Thoth_Graphics_FontHeight(Thoth_Graphics *g);
void Thoth_Graphics_mvprintw(Thoth_Graphics *g, float x, float y, char *str, int strLen);
void Thoth_Graphics_attron(Thoth_Graphics *g, u32 attr);
void Thoth_Graphics_init_pair(Thoth_Graphics *g, u8 pair, u8 fg, u8 bg);
void Thoth_Graphics_attroff(Thoth_Graphics *g, u32 attr);
void Thoth_Graphics_RenderNCurses(Thoth_Graphics *g);
void Thoth_Graphics_InitNCurses(Thoth_Graphics *g);
void Thoth_Graphics_Zoom(Thoth_Graphics *g, int by);
void Thoth_Graphics_Init(Thoth_Graphics *graphics, Thoth_Config *cfg, int w, int h);
void Thoth_Graphics_Clear(Thoth_Graphics *g);
void Thoth_Graphics_Close(Thoth_Graphics *g);
void Thoth_Graphics_Resize(Thoth_Graphics *g, int w, int h);
void Thoth_Graphics_Render(Thoth_Graphics *g);
void Thoth_Graphics_RenderString(Thoth_Graphics *graphics, char *str, u32 x, u32 y, u8 r, u8 g, u8 b);
void Thoth_Graphics_RenderRect(Thoth_Graphics *graphics, float x, float y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a);
void Thoth_Graphics_RenderRectLines(Thoth_Graphics *graphics, float x, float y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a);
void Thoth_Graphics_UseShader(Thoth_Graphics *graphics, int shader);
void Thoth_Graphics_RenderRotatedRect(Thoth_Graphics *graphics, float x, float y, u16 w, u16 h, float rotation, s16 ox, s16 oy, u8 r, u8 g, u8 b, u8 a);
void Thoth_Graphics_ViewportXY(Thoth_Graphics *graphics, int x, int y);

#endif