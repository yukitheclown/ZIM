#ifndef FREETYPE_DEF
#define FREETYPE_DEF
#include "math.h"
#include <ft2build.h>
#include <string.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include FT_FREETYPE_H

typedef struct {
    float ax;
    float ay;
    float bw;
    float bh;
    float bl;
    float bt;
    float tx;
    float ty;
} Thoth_FontCharacter;

typedef struct {
	unsigned int     fontTexture;
	FT_Face          fontFace;
	float            atlasWidth;
	float            atlasHeight;
	int              fontSize;
	int              fontHeight;
	Thoth_FontCharacter    fontCharacters[128];
} Thoth_FontFace;

// Vec2 FontFace_GetTextSize(FontFace *font, const char *text, int len, float sx, float sy);
void Thoth_Text_Close();
void Thoth_Text_Init();
void Thoth_FontFace_Delete(Thoth_FontFace *font);
int Thoth_FontFace_LoadFont(Thoth_FontFace *font, const char *path);
int Thoth_FontFace_SetSize(Thoth_FontFace *font, int size);

#endif