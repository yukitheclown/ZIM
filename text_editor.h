#ifndef TEXT_EDITOR_DEF
 #define TEXT_EDITOR_DEF
#include <sys/types.h>
#include "types.h"
#include "graphics.h"
#include "config.h"
#include "file_browser.h"

#define THOTH_CTRL_KEY    (unsigned int)0x100
#define THOTH_ALT_KEY     (unsigned int)0x200
#define THOTH_SHIFT_KEY   (unsigned int)0x400
#define THOTH_ARROW_UP    (unsigned int)0x800
#define THOTH_ARROW_DOWN  (unsigned int)0x1000
#define THOTH_ARROW_LEFT  (unsigned int)0x2000
#define THOTH_ARROW_RIGHT (unsigned int)0x4000
#define THOTH_ENTER_KEY   (unsigned int)0x8000

// #include "window.h"
// #include "text.h"


// // { "keys": ["ctrl+h"], "command": "move", "args": {"by": "characters", "forward": false} },
// // { "keys": ["ctrl+l"], "command": "move", "args": {"by": "characters", "forward": true} },
// // { "keys": ["ctrl+j"], "command": "move", "args": {"by": "lines", "forward": false} },
// // { "keys": ["ctrl+k"], "command": "move", "args": {"by": "lines", "forward": true} },
// // { "keys": ["ctrl+alt+h"], "command": "move", "args": {"by": "words", "forward": false} },
// // { "keys": ["ctrl+alt+l"], "command": "move", "args": {"by": "word_ends", "forward": true} },
// // { "keys": ["ctrl+shift+l"], "command": "expand_selection", "args": {"to": "line"} },

// // // { "keys": ["ctrl+shift+m"], "command": "expand_selection", "args": {"to": "brackets"} },
// // // { "keys": ["ctrl+m"], "command": "move_to", "args": {"to": "brackets"} },

// // { "keys": ["shift+alt+ctrl+h"], "command": "move", "args": {"extend": true, "by": "words", "forward": false} },
// // { "keys": ["shift+alt+ctrl+l"], "command": "move", "args": {"extend": true, "by": "word_ends", "forward": true} },

 enum {
  THOTH_LOGMODE_NUM = 1,
  THOTH_LOGMODE_TEXT,
  THOTH_LOGMODE_TEXT_INSENSITIVE,
  THOTH_LOGMODE_SAVE,
  THOTH_LOGMODE_OPEN,
  THOTH_LOGMODE_SWITCH_FILE,
  THOTH_LOGMODE_FILEBROWSER,
  THOTH_LOGMODE_MODES_INPUTLESS,
  THOTH_LOGMODE_ALERT,
  THOTH_LOGMODE_CONSOLE,
 };

 typedef struct {


  int startCursorPos;
  int len;

 } Thoth_EditorSelection;


 typedef struct Thoth_Editor Thoth_Editor;
 typedef struct Thoth_EditorCmd Thoth_EditorCmd;
 typedef struct Thoth_EditorCur Thoth_EditorCur;

 struct Thoth_EditorCmd {

  unsigned int *keyBinding;
  char *keys;
  int num;
  unsigned char scroll;
  
  Thoth_EditorCur *savedCursors;
  int nSavedCursors;

  void (*Execute)(Thoth_Editor *, Thoth_EditorCmd *c);
  void (*Undo)(Thoth_Editor *, Thoth_EditorCmd *c);
 };

 struct Thoth_EditorCur {


  Thoth_EditorSelection selection;

  //no longer used, now clipboard is from system, lines seperated by \n
  char *clipboard;
  int sClipboard;
  char *savedText;
  int addedLen;

  int pos;
 };

#define THOTH_MAX_AUTO_COMPLETE_STRLEN 35
#define THOTH_MAX_AUTO_COMPLETE 20

typedef struct {
  int offset;
  int len;
} Thoth_AutoCompleteOffset;

typedef struct {
  int                     scroll;
  int                     unsaved;
  int                     cursorPos;
  int                     historyPos;
  Thoth_EditorCmd       **history;
  int                     sHistory;


  char                    *text;
  int                     textLen;
  char                    name[MAX_FILENAME];
  char                    path[MAX_PATH_LEN];
} Thoth_EditorFile;

 struct Thoth_Editor {

  Thoth_Config            *cfg;
  Thoth_Graphics         *graphics;
  Thoth_EditorCmd       **commands;
  int                     nCommands;

  int                     selectNextWordTerminator; // "select" not get it in the phrase selecting

  int                     autoCompleteSearchLen;
  int                     autoCompleteLen;
  Thoth_AutoCompleteOffset      autoComplete[THOTH_MAX_AUTO_COMPLETE];
  int                     autoCompleteIndex;

  int                     logging;
  int                     logIndex;
  char                    *loggingText;

  Thoth_EditorFile          **files;
  int                     nFiles;
  Thoth_EditorFile          *file;

  Thoth_EditorCur        *cursors;
  int                     mouseSelection;
  int                     nCursors;
  int                     logX;
  int                     logY;

  int                     quit;
  Thoth_FileBrowser       fileBrowser;

  int                     ttyPid;
  int                     ttyMaster;
  int                     ttySlave;
  int                     _stdout;
  int                     _stderr;
 };


void Thoth_Editor_LoadFile(Thoth_Editor *t, char *path);
void Thoth_Editor_Draw(Thoth_Editor *t);
void Thoth_Editor_Event(Thoth_Editor *t,unsigned int key);
int Thoth_Editor_Destroy(Thoth_Editor *t);
void Thoth_Editor_Init(Thoth_Editor *t, Thoth_Graphics *g, Thoth_Config *cfg);
void Thoth_Editor_SetCursorPos(Thoth_Editor *t, int x, int y);
int Thoth_Editor_SetCursorPosSelection(Thoth_Editor *t, int x, int y);
void Thoth_Editor_SetCursorPosDoubleClick(Thoth_Editor *t, int x, int y);
 #endif