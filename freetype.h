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
} FontCharacter;

typedef struct {
	unsigned int     fontTexture;
	FT_Face          fontFace;
	float            atlasWidth;
	float            atlasHeight;
	int              fontSize;
	int              fontHeight;
	FontCharacter    fontCharacters[128];
} FontFace;

// Vec2 FontFace_GetTextSize(FontFace *font, const char *text, int len, float sx, float sy);
void Text_Close();
void Text_Init();
void FontFace_Delete(FontFace *font);
int FontFace_LoadFont(FontFace *font, const char *path);
int FontFace_SetSize(FontFace *font, int size);

#endif