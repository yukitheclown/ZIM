#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "thoth.h"
#include "window.h"

void Thoth_Config_Read(Thoth_Config *cfg){
	memset(cfg, 0, sizeof(Thoth_Config));
	struct{
	    int r;
	    int g;
	    int b;
	} defaultColors[] = { 
		{0,255,255}, // cyan
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
	
	cfg->keybinds[THOTH_MoveLines_UP] = THOTH_CTRL_KEY|THOTH_SHIFT_KEY|THOTH_ARROW_UP;
	cfg->keybinds[THOTH_MoveLines_DOWN] = THOTH_CTRL_KEY|THOTH_SHIFT_KEY|THOTH_ARROW_DOWN;
	cfg->keybinds[THOTH_OpenFileBrowser] = THOTH_CTRL_KEY|THOTH_SHIFT_KEY|'o';
	cfg->keybinds[THOTH_OpenFileZim] = THOTH_CTRL_KEY|'o';
	cfg->keybinds[THOTH_NewFile] = THOTH_CTRL_KEY|'n';
	cfg->keybinds[THOTH_CloseFile] = THOTH_CTRL_KEY|'w';
	cfg->keybinds[THOTH_SwitchFile] = THOTH_CTRL_KEY|'p';
	cfg->keybinds[THOTH_SaveAsFile] = THOTH_CTRL_KEY|THOTH_SHIFT_KEY|'s';
	cfg->keybinds[THOTH_SaveFile] = THOTH_CTRL_KEY|'s';
	cfg->keybinds[THOTH_ToggleComment] = THOTH_CTRL_KEY|'/';
	cfg->keybinds[THOTH_MoveBrackets] = THOTH_CTRL_KEY|'m';
	cfg->keybinds[THOTH_SelectBrackets] = THOTH_CTRL_KEY|THOTH_SHIFT_KEY|'j';
	cfg->keybinds[THOTH_GotoLine] = THOTH_CTRL_KEY|'g';
	cfg->keybinds[THOTH_FindTextInsensitive] = THOTH_CTRL_KEY|'f';
	cfg->keybinds[THOTH_FindTextZim] = THOTH_CTRL_KEY|THOTH_SHIFT_KEY|'f';
	cfg->keybinds[THOTH_EventCtrlEnter] = THOTH_ENTER_KEY|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_SelectNextWord] = 	'd'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_AddCursorCommand_UP] = THOTH_ARROW_UP|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_AddCursorCommand_DOWN] = THOTH_ARROW_DOWN|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_ExpandSelectionLines] = 	'l'|THOTH_SHIFT_KEY|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_DeleteLine] = 'k'|THOTH_SHIFT_KEY|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_MoveByChars_BACK] = 	'h'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_MoveByChars_FORWARD] = 	'l'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_MoveLines_UP] = 	'j'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_MoveLines_DOWN] = 	'k'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_MoveByWords_BACK] = 'h'|THOTH_ALT_KEY|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_MoveByWords_FORWARD] = 'l'|THOTH_ALT_KEY|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_IndentLine_FORWARD] = ']'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_IndentLine_BACK] = '['|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_ExpandSelectionWords_BACK] = THOTH_ARROW_LEFT|THOTH_SHIFT_KEY|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_ExpandSelectionWords_FORWARD] = THOTH_ARROW_RIGHT|THOTH_SHIFT_KEY|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_ScrollScreen_UP] = THOTH_ARROW_UP|THOTH_SHIFT_KEY;
	cfg->keybinds[THOTH_ScrollScreen_DOWN] = THOTH_ARROW_DOWN|THOTH_SHIFT_KEY;
	cfg->keybinds[THOTH_MoveByChars_BACK] = THOTH_ARROW_LEFT;
	cfg->keybinds[THOTH_MoveByChars_FORWARD] = THOTH_ARROW_RIGHT;
	cfg->keybinds[THOTH_MoveLines_UP] = THOTH_ARROW_UP;
	cfg->keybinds[THOTH_MoveLines_DOWN] = THOTH_ARROW_DOWN;
	cfg->keybinds[THOTH_SelectAll] = THOTH_CTRL_KEY|'a';
	cfg->keybinds[THOTH_Undo] = 	'z'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_Redo] = 	'y'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_Cut] = 	'x'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_Copy] = 	'c'|THOTH_CTRL_KEY;
	cfg->keybinds[THOTH_Paste] ='v'|THOTH_CTRL_KEY;



	memcpy(&cfg->colorPairs[THOTH_COLOR_SIDE_NUMBERS-1], (int[]){ THOTH_COLOR_WHITE, THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_NORMAL-1], (int[]){ THOTH_COLOR_WHITE, THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_KEYWORD-1], (int[]){ THOTH_COLOR_CYAN, THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_COMMENT-1], (int[]){ THOTH_COLOR_GREY, THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_TOKEN-1], (int[]){ THOTH_COLOR_BLUE, THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_NUM-1], (int[]){ THOTH_COLOR_RED, THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_FUNCTION-1], (int[]){ THOTH_COLOR_YELLOW, THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_STRING-1], (int[]){ THOTH_COLOR_MAGENTA, THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_SELECTED-1], (int[]){ THOTH_COLOR_BLACK ,THOTH_COLOR_CYAN }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_SELECTED_DIRECTORY-1], (int[]){ THOTH_COLOR_RED ,THOTH_COLOR_CYAN }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_UNSELECTED_DIRECTORY-1], (int[]){ THOTH_COLOR_RED ,THOTH_COLOR_WHITE }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_AUTO_COMPLETE-1], (int[]){ THOTH_COLOR_BLACK, THOTH_COLOR_WHITE }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_LOG_UNSELECTED-1], (int[]){ THOTH_COLOR_BLACK, THOTH_COLOR_WHITE }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_CURSOR-1], (int[]){ THOTH_COLOR_BLACK ,THOTH_COLOR_MAGENTA }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_FIND-1], (int[]){ THOTH_COLOR_BLACK ,THOTH_COLOR_WHITE }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_COLOR_LINE_NUM-1], (int[]){ THOTH_COLOR_GREY ,THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_TE_COLOR_BLACK-1], (int[]){ THOTH_COLOR_BLACK ,THOTH_COLOR_WHITE }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_TE_COLOR_WHITE-1], (int[]){ THOTH_COLOR_WHITE ,THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_TE_COLOR_CYAN-1], (int[]){ THOTH_COLOR_CYAN ,THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_TE_COLOR_RED-1], (int[]){ THOTH_COLOR_RED ,THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_TE_COLOR_YELLOW-1], (int[]){ THOTH_COLOR_YELLOW ,THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_TE_COLOR_BLUE-1], (int[]){ THOTH_COLOR_BLUE ,THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_TE_COLOR_GREEN-1], (int[]){ THOTH_COLOR_GREEN ,THOTH_COLOR_BLACK }, sizeof(int)*2);
	memcpy(&cfg->colorPairs[THOTH_TE_COLOR_MAGENTA-1], (int[]){ THOTH_COLOR_MAGENTA ,THOTH_COLOR_BLACK }, sizeof(int)*2);
	//gruvbox
	// aurora

	// {0xa1/255.0f,0xef/255.0f,0xe4/255.0f}, //cyan
	// {0xff/255.0f,0x58/255.0f,0x74/255.0f}, //red
	// {0xec/255.0f,0xc4/255.0f,0x8d/255.0f}, //yellow
	// {0x82/255.0f,0xaa/255.0f,0xf0/255.0f}, //blue
	// {0xad/255.0f,0xdb/255.0f,0x67/255.0f}, //green
	// {0xbd/255.0f,0x93/255.0f,0xf3/255.0f}, //magenta
	// {0xd6/255.0f,0xde/255.0f,0xeb/255.0f}, //white
	//     {0x14/255.0f,0x14/255.0f,0x15/255.0f}, // black
	//     {0x74/255.0f,0x74/255.0f,0x75/255.0f}, // grey
	// {0x14/255.0f,0x14/255.0f,0x15/255.0f}, //bg
	

	FILE *fp = fopen(THOTH_CONFIG_FILE,"r");
	if(fp){
	    while(!feof(fp)){
			char lineType[100];

			fscanf(fp, "%s : ", lineType);
			if(lineType[0] == '#'){
			    while(!feof(fp) && fgetc(fp) != '\n'){}
			    continue;
			}
			

			if(strcmp(lineType, "COLOR_CYAN") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_CYAN-1].r, &defaultColors[THOTH_COLOR_CYAN-1].g, &defaultColors[THOTH_COLOR_CYAN-1].b);
			else if(strcmp(lineType, "COLOR_RED") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_RED-1].r, &defaultColors[THOTH_COLOR_RED-1].g, &defaultColors[THOTH_COLOR_RED-1].b);
			else if(strcmp(lineType, "COLOR_YELLOW") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_YELLOW-1].r, &defaultColors[THOTH_COLOR_YELLOW-1].g, &defaultColors[THOTH_COLOR_YELLOW-1].b);
			else if(strcmp(lineType, "COLOR_BLUE") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_BLUE-1].r, &defaultColors[THOTH_COLOR_BLUE-1].g, &defaultColors[THOTH_COLOR_BLUE-1].b);
			else if(strcmp(lineType, "COLOR_GREEN") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_GREEN-1].r, &defaultColors[THOTH_COLOR_GREEN-1].g, &defaultColors[THOTH_COLOR_GREEN-1].b);
			else if(strcmp(lineType, "COLOR_MAGENTA") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_MAGENTA-1].r, &defaultColors[THOTH_COLOR_MAGENTA-1].g, &defaultColors[THOTH_COLOR_MAGENTA-1].b);
			else if(strcmp(lineType, "COLOR_WHITE") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_WHITE-1].r, &defaultColors[THOTH_COLOR_WHITE-1].g, &defaultColors[THOTH_COLOR_WHITE-1].b);
			else if(strcmp(lineType, "COLOR_BLACK") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_BLACK-1].r, &defaultColors[THOTH_COLOR_BLACK-1].g, &defaultColors[THOTH_COLOR_BLACK-1].b);
			else if(strcmp(lineType, "COLOR_GREY") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_GREY-1].r, &defaultColors[THOTH_COLOR_GREY-1].g, &defaultColors[THOTH_COLOR_GREY-1].b);
			else if(strcmp(lineType, "COLOR_BG") == 0)
			    fscanf(fp, "%x %x %x", &defaultColors[THOTH_COLOR_BG-1].r, &defaultColors[THOTH_COLOR_BG-1].g, &defaultColors[THOTH_COLOR_BG-1].b);
			

			while(fgetc(fp) != '\n' && !feof(fp)){}
		}

	    fclose(fp);

	}

	int k;
	for(k = 0; k < THOTH_NUM_COLORS-1; k++){
		cfg->colors[k].r = defaultColors[k].r / 255.0f;
		cfg->colors[k].g = defaultColors[k].g / 255.0f;
		cfg->colors[k].b = defaultColors[k].b / 255.0f;
	}

}