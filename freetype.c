#ifdef WINDOWS_COMPILE
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <stdio.h>
#include "freetype.h"

static FT_Library  ftLibrary;

int Thoth_FontFace_LoadFont(Thoth_FontFace *font, const char *path){

    font->fontFace = NULL;
    font->fontTexture = 0;

    if(FT_New_Face(ftLibrary, path, 0, &font->fontFace)){
        printf("Thoth_FontFace_LoadFont: Could not load font.\n");
        return 0;
    }

    return 1;
}

int Thoth_FontFace_SetSize(Thoth_FontFace *font, int size){

    font->fontSize = size;

    FT_Set_Pixel_Sizes(font->fontFace, 0, size);
    // FT_Set_Char_Size(font->fontFace, size << 6, size << 6, 96, 96);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &font->fontTexture);
    glBindTexture(GL_TEXTURE_2D, font->fontTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    int w = 0, h = 0, i = 0, x = 0;

    FT_GlyphSlot g = font->fontFace->glyph;

    for(i = 32; i < 128; i++){
        if(FT_Load_Char(font->fontFace, i, FT_LOAD_RENDER)){
            printf("Thoth_FontFace_SetSize: Error \n");
            continue;
        }
        w += g->bitmap.width;
        h  = h > g->bitmap.rows ? h : g->bitmap.rows;
    }

    font->fontHeight = h;

    font->atlasWidth  = w;
    font->atlasHeight = h;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,w,h,0,GL_RED,GL_UNSIGNED_BYTE,0);

    for(i = 32; i < 128; i++){

        if(FT_Load_Char(font->fontFace, i, FT_LOAD_RENDER)){
            printf("Thoth_FontFace_SetSize: Error \n");
            continue;
        }

        // unsigned char data[g->bitmap.width * g->bitmap.rows];

        // int y, b;
        // for(y = 0; y < g->bitmap.rows; y++){
        //     for(b = 0; b < g->bitmap.pitch; b++){

        //         int val = g->bitmap.buffer[(y * g->bitmap.pitch) + b];

        //         int len = g->bitmap.width - (b * 8);

        //         int rowStart = (y * g->bitmap.width) + (b * 8);

        //         int k;

        //         for(k = 0; k < (len > 8 ? 8: len); k++){

        //             int bit = val & (1 << (7 - k));

        //             data[(rowStart + k)] = bit > 0 ? 255 : 0;
        //         }
        //     }
        // }

        // glTexSubImage2D(GL_TEXTURE_2D,0,x,0,g->bitmap.width,g->bitmap.rows,GL_RED,GL_UNSIGNED_BYTE,data);

        glTexSubImage2D(GL_TEXTURE_2D,0,x,0,g->bitmap.width,g->bitmap.rows,GL_RED,GL_UNSIGNED_BYTE,g->bitmap.buffer);

        font->fontCharacters[i].ax = g->advance.x >> 6;
        font->fontCharacters[i].ay = g->advance.y >> 6;
        font->fontCharacters[i].bw = g->bitmap.width;
        font->fontCharacters[i].bh = g->bitmap.rows;
        font->fontCharacters[i].bl = g->bitmap_left;
        font->fontCharacters[i].bt = g->bitmap_top;
        font->fontCharacters[i].tx = ((float)x / (float)w);
        x += g->bitmap.width;
    }
    return 1;
}


// Vec2 Thoth_FontFace_GetTextSize(Thoth_FontFace *font, const char *text, int len, float sx, float sy){

    // if(!text) return (Vec2){0,font->fontSize * sy};

    // Vec2 cursorPos = (Vec2){0,font->fontSize * sy};

    // float lineWidth = 0;

    // int k;
    // for(k = 0; k < len; k++){

    //     char p = text[k];

    //     if(text[k] == '\n'){
    //         cursorPos.y += font->fontSize * sy;
    //         if(lineWidth > cursorPos.x) cursorPos.x = lineWidth;
    //         lineWidth = 0;
    //         continue;
    //     }
        
    //     if(p == '\t' ){
    //         lineWidth += font->fontCharacters[(int)' '].ax * sx * TAB_SPACING;
    //         continue;
    //     }

    //     lineWidth += sx * font->fontCharacters[(int)text[k]].ax;
    // }

    // if(lineWidth > cursorPos.x) cursorPos.x = lineWidth;

    // return cursorPos;
// }

void Thoth_FontFace_Delete(Thoth_FontFace *font){
    if(font->fontTexture != 0) glDeleteTextures(1,&font->fontTexture);
    if(font->fontFace) FT_Done_Face(font->fontFace);
}

void Thoth_Text_Close(){
    if(ftLibrary) FT_Done_FreeType(ftLibrary);
    ftLibrary = NULL;
}

void Thoth_Text_Init(){
    if(FT_Init_FreeType(&ftLibrary)) printf("Could not Init Freetype.\n");
}
