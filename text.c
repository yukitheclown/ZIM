#include <GL/glew.h>
#include "text.h"
#include "images.h"
#include "window.h"
#include "utils.h"
#include "types.h"

#define MAX_DRAW_CHARACTERS 512
#define TAB_SPACING 4
#define SS_CHAR_SIZE 0.0625f

static const struct {
    struct { float x; float y; } coord;
    struct { float x; float y; } pos;
} squareData[6] = {
    {{-0.5f, -0.5f }, { 0, 1 }},
    {{-0.5f, 0.5f }, { 0, 0 }},
    {{0.5f, 0.5f}, { 1, 0}},
    {{0.5f, 0.5f}, { 1, 0}},
    {{0.5f, -0.5f}, { 1, 1 }},
    {{-0.5f, -0.5f}, { 0, 1 }},
};

static u32 vao, positionVbo, uvVbo;
static Image fontTexture;

void Text_Close(void){

    glDeleteTextures(1, &fontTexture.texture);
    glDeleteBuffers(1, &positionVbo);
    glDeleteBuffers(1, &uvVbo);
    glDeleteVertexArrays(1, &vao);
}

void Text_Init(void){

    Utils_LoadImage(&fontTexture, IMAGE_FONT, GL_NEAREST, 4);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);

    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * MAX_DRAW_CHARACTERS * 6, NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &positionVbo);
    glBindBuffer(GL_ARRAY_BUFFER, positionVbo);
    
    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * MAX_DRAW_CHARACTERS * 6, NULL, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

static int ValidateXY(int *x, int *y, int fontSize, int startX, int vSpacing, int maxWidth){
    
    // bottom width is the smaller one.
    if(*x+fontSize >= maxWidth - fontSize){

        *y += (fontSize + vSpacing);
        *x = startX;

        // top and bottom height are the same.
        if(*y+fontSize >= WINDOW_INIT_HEIGHT)
            return -1;
    }

    return 1;
}

void Text_Draw(int x, int y, int hSpacing, int vSpacing, int maxWidth, const char *text){


    float fontSize = fontTexture.width / 16;

    float startX = x;

    int num = 0;

    int index = 0;

    glBindVertexArray(vao);

    while(1){

        int p = text[index++];

        if(!p) break;

        if(p == '\n'){

            y += (fontSize + vSpacing);
            
            x = startX;

            // top and bottom height are the same.
            if(y+fontSize >= WINDOW_INIT_HEIGHT)
                break;

            continue;
        
        } else if(p == '\t' ){
            
            x += (fontSize + hSpacing) * TAB_SPACING;
            
            if(ValidateXY(&x, &y, fontSize, startX, vSpacing, maxWidth) < 0) break;

            continue;

        } else if(p < 32){

            break;
        
        } else if(p == 32){

            x += (fontSize + hSpacing);

            if(ValidateXY(&x, &y, fontSize, startX, vSpacing, maxWidth) < 0) break;

            continue;
        }

        // p -= 32;

        float tX = (p % 16) / 16.0f;
        float tY = (1 - SS_CHAR_SIZE) - (floorf(p / 16.0f) / 16.0f);

        int k;
        for(k = 0; k < 6; k++){

            Vec2 pos, uv;
            
            uv.x = (squareData[k].coord.x * SS_CHAR_SIZE) + tX;
            uv.y = (squareData[k].coord.y * SS_CHAR_SIZE) + tY;
            pos.x = (squareData[k].pos.x * fontSize) + x;
            pos.y = (squareData[k].pos.y * fontSize) + y;

            glBindBuffer(GL_ARRAY_BUFFER, positionVbo);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vec2) * ((6 * num) + k), sizeof(Vec2), &pos.x);
            glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vec2) * ((6 * num) + k), sizeof(Vec2), &uv.x);
        }

        x += (fontSize + hSpacing);

        ValidateXY(&x, &y, fontSize, startX, vSpacing, maxWidth);

        ++num;

        if(num >= MAX_DRAW_CHARACTERS)
            break;

    }

    glUseProgram(Shaders_GetProgram(PROGRAM_2d));

    glBindTexture(GL_TEXTURE_2D, fontTexture.texture);

    glDrawArrays(GL_TRIANGLES, 0, 6 * num);
}