#ifdef WINDOWS_COMPILE
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <stdio.h>
#include "types.h"
#include "graphics.h"
#include "utils.h"
#include "log.h"
#include "window.h"
#include "freetype.h"

#define STRDEF(x) STR(x)
#define STR(x) #x

const static u8 RectTriangleVerts[] = {0,0,1,0,1,1,1,1,0,1,0,0};

static void Compile(Graphics *graphics, Shader_t *shader, const char *vSource, const char *fSource){

    shader->program = glCreateProgram();
    shader->fShader = glCreateShader(GL_FRAGMENT_SHADER);
    shader->vShader = glCreateShader(GL_VERTEX_SHADER);
    
    GLint status = GL_TRUE;

    char buffer[512];

    glShaderSource(shader->fShader, 1, (const GLchar **)&fSource, NULL);
    glCompileShader(shader->fShader);
    glGetShaderiv(shader->fShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE){
        glGetShaderInfoLog(shader->fShader, 512, NULL, buffer);
        LOG(LOG_RED, "FSHADER: %s", buffer);
        return;
    }

    glAttachShader(shader->program, shader->fShader);

    glShaderSource(shader->vShader, 1, (const GLchar **)&vSource, NULL);
    glCompileShader(shader->vShader);
    glGetShaderiv(shader->vShader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE){
        glGetShaderInfoLog(shader->vShader, 512, NULL, buffer);
        LOG(LOG_RED, "VSHADER: %s", buffer);
        return;
    }

    glBindAttribLocation(shader->program, POS_LOC, POS_ATTRIB);
    glBindAttribLocation(shader->program, UV_LOC, UV_ATTRIB);
    glBindAttribLocation(shader->program, COLORFG_LOC, COLORFG_ATTRIB);
    glBindAttribLocation(shader->program, COLORBG_LOC, COLORBG_ATTRIB);

    glAttachShader(shader->program, shader->vShader);
    glLinkProgram(shader->program);

    glUseProgram(shader->program);

    shader->uniColorLoc = glGetUniformLocation(shader->program, "uniformColor");
    shader->invViewportLoc = glGetUniformLocation(shader->program, "invViewport");
    shader->ncursesColorsLoc = glGetUniformLocation(shader->program, "colors");
    glUniform3fv(shader->ncursesColorsLoc, NUM_COLORS-1, (float *)&graphics->cfg->colors[0].r);
    glUniform2f(shader->invViewportLoc,1.0f/graphics->viewport.w, 1.0f/graphics->viewport.h);
}

static const char *VS_Source = "#version 120\n"
STR(
attribute vec2 pos;
attribute vec2 uv;
varying vec2 TexCoord;
uniform vec2 invViewport;
)


STR(

void main(){

    TexCoord = uv;
    gl_Position = vec4(((pos * invViewport) * 2) - 1, -1, 1);
}
);

static const char *VSNCursesBG_Source = "#version 120\n"
STR(
attribute vec2 pos;
attribute float colorBG;
varying float ColorOutBG;
uniform vec2 invViewport;
)


STR(

void main(){
    ColorOutBG = colorBG;
    gl_Position = vec4(((pos * invViewport) * 2) - 1, -1, 1);
}
);

static const char *FSNCursesBG_Source = "#version 120\n"
STR(
varying float ColorOutBG;
uniform vec3 colors[16];

void main(){

    gl_FragColor = vec4(colors[int(ColorOutBG)],1);
}
);
static const char *VSNCurses_Source = "#version 120\n"
STR(
attribute vec2 pos;
attribute vec2 uv;
attribute float colorFG;
attribute float colorBG;
varying vec2 TexCoord;
varying float ColorOutFG;
varying float ColorOutBG;
uniform vec2 invViewport;
)


STR(

void main(){

    ColorOutFG = colorFG;
    ColorOutBG = colorBG;
    TexCoord = uv;
    gl_Position = vec4(((pos * invViewport) * 2) - 1, -1, 1);
}
);

static const char *FSNCurses_Source = "#version 120\n"
STR(
varying vec2 TexCoord;
varying float ColorOutFG;
varying float ColorOutBG;
uniform sampler2D tex;
uniform vec3 colors[16];

void main(){

    vec4 color = texture2D(tex, TexCoord);

    if(color.r > 0.4)
        color = vec4(colors[int(ColorOutFG)],1);
    else
        color = vec4(mix(colors[int(ColorOutBG)], colors[int(ColorOutFG)], color.r),1);

    gl_FragColor = color;
}
);

static const char *FS_Source = "#version 120\n"
STR(
varying vec2 TexCoord;
uniform sampler2D tex;
uniform vec4 uniformColor = vec4(1,1,1,1);

void main(){

    vec4 color = texture2D(tex, TexCoord);
    if(color.a < 0.5) discard;

    gl_FragColor = color * vec4(uniformColor.rgb*uniformColor.a, 1);
}
);

static const char *VS_Quad_Source = "#version 120\n"
STR(
attribute vec2 pos;
varying vec2 TexCoord;
)

STR(

void main(){
    TexCoord = pos;
    vec2 screenSpace = vec2((pos.x * 2) - 1,((1-pos.y) * 2) - 1);
    gl_Position = vec4(screenSpace.x, screenSpace.y, -1, 1);
}
);

static const char *FS_Quad_Source = "#version 120\n"
STR(
varying vec2 TexCoord;
uniform sampler2D tex;

void main(){

    vec3 color = texture2D(tex, TexCoord).rgb;

    gl_FragColor = vec4(color, 1);
}
);

static const char *VS_Textureless_Source = "#version 120\n"
STR(
attribute vec2 pos;
uniform vec2 invViewport;
)

STR(

void main(){
    gl_Position = vec4(((pos * invViewport) * 2) - 1, -1, 1);
}
);

static const char *FS_Textureless_Source = "#version 120\n"
STR(
uniform vec4 uniformColor = vec4(1,1,1,1);

void main(){

    gl_FragColor = vec4(uniformColor.rgb*uniformColor.a, 1);
}
);

static void CreateFrameBuffer(Graphics *graphics){

    glGenFramebuffers(1,&graphics->fb_g);
    glBindFramebuffer(GL_FRAMEBUFFER, graphics->fb_g);

    glGenTextures(1, &graphics->fbTexture_g);
    glBindTexture(GL_TEXTURE_2D, graphics->fbTexture_g);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, graphics->viewport.w, graphics->viewport.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, graphics->fbTexture_g, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        glDeleteFramebuffers(1, &graphics->fb_g);
        glDeleteTextures(1, &graphics->fbTexture_g);
        LOG(LOG_RED, "Error creating framebuffer.");
    }

    glViewport(0, 0, graphics->viewport.w, graphics->viewport.h);

    glClearColor(graphics->cfg->colors[COLOR_BG-1].r,graphics->cfg->colors[COLOR_BG-1].g,graphics->cfg->colors[COLOR_BG-1].b,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics_Init(Graphics *graphics, Config *cfg){
    memset(graphics, 0, sizeof(Graphics));
    graphics->cfg = cfg;
    graphics->fontSize = FONTSIZE;

    graphics->viewport.w = WINDOW_INIT_WIDTH;
    graphics->viewport.h = WINDOW_INIT_HEIGHT;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_POLYGON_SMOOTH);
	    
   
    Text_Init();
	
    // textures

    // Utils_LoadImage(&graphics->font_g, FONT_PATH);
    FontFace_LoadFont(&graphics->fontTTF, FONT_PATH_TTF);
    FontFace_SetSize(&graphics->fontTTF, graphics->fontSize);
    // int k;
    // int x = 0;
    // for(k = 0; k < 128; k++){
    //     graphics->fontTTF.fontCharacters[k] = {
    //         .ax = 0,
    //         .ay = 0,
    //         .bw = graphics->fontSize,
    //         .bh = graphics->fontSize,
    //         .bl = 0,
    //         .bt = 0,
    //         .tx = (float)x%16 * graphics->fontTTF.atlasWidth;
    //         .ty = (float)x/16 * graphics->fontTTF.atlasHeight;
    //     }
    //     x += graphics->bitmap.width;
    // }

    // compile graphics->shaders
    Compile(graphics, &graphics->shaders[TEXTURED_SHADER], VS_Source, FS_Source);
    Compile(graphics, &graphics->shaders[QUAD_SHADER], VS_Quad_Source, FS_Quad_Source);
    Compile(graphics, &graphics->shaders[NCURSES_SHADER], VSNCurses_Source, FSNCurses_Source);
    Compile(graphics, &graphics->shaders[NCURSES_BG_SHADER], VSNCursesBG_Source, FSNCursesBG_Source);
    Compile(graphics, &graphics->shaders[TEXTURELESS_SHADER], VS_Textureless_Source, FS_Textureless_Source);


    glGenVertexArrays(1, &graphics->quadVao_g);
    glBindVertexArray(graphics->quadVao_g);

    glGenBuffers(1, &graphics->quadVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->quadVbo_g);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RectTriangleVerts), RectTriangleVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 2, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);

    // ncurses vao

    glGenVertexArrays(1, &graphics->ncursesVao_g);
    glBindVertexArray(graphics->ncursesVao_g);

    glGenBuffers(1, &graphics->ncursesPosVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesPosVbo_g);
    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 2, GL_SHORT, GL_FALSE, 0, 0);

    glGenBuffers(1, &graphics->ncursesColorBGVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesColorBGVbo_g);
    glEnableVertexAttribArray(COLORFG_LOC);
    glVertexAttribPointer(COLORFG_LOC, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);

    glGenBuffers(1, &graphics->ncursesColorFGVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesColorFGVbo_g);
    glEnableVertexAttribArray(COLORBG_LOC);
    glVertexAttribPointer(COLORBG_LOC, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);

    glGenBuffers(1, &graphics->ncursesUvVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesUvVbo_g);
    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    // ncurses vao backgrounds
    glGenVertexArrays(1, &graphics->ncursesBgVao_g);
    glBindVertexArray(graphics->ncursesBgVao_g);

    glGenBuffers(1, &graphics->ncursesBgPosVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesBgPosVbo_g);
    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 2, GL_SHORT, GL_FALSE, 0, 0);

    glGenBuffers(1, &graphics->ncursesBgColorVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesBgColorVbo_g);
    glEnableVertexAttribArray(COLORBG_LOC);
    glVertexAttribPointer(COLORBG_LOC, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);

    // textured/textureless vao/vbos

    glGenVertexArrays(1, &graphics->vao_g);
    glBindVertexArray(graphics->vao_g);

    glGenBuffers(1, &graphics->posVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->posVbo_g);
    glBufferData(GL_ARRAY_BUFFER, sizeof(u16)*2*RENDER_VRAM_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 2, GL_SHORT, GL_FALSE, 0, 0);

    glGenBuffers(1, &graphics->uvVbo_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->uvVbo_g);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*RENDER_VRAM_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // init framebuffer
    
    CreateFrameBuffer(graphics);

    glBindFramebuffer(GL_FRAMEBUFFER, graphics->fb_g);
}

static void DeleteShader(Shader_t *shader){
    glDeleteProgram(shader->program);
    glDeleteShader(shader->fShader);
    glDeleteShader(shader->vShader);
}

void Graphics_Close(Graphics *graphics){

    FontFace_Delete(&graphics->fontTTF);
    Text_Close();

    glDeleteFramebuffers(1, &graphics->fb_g);
    glDeleteTextures(1, &graphics->fbTexture_g);
    
    DeleteShader(&graphics->shaders[TEXTURED_SHADER]);
    DeleteShader(&graphics->shaders[TEXTURELESS_SHADER]);
    DeleteShader(&graphics->shaders[NCURSES_SHADER]);

    glDeleteVertexArrays(1, &graphics->quadVao_g);
    glDeleteBuffers(1, &graphics->quadVbo_g);

    glDeleteVertexArrays(1, &graphics->vao_g);
    glDeleteBuffers(1, &graphics->uvVbo_g);
    glDeleteBuffers(1, &graphics->posVbo_g);

    glDeleteVertexArrays(1, &graphics->ncursesPosVbo_g);
    glDeleteBuffers(1, &graphics->ncursesUvVbo_g);
    glDeleteBuffers(1, &graphics->ncursesColorFGVbo_g);
    glDeleteBuffers(1, &graphics->ncursesColorBGVbo_g);

    glDeleteTextures(1, &graphics->font_g.texture);
}

void Graphics_Resize(Graphics *graphics, int w, int h){
    glBindTexture(GL_TEXTURE_2D, graphics->fbTexture_g);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    graphics->viewport.w = w;
    graphics->viewport.h = h;
}

void Graphics_Zoom(Graphics *graphics, int by){
    if(by < 0 && graphics->fontSize > MINFONTSIZE) graphics->fontSize += by;
    if(by > 0 && graphics->fontSize < MAXFONTSIZE) graphics->fontSize += by;
    FontFace_SetSize(&graphics->fontTTF, graphics->fontSize);
}

void Graphics_Render(Graphics *graphics){

    // render

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindVertexArray(graphics->quadVao_g);

    glViewport(0, 0, graphics->viewport.w, graphics->viewport.h);

    glUseProgram(graphics->shaders[QUAD_SHADER].program);
    glUniform2f(graphics->shaders[QUAD_SHADER].invViewportLoc,1.0f, 1.0f);

    glCullFace(GL_FRONT);

    glBindTexture(GL_TEXTURE_2D, graphics->fbTexture_g);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // // end
}

void Graphics_Clear(Graphics *graphics){
    glBindFramebuffer(GL_FRAMEBUFFER, graphics->fb_g);
    glViewport(0, 0, graphics->viewport.w, graphics->viewport.h);

    glClearColor(graphics->cfg->colors[COLOR_BG-1].r,graphics->cfg->colors[COLOR_BG-1].g,graphics->cfg->colors[COLOR_BG-1].b,1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glCullFace(GL_BACK);
}

void Graphics_RenderSprite(Graphics *graphics, float x, float y, u16 w, u16 h, u16 tx, u16 ty, u16 tw, u16 th, Image_t tex){

    glBindVertexArray(graphics->vao_g);

    u32 offset = 0;
    u16 pos[2];

    float uv[2];

    u32 k;
    for(k = 0; k < 12; k+=2){

        pos[0] = x + ((s16)RectTriangleVerts[k] * w);
        pos[1] = y + ((s16)RectTriangleVerts[k+1] * h);

        uv[0] = ((float)(RectTriangleVerts[k] * tw) + tx) * tex.invW;
        uv[1] = ((float)(RectTriangleVerts[k+1] * th) + ty) * tex.invH;

        glBindBuffer(GL_ARRAY_BUFFER, graphics->posVbo_g);
        glBufferSubData(GL_ARRAY_BUFFER, offset*sizeof(pos), sizeof(pos), pos);
        glBindBuffer(GL_ARRAY_BUFFER, graphics->uvVbo_g);
        glBufferSubData(GL_ARRAY_BUFFER, offset*sizeof(uv), sizeof(uv), uv);
    
        ++offset;
    }


    glUseProgram(graphics->shaders[TEXTURED_SHADER].program);
    glUniform2f(graphics->shaders[TEXTURED_SHADER].invViewportLoc, 1.0f/graphics->viewport.w, 1.0f/graphics->viewport.h); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.texture);

    glBindVertexArray(graphics->vao_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->posVbo_g);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Graphics_RenderRect(Graphics *graphics, float x, float y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a){

    glBindFramebuffer(GL_FRAMEBUFFER, graphics->fb_g);
    glBindVertexArray(graphics->vao_g);

    u32 offset = 0;
    u16 pos[2];

    u32 k;
    for(k = 0; k < 12; k+=2){
        pos[0] = x + ((s16)RectTriangleVerts[k] * w);
        pos[1] = y + ((s16)RectTriangleVerts[k+1] * h);

        glBindBuffer(GL_ARRAY_BUFFER, graphics->posVbo_g);
        glBufferSubData(GL_ARRAY_BUFFER, offset*sizeof(pos), sizeof(pos), pos);
        // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RectTriangleVerts), RectTriangleVerts);
    
        ++offset;
    }


    glUseProgram(graphics->shaders[TEXTURELESS_SHADER].program);
    glUniform4f(graphics->shaders[TEXTURELESS_SHADER].uniColorLoc, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); 
    glUniform2f(graphics->shaders[TEXTURELESS_SHADER].invViewportLoc, 1.0f/graphics->viewport.w, 1.0f/graphics->viewport.h); 

    glBindVertexArray(graphics->vao_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->posVbo_g);

    glDisableVertexAttribArray(UV_LOC);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnableVertexAttribArray(UV_LOC);

    glUniform4f(graphics->shaders[TEXTURELESS_SHADER].uniColorLoc, 1, 1, 1, 1); 
}

void Graphics_RenderRectLines(Graphics *graphics, float x, float y, u16 w, u16 h, u8 r, u8 g, u8 b, u8 a){

    glBindVertexArray(graphics->vao_g);

    u16 buffer[2*4];

    buffer[0] = x;
    buffer[1] = y;
    buffer[2] = x + w;
    buffer[3] = y;
    buffer[4] = x + w;
    buffer[5] = y + h;
    buffer[6] = x;
    buffer[7] = y + h;

    glBindBuffer(GL_ARRAY_BUFFER, graphics->posVbo_g);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(buffer), buffer);


    glUseProgram(graphics->shaders[TEXTURELESS_SHADER].program);
    glUniform4f(graphics->shaders[TEXTURELESS_SHADER].uniColorLoc, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); 
    glUniform2f(graphics->shaders[TEXTURELESS_SHADER].invViewportLoc, 1.0f/graphics->viewport.w, 1.0f/graphics->viewport.h); 

    glBindVertexArray(graphics->vao_g);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->posVbo_g);

    glDisableVertexAttribArray(UV_LOC);

    glDrawArrays(GL_LINE_STRIP, 0, 4);

    glEnableVertexAttribArray(UV_LOC);

    glUniform4f(graphics->shaders[TEXTURELESS_SHADER].uniColorLoc, 1, 1, 1, 1); 
}

void Graphics_UseShader(Graphics *graphics, int shader){
    glUseProgram(graphics->shaders[shader].program);
}

u32 Graphics_FontWidth(Graphics *graphics){ return graphics->fontTTF.fontCharacters[(int)' '].ax; }
u32 Graphics_FontHeight(Graphics *graphics){ return graphics->fontTTF.fontSize*1.3; }

u32 Graphics_TextCollumns(Graphics *graphics){
   return graphics->viewport.h / Graphics_FontHeight(graphics); 
}
u32 Graphics_TextRows(Graphics *graphics){
   return graphics->viewport.w / Graphics_FontWidth(graphics);
}

void Graphics_SetFontSize(Graphics *graphics, u8 fs){
    graphics->fontSize = fs;
}

void Graphics_mvprintw(Graphics *graphics, float x, float y, char *str, int strLen){


    float fontSizeX = Graphics_FontWidth(graphics);
    float fontSizeY = Graphics_FontHeight(graphics);
    u32 k;

    // float graphics->fontSize = graphics->font_g.width / 16;
    x *= fontSizeX;
    y++;
    y *= fontSizeY;

    char p;

    int j;
    for(j = 0; j < strLen && graphics->stringOffset < MAX_TEXT_CHARS*6; j++){

        if(x > graphics->viewport.w) break;
        if(y > graphics->viewport.h) break;

        p = str[j];
        // if(p < 32) continue;

        FontCharacter fc = graphics->fontTTF.fontCharacters[(int)p];

        float x2 = x + fc.bl;
        float y2 = y - fc.bt;
        float w = fc.bw;
        float h = fc.bh;

        // x += fc.ax;
        // y += fc.ay;

        if(graphics->cfg->colorPairs[graphics->currentColorPair][0] != COLOR_BLACK){
            for(k = 0; k < 12; k+=2){
                graphics->ncursesBgPos[graphics->bgOffset][0] = ((RectTriangleVerts[k] * fontSizeX) + x);
                graphics->ncursesBgPos[graphics->bgOffset][1] = ((1-RectTriangleVerts[k+1] * fontSizeY) + y);
                graphics->ncursesBgColors[graphics->bgOffset] = graphics->cfg->colorPairs[graphics->currentColorPair][0];
                ++graphics->bgOffset;
            }
        }

        x += fontSizeX;

        if(!w || !h) continue;

        // float tX = (p % 16) / 16.0f;
        //float tY = (1 - graphics->fontTTF.atlasWidth) - (floorf(p / 16.0f) / 16.0f);
        float tX = fc.tx;
        float tY = 0;

        float ah = graphics->fontTTF.atlasHeight;
        float aw = graphics->fontTTF.atlasWidth;


        for(k = 0; k < 12; k+=2){

                // graphics->ncursesuv[graphics->stringOffset][0] = ((RectTriangleVerts[k] * SS_CHAR_SIZE) + tX);
                // graphics->ncursesuv[graphics->stringOffset][1] = 1 - ((RectTriangleVerts[k+1] * SS_CHAR_SIZE) + tY);
                graphics->ncursesuv[graphics->stringOffset][0] = ((RectTriangleVerts[k] * (fc.bw / aw)) + tX);
                graphics->ncursesuv[graphics->stringOffset][1] = (((1-RectTriangleVerts[k+1]) * ((float)fc.bh / ah)) + tY);

                graphics->ncursespos[graphics->stringOffset][0] = (RectTriangleVerts[k] * w) + x2;
                graphics->ncursespos[graphics->stringOffset][1] = ((1-RectTriangleVerts[k+1]) * h) + y2;
                graphics->ncursesfgs[graphics->stringOffset] = graphics->cfg->colorPairs[graphics->currentColorPair][0];
                graphics->ncursesbgs[graphics->stringOffset] = graphics->cfg->colorPairs[graphics->currentColorPair][1];

                ++graphics->stringOffset;
        }

        // x += graphics->fontSize;
    }

}

void Graphics_RenderNCurses(Graphics *graphics){

    glUseProgram(graphics->shaders[NCURSES_BG_SHADER].program);
    glUniform2f(graphics->shaders[NCURSES_BG_SHADER].invViewportLoc, 1.0f/graphics->viewport.w, 1.0f/graphics->viewport.h); 

    glBindVertexArray(graphics->ncursesBgVao_g);
    glCullFace(GL_FRONT);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesBgPosVbo_g);
    glBufferData(GL_ARRAY_BUFFER, graphics->bgOffset*2*sizeof(u16), &graphics->ncursesBgPos[0][0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesBgColorVbo_g);
    glBufferData(GL_ARRAY_BUFFER, graphics->bgOffset, &graphics->ncursesBgColors[0], GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, graphics->bgOffset);

    graphics->bgOffset = 0;

    glUseProgram(graphics->shaders[NCURSES_SHADER].program);
    glUniform2f(graphics->shaders[NCURSES_SHADER].invViewportLoc, 1.0f/graphics->viewport.w, 1.0f/graphics->viewport.h); 


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, graphics->fontTTF.fontTexture);
    // glBindTexture(GL_TEXTURE_2D, graphics->font_g.texture);

    glBindVertexArray(graphics->ncursesVao_g);

    // use instancing.
    glCullFace(GL_FRONT);

    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesPosVbo_g);
    glBufferData(GL_ARRAY_BUFFER, graphics->stringOffset*2*sizeof(u16), &graphics->ncursespos[0][0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesUvVbo_g);
    glBufferData(GL_ARRAY_BUFFER, graphics->stringOffset*2*sizeof(float), &graphics->ncursesuv[0][0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesColorFGVbo_g);
    glBufferData(GL_ARRAY_BUFFER, graphics->stringOffset, &graphics->ncursesfgs[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->ncursesColorBGVbo_g);
    glBufferData(GL_ARRAY_BUFFER, graphics->stringOffset, &graphics->ncursesbgs[0], GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, graphics->stringOffset);

    graphics->stringOffset = 0;

}

void Graphics_attron(Graphics *graphics, u32 attr){
    graphics->currentColorPair = attr;
}

void Graphics_attroff(Graphics *graphics, u32 attr){
}

void Graphics_init_pair(Graphics *graphics, u8 pair, u8 bg, u8 fg){
    graphics->cfg->colorPairs[pair][0] = fg-1;
    graphics->cfg->colorPairs[pair][1] = bg-1;
}
