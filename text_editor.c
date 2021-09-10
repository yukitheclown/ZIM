#include "text_editor.h"
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <math.h>

#define COLOR_CHAR1 240
#define COLOR_CHAR2 241
#define COLOR_CHAR3 242
#define COLOR_CHAR4 243
#define COLOR_CHAR5 244
#define COLOR_CHAR6 239
#define COLOR_CHAR7 238

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

enum {

    COLOR1 = 0,
    COLOR2,
    COLOR3,
    COLOR4,
    COLOR5,
    COLOR6,
    COLOR7,
    COLOR_SELECTED,
    COLOR_CURSOR,
    COLOR_SIDE_NUMBERS,
};

// #define TEXT_EDITOR_DEF

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


// typedef struct {


//  int startCursorPos;
//  int len;

// } TextEditorSelection;


// typedef struct TextEditor TextEditor;
// typedef struct TextEditorCommand TextEditorCommand;
// typedef struct TextEditorCursor TextEditorCursor;

// struct TextEditorCommand {

//  char *keyBinding;
//  char *keys;
//  int num;

//  TextEditorCursor *savedCursors;
//  int nSavedCursors;

//  char *savedTexts;

//  void (*Execute)(TextEditor *, TextEditorCommand *c);
//  void (*Undo)(TextEditor *, TextEditorCommand *c);
// };

// struct TextEditorCursor {


//  TextEditorSelection *selections;
//  int nSelections;

//  TextEditorCommand *clipboard;
//  int sClipboard;

//  int pos;
// };


// struct TextEditor {

//  FontFace font;
//  Text textData;

//  TextEditorCommand *history;
//  int sHistory;

//  int historyPos;

//  TextEditorCommand *commands;
//  int nCommands;

//  TextEditorCursor *cursors;
//  int nCursors;

//  char *text;
//  int textLen;

//  char *coloredText;
//  int coloredTextLen;
// };

// void TextEditor_Draw(TextEditor *t);
// void TextEditor_Event(TextEditor *t, SDL_Event event);
// void TextEditor_Destroy(TextEditor *t);
// void TextEditor_Init(TextEditor *t, FontFace font);



// #endif

static void SearchingRemoveChars(TextEditor *t, int num);
static void SearchingAddText(TextEditor *t, char *str);
static void FindCommand(TextEditor *t, TextEditorCommand *command);
static void AddSavedText(TextEditorCommand *command, char *str, int len, int index);
static void EraseAllSelectedText(TextEditor *t, int index, TextEditorCommand *command);
static void DrawSearchBar(TextEditor *t);
static char IsToken(char c);
static void AddCharToString(char **string, char character, int *strLen);
static void SyntaxHighlight(TextEditor *tEditor);
static int GetStartOfNextLine(char *text, int cPos);
static int GetNumLinesToPos(char *text, int cPos);
static int GetCharsIntoLine(char *text, int cPos);
static int GetCharsToLine(char *text, int line);
static void GetWordStartEnd(char *text, int cPos, int *s, int *e);
static int GetWordEnd(char *text, int cPos);
static int GetWordStart(char *text, int cPos);
static int GetStartOfPrevLine(char *text, int cPos);
static void GetCursorPos(char *text, int cPos, int *x, int *y);
static void RefreshEditorCommand(TextEditorCommand *c);
static void ResolveCursorCollisions(TextEditor *t);
static void MoveCursorsAndSelection(TextEditor *t, int pos, int by);
static void RemoveSelections(TextEditor *t);
static void SetCursorToSelection(TextEditorCursor *cursor, int n);
static void MoveByChars(TextEditor *t, TextEditorCommand *c);
static void MoveByWords(TextEditor *t, TextEditorCommand *c);
static void FreeCursors(TextEditor *t);
static void MoveCursorUpLine(TextEditor *t, TextEditorCursor *cursor);
static void MoveCursorDownLine(TextEditor *t, TextEditorCursor *cursor);
static void GetSelectionStartEnd(TextEditorSelection selection, int *s, int *e);
static void MoveLines(TextEditor *t, TextEditorCommand *c);
static void ExpandSelectionChars(TextEditor *t,TextEditorCommand *c);
static void ExpandSelectionWords(TextEditor *t,TextEditorCommand *c);
static void AddCursorCommand(TextEditor *t, TextEditorCommand *c);
static TextEditorCursor *AddCursor(TextEditor *t);
static int Find(char *text, char *str, int len);
static void SelectNextWord(TextEditor *t, TextEditorCommand *c);
static void SaveCursors(TextEditor *t, TextEditorCommand *c);
static void LoadCursors(TextEditor *t, TextEditorCommand *c);
static void AddStrToText(TextEditor *t, int pos, char *text);
static void RemoveCharactersFromText(TextEditor *t, int pos, int len);
static void UndoRemoveCharacters(TextEditor *t, TextEditorCommand *c);
static void RemoveCharacters(TextEditor *t, TextEditorCommand *c);
static void UndoAddCharacters(TextEditor *t, TextEditorCommand *c);
static void AddCharacters(TextEditor *t, TextEditorCommand *c);
static void Undo(TextEditor *t, TextEditorCommand *c);
static void Copy(TextEditor *t, TextEditorCommand *c);
static void ExpandSelectionLines(TextEditor *t, TextEditorCommand *c);
static void DeleteLine(TextEditor *t, TextEditorCommand *c);
static void Cut(TextEditor *t, TextEditorCommand *c);
static void Redo(TextEditor *t, TextEditorCommand *c);
static void FreeCommand(TextEditorCommand *c);
static TextEditorCommand *CopyCommand(TextEditorCommand *c);
static TextEditorCommand *CreateCommand(const int binding[], const char *keys, int n,
    void (*E)(TextEditor *, TextEditorCommand *), void (*U)(TextEditor *, TextEditorCommand *));
static void UndoCommands(TextEditor *t, int num);
static void RedoCommands(TextEditor *t, int num);
static void RemoveExtraCursors(TextEditor *t);
static void ExecuteCommand(TextEditor *t, TextEditorCommand *c);
static void AddCommand(TextEditor *t, TextEditorCommand *c);

const char *keywords[] = {
    "int8", "int16", "int32", "int64", "break", "and", "case", "continue", "default", "do", "else", "false", "for", "if", "namespace",
    "abstract", "final", "from", "cast", "funcdef", "get", "import", "in", "inout", "interface", "is", "mixin", "not", "null", "or",
    "out", "override", "private", "protected", "return", "set", "shared", "super", "switch", "this", "true", "typedef", "uint",
    "uint8", "uint16", "uint32", "uint64", "void", "while", "xor", "end", "function", "local", "nil", "repeat", "then", "until",
    "auto", "bool", "char", "class", "double", "float", "int", "enum", "const", "static", "include", "define", "ifndef", "endif"
};

static char IsToken(char c){
    if(c == '(' || c == ' ' || c == '\n'|| c == ',' || c == '+' || c == '=' || c == '~' || c == '<' || 
        c == '>' || c == '*' || c == ')' || c == '/' || c == '{' || c == '}' || c == '%' || c == '^' || 
        c == ':' || c == '.' || c == ';' || c == ']' || c == '[' || c == '-' || c == ']' || c == '"' ||
        c == '\''|| c == '\t'|| c == '\0' || c == '#' || c == '&' || c == '!') return 1;

    return 0;
}

static void AddCharToString(char **string, char character, int *strLen){
    
    *string = (char *)realloc(*string, ++(*strLen));
    
    (*string)[(*strLen)-1] = character;
}

static void SyntaxHighlight(TextEditor *tEditor){
    
    if(tEditor->text == NULL){

        if(tEditor->coloredText) free(tEditor->coloredText);
        tEditor->coloredText = NULL;

        return;
    }

    int len = strlen(tEditor->text);

    if(tEditor->coloredText) free(tEditor->coloredText);

    tEditor->coloredText = NULL;
    int coloredTextLen = 0;

    for(int k = 0; k < len; k++){
        
        AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR1, &coloredTextLen);

        char *temp = NULL;
        int tempLen = 0;

        int j;
        for(j = k; j < len; j++){

            char c = tEditor->text[j];
            
            char token = IsToken(c);

            if(token || j == len-1){

                int comment = 0, string = 0;
                if(j != len-1 && c == '/' && tEditor->text[j+1] == '/') comment = 1;
                else if(j != len-1 && c == '-' && tEditor->text[j+1] == '-') comment = 1;
                else if(j != len-1 && c == '/' && tEditor->text[j+1] == '*') comment = 2;
                else if(j != len-1 && c == '"') string = 1;
                else if(j != len-1 && c == '\'') string = 2;

                if(j == len-1 && !token) AddCharToString(&temp, c, &tempLen);

                AddCharToString(&temp, 0, &tempLen);

                char keyword = 0;
                
                for(int m = 0; m < (int)(sizeof(keywords)/sizeof(char *)); m++)
                    if(strcmp(temp, keywords[m]) == 0) {
                        keyword = 1;
                        AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR2, &coloredTextLen);
                        break;
                    }

                if(c == '(' && !keyword) AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR4, &coloredTextLen);

                int f, isText = 0;
                for (f = 0; f < (int)strlen(temp); f++) {
                    if( !isText && (temp[f] == '0' || temp[f] == '1' || temp[f] == '2' || temp[f] == '3' || 
                        temp[f] == '4' || temp[f] == '5' || temp[f] == '6' || temp[f] == '7' || 
                        temp[f] == '8' || temp[f] == '9')){
                        AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR5, &coloredTextLen);
                        AddCharToString(&tEditor->coloredText, temp[f], &coloredTextLen);
                        AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR1, &coloredTextLen);

                    } else {
                        isText = 1;
                        AddCharToString(&tEditor->coloredText, temp[f], &coloredTextLen);
                    }
                }

                if(c == ')' || c == '(') AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR4, &coloredTextLen);
                
                if(c != '\n' && c != ' ' && c != '(' && c != ')' && token && !comment && !string)
                    AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR3, &coloredTextLen);
                

                if(token && !comment && !string) AddCharToString(&tEditor->coloredText, c, &coloredTextLen);
                
                k = j;

                if(comment){
                    AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR6, &coloredTextLen);

                    if(comment == 1){
                        for (; k < len && tEditor->text[k] != '\n'; k++)
                            AddCharToString(&tEditor->coloredText, tEditor->text[k], &coloredTextLen);
                        
                        if(k < len)
                            AddCharToString(&tEditor->coloredText, tEditor->text[k], &coloredTextLen);

                    } else {
                        AddCharToString(&tEditor->coloredText, tEditor->text[k++], &coloredTextLen);
                        AddCharToString(&tEditor->coloredText, tEditor->text[k++], &coloredTextLen);

                        for(;k < len-1 && tEditor->text[k] != '*' && tEditor->text[k+1] != '/'; k++)
                            AddCharToString(&tEditor->coloredText, tEditor->text[k], &coloredTextLen);

                        if(k < len-1){
                            AddCharToString(&tEditor->coloredText, tEditor->text[k++], &coloredTextLen);
                            AddCharToString(&tEditor->coloredText, tEditor->text[k], &coloredTextLen);
                        } else if(k < len){
                            AddCharToString(&tEditor->coloredText, tEditor->text[k++], &coloredTextLen);
                        }
                    }
                }

                if(string){

                    AddCharToString(&tEditor->coloredText, (char)COLOR_CHAR7, &coloredTextLen);
                    AddCharToString(&tEditor->coloredText, tEditor->text[k++], &coloredTextLen);

                    int escaped = 0;

                    while(k < len){

                        if(tEditor->text[k] == '"' && string == 1 && !escaped) break;
                        if(tEditor->text[k] == '\'' && string == 2 && !escaped) break;

                        
                        AddCharToString(&tEditor->coloredText, tEditor->text[k], &coloredTextLen);

                        if(tEditor->text[k] == '\\' && !escaped)
                            escaped = 1;
                        else
                            escaped = 0;

                        k++;
                    }
                    
                    if(k < len)
                        AddCharToString(&tEditor->coloredText, tEditor->text[k], &coloredTextLen);
                }

                break;

            } else {

                AddCharToString(&temp, c, &tempLen);
            }
        }

        if(temp) free(temp);
        temp = NULL;
    }

    AddCharToString(&tEditor->coloredText, 0, &coloredTextLen);
}

static int GetStartOfNextLine(char *text, int cPos){

    int k;
    for(k = cPos; k < (int)strlen(text); k++)
        if(text[k] == '\n') { k++; break; }

    return k;
}

static int GetNumLinesToPos(char *text, int cPos){

    int k, lines = 0;
    for(k = 0; k < cPos; k++)
        if(text[k] == '\n') lines++;

    return lines;
}

static int GetCharsToLine(char *text, int line){

    if(line <= 0) return 0;

    int k, currLine = 0;
    for(k = 0; k < (int)strlen(text); k++)
        if(text[k] == '\n') if(++currLine >= line) { k++; break; }

    return k;

}

static int GetCharsIntoLine(char *text, int cPos){

    int k = cPos > 0 ? cPos-1 : 0;

    for(; k > 0; k--)
        if(text[k] == '\n') { k++; break; }

    return (cPos - k);
}


static void GetCursorPos(char *text, int cPos, int *x, int *y){

    *x = 0;
    *y = 0;

    int k;
    for(k = 0; k < cPos; k++){

        if(text[k] == (char)COLOR_CHAR7) continue;
        else if(text[k] == (char)COLOR_CHAR6) continue;
        else if(text[k] == (char)COLOR_CHAR5) continue;
        else if(text[k] == (char)COLOR_CHAR4) continue;
        else if(text[k] == (char)COLOR_CHAR3) continue;
        else if(text[k] == (char)COLOR_CHAR2) continue;
        else if(text[k] == (char)COLOR_CHAR1) continue;

        (*x)++;

        if(text[k] == '\n'){
            (*y)++;
            *x = 0;
        }
    }
}

static void GetWordStartEnd(char *text, int cPos, int *s, int *e){

    int k;
    for(k = cPos; k >= 0; k--)
        if(!IsToken(text[k])) break;

    for(; k >= 0; k--)
        if(IsToken(text[k])) break;

    *s = k+1;

    for(k = *s; k < (int)strlen(text); k++)
        if(IsToken(text[k])) break;

    *e = k;
}

static int GetWordEnd(char *text, int cPos){

    int k;

    for(k = cPos; k < (int)strlen(text); k++)
        if(!IsToken(text[k])) break;

    for(; k < (int)strlen(text); k++)
        if(IsToken(text[k])) break;

    return k;
}

static int GetWordStart(char *text, int cPos){

    int k;
    for(k = cPos; k >= 0; k--)
        if(!IsToken(text[k])) break;

    for(; k >= 0; k--)
        if(IsToken(text[k])) break;

    return k+1;
}

static int GetStartOfPrevLine(char *text, int cPos){

    int k;;
    for(k = cPos-1; k >= 0; k--)
        if(text[k] == '\n'){
            for(k = k-1; k >= 0; k--)
                if(text[k] == '\n') break;

            break;
        }

    return k < 0 ? 0 : k+1;
}

static void ResolveCursorCollisions(TextEditor *t){

    int j, f;
    for(j = 0; j < t->nCursors; j++){
        for(f = 0; f < t->nCursors; f++){
            if(f != j && t->cursors[j].pos == t->cursors[f].pos){

                int m;
                for(m = j; m < t->nCursors-1; m++)
                    t->cursors[m] = t->cursors[m+1];

                t->cursors = realloc(t->cursors, --t->nCursors * sizeof(TextEditorCursor));                 

                j--;
                break;
            }
        }
    }
}

static void MoveCursorsAndSelection(TextEditor *t, int pos, int by){

    int k;
    for(k = 0; k < t->nCursors; k++){

        int start, end;
        GetSelectionStartEnd(t->cursors[k].selection, &start, &end);

        if(start > pos){

            t->cursors[k].selection.startCursorPos += by;

        } else if(end > pos){

            if(t->cursors[k].selection.len < 0)     
                t->cursors[k].selection.len += by;
            else
                t->cursors[k].selection.startCursorPos += by;
        }

        if(t->cursors[k].pos > pos)
            t->cursors[k].pos += by;
    }

    ResolveCursorCollisions(t);
}

static void RemoveSelections(TextEditor *t){
    
    // memset(&t->cursors[0].selection, 0, sizeof(TextEditorSelection));

    // if(t->nCursors > 1)
    //  t->cursors = (TextEditorCursor *)realloc(t->cursors, sizeof(TextEditorCursor));
    // t->nCursors = 1;

    int k;
    for(k = 0; k < t->nCursors; k++)
        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));

}

static void SetCursorToSelection(TextEditorCursor *cursor, int n){

    int end, start;
    GetSelectionStartEnd(cursor->selection, &start, &end);
    
    if(n < 0) cursor->pos = start;
    else cursor->pos = end;
}


static void MoveByChars(TextEditor *t, TextEditorCommand *c){

    int textLen = strlen(t->text);

    int k;
    for(k = 0; k < t->nCursors; k++){
        
        if(t->cursors[k].selection.len != 0){
            SetCursorToSelection(&t->cursors[k], c->num);
            continue;   
        }

        t->cursors[k].pos += c->num;

        if(t->cursors[k].pos < 0) t->cursors[k].pos = 0;
        if(t->cursors[k].pos > textLen) t->cursors[k].pos = textLen;
    }

    RemoveSelections(t);

    ResolveCursorCollisions(t);
}

static void MoveByWords(TextEditor *t, TextEditorCommand *c){

    int k;
    for(k = 0; k < t->nCursors; k++){

        if(t->cursors[k].selection.len != 0){
            SetCursorToSelection(&t->cursors[k], c->num);
            continue;   
        }

        if(c->num < 0)
            t->cursors[k].pos = GetWordStart(t->text, t->cursors[k].pos-1);         
        else
            t->cursors[k].pos = GetWordEnd(t->text, t->cursors[k].pos+1);           


        if(t->cursors[k].pos < 0) t->cursors[k].pos = 0;
        if(t->cursors[k].pos > (int)strlen(t->text)) t->cursors[k].pos = strlen(t->text);

        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));
    }

    ResolveCursorCollisions(t);
}


static void FreeCursors(TextEditor *t){

    int k;
    for(k = 0; k < t->nCursors; k++)
        if(t->cursors[k].clipboard)
            free(t->cursors[k].clipboard);

    t->nCursors = 0;
    free(t->cursors);
    t->cursors = NULL;
}

static void MoveCursorUpLine(TextEditor *t, TextEditorCursor *cursor){

    if(t->text == NULL) return;

    int charsIntoLine = GetCharsIntoLine(t->text, cursor->pos);
    int startOfPrevLine = GetStartOfPrevLine(t->text, cursor->pos);

    int k;
    for(k = startOfPrevLine; k < (int)strlen(t->text); k++)
        if(t->text[k] == '\n') break;

    int charsOnPrevLine = (k - startOfPrevLine);
    cursor->pos = charsIntoLine <= charsOnPrevLine ? startOfPrevLine + charsIntoLine: startOfPrevLine + charsOnPrevLine; 
}

static void MoveCursorDownLine(TextEditor *t, TextEditorCursor *cursor){

    if(t->text == NULL) return;

    int charsIntoLine = GetCharsIntoLine(t->text, cursor->pos);
    int startOfNextLine = GetStartOfNextLine(t->text, cursor->pos);

    if(startOfNextLine == (int)strlen(t->text)) return;

    int k;
    for(k = startOfNextLine; k < (int)strlen(t->text); k++)
        if(t->text[k] == '\n') break;

    int charsOnNextLine = (k - startOfNextLine);

    cursor->pos = charsIntoLine <= charsOnNextLine ? startOfNextLine + charsIntoLine : startOfNextLine + charsOnNextLine;
}

static void GetSelectionStartEnd(TextEditorSelection selection, int *s, int *e){

    *s = selection.startCursorPos;
    *e = selection.startCursorPos + selection.len;

    if(*e < *s){
        int tmp = *s;
        *s = *e;
        *e = tmp;
    }
}

static void MoveLines(TextEditor *t, TextEditorCommand *c){

    int k;

    if(t->text == NULL || !strlen(t->text)){
        // FreeCursors(t);
        return;
    }

    for(k = 0; k < t->nCursors; k++){

        TextEditorCursor *cursor = &t->cursors[k];

        
        if(c->num > 0) {

            MoveCursorDownLine(t, cursor);

        } else if(c->num < 0 && GetNumLinesToPos(t->text, cursor->pos) > 0){

            MoveCursorUpLine(t, cursor);
        }
    }

    RemoveSelections(t);

    ResolveCursorCollisions(t);
}

static void ExpandSelectionChars(TextEditor *t,TextEditorCommand *c){

    if(t->text == NULL) return;

    int f;
    for(f = 0; f < t->nCursors; f++){

        TextEditorCursor *cursor = &t->cursors[f];

        if(cursor->selection.len == 0){
            cursor->selection.startCursorPos = cursor->pos;
        }

        cursor->pos += c->num;

        if(cursor->pos < 0) cursor->pos = 0;
        if(cursor->pos > (int)strlen(t->text)) cursor->pos = strlen(t->text);

        cursor->selection.len += c->num;

        if(cursor->selection.startCursorPos+cursor->selection.len < 0)
            cursor->selection.len -= c->num;

        if(cursor->selection.startCursorPos+cursor->selection.len > (int)strlen(t->text))
            cursor->selection.len -= c->num;
    }

    ResolveCursorCollisions(t);
}

static void ExpandSelectionWords(TextEditor *t,TextEditorCommand *c){

    if(t->text == NULL) return;

    int f;
    for(f = 0; f < t->nCursors; f++){

        TextEditorCursor *cursor = &t->cursors[f];

        if(cursor->selection.len == 0){
            cursor->selection.startCursorPos = cursor->pos;
        }

        if(c->num < 0)
            cursor->pos = GetWordStart(t->text, cursor->pos-1);         
        else
            cursor->pos = GetWordEnd(t->text, cursor->pos+1);           

        if(cursor->pos < 0) cursor->pos = 0;
        if(cursor->pos > (int)strlen(t->text)) cursor->pos = strlen(t->text);

        cursor->selection.len = cursor->pos - cursor->selection.startCursorPos;

        if(cursor->selection.startCursorPos+cursor->selection.len < 0)
            cursor->selection.len -= c->num;

        if(cursor->selection.startCursorPos+cursor->selection.len > (int)strlen(t->text))
            cursor->selection.len -= c->num;
    }

    ResolveCursorCollisions(t);
}

static TextEditorCursor *AddCursor(TextEditor *t){

    t->cursors = (TextEditorCursor *)realloc(t->cursors, ++t->nCursors * sizeof(TextEditorCursor));

    TextEditorCursor *cursor = &t->cursors[t->nCursors-1];

    memset(cursor, 0, sizeof(TextEditorCursor));

    char *last = t->cursors[t->nCursors-2].clipboard;

    if(last){

        int len = strlen(last);

        cursor->clipboard = (char *)malloc(len + 1);
        cursor->clipboard[len] = 0;

        memcpy(cursor->clipboard, last, len);
    }

    return cursor;
}

static void AddCursorCommand(TextEditor *t, TextEditorCommand *c){

    if(t->text == NULL) return;

    TextEditorCursor *cursor = AddCursor(t);

    int minCursor = -1;
    int maxCursor = -1;

    int k;
    for(k = 0; k < t->nCursors-1; k++){
        
        if(t->cursors[k].pos < minCursor || minCursor < 0)
            minCursor = t->cursors[k].pos;
        
        if(t->cursors[k].pos > maxCursor || maxCursor < 0)
            maxCursor = t->cursors[k].pos;
    }

    if(c->num < 0 && GetNumLinesToPos(t->text, minCursor) > 0){

        cursor->pos = minCursor;

        MoveCursorUpLine(t, cursor);

    } else if(c->num > 0) {

        cursor->pos = maxCursor;

        MoveCursorDownLine(t, cursor);
    }
}

static int Find(char *text, char *str, int len){

    char *res = (char *)malloc(len + 1);
    res[len] = 0;
    memcpy(res, str, len);

    char *pch = strstr(text, res);

    free(res);

    return pch == NULL ? -1 : pch - text;
}

static void SelectNextWord(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    if(t->text == NULL) return;

    if(t->nCursors <= 0) return;

    int f;

    if(t->cursors[0].selection.len == 0){
    
        for(f = 0; f < t->nCursors; f++){

            TextEditorCursor *cursor = &t->cursors[f];

            int start = 0;
            GetWordStartEnd(t->text, cursor->pos, &start, &cursor->pos);

            cursor->selection.startCursorPos = start;
            cursor->selection.len = cursor->pos - start;
        }

    } else {
    
        TextEditorCursor *prev = &t->cursors[t->nCursors-1];

        int startPos, end;

        GetSelectionStartEnd(prev->selection, &startPos, &end);

        int len = end-startPos;

        int next = Find(&t->text[end], &t->text[startPos], len);

        if(next >= 0){

            next += end;

            TextEditorCursor *cursor = AddCursor(t);

            cursor->pos = next+len;
            cursor->selection.startCursorPos = next;
            cursor->selection.len = len;
        }
    }

    ResolveCursorCollisions(t);
}

// static void ExpandSelectionBrackets(TextEditor *t,TextEditorCommand *c){

// }

static void Paste(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    RefreshEditorCommand(c);

    int k;
    for(k = 0; k < t->nCursors; k++){
        
        EraseAllSelectedText(t, k, c);

        if(t->cursors[k].clipboard == NULL) continue;

        AddStrToText(t, t->cursors[k].pos, t->cursors[k].clipboard);

        c->addedLens[k] = strlen(t->cursors[k].clipboard);
    }

    SaveCursors(t, c);

    for(k = 0; k < t->nCursors; k++)
        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));

    SyntaxHighlight(t);
}

static void UndoPaste(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    int k;
    for(k = t->nCursors-1; k >= 0; k--){

        RemoveCharactersFromText(t, t->cursors[k].pos, c->addedLens[k]);

        if(k < c->nSavedCursors && c->savedTexts[k])
            AddStrToText(t, t->cursors[k].pos, c->savedTexts[k]);
    }

    SaveCursors(t, c);

    SyntaxHighlight(t);
}

static void ExpandSelectionLines(TextEditor *t, TextEditorCommand *c){

    if(t->text == NULL) return;

    RemoveExtraCursors(t);

    int f;
    for(f = 0; f < t->nCursors; f++){

        TextEditorCursor *cursor = &t->cursors[f];

        if(cursor->selection.len == 0){
            cursor->selection.startCursorPos = cursor->pos - GetCharsIntoLine(t->text, cursor->pos);
        }

        if(c->num < 0)
            cursor->pos = GetStartOfPrevLine(t->text, cursor->pos);
        else
            cursor->pos = GetStartOfNextLine(t->text, cursor->pos);         

        if(cursor->pos < 0) cursor->pos = 0;
        if(cursor->pos > (int)strlen(t->text)) cursor->pos = strlen(t->text);

        cursor->selection.len = cursor->pos - cursor->selection.startCursorPos;

        if(cursor->selection.startCursorPos+cursor->selection.len < 0)
            cursor->selection.len -= c->num;

        if(cursor->selection.startCursorPos+cursor->selection.len > (int)strlen(t->text))
            cursor->selection.len -= c->num;
    }
}

static void DeleteLine(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    if(t->text == NULL) return;

    RemoveSelections(t);

    int k;
    for(k = 0; k < t->nCursors; k++){

        TextEditorCursor *cursor = &t->cursors[k];

        int start = cursor->pos - GetCharsIntoLine(t->text, cursor->pos);
        int end = GetStartOfNextLine(t->text, cursor->pos);

        cursor->pos = end;

        TextEditorCommand *command = CreateCommand((const int[]){0}, 0, end-start, RemoveCharacters, UndoRemoveCharacters);
        ExecuteCommand(t, command);
        FreeCommand(command);
    }
}

static void Copy(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    if(t->text == NULL) return;

    int k;
    for(k = 0; k < t->nCursors; k++){

        int start, end;

        GetSelectionStartEnd(t->cursors[k].selection, &start, &end);


        if(t->cursors[k].clipboard) free(t->cursors[k].clipboard);

        t->cursors[k].clipboard = (char *)malloc((end-start) + 1);
        t->cursors[k].clipboard[end-start] = 0;

        memcpy(t->cursors[k].clipboard, &t->text[start], end-start);
    }
}

static void Cut(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    TextEditorCommand *command = CreateCommand((const int[]){0}, 0, 1, Copy, NULL);
    ExecuteCommand(t, command);
    FreeCommand(command);

    command = CreateCommand((const int[]){0}, 0, 0, RemoveCharacters, UndoRemoveCharacters);
    ExecuteCommand(t, command);
    FreeCommand(command);
}

static void AddSavedText(TextEditorCommand *command, char *str, int len, int index){
    if(len <= 0) return;
    command->savedTexts[index] = (char *)malloc(len + 1);
    command->savedTexts[index][len] = 0;
    memcpy(command->savedTexts[index], str, len);
}

static void EraseAllSelectedText(TextEditor *t, int index, TextEditorCommand *command){
        
    if(t->text == NULL) return;

    TextEditorCursor *cursor = &t->cursors[index];

    if(t->text == NULL || cursor->selection.len == 0) return;

    int textLen = strlen(t->text);

    int startCursorPos, endCursorPos;

    GetSelectionStartEnd(cursor->selection, &startCursorPos, &endCursorPos);

    int newSize = textLen - (endCursorPos - startCursorPos);

    if(newSize <= 0){

        if(t->text) free(t->text);
        t->text = NULL;

        return;
    }

    AddSavedText(command, &t->text[startCursorPos], endCursorPos - startCursorPos, index);

    memmove(&t->text[startCursorPos], &t->text[endCursorPos], textLen - endCursorPos);

    t->text = (char *)realloc(t->text, newSize+1);
    t->text[newSize] = 0;

    MoveCursorsAndSelection(t, startCursorPos, (startCursorPos - endCursorPos));

    // memset(&cursor->selection, 0, sizeof(TextEditorSelection));
}

static void FindCommand(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    t->searching = 1;
}

static void SaveCursors(TextEditor *t, TextEditorCommand *c){

    if(c->savedCursors){

        free(c->savedCursors);
    }

    c->savedCursors = (TextEditorCursor *)malloc(sizeof(TextEditorCursor) * t->nCursors);
    c->nSavedCursors = t->nCursors;

    int k;
    for(k = 0; k < t->nCursors; k++)
        memcpy(&c->savedCursors[k], &t->cursors[k], sizeof(TextEditorCursor));

}

static void LoadCursors(TextEditor *t, TextEditorCommand *c){

    if(c->nSavedCursors <= 0){
        SaveCursors(t, c);
        return;
    }

    // FreeCursors(t);
    // t->cursors = (TextEditorCursor *)malloc(sizeof(TextEditorCursor) * c->nSavedCursors);

    t->cursors = (TextEditorCursor *)realloc(t->cursors, sizeof(TextEditorCursor) * c->nSavedCursors);

    t->nCursors = c->nSavedCursors;

    int k;
    for(k = 0; k < c->nSavedCursors; k++){
        memcpy(&t->cursors[k], &c->savedCursors[k], sizeof(TextEditorCursor));
    }
}

static void AddStrToText(TextEditor *t, int pos, char *text){
    
    int textLen = 0;

    if(t->text != NULL)
        textLen = strlen(t->text);

    int len = strlen(text);

    t->text = (char *)realloc(t->text, textLen + len + 1);
    t->text[textLen + len] = 0;

    if(textLen - pos > 0)
        memmove(&t->text[pos + len], &t->text[pos], textLen - pos);

    memcpy(&t->text[pos], text, len);

    MoveCursorsAndSelection(t, pos-1, len);

}

static void RemoveCharactersFromText(TextEditor *t, int pos, int len){

    if(t->text == NULL) return;

    int textLen = strlen(t->text);

    if(pos - len < 0) return;

    if(pos < textLen){
        // memmove(&t->text[pos - c->num], &t->text[pos], textLen - (pos+1));
        // memmove(&t->text[pos - c->num], &t->text[pos], textLen - (pos+c->num));
        memmove(&t->text[pos - len], &t->text[pos], textLen - pos);
    }

    t->text = (char *)realloc(t->text, (textLen - len) + 1);
    
    t->text[textLen - len] = 0;

    MoveCursorsAndSelection(t, pos-1, -len);
}

static void UndoRemoveCharacters(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    int k;
    for(k = t->nCursors-1; k >= 0; k--){

        if(k < c->nSavedCursors && c->savedTexts[k])
            AddStrToText(t, t->cursors[k].pos, c->savedTexts[k]);
    }

    SyntaxHighlight(t);

}

static void RemoveCharacters(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    RefreshEditorCommand(c);

    int k;
    for(k = 0; k < t->nCursors; k++){

        if(t->text == NULL) break;
        
        if(t->cursors[k].selection.len == 0){

            int pos = t->cursors[k].pos-c->num;
            int len = c->num;

            if(pos < 0){
                pos = 0;
                len = t->cursors[k].pos;
            }

            AddSavedText(c, &t->text[pos], len, k);
            RemoveCharactersFromText(t, t->cursors[k].pos, c->num);
        }

        if(t->text == NULL) break;

        EraseAllSelectedText(t, k, c);
    }

    SaveCursors(t, c);

    for(k = 0; k < t->nCursors; k++)
        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));

    SyntaxHighlight(t);
}

static void UndoAddCharacters(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    int k;
    for(k = t->nCursors-1; k >= 0; k--){

        RemoveCharactersFromText(t, t->cursors[k].pos, c->addedLens[k]);

        if(k < c->nSavedCursors && c->savedTexts[k])
            AddStrToText(t, t->cursors[k].pos, c->savedTexts[k]);
    }

    SaveCursors(t, c);

    SyntaxHighlight(t);

}

static void RefreshEditorCommand(TextEditorCommand *c){

    if(c->addedLens) free(c->addedLens);

    if(c->savedTexts){

        int k;
        for(k = 0; k < c->nSavedCursors; k++)
            if(c->savedTexts[k]) free(c->savedTexts[k]);
    
        free(c->savedTexts);
    }

    c->addedLens = (int *)malloc(c->nSavedCursors * sizeof(int));
    memset(c->addedLens, 0, sizeof(int) * c->nSavedCursors);

    c->savedTexts = (char **)malloc(c->nSavedCursors * sizeof(char*));
    memset(c->savedTexts, 0, sizeof(char*) * c->nSavedCursors);
}

static void AddCharacters(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    RefreshEditorCommand(c);

    int k;
    for(k = 0; k < t->nCursors; k++){
        
        EraseAllSelectedText(t, k, c);

        AddStrToText(t, t->cursors[k].pos, c->keys);
    
        c->addedLens[k] = strlen(c->keys);
    }

    SaveCursors(t, c);

    for(k = 0; k < t->nCursors; k++)
        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));

    SyntaxHighlight(t);
}

static void Undo(TextEditor *t, TextEditorCommand *c){

    UndoCommands(t, c->num);
}

static void Redo(TextEditor *t, TextEditorCommand *c){

    RedoCommands(t, c->num);
}

static void SearchingAddText(TextEditor *t, char *str){

    int textLen = 0;

    if(t->searchingText)
        textLen = strlen(t->searchingText);

    int len = strlen(str);

    t->searchingText = (char *)realloc(t->searchingText, textLen + len + 1);
    t->searchingText[textLen+len] = 0;

    memcpy(&t->searchingText[textLen], str, len);
}

static void SearchingRemoveChars(TextEditor *t, int num){

    int len = strlen(t->searchingText);

    if(len - num < 0) return;

    t->searchingText = (char *)realloc(t->searchingText, (len-num) + 1);
    t->searchingText[len-num] = 0;
}

static void FreeCommand(TextEditorCommand *c){

    if(c->keys) free(c->keys);
    if(c->keyBinding) free(c->keyBinding);

    if(c->savedCursors){

        free(c->savedCursors);
    }
    
    if(c->savedTexts){

        int k;
        for(k = 0; k < c->nSavedCursors; k++)
            free(c->savedTexts[k]);

        free(c->savedTexts);
    }

    if(c->addedLens) free(c->addedLens);

    free(c);
}

static TextEditorCommand *CopyCommand(TextEditorCommand *c){

    TextEditorCommand *ret = CreateCommand(c->keyBinding, c->keys, c->num, c->Execute, c->Undo);

    ret->savedCursors = (TextEditorCursor *)malloc(sizeof(TextEditorCursor) * c->nSavedCursors);
    ret->nSavedCursors = c->nSavedCursors;

    int k;
    for(k = 0; k < c->nSavedCursors; k++)
        memcpy(&c->savedCursors[k], &ret->savedCursors[k], sizeof(TextEditorCursor));


    if(c->savedTexts){

        ret->savedTexts = (char **)malloc(sizeof(char *) * c->nSavedCursors);

        for(k = 0; k < c->nSavedCursors; k++){

            int len = strlen(c->savedTexts[k]);

            ret->savedTexts[k] = (char *)malloc(len + 1);
            ret->savedTexts[k][len] = 0;

            memcpy(ret->savedTexts[k], c->savedTexts[k], len);
        }
    }

    if(c->addedLens){
        ret->addedLens = (int *)malloc(sizeof(int) * c->nSavedCursors);
        memcpy(ret->addedLens, c->addedLens, sizeof(int) * c->nSavedCursors);
    }

    return ret;
}

static TextEditorCommand *CreateCommand(const int binding[], const char *keys, int n,
    void (*E)(TextEditor *, TextEditorCommand *), void (*U)(TextEditor *, TextEditorCommand *)){

    TextEditorCommand *res = (TextEditorCommand *)malloc(sizeof(TextEditorCommand));
    memset(res, 0, sizeof(TextEditorCommand));

    res->num = n;

    if(binding){

        int len = 0;
        while(binding[len] != 0) len++;

        res->keyBinding = (int *)malloc(len+1);
        res->keyBinding[len+1] = 0;
        memcpy(res->keyBinding, binding, len * sizeof(int));
    }

    if(keys){
        res->keys = (char *)malloc(strlen(keys)+1);
        res->keys[strlen(keys)] = 0;
        memcpy(res->keys, keys, strlen(keys));
    }

    res->Undo = U;
    res->Execute = E;

    return res;

}

static void UndoCommands(TextEditor *t, int num){

    if(t->historyPos - num < 0)
        return;

    int k;
    for(k = 1; k <= num; k++){
        if(t->history[t->historyPos-k]->Undo){
            t->history[t->historyPos-k]->Undo(t, t->history[t->historyPos-k]);
        }
    }

    t->historyPos -= num;
}

static void RedoCommands(TextEditor *t, int num){

    if(t->historyPos + num > t->sHistory)
        num = t->sHistory - t->historyPos;

    if(num <= 0) return;

    int k;
    for(k = 0; k < num; k++)
        t->history[(t->historyPos)+k]->Execute(t, t->history[(t->historyPos)+k]);

    t->historyPos += num;
}

static void RemoveExtraCursors(TextEditor *t){

    t->cursors = (TextEditorCursor *)realloc(t->cursors, sizeof(TextEditorCursor));
    t->nCursors = 1;
}

static void ExecuteCommand(TextEditor *t, TextEditorCommand *c){

    if(c->Undo == NULL){
        c->Execute(t, c);
        return;
    }

    if(t->historyPos < t->sHistory){

        int k;
        for(k = t->historyPos; k < t->sHistory; k++)
            FreeCommand(t->history[k]);

        t->sHistory = t->historyPos;
    }

    t->history = (TextEditorCommand **)realloc(t->history, ++t->sHistory * sizeof(TextEditorCommand));
    t->history[t->sHistory-1] = CopyCommand(c);
    t->history[t->sHistory-1]->Execute(t,t->history[t->sHistory-1]);

    t->historyPos++;
}

static void AddCommand(TextEditor *t, TextEditorCommand *c){
    t->commands = (TextEditorCommand **)realloc(t->commands, ++t->nCommands * sizeof(TextEditorCommand));
    t->commands[t->nCommands-1] = c;
}

void TextEditor_Init(TextEditor *t){
    
    memset(t, 0, sizeof(TextEditor));

    init_pair(COLOR_SIDE_NUMBERS, -1, -1);
    init_pair(COLOR1, -1, -1);
    init_pair(COLOR2, COLOR_CYAN, -1);
    init_pair(COLOR3, COLOR_BLUE, -1);
    init_pair(COLOR7, COLOR_GREEN, -1);
    init_pair(COLOR5, COLOR_RED, -1);
    init_pair(COLOR4, COLOR_YELLOW, -1);
    init_pair(COLOR6, COLOR_MAGENTA, -1);

    init_pair(COLOR_SELECTED, COLOR_BLACK ,COLOR_WHITE);
    init_pair(COLOR_CURSOR, COLOR_BLACK ,COLOR_RED);

    AddCommand(t, CreateCommand((const int[]){8 /* ctrl + h */, 0}, "", 0, SelectNextWord, NULL));
    AddCommand(t, CreateCommand((const int[]){11 /* ctrl + k */, 0}, "", -1, AddCursorCommand, NULL));
    AddCommand(t, CreateCommand((const int[]){12 /* ctrl + l */, 0}, "", 1, AddCursorCommand, NULL));
    AddCommand(t, CreateCommand((const int[]){21 /* ctrl + u */, 0}, "", -1, ExpandSelectionWords, NULL));
    AddCommand(t, CreateCommand((const int[]){16 /* ctrl + p */, 0}, "", 1, ExpandSelectionWords, NULL));
    // AddCommand(t, CreateCommand((const int[]){unbound, 0}, "", -1, ExpandSelectionChars, NULL));
    // AddCommand(t, CreateCommand((const int[]){unbound, 0}, "", 1, ExpandSelectionChars, NULL));
    AddCommand(t, CreateCommand((const int[]){17 /* ctrl + q */, 0}, "", -1, MoveByWords, NULL));
    AddCommand(t, CreateCommand((const int[]){18 /* ctrl + r */, 0}, "", 1, MoveByWords, NULL));
    AddCommand(t, CreateCommand((const int[]){15 /* ctrl + o */, 0}, "", 1, ExpandSelectionLines, NULL));
    AddCommand(t, CreateCommand((const int[]){9 /* ctrl + i */, 0}, "", 1, DeleteLine, NULL));
    AddCommand(t, CreateCommand((const int[]){1 /* ctrl + a */, 0}, "", -1, MoveByChars, NULL));
    AddCommand(t, CreateCommand((const int[]){6 /* ctrl + f */, 0}, "", 1, MoveByChars, NULL));
    AddCommand(t, CreateCommand((const int[]){19 /* ctrl + s */, 0}, "", -1, MoveLines, NULL));
    AddCommand(t, CreateCommand((const int[]){4 /* ctrl + d */, 0}, "", 1, MoveLines, NULL));
    AddCommand(t, CreateCommand((const int[]){26 /* ctrl + z */, 0}, "", 1, Undo, NULL));
    AddCommand(t, CreateCommand((const int[]){14 /* ctrl + n */, 0}, "", 1, Redo, NULL));
    AddCommand(t, CreateCommand((const int[]){24 /* ctrl + x */, 0}, "", 1, Cut, NULL));
    AddCommand(t, CreateCommand((const int[]){3 /* ctrl + c */, 0}, "", 1, Copy, NULL));
    AddCommand(t, CreateCommand((const int[]){22 /* ctrl + v */, 0}, "", 1, Paste, UndoPaste));
    AddCommand(t, CreateCommand((const int[]){31 /* ctrl + / */, 0}, "", 1, FindCommand, NULL));

    FILE *fp = fopen("text_editor.c", "r");

    fseek(fp, 0, SEEK_END);

    int len = ftell(fp);

    rewind(fp);

    t->text = (char *)malloc(len + 1);
    t->text[len] = 0;

    fread(t->text, sizeof(char), len, fp);

    fclose(fp);

    t->cursors = (TextEditorCursor *)malloc(sizeof(TextEditorCursor) * ++t->nCursors);
    memset(&t->cursors[0], 0, sizeof(TextEditorCursor));

    SyntaxHighlight(t);
}

static void DrawSearchBar(TextEditor *t){
}

void TextEditor_Draw(TextEditor *t){

    // if(t->searching) DrawSearchBar(t);

    if(t->text == NULL)
        return;

    int currColor = COLOR1;
    attron(COLOR_PAIR(COLOR1));

    int nDrawCursors = t->nCursors;
    int *drawCursors = (int *)malloc(nDrawCursors * sizeof(int));

    int k;
    for(k = 0; k < nDrawCursors; k++)
        drawCursors[k] = t->cursors[k].pos;

    int nLinesToLastCursor = GetNumLinesToPos(t->text,t->cursors[t->nCursors-1].pos);

    if(nLinesToLastCursor < t->scroll)
        t->scroll = nLinesToLastCursor;
    else if(nLinesToLastCursor >= t->scroll + LINES)
        t->scroll = nLinesToLastCursor - (LINES-1);

    int nDigits = log10(t->scroll + LINES) + 1;

    int startPosColored = GetCharsToLine(t->coloredText, t->scroll);
    int startPos = GetCharsToLine(t->text, t->scroll);

    int cursorPos = startPosColored;

    int x = nDigits+1, y = 0;

    for(k = cursorPos; k < (int)strlen(t->coloredText); k++, cursorPos++){

        if(t->coloredText[k] == '\n'){

            if(y >= 0){

                attroff(COLOR_PAIR(currColor));
                attron(COLOR_PAIR(COLOR1));

                int j;
                for(j = 0; j < COLS; j++)
                    mvprintw(y, x+j, " ");

                attroff(COLOR_PAIR(COLOR1));

                attron(COLOR_PAIR(COLOR_SIDE_NUMBERS));

                for(j = 0; j < nDigits+1; j++)
                    mvprintw(y, j, " ");

                // attron(A_BOLD);
                mvprintw(y, 0, "%i", y+t->scroll);
                // attroff(A_BOLD);

                attroff(COLOR_PAIR(COLOR_SIDE_NUMBERS));
                attron(COLOR_PAIR(currColor));
            }

            x = nDigits+1;
            y++;

            continue;
        }

        GetCursorPos(&t->coloredText[startPosColored], k - startPosColored, &x, &y);

        x += nDigits+1;

        if(t->coloredText[k] == (char)COLOR_CHAR7)      { attroff(COLOR_PAIR(currColor)); attron(COLOR_PAIR((currColor = COLOR7))); --cursorPos; continue; }
        else if(t->coloredText[k] == (char)COLOR_CHAR6) { attroff(COLOR_PAIR(currColor)); attron(COLOR_PAIR((currColor = COLOR6))); --cursorPos; continue; }
        else if(t->coloredText[k] == (char)COLOR_CHAR5) { attroff(COLOR_PAIR(currColor)); attron(COLOR_PAIR((currColor = COLOR5))); --cursorPos; continue; }
        else if(t->coloredText[k] == (char)COLOR_CHAR4) { attroff(COLOR_PAIR(currColor)); attron(COLOR_PAIR((currColor = COLOR4))); --cursorPos; continue; }
        else if(t->coloredText[k] == (char)COLOR_CHAR3) { attroff(COLOR_PAIR(currColor)); attron(COLOR_PAIR((currColor = COLOR3))); --cursorPos; continue; }
        else if(t->coloredText[k] == (char)COLOR_CHAR2) { attroff(COLOR_PAIR(currColor)); attron(COLOR_PAIR((currColor = COLOR2))); --cursorPos; continue; }
        else if(t->coloredText[k] == (char)COLOR_CHAR1) { attroff(COLOR_PAIR(currColor)); attron(COLOR_PAIR((currColor = COLOR1))); --cursorPos; continue; }

        if(x > COLS) continue;
        if(y > LINES) break;

        int localColor = -1;

        int cursorPosNonColored = (cursorPos - startPosColored) + startPos;

        int j;
        for(j = 0; j < t->nCursors; j++){


            if(t->cursors[j].pos == cursorPosNonColored){

                localColor = COLOR_CURSOR;

                int f;
                for(f = j; f < t->nCursors-1; f++)
                    drawCursors[f] = t->cursors[f+1].pos;

                drawCursors = (int *)realloc(drawCursors, sizeof(int) * --nDrawCursors);

            } else {

                TextEditorSelection *selection = &t->cursors[j].selection;

                if(selection->len == 0) continue;

                int startCursorPos;
                int endCursorPos;

                GetSelectionStartEnd(*selection, &startCursorPos, &endCursorPos);

                if(cursorPosNonColored >= startCursorPos && cursorPosNonColored <= endCursorPos)
                    localColor = COLOR_SELECTED;
            }
        }

        if(localColor != -1){
            attroff(COLOR_PAIR(currColor));
            attron(COLOR_PAIR(localColor));
            mvprintw(y, x, "%c", t->coloredText[k]);
            attroff(COLOR_PAIR(localColor));
            attron(COLOR_PAIR(currColor));
        } else {

            mvprintw(y, x, "%c", t->coloredText[k]);
        }

        x++;
    }

    attroff(COLOR_PAIR(currColor));

    if(y >= 0 && y <= LINES){

        attron(COLOR_PAIR(COLOR1));
    
        int j;
        for(j = 0; j < COLS; j++)
            mvprintw(y, x+j, " ");

        x = nDigits+1;
        y++;

        for(; y < LINES; y++){

            for(; x < COLS; x++)
                mvprintw(y, x, " ");
        
            x = nDigits+1;
        }
    
        attroff(COLOR_PAIR(COLOR1));
    }

    attron(COLOR_PAIR(COLOR_CURSOR));

    for(k = 0; k < nDrawCursors; k++){

        int x, y;
        GetCursorPos(t->text, drawCursors[k], &x, &y);

        x += nDigits+1;
        y -= t->scroll;

        if(y < 0 || y > LINES || x > COLS) continue;

        mvprintw(y, x, " ");
    }

    attroff(COLOR_PAIR(COLOR_CURSOR));

    if(drawCursors)
        free(drawCursors);
}

void TextEditor_Event(TextEditor *t, int key){

    if(key == 127){ // backspace

        if(t->searching){
            SearchingRemoveChars(t, 1);
            return;
        }

        TextEditorCommand *command = CreateCommand((const int[]){0}, 0, 1, RemoveCharacters, UndoRemoveCharacters);
        ExecuteCommand(t, command);
        FreeCommand(command);
        return;
    }

    if(key >= 32 && key <= 126){

        if(t->searching){
            SearchingAddText(t, (char[]){key, 0});
            return;
        }

        TextEditorCommand *command = CreateCommand((const int[]){0}, (const char[]){key, 0}, 0, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);
        return;
    }

    if(key == 27){ // escape
        RemoveExtraCursors(t);
        return;
    }

    if(key == 10){ // enter
        TextEditorCommand *command = CreateCommand((const int[]){0}, (const char[]){'\n', 0}, 0, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);
        return;
    }

    if(key == 9){ // tab
        TextEditorCommand *command = CreateCommand((const int[]){0}, "    ", 0, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);
        return;
    }

    int k;
    for(k = 0; k < t->nCommands; k++){

        if(t->commands[k]->keyBinding[0] == key){
            ExecuteCommand(t,t->commands[k]);
            break;
        }
    }
}

void TextEditor_Destroy(TextEditor *t){

    if(t->text) free(t->text);
    if(t->coloredText) free(t->coloredText);

    int k;
    for(k = 0; k < t->nCommands; k++)
        FreeCommand(t->commands[k]);

    FreeCursors(t);
}