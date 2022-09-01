#ifndef CONFIG_DEF
#define CONFIG_DEF

#include "types.h"

enum {
	THOTH_MoveLinesText_UP= 1,
	THOTH_MoveLinesText_DOWN,
	THOTH_OpenFileBrowser,
	THOTH_OpenFileZim,
	THOTH_NewFile,
	THOTH_CloseFile,
	THOTH_SwitchFile,
	THOTH_SaveAsFile,
	THOTH_SaveFile,
	THOTH_ToggleComment,
	THOTH_MoveBrackets,
	THOTH_SelectBrackets,
	THOTH_GotoLine,
	THOTH_FindTextInsensitive,
	THOTH_FindTextZim,
	THOTH_EventCtrlEnter,
	THOTH_SelectNextWord,
	THOTH_AddCursorCommand_DOWN,
	THOTH_AddCursorCommand_UP,
	THOTH_ExpandSelectionChars_FORWARD,
	THOTH_ExpandSelectionChars_BACK,
	THOTH_ExpandSelectionLines,
	THOTH_DeleteLine,
	THOTH_MoveByWords_FORWARD,
	THOTH_MoveByWords_BACK,
	THOTH_IndentLine_FORWARD,
	THOTH_IndentLine_BACK,
	THOTH_ExpandSelectionWords_FORWARD,
	THOTH_ExpandSelectionWords_BACK,
	THOTH_ScrollScreen_DOWN,
	THOTH_ScrollScreen_UP,
	THOTH_MoveByChars_FORWARD,
	THOTH_MoveByChars_BACK,
	THOTH_MoveLines_UP,
	THOTH_MoveLines_DOWN,
	THOTH_SelectAll,
	THOTH_Undo,
	THOTH_Redo,
	THOTH_Cut,
	THOTH_Copy,
	THOTH_Paste,
	THOTH_NUM_KEYBINDS
};

enum {
	THOTH_COLOR_CYAN = 1,
	THOTH_COLOR_RED,
	THOTH_COLOR_YELLOW,
	THOTH_COLOR_BLUE,
	THOTH_COLOR_GREEN,
	THOTH_COLOR_MAGENTA,
	THOTH_COLOR_WHITE,
	THOTH_COLOR_BLACK,
	THOTH_COLOR_GREY,
	THOTH_COLOR_BG,
	THOTH_NUM_COLORS,
};
enum {
    THOTH_COLOR_NORMAL = 1,
    THOTH_COLOR_KEYWORD,
    THOTH_COLOR_TOKEN,
    THOTH_COLOR_FUNCTION,
    THOTH_COLOR_NUM,
    THOTH_COLOR_COMMENT,
    THOTH_COLOR_STRING,
    THOTH_COLOR_SELECTED,
    THOTH_COLOR_SELECTED_DIRECTORY,
    THOTH_COLOR_UNSELECTED_DIRECTORY,
    THOTH_COLOR_LOG_UNSELECTED,
    THOTH_COLOR_FIND,
    THOTH_COLOR_LINE_NUM,
    THOTH_COLOR_AUTO_COMPLETE,
    THOTH_COLOR_CURSOR,
    THOTH_COLOR_SIDE_NUMBERS,
    THOTH_TE_COLOR_BLACK,
    THOTH_TE_COLOR_RED,
    THOTH_TE_COLOR_GREEN,
    THOTH_TE_COLOR_YELLOW,
    THOTH_TE_COLOR_BLUE,
    THOTH_TE_COLOR_CYAN,
    THOTH_TE_COLOR_MAGENTA,
    THOTH_TE_COLOR_WHITE,
    THOTH_TE_NUM_COLOR_PAIRS,
};

typedef struct {
    float r;
    float g;
    float b;
} Thoth_RGBColor;

typedef struct {
	Thoth_RGBColor colors[THOTH_NUM_COLORS];
	int colorPairs[THOTH_TE_NUM_COLOR_PAIRS][2];
	unsigned int keybinds[THOTH_NUM_KEYBINDS];
} Thoth_Config;

void Thoth_Config_Read(Thoth_Config *cfg);


#endif