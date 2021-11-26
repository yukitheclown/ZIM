#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "window.h"

void Config_Read(Config *cfg){
    memset(cfg, 0, sizeof(Config));

    struct{
        int r;
        int g;
        int b;
    } defaultColors[] = { 
        {0,255,255}, //cyan
        {255,0,0},//red
        {255,255,0},//yellow
        {0,0,255},//blue
        {0,255,0}, //green
        {255,0,255}, //magenta purple
        {255,255,255},//fg
        {0,0,0},//bg
        {127,127,127},//grey
        {0,0,0},//bg
    };

    memcpy(&cfg->colorPairs[COLOR_SIDE_NUMBERS-1], (int[]){ COLOR_WHITE, COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_NORMAL-1], (int[]){ COLOR_WHITE, COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_KEYWORD-1], (int[]){ COLOR_CYAN, COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_COMMENT-1], (int[]){ COLOR_GREY, COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_TOKEN-1], (int[]){ COLOR_BLUE, COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_NUM-1], (int[]){ COLOR_RED, COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_FUNCTION-1], (int[]){ COLOR_YELLOW, COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_STRING-1], (int[]){ COLOR_MAGENTA, COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_SELECTED-1], (int[]){ COLOR_BLACK ,COLOR_CYAN }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_SELECTED_DIRECTORY-1], (int[]){ COLOR_RED ,COLOR_CYAN }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_UNSELECTED_DIRECTORY-1], (int[]){ COLOR_RED ,COLOR_WHITE }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_AUTO_COMPLETE-1], (int[]){ COLOR_BLACK, COLOR_WHITE }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_LOG_UNSELECTED-1], (int[]){ COLOR_BLACK, COLOR_WHITE }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_CURSOR-1], (int[]){ COLOR_BLACK ,COLOR_MAGENTA }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_FIND-1], (int[]){ COLOR_BLACK ,COLOR_WHITE }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[COLOR_LINE_NUM-1], (int[]){ COLOR_GREY ,COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[TE_COLOR_BLACK-1], (int[]){ COLOR_BLACK ,COLOR_WHITE }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[TE_COLOR_WHITE-1], (int[]){ COLOR_WHITE ,COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[TE_COLOR_CYAN-1], (int[]){ COLOR_CYAN ,COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[TE_COLOR_RED-1], (int[]){ COLOR_RED ,COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[TE_COLOR_YELLOW-1], (int[]){ COLOR_YELLOW ,COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[TE_COLOR_BLUE-1], (int[]){ COLOR_BLUE ,COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[TE_COLOR_GREEN-1], (int[]){ COLOR_GREEN ,COLOR_BLACK }, sizeof(int)*2);
    memcpy(&cfg->colorPairs[TE_COLOR_MAGENTA-1], (int[]){ COLOR_MAGENTA ,COLOR_BLACK }, sizeof(int)*2);
    //gruvbox


    // aurora
    // {0xa1/255.0f,0xef/255.0f,0xe4/255.0f}, //cyan
    // {0xff/255.0f,0x58/255.0f,0x74/255.0f}, //red
    // {0xec/255.0f,0xc4/255.0f,0x8d/255.0f}, //yellow
    // {0x82/255.0f,0xaa/255.0f,0xf0/255.0f}, //blue
    // {0xad/255.0f,0xdb/255.0f,0x67/255.0f}, //green
    // {0xbd/255.0f,0x93/255.0f,0xf3/255.0f}, //magenta
    // {0xd6/255.0f,0xde/255.0f,0xeb/255.0f}, //white
    // {0x14/255.0f,0x14/255.0f,0x15/255.0f}, //black
    // {0x74/255.0f,0x74/255.0f,0x75/255.0f}, //grey
    // {0x14/255.0f,0x14/255.0f,0x15/255.0f}, //bg


    FILE *fp = fopen(CONFIG_FILE,"r");
    if(fp){

        while(!feof(fp)){

            char lineType[100];
            fscanf(fp, "%s : ", lineType);

            if(lineType[0] == '#'){
                while(!feof(fp) && fgetc(fp) != '\n'){}
                continue;
            }

            if(strcmp(lineType, "COLOR_CYAN") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_CYAN-1].r, &defaultColors[COLOR_CYAN-1].g, &defaultColors[COLOR_CYAN-1].b);
            else if(strcmp(lineType, "COLOR_RED") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_RED-1].r, &defaultColors[COLOR_RED-1].g, &defaultColors[COLOR_RED-1].b);
            else if(strcmp(lineType, "COLOR_YELLOW") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_YELLOW-1].r, &defaultColors[COLOR_YELLOW-1].g, &defaultColors[COLOR_YELLOW-1].b);
            else if(strcmp(lineType, "COLOR_BLUE") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_BLUE-1].r, &defaultColors[COLOR_BLUE-1].g, &defaultColors[COLOR_BLUE-1].b);
            else if(strcmp(lineType, "COLOR_GREEN") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_GREEN-1].r, &defaultColors[COLOR_GREEN-1].g, &defaultColors[COLOR_GREEN-1].b);
            else if(strcmp(lineType, "COLOR_MAGENTA") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_MAGENTA-1].r, &defaultColors[COLOR_MAGENTA-1].g, &defaultColors[COLOR_MAGENTA-1].b);
            else if(strcmp(lineType, "COLOR_WHITE") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_WHITE-1].r, &defaultColors[COLOR_WHITE-1].g, &defaultColors[COLOR_WHITE-1].b);
            else if(strcmp(lineType, "COLOR_BLACK") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_BLACK-1].r, &defaultColors[COLOR_BLACK-1].g, &defaultColors[COLOR_BLACK-1].b);
            else if(strcmp(lineType, "COLOR_GREY") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_GREY-1].r, &defaultColors[COLOR_GREY-1].g, &defaultColors[COLOR_GREY-1].b);
            else if(strcmp(lineType, "COLOR_BG") == 0)
                fscanf(fp, "%x %x %x", &defaultColors[COLOR_BG-1].r, &defaultColors[COLOR_BG-1].g, &defaultColors[COLOR_BG-1].b);


            while(fgetc(fp) != '\n' && !feof(fp)){}
        }
        fclose(fp);
    }


    int k;
    for(k = 0; k < NUM_COLORS-1; k++){
        cfg->colors[k].r = defaultColors[k].r / 255.0f;
        cfg->colors[k].g = defaultColors[k].g / 255.0f;
        cfg->colors[k].b = defaultColors[k].b / 255.0f;
    }

}