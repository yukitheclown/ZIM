#include "graphics.h"
#include "text_editor.h"
#include "file_browser.h"
#include <stdio.h>
#include <fcntl.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX_COMPILE
#include <termios.h>
#include <pty.h>
#endif
#ifdef WINDOWS_COMPILE
#include <fileapi.h>    
#endif
#include <math.h>

#define LOGFILE "kekproject.txt"
#define LOGCOMPILEFILE "keklog.txt"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

enum {
    COLOR_NORMAL = 1,
    COLOR_KEYWORD,
    COLOR_TOKEN,
    COLOR_FUNCTION,
    COLOR_NUM,
    COLOR_COMMENT,
    COLOR_STRING,
    COLOR_SELECTED,
    COLOR_SELECTED_DIRECTORY,
    COLOR_UNSELECTED_DIRECTORY,
    COLOR_LOG_UNSELECTED,
    COLOR_FIND,
    COLOR_LINE_NUM,
    COLOR_AUTO_COMPLETE,
    COLOR_CURSOR,
    COLOR_SIDE_NUMBERS,
    TE_COLOR_BLACK,
    TE_COLOR_RED,
    TE_COLOR_GREEN,
    TE_COLOR_YELLOW,
    TE_COLOR_BLUE,
    TE_COLOR_CYAN,
    TE_COLOR_MAGENTA,
    TE_COLOR_WHITE,
    TE_COLOR_NUM_STANDARD,
};

enum {
  SCR_NORM = 0,
  SCR_CENT,
};

static void FreeFile(TextEditorFile *f);
static void RefreshFile(TextEditor *t);
static int AddFile(TextEditor *t, TextEditorFile *f);
static TextEditorFile *CreateTextEditorFile(char *path);
static void EndLogging(TextEditor *t);
static int FindInsensitive(char *text, char *str, int len);
static void FindTextGoto(TextEditor *t, int dir, int insensitive);
static void ScrollToLine(TextEditor *t);
static int MoveByWordsFunc(char *text, int len, int start, int dir);
static void DoSwitchFile(TextEditor *t);
static void DoFileBrowser(TextEditor *t);
static void FindTextInsensitive(TextEditor *t, TextEditorCommand *c);
static void CloseFile(TextEditor *t, TextEditorCommand *c);
static void AddSavedText(TextEditor *t, char *str, int len, int *cursorIndex);
static void EraseAllSelectedText(TextEditor *t, int *cursorIndex, TextEditorCommand *c);
static void AutoComplete(TextEditor *t);
static int IsToken(char c);
static char CaseLower(char c);
static int IsDigit(char c);
static int GetStartOfNextLine(char *text, int textLen, int cPos);
static int GetNumLinesToPos(char *text, int cPos);
static int GetCharsIntoLine(char *text, int cPos);
static void GetWordStartEnd(char *text, int cPos, int *s, int *e);
static int GetWordEnd(char *text, int cPos);
static int GetWordStart(char *text, int cPos);
static int GetStartOfPrevLine(char *text, int cPos);
static void DoOpenFile(TextEditor *t);
static void DoSaveFile(TextEditor *t);
static void OpenFile(TextEditor *t, TextEditorCommand *c);
static void NewFile(TextEditor *t, TextEditorCommand *c);
static void SwitchFile(TextEditor *t, TextEditorCommand *c);
void TextEditor_LoadFile(TextEditor *t, char *path);
static void SaveAsFile(TextEditor *t, TextEditorCommand *c);
static void SaveFile(TextEditor *t, TextEditorCommand *c);
// static void RefreshEditorCommand(TextEditorCommand *c);
static void ResolveCursorCollisions(TextEditor *t, int *cursorIndex);
static void MoveCursorsAndSelection(TextEditor *t, int pos, int by, int *cursorIndex);
static void MoveLineUp(TextEditor *t, TextEditorCursor *c);
static void MoveLineDown(TextEditor *t, TextEditorCursor *c);
static void RemoveSelections(TextEditor *t);
static void OpenFileBrowser(TextEditor *t, TextEditorCommand *c);
static void SetCursorToSelection(TextEditorCursor *cursor, int n);
static void MoveByChars(TextEditor *t, TextEditorCommand *c);
static void SelectAll(TextEditor *t, TextEditorCommand *c);
static void MoveByWords(TextEditor *t, TextEditorCommand *c);
static void FreeCursors(TextEditor *t);
static void InitCursors(TextEditor *t);
static int MoveCursorUpLine(TextEditor *t, TextEditorCursor *cursor);
static int MoveCursorDownLine(TextEditor *t, TextEditorCursor *cursor);
static void MoveLines(TextEditor *t, TextEditorCommand *c);
// static void ExpandSelectionChars(TextEditor *t,TextEditorCommand *c);
static void ExpandSelectionWords(TextEditor *t,TextEditorCommand *c);
static void AddCursorCommand(TextEditor *t, TextEditorCommand *c);
static TextEditorCursor *AddCursor(TextEditor *t);
static int Find(char *text, char *str, int len);
static void SelectNextWord(TextEditor *t, TextEditorCommand *c);
static void EventEnter(TextEditor *t);
static void EventCtrlEnter(TextEditor *t, TextEditorCommand *c);
static void SaveCursors(TextEditor *t, TextEditorCommand *c);
static void LoadCursors(TextEditor *t, TextEditorCommand *c);
static void AddStrToText(TextEditor *t, int *cursorIndex, char *text);
static void RemoveStrFromText(TextEditor *t, int *cursorIndex, int len);
static void UndoRemoveCharacters(TextEditor *t, TextEditorCommand *c);
static void RemoveCharacters(TextEditor *t, TextEditorCommand *c);
static void UndoAddCharacters(TextEditor *t, TextEditorCommand *c);
static void AddCharacters(TextEditor *t, TextEditorCommand *c);
static void Undo(TextEditor *t, TextEditorCommand *c);
static void Copy(TextEditor *t, TextEditorCommand *c);
static void MoveBrackets(TextEditor *t, TextEditorCommand *c);
static int GetBetweenBrackets(char *text, int len, int pos, int *first, int *last);
static void ExpandSelectionLines(TextEditor *t, TextEditorCommand *c);
static void DeleteLine(TextEditor *t, TextEditorCommand *c);
static void UndoDeleteLine(TextEditor *t, TextEditorCommand *c);
static void Cut(TextEditor *t, TextEditorCommand *c);
static void Redo(TextEditor *t, TextEditorCommand *c);
static void FreeCommand(TextEditorCommand *c);
static void UpdateScrollCenter(TextEditor *t);
static void UpdateScroll(TextEditor *t);
static TextEditorCommand *CopyCommand(TextEditorCommand *c);
static TextEditorCommand *CreateCommand(const unsigned int binding[], const char *keys, int n, int scroll,
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

static int IsDigit(char c){
    if((c == '0' || c == '1' || c == '2' || c == '3' || 
        c == '4' || c == '5' || c == '6' || c == '7' || 
        c == '8' || c == '9')) return 1;

    return 0;

}
static char CaseLower(char c){
    if(c > 'A' && c < 'Z')
        c -= 'A' - 'a';
    return c;
}
static int CaseLowerStrnCmp(char *str, char *cmp, int len){
    int k;
    for(k = 0; k < len; k++){
        if(CaseLower(str[k]) != CaseLower(cmp[k])) break;
    }
    return len == k;

}

static int IsToken(char c){
    if(c == '(' || c == ' ' || c == '\n'|| c == ',' || c == '+' || c == '=' || c == '~' || c == '<' || 
        c == '>' || c == '*' || c == ')' || c == '/' || c == '{' || c == '}' || c == '%' || c == '^' || 
        c == ':' || c == '.' || c == ';' || c == ']' || c == '[' || c == '-' || c == ']' || c == '"' ||
        c == '\''|| c == '\t'|| c == '\0' || c == '#' || c == '&' || c == '!') return 1;

    return 0;
}

static int GetNumLinesToPos(char *text, int cPos){

    int k, lines = 0;
    for(k = 0; k < cPos; k++)
        if(text[k] == '\n') lines++;

    return lines;
}


static int GetCharsIntoLine(char *text, int cPos){

    int k = cPos > 0 ? cPos-1 : 0;

    for(; k > 0; k--)
        if(text[k] == '\n') { k++; break; }

    return (cPos - k);
}

static void EndLogging(TextEditor *t){
    t->logIndex = -1;
    if(t->loggingText) free(t->loggingText);

    if(t->logging == LOGMODE_CONSOLE){
#ifdef LINUX_COMPILE
        fsync(STDERR_FILENO);
        fsync(STDOUT_FILENO);
        dup2(t->_stderr, STDERR_FILENO);
        dup2(t->_stdout, STDOUT_FILENO);
        close(t->ttyMaster);
        close(t->ttySlave);
#endif
    }

    t->logging = 0;
    t->loggingText = NULL;
}

static void FindTextGoto(TextEditor *t, int dir, int insensitive){

    RemoveSelections(t);
    RemoveExtraCursors(t);
    if(!t->loggingText) return;
    int searchLen = strlen(t->loggingText);

    int (*sensitiveFind)(char *text, char *str, int len);
    if(insensitive)
        sensitiveFind = FindInsensitive;
    else
        sensitiveFind = Find;

    if(dir > 0){

        int next = sensitiveFind(&t->file->text[t->cursors[0].pos], t->loggingText, searchLen);

        if(next < 0 || t->cursors[0].pos == strlen(t->file->text)){
    
            next = sensitiveFind(&t->file->text[0], t->loggingText, searchLen);
    
            if(next < 0)
                next = 0;
            else
                t->cursors[0].selection.len = searchLen;

            t->cursors[0].pos = next;
        
        } else if(next == 0){
            next = sensitiveFind(&t->file->text[t->cursors[0].pos+searchLen], t->loggingText, searchLen);

            if(next < 0){
                next = sensitiveFind(&t->file->text[0], t->loggingText, searchLen);
                t->cursors[0].pos = next;
                
                if(next >= 0)
                    t->cursors[0].selection.len = searchLen;
            } else {
                t->cursors[0].pos += next + searchLen;
                t->cursors[0].selection.len = searchLen;
            }
        } else {
            t->cursors[0].pos += next;
            t->cursors[0].selection.len = searchLen;
        }

        t->cursors[0].selection.startCursorPos = t->cursors[0].pos;
    }

    if(dir < 0){

        t->file->textLen = strlen(t->file->text);

        int curr = sensitiveFind(&t->file->text[0], t->loggingText, searchLen);
        if(curr < 0) {
            return;
        }

        int start = curr;
        int next = 0;

        do{
            curr += next+searchLen;
            next = sensitiveFind(&t->file->text[curr], t->loggingText, searchLen);

        } while(next > 0 && curr+next+searchLen < t->cursors[0].pos && curr+next+searchLen < t->file->textLen);


        if(curr-searchLen == start && t->cursors[0].pos == curr-searchLen){ // sensitiveFind last occurance
            do{
                curr += next+searchLen;
                next = sensitiveFind(&t->file->text[curr], t->loggingText, searchLen);

            } while(next > 0 && curr+next+searchLen < t->file->textLen);

        }

        t->cursors[0].pos = curr-searchLen;            
        t->cursors[0].selection.startCursorPos = t->cursors[0].pos;
        t->cursors[0].selection.len = searchLen;
    }
    
    UpdateScrollCenter(t);
}
static void UpdateScroll(TextEditor *t){

    if(t->file->text == NULL) return;

    int nLinesToCursor = GetNumLinesToPos(t->file->text,t->cursors[t->nCursors-1].pos);

    if(nLinesToCursor < t->file->scroll)
        t->file->scroll = nLinesToCursor;
    else if(nLinesToCursor >= (t->file->scroll + Graphics_TextCollumns())  )
        t->file->scroll = (nLinesToCursor - Graphics_TextCollumns())+1;
}


static void UpdateScrollCenter(TextEditor *t){

    int nLinesToCursor = GetNumLinesToPos(t->file->text,t->cursors[t->nCursors-1].pos);

	if(nLinesToCursor >= t->file->scroll && nLinesToCursor < t->file->scroll + Graphics_TextCollumns()){
		return;
	}	

    t->file->scroll = nLinesToCursor  - (Graphics_TextCollumns()/2);

    if(t->file->scroll < 0) t->file->scroll = 0;


}

static void ScrollToLine(TextEditor *t){

    t->logging = 0;
    RemoveSelections(t);
    RemoveExtraCursors(t);

    if(!t->loggingText) return;

    int line = atoi(t->loggingText);

    free(t->loggingText);
    t->loggingText = NULL;

    if(line == 0){
        t->cursors[0].pos = 0;
        return;
    }
    if(!t->file->text) return;

    t->file->textLen = strlen(t->file->text);

    int scroll = 0, scrollPos = 0;

    int k;
    for(k = 0; k < t->file->textLen; k++){

        if(t->file->text[k] == '\n'){
            scroll++;
            if(scroll == line+1){
                break;
            }
            scrollPos = k+1;
        }
    }

    if(scroll == 0) return;

    if(scrollPos >= t->file->textLen)
        scrollPos = t->file->textLen-1;

    t->cursors[0].pos = scrollPos;
    UpdateScrollCenter(t);
}

static void EventCtrlEnter(TextEditor *t, TextEditorCommand *c){
    if(t->logging == LOGMODE_TEXT) FindTextGoto(t, -1, 0);
    if(t->logging == LOGMODE_TEXT_INSENSITIVE) FindTextGoto(t, -1, 1);
}

static void EventEnter(TextEditor *t){

    if(t->logging == LOGMODE_SAVE){
        if(!t->loggingText) return;
        strcpy(t->file->path,t->loggingText);
        DoSaveFile(t);
        return;
    }
    if(t->logging == LOGMODE_OPEN){
        DoOpenFile(t);
        return;
    }
    if(t->logging == LOGMODE_SWITCH_FILE){
        DoSwitchFile(t);
        return;
    }
    if(t->logging == LOGMODE_FILEBROWSER){
        DoFileBrowser(t);
        return;
    }
    if(t->logging == LOGMODE_NUM){
        ScrollToLine(t);
        return;
    }
    if(t->logging == LOGMODE_TEXT){
        FindTextGoto(t, 1, 0);
        return;
    }
    if(t->logging == LOGMODE_TEXT_INSENSITIVE){
        FindTextGoto(t, 1, 1);
        return;
    }

    if(t->autoCompleteLen){
        AutoCompleteOffset *ac = &t->autoComplete[t->autoCompleteIndex];

        char buffer[MAX_AUTO_COMPLETE_STRLEN];
        int len = ac->len - t->autoCompleteSearchLen;
        int offset = ac->offset+t->autoCompleteSearchLen;
        memcpy(buffer, &t->file->text[offset], len);
        buffer[len] = 0;


        TextEditorCommand *command = 
        CreateCommand((const unsigned int[]){0}, buffer, 0, SCR_CENT, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);

        t->autoCompleteSearchLen = 0;
        t->autoCompleteLen = 0;
        t->autoCompleteIndex = 0;
        return;
    }

    TextEditorCommand *command = 
    CreateCommand((const unsigned int[]){0}, (const char[]){'\n', 0}, 0, SCR_CENT, AddCharacters, UndoAddCharacters);
    ExecuteCommand(t,command);
    FreeCommand(command);
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
    for(k=cPos; k < (int)strlen(text); k++)
        if(IsToken(text[k])) break;

    return k;
}

static int GetWordStart(char *text, int cPos){

    int k;
    for(k=cPos; k >= 0; k--)
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

static void ResolveCursorCollisions(TextEditor *t, int *cursorIndex){

    int j, f;
    for(j = 0; j < t->nCursors; j++){
        for(f = 0; f < t->nCursors; f++){
            if(f != j && t->cursors[j].pos == t->cursors[f].pos){
                
                

                if(j+1 < t->nCursors){

                    if(t->cursors[j].savedText){
                        
                        if(t->cursors[f].savedText){
                            int len1 = strlen(t->cursors[j].savedText);
                            int len2 = strlen(t->cursors[f].savedText);
                            t->cursors[j].savedText = realloc(t->cursors[j].savedText, len1+len2+1);
                            t->cursors[j].savedText[len1+len2] = 0;

                            memcpy(&t->cursors[j].savedText[len1], t->cursors[f].savedText, len2);
                        }
                        
                        free(t->cursors[f].savedText);

                        t->cursors[f].savedText = t->cursors[j].savedText;
                    }

                    t->cursors[f].addedLen += t->cursors[j].addedLen;
                }

                int m;
                for(m = j; m < t->nCursors-1; m++){
                    t->cursors[m] = t->cursors[m+1];
                }

                t->cursors = realloc(t->cursors, --t->nCursors * sizeof(TextEditorCursor));                 


                if(cursorIndex && *cursorIndex >= j)
                    (*cursorIndex)--;

                j--;
                break;
            }
        }
    }
}

static void MoveCursorsAndSelection(TextEditor *t, int pos, int by, int *cursorIndex){


    int k;
    for(k = 0; k < t->nCursors; k++){
        if(k == *cursorIndex) continue;

        if(t->cursors[k].selection.len > 0){

            if(t->cursors[k].selection.startCursorPos > pos){
                t->cursors[k].selection.startCursorPos += by;
            }
        }

        if(t->cursors[k].pos > pos)
            t->cursors[k].pos += by;

    }
    ResolveCursorCollisions(t, cursorIndex);
}

static void RemoveSelections(TextEditor *t){
    
    t->selectNextWordTerminator = 0;    
    t->autoCompleteSearchLen = 0;
    t->autoCompleteLen = 0;
    t->autoCompleteIndex = 0;

    // memset(&t->cursors[0].selection, 0, sizeof(TextEditorSelection));

    // if(t->nCursors > 1)
    //  t->cursors = (TextEditorCursor *)realloc(t->cursors, sizeof(TextEditorCursor));
    // t->nCursors = 1;

    int k;
    for(k = 0; k < t->nCursors; k++)
        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));

}

static void SetCursorToSelection(TextEditorCursor *cursor, int n){

    if(n < 0) cursor->pos = cursor->selection.startCursorPos;
    else cursor->pos = cursor->selection.startCursorPos+cursor->selection.len;
}


static void MoveByChars(TextEditor *t, TextEditorCommand *c){

    int textLen = t->file->textLen;

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

    ResolveCursorCollisions(t, 0);
}
static int MoveByWordsFunc(char *text, int len, int start, int dir){

    if((start > len && dir > 0) || (start == 0 && dir < 0)) return start;

    if(dir < 0 && start > 0){
        
        if(text[start-1] == '\n'){
            start -= 1;
        } else {

            // @#$|(a)sdff             
            // if(!IsToken(text[start] && IsToken(text[start-1]))){
            if(text[start] == '\n'){
                start--;
            } else {

                if(IsToken(text[start-1]))
                    start -= 2;

                if(text[start] != '\n' && IsToken(text[start])){
                    while(start >= 0){
                        char c = text[--start];
                        if(c == '\n') { ++start; break; }
                        if(!IsToken(c)) { break; }
                    }
                } else {
                    start = GetWordStart(text, start); 
                }
            }
        }

    } else {

        // if(!IsToken(text[start] && IsToken(text[start+1])))
        if(text[start] == '\n'){

            start++;

        } else {

            if(IsToken(text[start]))
                start++;

            if(text[start] != '\n' && text[start-1] != '\n' && IsToken(text[start])){
                while(start < len){
                    char c = text[++start];
                    if(c == '\n') { --start; break; }
                    if(!IsToken(c)) break;
                }
            } else {
                int  end = GetWordEnd(text, start);
                start = end;
            }
        }
    }


    if(start < 0) start = 0;
    if(start > len) start = len;

    return start;
}

static void MoveByWords(TextEditor *t, TextEditorCommand *c){

    int k;
    for(k = 0; k < t->nCursors; k++){

        if(t->cursors[k].selection.len != 0){
            SetCursorToSelection(&t->cursors[k], c->num);
            continue;   
        }

        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));


        t->cursors[k].pos = MoveByWordsFunc(t->file->text, t->file->textLen,t->cursors[k].pos,c->num);
    }

    ResolveCursorCollisions(t,0);
}
static void InitCursors(TextEditor *t){
    t->cursors = (TextEditorCursor *)malloc(sizeof(TextEditorCursor) * ++t->nCursors);
    memset(&t->cursors[0], 0, sizeof(TextEditorCursor));
}
static void FreeCursors(TextEditor *t){

    int k;
    for(k = 0; k < t->nCursors; k++){
        if(t->cursors[k].clipboard)
            free(t->cursors[k].clipboard);
        if(t->cursors[k].savedText)
            free(t->cursors[k].savedText);
    }

    t->nCursors = 0;
    free(t->cursors);
    t->cursors = NULL;
}

static void MoveLineUp(TextEditor *t, TextEditorCursor *cursor){
    if(t->file->text == NULL) return;

    // int k;
    // for(k = 0; k < t->nCursors; k++){
    
    // TextEditorCursor *cursor = &t->cursors[k];            


    if(cursor->selection.len == 0){

        int lineStart = cursor->pos == 0 ? 0 : cursor->pos-1;
        for(; lineStart > 0 && t->file->text[lineStart] != '\n'; lineStart--);
        if(lineStart == 0) return; // already top line
        lineStart++; // skip \n of prev line

        int charsOnLine = cursor->pos;
        for(; charsOnLine < t->file->textLen && t->file->text[charsOnLine] != '\n'; charsOnLine++);
        charsOnLine = (charsOnLine - lineStart);

        int startOfPrevLine = lineStart-2;
        for(; startOfPrevLine > 0 && t->file->text[startOfPrevLine] != '\n'; startOfPrevLine--);
        if(startOfPrevLine > 0) startOfPrevLine++;
        int prevLineLen = lineStart - startOfPrevLine;

        if(prevLineLen > charsOnLine){

            char *tmp = malloc(charsOnLine);
            memcpy(tmp, &t->file->text[lineStart], charsOnLine);

            memcpy(&t->file->text[startOfPrevLine+charsOnLine+1], &t->file->text[startOfPrevLine], prevLineLen);
            t->file->text[startOfPrevLine+charsOnLine] = '\n';
            memcpy(&t->file->text[startOfPrevLine], tmp, charsOnLine);

            free(tmp);
        } else {
            char *tmp = malloc(prevLineLen);
            memcpy(tmp, &t->file->text[startOfPrevLine], prevLineLen);

            memcpy(&t->file->text[startOfPrevLine], &t->file->text[lineStart], charsOnLine);
            t->file->text[startOfPrevLine+charsOnLine] = '\n';
            memcpy(&t->file->text[startOfPrevLine+charsOnLine+1], tmp, prevLineLen);
            free(tmp);
        }

        cursor->pos = startOfPrevLine + (cursor->pos - lineStart);
    } else {

        int lineStart = cursor->selection.startCursorPos == 0 ? 0 : cursor->selection.startCursorPos-1;
        for(; lineStart > 0 && t->file->text[lineStart] != '\n'; lineStart--);
        if(lineStart > 0) lineStart++; // skip \n of prev line

        int charsOnLine = cursor->selection.startCursorPos+cursor->selection.len-1;
        for(; charsOnLine < t->file->textLen && t->file->text[charsOnLine] != '\n'; charsOnLine++);
        if(charsOnLine >= t->file->textLen || t->file->text[charsOnLine] != '\n') return; // end of file dont move down
        charsOnLine = (charsOnLine - lineStart);

        int startOfPrevLine = lineStart-2;
        for(; startOfPrevLine > 0 && t->file->text[startOfPrevLine] != '\n'; startOfPrevLine--);
        if(startOfPrevLine > 0) startOfPrevLine++;
        int prevLineLen = lineStart - startOfPrevLine;

        if(prevLineLen > charsOnLine){

            char *tmp = malloc(charsOnLine);
            memcpy(tmp, &t->file->text[lineStart], charsOnLine);

            memcpy(&t->file->text[startOfPrevLine+charsOnLine+1], &t->file->text[startOfPrevLine], prevLineLen);
            t->file->text[startOfPrevLine+charsOnLine] = '\n';
            memcpy(&t->file->text[startOfPrevLine], tmp, charsOnLine);

            free(tmp);

        } else {
            char *tmp = malloc(prevLineLen);
            memcpy(tmp, &t->file->text[startOfPrevLine], prevLineLen);

            memcpy(&t->file->text[startOfPrevLine], &t->file->text[lineStart], charsOnLine);
            t->file->text[startOfPrevLine+charsOnLine] = '\n';
            memcpy(&t->file->text[startOfPrevLine+charsOnLine+1], tmp, prevLineLen);
            free(tmp);

        }

        cursor->selection.startCursorPos = startOfPrevLine + (cursor->selection.startCursorPos - lineStart);
        cursor->pos = startOfPrevLine + (cursor->pos - lineStart);

    }
    // }
}

static void MoveLineDown(TextEditor *t, TextEditorCursor *cursor){
    if(t->file->text == NULL) return;

    // int k;
    // for(k = 0; k < t->nCursors; k++){
        
    //     TextEditorCursor *cursor = &t->cursors[k];            


    if(cursor->selection.len == 0){

        int lineStart = cursor->pos == 0 ? 0 : cursor->pos-1;
        for(; lineStart > 0 && t->file->text[lineStart] != '\n'; lineStart--);
        if(lineStart > 0) lineStart++; // skip \n of prev line

        int charsOnLine = cursor->pos;
        for(; charsOnLine < t->file->textLen && t->file->text[charsOnLine] != '\n'; charsOnLine++);
        if(charsOnLine >= t->file->textLen || t->file->text[charsOnLine] != '\n') return; // end of file dont move down
        charsOnLine = (charsOnLine - lineStart);

        int startOfNextLine = lineStart+charsOnLine+1;
        int nextLineLen = startOfNextLine;
        for(; nextLineLen < t->file->textLen && t->file->text[nextLineLen] != '\n'; nextLineLen++);

        nextLineLen = (nextLineLen - startOfNextLine);

        if(nextLineLen > charsOnLine){
            char *tmp = malloc(charsOnLine);
            memcpy(tmp, &t->file->text[lineStart], charsOnLine);
            memcpy(&t->file->text[lineStart], &t->file->text[startOfNextLine], nextLineLen);
            t->file->text[lineStart+nextLineLen] = '\n';
            memcpy(&t->file->text[lineStart+nextLineLen+1], tmp, charsOnLine);

            free(tmp);
        } else {
            char *tmp = malloc(nextLineLen);
            memcpy(tmp, &t->file->text[startOfNextLine], nextLineLen);

            memcpy(&t->file->text[lineStart+nextLineLen+1], &t->file->text[lineStart], charsOnLine);
            t->file->text[lineStart+nextLineLen] = '\n';
            memcpy(&t->file->text[lineStart], tmp, nextLineLen);
            free(tmp);
        }

        cursor->pos = lineStart+nextLineLen+1 + (cursor->pos - lineStart);
    } else {

        int lineStart = cursor->selection.startCursorPos == 0 ? 0 : cursor->selection.startCursorPos-1;
        for(; lineStart > 0 && t->file->text[lineStart] != '\n'; lineStart--);
        if(lineStart > 0) lineStart++; // skip \n of prev line

        int charsOnLine = cursor->selection.startCursorPos+cursor->selection.len-1;
        for(; charsOnLine < t->file->textLen && t->file->text[charsOnLine] != '\n'; charsOnLine++);
        if(charsOnLine >= t->file->textLen || t->file->text[charsOnLine] != '\n') return; // end of file dont move down

        charsOnLine = (charsOnLine - lineStart);

        int startOfNextLine = lineStart+charsOnLine+1;
        int nextLineLen = startOfNextLine;
        for(; nextLineLen < t->file->textLen && t->file->text[nextLineLen] != '\n'; nextLineLen++);

        nextLineLen = (nextLineLen - startOfNextLine);

        if(nextLineLen > charsOnLine){
            char *tmp = malloc(charsOnLine);
            memcpy(tmp, &t->file->text[lineStart], charsOnLine);
            memcpy(&t->file->text[lineStart], &t->file->text[startOfNextLine], nextLineLen);
            t->file->text[lineStart+nextLineLen] = '\n';
            memcpy(&t->file->text[lineStart+nextLineLen+1], tmp, charsOnLine);

            free(tmp);
        } else {
            char *tmp = malloc(nextLineLen);
            memcpy(tmp, &t->file->text[startOfNextLine], nextLineLen);

            memcpy(&t->file->text[lineStart+nextLineLen+1], &t->file->text[lineStart], charsOnLine);
            t->file->text[lineStart+nextLineLen] = '\n';
            memcpy(&t->file->text[lineStart], tmp, nextLineLen);
            free(tmp);
        }

        cursor->pos = lineStart+nextLineLen+1 + (cursor->pos - lineStart);
        cursor->selection.startCursorPos = lineStart+nextLineLen+1 + (cursor->selection.startCursorPos - lineStart);

    }
    // }
}

static void MoveLinesText(TextEditor *t, TextEditorCommand *c){
    LoadCursors(t,c);
    int k;

    if(t->file->text == NULL || !t->file->textLen)
        return;

    for(k = 0; k < t->nCursors; k++){

        TextEditorCursor *cursor = &t->cursors[k];

        if(c->num > 0)
            MoveLineDown(t, cursor);
        else if(c->num < 0 && GetNumLinesToPos(t->file->text, cursor->pos) > 0)
            MoveLineUp(t, cursor);
    }

    SaveCursors(t,c);
    ResolveCursorCollisions(t,0);
}

static void UndoMoveLinesText(TextEditor *t, TextEditorCommand *c){

    int k;
    LoadCursors(t,c);

    if(t->file->text == NULL || !t->file->textLen)
        return;

    for(k = 0; k < t->nCursors; k++){

        TextEditorCursor *cursor = &t->cursors[k];

        if(c->num > 0)
            MoveLineUp(t, cursor);
        else if(c->num < 0 && GetNumLinesToPos(t->file->text, cursor->pos) > 0)
            MoveLineDown(t, cursor);
    }

    ResolveCursorCollisions(t,0);
    SaveCursors(t,c);
}


static void GotoLine(TextEditor *t, TextEditorCommand *c){
    EndLogging(t);
    t->logging = LOGMODE_NUM;
}

static void DoOpenFile(TextEditor *t){
    TextEditor_LoadFile(t, t->loggingText);
    RefreshFile(t);
    EndLogging(t);
}

static void DoFileBrowser(TextEditor *t){

    int matches = 0;
    int k;
    for(k = 0; k < t->fileBrowser.nFiles; k++){
        int nameLen = strlen(t->fileBrowser.files[k].name);
        int logNameLen = t->loggingText ? strlen(t->loggingText) : 0;
        
        if(nameLen > 0 && logNameLen <= nameLen){
            if(logNameLen == 0 || CaseLowerStrnCmp(t->loggingText, t->fileBrowser.files[k].name, logNameLen)){
                if(t->logIndex == matches){
                    if(t->fileBrowser.files[k].dir){
                        strcpy(t->fileBrowser.directory, t->fileBrowser.files[k].path);
                        FileBrowser_ChangeDirectory(&t->fileBrowser);
                        t->logIndex = 0;
                    } else {
                        TextEditor_LoadFile(t, t->fileBrowser.files[k].path);
                        RefreshFile(t);
                        EndLogging(t);
                    }
                }
                matches++;
            }
        }
    }
}

static void DoSwitchFile(TextEditor *t){

    t->file->cursorPos = t->cursors[0].pos;

    if(t->loggingText && strlen(t->loggingText) > 0){
        int matches = 0;
        int k;
        for(k = 0; k < t->nFiles; k++){
            int nameLen = strlen(t->files[k]->name);
            int logNameLen = strlen(t->loggingText);
            
            if(nameLen > 0 && logNameLen <= nameLen){
                if(CaseLowerStrnCmp(t->loggingText, t->files[k]->name, logNameLen)){
                    if(t->logIndex == matches){

                        t->file = t->files[k];

                        if(t->file != t->files[k]){
                            int j;
                            for(j = 1; j <= k; j++){
                                t->files[j] = t->files[j-1];
                            }
                            t->files[0] = t->file; // stack like
                        }
                        break;
                    }
                    matches++;
                }
            }
        }
    }

    RefreshFile(t);
    EndLogging(t);
}

static void DoSaveFile(TextEditor *t){
    FILE *fp = fopen(t->file->path, "w");
    fwrite(t->file->text,1,strlen(t->file->text),fp);
    fclose(fp);
    EndLogging(t);
}
static void RefreshFile(TextEditor *t){
    FreeCursors(t);
    InitCursors(t);
    t->cursors[0].pos = t->file->cursorPos;
    t->autoCompleteSearchLen = 0;
    t->autoCompleteLen = 0;
    t->autoCompleteIndex = 0;
    t->selectNextWordTerminator = 0;
}
static void NewFile(TextEditor *t, TextEditorCommand *c){
    TextEditor_LoadFile(t, NULL);
    RefreshFile(t);
}
static void CloseFile(TextEditor *t, TextEditorCommand *c){
    int k;
    for(k = 0; k < t->nFiles; k++){
        if(t->files[k] == t->file){
            FreeFile(t->file);
            t->file = NULL;
            for(; k < t->nFiles-1; k++)
                t->files[k] = t->files[k+1];
            
            t->files = (TextEditorFile **)realloc(t->files,sizeof(TextEditorFile*) * t->nFiles--);
            break;
        }
    }
    if(t->nFiles == 0){
        TextEditor_LoadFile(t, NULL);
    }
    else{
        t->file = t->files[0];
    }

    RefreshFile(t);
}
static void OpenFileBrowser(TextEditor *t, TextEditorCommand *c){
    EndLogging(t);
    t->logIndex = 0;
    t->logging = LOGMODE_FILEBROWSER;
}
static void OpenFile(TextEditor *t, TextEditorCommand *c){
    EndLogging(t);
    t->logging = LOGMODE_OPEN;
}
static void SwitchFile(TextEditor *t, TextEditorCommand *c){
    EndLogging(t);
    t->logging = LOGMODE_SWITCH_FILE;
}
static void SaveAsFile(TextEditor *t, TextEditorCommand *c){
    EndLogging(t);
    t->logging = LOGMODE_SAVE;
}
static void SaveFile(TextEditor *t, TextEditorCommand *c){
    EndLogging(t);
    if(strlen(t->file->path) == 0){
        t->logging = LOGMODE_SAVE;
        return;
    }
    DoSaveFile(t);
}

static void FindText(TextEditor *t, TextEditorCommand *c){
    EndLogging(t);
    t->logging = LOGMODE_TEXT;
}

static void FindTextInsensitive(TextEditor *t, TextEditorCommand *c){
    EndLogging(t);
    t->logging = LOGMODE_TEXT_INSENSITIVE;
}

static int MoveCursorUpLine(TextEditor *t, TextEditorCursor *cursor){

    if(t->file->text == NULL) return 0;
    if(GetNumLinesToPos(t->file->text, cursor->pos) == 0) return 0;

    int charsIntoLine = GetCharsIntoLine(t->file->text, cursor->pos);
    int startOfPrevLine = GetStartOfPrevLine(t->file->text, cursor->pos);

    int k;
    for(k = startOfPrevLine; k < (int)t->file->textLen; k++)
        if(t->file->text[k] == '\n') break;

    int charsOnPrevLine = (k - startOfPrevLine);
    cursor->pos = charsIntoLine <= charsOnPrevLine ? startOfPrevLine + charsIntoLine: startOfPrevLine + charsOnPrevLine; 

    return 1;
}

static int GetStartOfNextLine(char *text, int textLen, int cPos){

    int k;
    for(k = cPos; k < textLen; k++)
        if(text[k] == '\n') { k++; break; }

    return k;
}

static int MoveCursorDownLine(TextEditor *t, TextEditorCursor *cursor){

    if(t->file->text == NULL) return 0;

    t->file->textLen = strlen(t->file->text);

    int charsIntoLine = GetCharsIntoLine(t->file->text, cursor->pos);
    int startOfNextLine = GetStartOfNextLine(t->file->text, t->file->textLen, cursor->pos);

    if(startOfNextLine == (int)t->file->textLen) return 0;

    int k;
    for(k = startOfNextLine; k < (int)t->file->textLen; k++)
        if(t->file->text[k] == '\n') break;

    int charsOnNextLine = (k - startOfNextLine);

    cursor->pos = charsIntoLine <= charsOnNextLine ? startOfNextLine + charsIntoLine : startOfNextLine + charsOnNextLine;
    return 1;
}

static void MoveLines(TextEditor *t, TextEditorCommand *c){

    if(t->autoCompleteLen > 0){
        t->autoCompleteIndex += c->num;
        if(t->autoCompleteIndex < 0) t->autoCompleteIndex = 0;
        if(t->autoCompleteIndex > MAX_AUTO_COMPLETE-1) t->autoCompleteIndex = MAX_AUTO_COMPLETE-1;
        return;
    }

    if(t->logging == LOGMODE_FILEBROWSER || t->logging == LOGMODE_SWITCH_FILE){
        if(t->logging == LOGMODE_FILEBROWSER && (t->loggingText == NULL || strlen(t->loggingText) == 0)){
            t->logIndex += c->num;
            if(t->logIndex < 0) t->logIndex = 0;
            if(t->logIndex > t->fileBrowser.nFiles-1) t->logIndex = t->fileBrowser.nFiles-1;
        }
        
        if(t->loggingText && strlen(t->loggingText) > 0){
            t->logIndex += c->num;
            if(t->logIndex < 0) t->logIndex = 0;
    
            int nMatching = 0;
            int k;
            int nFiles = t->logging == LOGMODE_FILEBROWSER ? t->fileBrowser.nFiles : t->nFiles;
            for(k = 0; k < nFiles; k++){
                char *name = t->logging == LOGMODE_FILEBROWSER ? t->fileBrowser.files[k].name : t->files[k]->name;
                int nameLen = strlen(name);
                int logNameLen = strlen(t->loggingText);
                if(nameLen > 0 && logNameLen <= nameLen){
                    if(CaseLowerStrnCmp(t->loggingText, name, logNameLen))
                        nMatching++;

                }
            }
            if(t->logIndex >= nMatching) t->logIndex = nMatching-1;

        }

        return;
    }

    int k;

    if(t->file->text == NULL || !t->file->textLen){
        // FreeCursors(t);
        return;
    }

    for(k = 0; k < t->nCursors; k++){

        TextEditorCursor *cursor = &t->cursors[k];

        
        if(c->num > 0) {

            MoveCursorDownLine(t, cursor);

        } else if(c->num < 0 && GetNumLinesToPos(t->file->text, cursor->pos) > 0){

            MoveCursorUpLine(t, cursor);
        }
    }

    RemoveSelections(t);
    ResolveCursorCollisions(t,0);
}

static void ExpandSelectionWords(TextEditor *t,TextEditorCommand *c){

    int k;
    for(k = 0; k < t->nCursors; k++){

        int prev = t->cursors[k].pos;
        t->cursors[k].pos = MoveByWordsFunc(t->file->text, t->file->textLen, t->cursors[k].pos,c->num);
        
        if(t->cursors[k].selection.len == 0)
            t->cursors[k].selection.startCursorPos = prev;

        if(t->cursors[k].pos > t->cursors[k].selection.startCursorPos+t->cursors[k].selection.len){

            t->cursors[k].selection.len += t->cursors[k].pos - prev;

        } else if(t->cursors[k].pos < t->cursors[k].selection.startCursorPos) {

            t->cursors[k].selection.len += t->cursors[k].selection.startCursorPos - t->cursors[k].pos;
            t->cursors[k].selection.startCursorPos = t->cursors[k].pos;
        }
    }


    ResolveCursorCollisions(t,0);
}

static TextEditorCursor *AddCursor(TextEditor *t){

    t->cursors = (TextEditorCursor *)realloc(t->cursors, ++t->nCursors * sizeof(TextEditorCursor));

    TextEditorCursor *cursor = &t->cursors[t->nCursors-1];

    memset(cursor, 0, sizeof(TextEditorCursor));

    // char *last = t->cursors[t->nCursors-2].clipboard;

    // if(last){

    //     int len = strlen(last);

    //     cursor->clipboard = (char *)malloc(len + 1);
    //     cursor->clipboard[len] = 0;

    //     memcpy(cursor->clipboard, last, len);
    // }

    return cursor;
}

static void AddCursorCommand(TextEditor *t, TextEditorCommand *c){

    if(t->file->text == NULL) return;

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

    if(c->num < 0){

        cursor->pos = minCursor;

        if(!MoveCursorUpLine(t, cursor)){
            t->cursors = (TextEditorCursor *)realloc(t->cursors, --t->nCursors * sizeof(TextEditorCursor));
        }

    } else if(c->num > 0) {

        cursor->pos = maxCursor;

        if(!MoveCursorDownLine(t, cursor)){

            t->file->textLen = strlen(t->file->text);
            if(t->file->textLen == 0){
                t->file->text = realloc(t->file->text, t->file->textLen+3);
                t->file->text[t->file->textLen] = '\n';
                t->file->text[t->file->textLen+1] = '\n';
                t->file->text[t->file->textLen+2] = '\0';
            } else{
                t->file->text = realloc(t->file->text, t->file->textLen+2);
                t->file->text[t->file->textLen] = '\n';
                t->file->text[t->file->textLen+1] = '\0';
            }
            MoveCursorDownLine(t, cursor);

            // t->cursors = (TextEditorCursor *)realloc(t->cursors, --t->nCursors * sizeof(TextEditorCursor));

        }
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

static int FindInsensitive(char *text, char *str, int len){

    int k;
    for(k = 0; k < len; k++)
        str[k] = CaseLower(str[k]);

    int textLen = strlen(text);

    int match = 0;
    for(k = 0; k < textLen; k++){

        char c = CaseLower(text[k]);

        if(str[match] == c){
            match++;
            if(match == len)
                break;
        } else {
            match = 0;
        }
    }

    return match == len ? (k-len)+1 : -1;
}

static void SelectNextWord(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    if(t->file->text == NULL) return;

    if(t->nCursors <= 0) return;

    int f;

    if(t->cursors[0].selection.len == 0){
    
        for(f = 0; f < t->nCursors; f++){

            TextEditorCursor *cursor = &t->cursors[f];

            int start = 0;
            GetWordStartEnd(t->file->text, cursor->pos, &start, &cursor->pos);

            t->selectNextWordTerminator = 1;

            cursor->selection.startCursorPos = start;
            cursor->selection.len = cursor->pos - start;
        }

    } else {
    
        TextEditorCursor *prev = &t->cursors[t->nCursors-1];

        int startPos, end;

        startPos = prev->selection.startCursorPos;
        end = startPos+prev->selection.len;

        int len = end-startPos;

        int next = 0;

        while(next >= 0){

            next = Find(&t->file->text[end], &t->file->text[startPos], len);

            if(next >= 0){
                if(t->selectNextWordTerminator && !IsToken(t->file->text[end+next+len])){
                    end += next+len;
                    continue;
                }
            }
            break;
        }

        if(next >= 0){
            next += end;
            TextEditorCursor *cursor = AddCursor(t);
            cursor->pos = next+len;
            cursor->selection.startCursorPos = next;
            cursor->selection.len = len;
        } else {
            next = Find(&t->file->text[0], &t->file->text[startPos], len);
            if(next >= 0 && next != startPos){
                TextEditorCursor *cursor = AddCursor(t);
                cursor->pos = next+len;
                cursor->selection.startCursorPos = next;
                cursor->selection.len = len;
            }
        }
    }

    ResolveCursorCollisions(t,0);
}

// static void ExpandSelectionBrackets(TextEditor *t,TextEditorCommand *c){

// }

static void Paste(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    // RefreshEditorCommand(c);

    char *clipboard = SDL_GetClipboardText();
    int clipboardLen = strlen(clipboard);

    int line = 0;

    int f;
    for(f = 0; f < clipboardLen; f++){
        if(clipboard[f] == '\n') line++;
    }

    if(line != t->nCursors){
        // \n is fine, but not for the final line
        clipboard[strlen(clipboard)-1] = 0;
        clipboardLen--;
    }

    f = 0;

    int k;

    for(k = 0; k < t->nCursors; k++){
        
        EraseAllSelectedText(t, &k, c);

        // if(t->cursors[k].clipboard == NULL) continue;
        // c->addedLens[k] = strlen(t->cursors[k].clipboard);

        if(t->nCursors == line){
            int start = f;
            char tmp = 0;

            for(; f < clipboardLen; f++){
                if(clipboard[f] == '\n'){
                    tmp = clipboard[f];
                    f++;
                    break;
                }
            }
            if(tmp) clipboard[f-1] = 0;
            AddStrToText(t, &k, &clipboard[start]);
            t->cursors[k].addedLen = (f-1) - start;
            clipboard[f-1] = tmp;
        } else {
            AddStrToText(t, &k, clipboard);
            t->cursors[k].addedLen = clipboardLen;
        }


    }

    SaveCursors(t, c);

    for(k = 0; k < t->nCursors; k++)
        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));

}

static void UndoPaste(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    int k;
    for(k = c->nSavedCursors-1; k >= 0; k--){
        RemoveStrFromText(t, &k, c->savedCursors[k].addedLen);

        if(k < c->nSavedCursors && c->savedCursors[k].savedText){
            AddStrToText(t, &k, c->savedCursors[k].savedText);
        }
    }

    SaveCursors(t, c);

}

static int GetBetweenBrackets(char *text, int len, int pos, int *first, int *last){

    *first = -1;
    int endBracket = -1;
    int j;

    struct {
        int scope;
        char start;
        char end;
        int found;
    } brackets[3] = {
        {0,'{','}',0},
        {0,'[',']',0},
        {0,'(',')',0},
    };

    for(j = pos; j >= 0; j--){

        if(j > 1 && text[j] == '/' && text[j-1] == '*'){ // comment /*  -> */
            j -= 2;
            for(; j > 0 && text[j-1] != '*' && text[j] != '/'; j--);
            if(j < 0) break;
        }

        if(text[j] == '"'){ //string ""
            j--;
            for(; j >= 0; j--){
                if(j > 0 && text[j-1] == '\\') { j--; continue; } // escaped
                if(text[j] == '"') break;
            }
            if(j < 0) break;
            continue;
        }
        if(text[j] == '\''){ // char string ''
            j--;
            for(; j >= 0; j--){
                if(j > 0 && text[j-1] == '\\') { j--; continue; } // escaped
                if(text[j] == '\'') { break; }
            }
            if(j < 0) break;
            continue;
        }

        int m;
        for(m = 0; m < 3; m++){
            if(text[j] == brackets[m].end && j < pos) brackets[m].scope++;

            if(text[j] == brackets[m].start){
                if(brackets[m].scope)
                    brackets[m].scope--;
                else 
                    endBracket = m;
            }
        }


        if(endBracket != -1){ // comment , check up to first '\n' of line its on
            int f;
            for(f = j; f >= 1; f--){
                if(text[f] == '\n') break;
                if(text[f] == '/' && text[f-1] == '/'){
                    endBracket = -1;
                    break;
                }
            }
            if(endBracket != -1) break;
        }    
    }

    if(endBracket == -1) return 0;

    *first = j;

    for(j = pos; j < len; j++){

        if(j < len-1 && text[j] == '/' && text[j+1] == '*'){ // comment /*  -> */
            j += 2;
            for(; j < len-1 && text[j+1] != '*' && text[j] != '/'; j++);
            if(j >= len) break;
        }

        if(j < len-1 && text[j] == '/' && text[j+1] == '/'){ // comment //
            for(; j < len; j++)
                if(text[j] == '\n') break;
            if(j >= len) break;
        }
        if(text[j] == '"'){ //string ""
            j++;
            for(; j < len; j++){
                if(text[j] == '\\') { j++; continue; } // escaped
                if(text[j] == '"') break;
            }
            if(j >= len) break;
            continue;
        }
        if(text[j] == '\''){ // char string ''
            j++;
            for(; j < len; j++){
                if(text[j] == '\\') { j++; continue; } // escaped
                if(text[j] == '\'') break;
            }
            if(j >= len) break;
            continue;
        }

        if(text[j] == brackets[endBracket].start && j > pos){
            brackets[endBracket].scope++;
        }

        if(text[j] == brackets[endBracket].end){
            if(brackets[endBracket].scope){

                brackets[endBracket].scope--;
            } else {
                *last = j;
                return 1;

            }

        }
    }
    *last = -1;
    return 0;

}

static void ToggleComment(TextEditor *t, TextEditorCommand *c){

    if(!t->file->text) return;

    t->file->textLen = strlen(t->file->text);

    LoadCursors(t,c);

    int k;
    for(k = 0; k < t->nCursors; k++){


        if(t->cursors[k].selection.len == 0){

            int lineEnd = GetStartOfNextLine(t->file->text, t->file->textLen, t->cursors[k].pos)-1;
            int lineStart = t->cursors[k].pos - GetCharsIntoLine(t->file->text, t->cursors[k].pos);

            int j;
            int toggled = 0;
            for(j = lineEnd-1; j >= lineStart+1; j--){

                if(t->file->text[j] == '/' && t->file->text[j-1] == '/') {
                    t->cursors[k].pos = j+1;
                    RemoveStrFromText(t, &k, 2);
                    toggled = 1;
                    break;
                }
            }

            if(toggled) continue;
            t->cursors[k].pos = lineStart;
            AddStrToText(t, &k, "//");
            
            continue;
        }


        int startSelection = t->cursors[k].selection.startCursorPos - 
        GetCharsIntoLine(t->file->text, t->cursors[k].selection.startCursorPos);
        int endSelection = t->cursors[k].selection.startCursorPos + t->cursors[k].selection.len;

        int toggled = 0;

        int m;
        for(m = startSelection; m < endSelection; m++){
            if(t->file->text[m] == '/' && t->file->text[m-1] == '/') {
                toggled = 1;    
                break;
            }
            if(t->file->text[m] == '\n')
                break;
        }

        if(toggled == 1){
            for(m = startSelection; m < endSelection; m++){
                if(t->file->text[m] == '/' && t->file->text[m-1] == '/') {
                    t->cursors[k].pos = m+1;
                    RemoveStrFromText(t, &k, 2);
                    t->cursors[k].selection.len -= 2;
                    m--;
                }
            }

            continue;
        }


        // if mixture of commenting/not in selection double comment, every line gets it.
        t->cursors[k].pos = startSelection;
        t->cursors[k].selection.len += 2;

        for(m = startSelection; m < endSelection; m++){
            if(t->file->text[m] == '\n'){
                t->cursors[k].pos = m+1;
                t->cursors[k].selection.len += 2;
                AddStrToText(t, &k, "//");
            }
        }
    }

    SaveCursors(t,c);
}

static void MoveBrackets(TextEditor *t, TextEditorCommand *c){

    if(!t->file->text) return;

    t->file->textLen = strlen(t->file->text);

    int k;
    for(k = 0; k < t->nCursors; k++){
        int first, last;
        if(GetBetweenBrackets(t->file->text, t->file->textLen, t->cursors[k].pos, &first, &last)){
            if(last == t->cursors[k].pos){
                t->cursors[k].pos = first+1; 
            } else {
                t->cursors[k].pos = last;
            }
        }
    }


    ResolveCursorCollisions(t,0);
}

static void SelectBrackets(TextEditor *t, TextEditorCommand *c){

    if(!t->file->text) return;

    t->file->textLen = strlen(t->file->text);

    int k;
    for(k = 0; k < t->nCursors; k++){

        TextEditorCursor *cursor = &t->cursors[k];
        int first, last;
        if(GetBetweenBrackets(t->file->text, t->file->textLen, cursor->pos, &first, &last)){
            if(last == cursor->pos){
                cursor->pos = first; 
            } else {
                cursor->pos = last;
            }
            cursor->selection.startCursorPos = first + 1;
            cursor->selection.len = (last - first) - 1; // exclude brackets from selection
        }
    }

    ResolveCursorCollisions(t,0);
}

static void ExpandSelectionLines(TextEditor *t, TextEditorCommand *c){

    if(t->file->text == NULL) return;

    RemoveExtraCursors(t);
    t->file->textLen = strlen(t->file->text);

    int f;
    for(f = 0; f < t->nCursors; f++){

        TextEditorCursor *cursor = &t->cursors[f];

        if(cursor->selection.len == 0){
            cursor->selection.startCursorPos = cursor->pos - GetCharsIntoLine(t->file->text, cursor->pos);
        }

        if(c->num < 0)
            cursor->pos = GetStartOfPrevLine(t->file->text, cursor->pos);
        else
            cursor->pos = GetStartOfNextLine(t->file->text, t->file->textLen, cursor->pos);         

        if(cursor->pos < 0) cursor->pos = 0;
        if(cursor->pos > (int)t->file->textLen) cursor->pos = t->file->textLen;

        cursor->selection.len = cursor->pos - cursor->selection.startCursorPos;
        if(cursor->selection.startCursorPos+cursor->selection.len < 0)
            cursor->selection.len -= c->num;

        if(cursor->selection.startCursorPos+cursor->selection.len > (int)t->file->textLen)
            cursor->selection.len -= c->num;
    }
}

static void SelectAll(TextEditor *t, TextEditorCommand *c){

    if(t->file->text == NULL) return;

    RemoveExtraCursors(t);
    t->file->textLen = strlen(t->file->text);
    t->cursors[0].selection.len = t->file->textLen;
    t->cursors[0].selection.startCursorPos = 0;
    t->cursors[0].pos = 0;
}

static void UndoDeleteLine(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    if(t->file->text == NULL) return;

    // RemoveSelections(t);
    LoadCursors(t, c);
    // RefreshEditorCommand(c);



    int k;
    for(k = 0; k < t->nCursors; k++){

        // TextEditorCursor *cursor = &t->cursors[k];
        AddStrToText(t, &k, c->savedCursors[k].savedText);
    }
    // SaveCursors(t, c);
}

static void DeleteLine(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    if(t->file->text == NULL) return;

    LoadCursors(t,c);
    // RefreshEditorCommand(c);

/*
    problem, resolvecollisions is making there be less cursors, not as many as savedtexts,
    so need to know to insert twice for combined cursors that were in the same location.
*/

    int k;
    for(k = 0; k < t->nCursors; k++){

        TextEditorCursor *cursor = &t->cursors[k];

        int start;
        int end;

        if(cursor->selection.len){
            start = cursor->selection.startCursorPos - GetCharsIntoLine(t->file->text, cursor->selection.startCursorPos);
            end = GetStartOfNextLine(t->file->text,t->file->textLen, cursor->selection.startCursorPos+cursor->selection.len - 1);
        } else {
            start = cursor->pos - GetCharsIntoLine(t->file->text, cursor->pos);
            end = GetStartOfNextLine(t->file->text,t->file->textLen, cursor->pos);
        }


        cursor->pos = end;
        AddSavedText(t, &t->file->text[start], end-start, &k);
        RemoveStrFromText(t, &k, end-start);
    }

    SaveCursors(t,c);
    RemoveSelections(t);
    RemoveExtraCursors(t);
}

static void Copy(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    if(t->file->text == NULL) return;

    char *buffer = NULL;
    int bufferLen = 0;

    int k;
    for(k = 0; k < t->nCursors; k++){

        int start, end;

        start = t->cursors[k].selection.startCursorPos;
        end = start+t->cursors[k].selection.len;


        if(buffer){
            bufferLen += (end-start)+1;
            buffer = realloc(buffer, bufferLen+1);
            buffer[bufferLen] = 0;
            memcpy(&buffer[bufferLen-((end-start)+1)], &t->file->text[start], end-start);
            buffer[bufferLen - 1] = '\n';
        } else {
            bufferLen = (end-start) + 1;
            buffer = malloc(bufferLen+1);
            buffer[bufferLen] = 0;
            buffer[bufferLen - 1] = '\n';
            memcpy(buffer, &t->file->text[start], end-start);
        }

        // if(t->cursors[k].clipboard) free(t->cursors[k].clipboard);

        // t->cursors[k].clipboard = (char *)malloc((end-start) + 1);
        // t->cursors[k].clipboard[end-start] = 0;

        // memcpy(t->cursors[k].clipboard, &t->file->text[start], end-start);
    }
    if(buffer)
        SDL_SetClipboardText(buffer);
}

static void Cut(TextEditor *t, TextEditorCommand *c){

    UNUSED(c);

    TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, 1, SCR_CENT, Copy, NULL);
    ExecuteCommand(t, command);
    FreeCommand(command);

    command = CreateCommand((const unsigned int[]){0}, 0, 0, SCR_CENT, RemoveCharacters, UndoRemoveCharacters);
    ExecuteCommand(t, command);
    FreeCommand(command);
}

static void AddSavedText(TextEditor *t, char *str, int len, int *cursorIndex){
    if(len <= 0) return;
    if(t->cursors[*cursorIndex].savedText){
        free(t->cursors[*cursorIndex].savedText);
    }
        // int len2 = strlen(t->cursors[*cursorIndex].savedText);
        // t->cursors[*cursorIndex].savedText = (char *)realloc(t->cursors[*cursorIndex].savedText, len2 + len + 1);
        // t->cursors[*cursorIndex].savedText[len+len2] = 0;
        // memcpy(&t->cursors[*cursorIndex].savedText[len2], str, len);
    // } else {
        t->cursors[*cursorIndex].savedText = (char *)malloc(len + 1);
        t->cursors[*cursorIndex].savedText[len] = 0;
        memcpy(t->cursors[*cursorIndex].savedText, str, len);
    // }
}

static void EraseAllSelectedText(TextEditor *t, int *cursorIndex, TextEditorCommand *command){
        

    if(t->file->text == NULL) return;

    TextEditorCursor *cursor = &t->cursors[*cursorIndex];

    if(t->file->text == NULL || cursor->selection.len == 0) return;

    int textLen = t->file->textLen;

    int startCursorPos, endCursorPos;

    startCursorPos = cursor->selection.startCursorPos;
    endCursorPos = startCursorPos + cursor->selection.len;

    int newSize = textLen - (endCursorPos - startCursorPos);

    if(newSize <= 0){
        if(t->file->text) free(t->file->text);
        t->file->text = malloc(1);
        t->file->text[0] = 0;
        return;
    }

    AddSavedText(t, &t->file->text[startCursorPos], cursor->selection.len, cursorIndex);
    cursor->pos = endCursorPos;
    RemoveStrFromText(t, cursorIndex, cursor->selection.len);
}

static void SaveCursors(TextEditor *t, TextEditorCommand *c){

    if(c->savedCursors){

        free(c->savedCursors);
    }

    c->savedCursors = (TextEditorCursor *)malloc(sizeof(TextEditorCursor) * t->nCursors);
    c->nSavedCursors = t->nCursors;

    int k;
    for(k = 0; k < t->nCursors; k++){
        if(t->cursors[k].pos < 0) t->cursors[k].pos = 0;

        memcpy(&c->savedCursors[k], &t->cursors[k], sizeof(TextEditorCursor));

        if(t->cursors[k].savedText){
            int len = strlen(t->cursors[k].savedText);
            c->savedCursors[k].savedText = malloc(len+1);
            c->savedCursors[k].savedText[len] = 0;
            memcpy(c->savedCursors[k].savedText, t->cursors[k].savedText, len);
            free(t->cursors[k].savedText);
            t->cursors[k].savedText = NULL;
        }
        t->cursors[k].addedLen = 0;
    }

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

static void AddStrToText(TextEditor *t, int *cursorIndex, char *text){


    int textLen = strlen(t->file->text);

    int len = strlen(text);

    int pos = t->cursors[*cursorIndex].pos;

    char *text1 = (char *)malloc(textLen);

    memcpy(text1,t->file->text,textLen);
    free(t->file->text);

    t->file->text = (char *)malloc(textLen+len+1);
    memcpy(t->file->text, text1, pos);
    t->file->text[textLen + len] = 0;

    // if(textLen - pos > 0)
    memcpy(&t->file->text[pos + len], &text1[pos], (textLen - pos));
    memcpy(&t->file->text[pos], text, len);
    free(text1);

    t->file->textLen = strlen(t->file->text);
    t->cursors[*cursorIndex].pos += len;
    MoveCursorsAndSelection(t, pos, len, cursorIndex);
}

static void RemoveStrFromText(TextEditor *t, int *cursorIndex, int len){
    if(t->file->text == NULL) return;

    int pos = t->cursors[*cursorIndex].pos;
    if(pos < 0) return;
    if(pos - len < 0) len = pos;

    pos -= len;


    int textLen = strlen(t->file->text);

    if(pos < textLen){
        memcpy(&t->file->text[pos], &t->file->text[pos+len], textLen - (pos + len));
    }

    t->file->text = (char *)realloc(t->file->text, (textLen - len) + 1);

    t->file->text[textLen - len] = 0;
    t->file->textLen = strlen(t->file->text);
    t->cursors[*cursorIndex].pos = pos;
    MoveCursorsAndSelection(t, pos, -len, cursorIndex);
}

static void UndoRemoveCharacters(TextEditor *t, TextEditorCommand *c){

    if(t->logging) return;

    LoadCursors(t, c);

    int k;
    for(k = t->nCursors-1; k >= 0; k--){

        if(k < c->nSavedCursors && t->cursors[k].savedText){
            AddStrToText(t, &k, t->cursors[k].savedText);
        }
    }

    SaveCursors(t,c);
}

static void AutoComplete(TextEditor *t){

    t->autoCompleteLen = 0;
    if(t->autoCompleteSearchLen > 0){

        if(t->autoCompleteSearchLen > MAX_AUTO_COMPLETE_STRLEN) return;

        TextEditorCursor *c = &t->cursors[t->nCursors-1];

        if(t->autoCompleteSearchLen > 3 && t->autoCompleteSearchLen < MAX_AUTO_COMPLETE_STRLEN){

            int search = c->pos - t->autoCompleteSearchLen;
            int findEnd = 0;
            int res;

            while(findEnd+t->autoCompleteSearchLen < t->file->textLen &&
                (res = Find(&t->file->text[findEnd], &t->file->text[search], t->autoCompleteSearchLen)) != -1){

                if(findEnd+res == search){
                    findEnd = c->pos;
                    continue;
                }

                int j;
                for(j = 0; findEnd + res + j < t->file->textLen && 
                    j < MAX_AUTO_COMPLETE_STRLEN && !IsToken(t->file->text[findEnd+res+j]); j++);


                int m;
                for(m = 0; m < t->autoCompleteLen; m++){
                    if(t->autoComplete[m].len == j && 
                        memcmp(&t->file->text[t->autoComplete[m].offset], &t->file->text[findEnd+res],j) == 0){
        
                        break;
                    }
                }

                if(m != t->autoCompleteLen) {
                    findEnd += res + j;
                    continue;
                }

                t->autoComplete[t->autoCompleteLen].offset = findEnd+res;
                t->autoComplete[t->autoCompleteLen].len = j;
                ++t->autoCompleteLen;
                findEnd += res + j;
                if(t->autoCompleteLen >= MAX_AUTO_COMPLETE) break;
            }
        }

    }
}

static void ScrollScreen(TextEditor *t, TextEditorCommand *c){
    RemoveSelections(t);
    RemoveExtraCursors(t);

    int scroll = Graphics_TextCollumns();
    int k = 0;

    if(c->num < 0){
        for(k = t->cursors[0].pos; k > 0; k--){
            if(t->file->text[k] == '\n'){
                scroll--;
                if(scroll == 0) { 
                    k++;
                    break; 
                }
            }
        }
    }else{
        for(k = t->cursors[0].pos; k < strlen(t->file->text); k++){
            if(t->file->text[k] == '\n'){
                scroll--;
                if(scroll == 1) {
                    k++;
                    break;
                }
            }
        }
    }

    t->cursors[0].pos = k;
}

static void RemoveCharacters(TextEditor *t, TextEditorCommand *c){

    if(t->logging){

        if(t->logging < LOGMODE_MODES_INPUTLESS){
            
            t->logIndex = -1;

            if(t->loggingText){
                int searchLen = strlen(t->loggingText);
                if(searchLen - c->num <= 0){
                    free(t->loggingText);
                    t->loggingText = NULL;
                    // t->logging = 0;
                    return;
                } else {
                    char *tmp = malloc(searchLen - c->num + 1);
                    memcpy(tmp, t->loggingText, searchLen - c->num);
                    tmp[searchLen - c->num] = 0;
                    free(t->loggingText);
                    t->loggingText = tmp;
                }
            }
        }

        return;
    }

    if(t->file->text == NULL) return;

    LoadCursors(t, c);

    // RefreshEditorCommand(c);



    int k;
    for(k = 0; k < t->nCursors; k++){

        if(t->cursors[k].selection.len == 0){
            AddSavedText(t, &t->file->text[t->cursors[k].pos-c->num], c->num, &k);
            RemoveStrFromText(t, &k, c->num);
        } else {

            EraseAllSelectedText(t, &k, c);
        }
    }

    SaveCursors(t, c);

    if(t->autoCompleteSearchLen > 0){
        t->autoCompleteSearchLen -= c->num;
        AutoComplete(t);
    }

    for(k = 0; k < t->nCursors; k++)
        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));

}

static void UndoAddCharacters(TextEditor *t, TextEditorCommand *c){
    
    if(t->logging) return;
    if(t->file->text == NULL) return;

    LoadCursors(t, c);

    int k;
    for(k = c->nSavedCursors-1; k >= 0; k--){
        RemoveStrFromText(t, &k, c->savedCursors[k].addedLen);
        if(k < c->nSavedCursors && c->savedCursors[k].savedText){
            AddStrToText(t, &k, c->savedCursors[k].savedText);
        }
    }

    SaveCursors(t, c);

    if(t->autoCompleteSearchLen > 0){
        t->autoCompleteSearchLen -= t->cursors[0].addedLen;
        if(t->autoCompleteSearchLen > 0)
            AutoComplete(t);
        else
            t->autoCompleteSearchLen = 0;
    }


}

// static void RefreshEditorCommand(TextEditorCommand *c){

    // if(c->addedLens) free(c->addedLens);

    // if(c->savedTexts){

    //     int k;
    //     for(k = 0; k < c->nSavedCursors; k++)
    //         if(c->savedTexts[k]) free(c->savedTexts[k]);
    
    //     free(c->savedTexts);
    // }

    // c->addedLens = (int *)malloc(c->nSavedCursors * sizeof(int));
    // memset(c->addedLens, 0, sizeof(int) * c->nSavedCursors);

    // c->savedTexts = (char **)malloc(c->nSavedCursors * sizeof(char*));
    // memset(c->savedTexts, 0, sizeof(char*) * c->nSavedCursors);
// }

static void IndentLine(TextEditor *t, TextEditorCommand *c){

    // LoadCursors(t,c);

    int k;
    for(k = 0; k < t->nCursors; k++){

        int prev = t->cursors[k].pos;
        if(t->cursors[k].selection.len == 0){

            if(t->file->text[prev] != '\n')
                t->cursors[k].pos = prev - GetCharsIntoLine(t->file->text, t->cursors[k].pos);

            if(c->num > 0){
                TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, "\t", 0, SCR_CENT, AddCharacters, UndoAddCharacters);
                ExecuteCommand(t,command);
                FreeCommand(command);
            } else {
                if (t->file->text[t->cursors[k].pos] == '\t' || t->file->text[t->cursors[k].pos] == ' '){
                    t->cursors[k].pos++;
                    TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, 1, SCR_CENT,
                        RemoveCharacters, UndoRemoveCharacters);
                    ExecuteCommand(t,command);
                    FreeCommand(command);
                } 
            }

        } else {

            TextEditorSelection selection = t->cursors[k].selection;

            int startCursorPos=t->cursors[k].selection.startCursorPos;
            RemoveSelections(t);

            int next = startCursorPos;

            if(t->file->text[next] != '\n')
                next = startCursorPos - GetCharsIntoLine(t->file->text, next);

            do {
                t->cursors[k].pos = next;

                if(c->num > 0){
                    if(next == selection.startCursorPos){
                        selection.startCursorPos++;
                    } else {
                        selection.len++;
                    }
                    TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 
                        "\t", 0, SCR_CENT, AddCharacters, UndoAddCharacters);
                    ExecuteCommand(t,command);
                    FreeCommand(command);
                    prev++;
                } else {
                    if (t->file->text[t->cursors[k].pos] == '\t' || t->file->text[t->cursors[k].pos] == ' '){
    
                        if(next == selection.startCursorPos){
                            selection.startCursorPos--;
                        } else {
                            selection.len--;
                        }
    
                        t->cursors[k].pos++;
                        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, 1, SCR_CENT, 
                            RemoveCharacters, UndoRemoveCharacters);
                        ExecuteCommand(t,command);
                        FreeCommand(command);
                        prev--;
                    } 
                }

                next = GetStartOfNextLine(t->file->text, t->file->textLen, next);

    
            } while(next < selection.startCursorPos+selection.len);

            // AddCharacters in Tab will delete everything;
            t->cursors[k].selection = selection;
        }


        t->cursors[k].pos = prev;
    }
    SaveCursors(t,c);
}

static void AddCharacters(TextEditor *t, TextEditorCommand *c){
    
    if(t->logging){
        int nKeys = strlen(c->keys);

        if(t->logging == LOGMODE_NUM){
            int k;
            for(k = 0; k < nKeys; k++) if(!IsDigit(c->keys[k])) return;
        }
        if(t->logging < LOGMODE_MODES_INPUTLESS){

            if(t->loggingText){
                int searchLen = strlen(t->loggingText);
                t->loggingText = realloc(t->loggingText, searchLen+nKeys+1);
                memcpy(&t->loggingText[searchLen], c->keys, nKeys);
                t->loggingText[searchLen+nKeys] = 0;
            }
            else{
                t->logIndex = 0;
                t->loggingText = malloc(nKeys);
                memcpy(t->loggingText, c->keys, nKeys);
                t->loggingText[nKeys] = 0;
            }
        }
        return;
    }

    if(t->file->text == NULL) return;

    LoadCursors(t, c);

    // RefreshEditorCommand(c);

    int k;


    for(k = 0; k < t->nCursors; k++){
        EraseAllSelectedText(t, &k, c);
        AddStrToText(t, &k, c->keys);
        t->cursors[k].addedLen = strlen(c->keys);
    }

    SaveCursors(t, c);

    for(k = 0; k < strlen(c->keys); k++){
        if(!IsToken(c->keys[k])){
            t->autoCompleteSearchLen++;
        } else {
            t->autoCompleteSearchLen = 0;
        }
    }        
    AutoComplete(t);

    for(k = 0; k < t->nCursors; k++)
        memset(&t->cursors[k].selection, 0, sizeof(TextEditorSelection));

}

static void Undo(TextEditor *t, TextEditorCommand *c){

    UndoCommands(t, c->num);
}

static void Redo(TextEditor *t, TextEditorCommand *c){

    RedoCommands(t, c->num);
}

static void FreeCommand(TextEditorCommand *c){

    if(c->keys) free(c->keys);
    if(c->keyBinding) free(c->keyBinding);

    if(c->savedCursors){

        int k;
        for(k = 0; k < c->nSavedCursors; k++){
            if(c->savedCursors[k].savedText) free(c->savedCursors[k].savedText);
        }
        free(c->savedCursors);
    }
    
    // if(c->savedTexts){

    //     int k;
    //     for(k = 0; k < c->nSavedCursors; k++)
    //         free(c->savedTexts[k]);

    //     free(c->savedTexts);
    // }

    // if(c->addedLens) free(c->addedLens);

    free(c);
}

static TextEditorCommand *CopyCommand(TextEditorCommand *c){

    TextEditorCommand *ret = CreateCommand((const unsigned int *)c->keyBinding, c->keys, c->num, c->scroll, c->Execute, c->Undo);

    ret->savedCursors = (TextEditorCursor *)malloc(sizeof(TextEditorCursor) * c->nSavedCursors);
    ret->nSavedCursors = c->nSavedCursors;

    int k;
    for(k = 0; k < c->nSavedCursors; k++){

        memcpy(&ret->savedCursors[k], &c->savedCursors[k], sizeof(TextEditorCursor));

        if(c->savedCursors[k].savedText){
            int len = strlen(c->savedCursors[k].savedText);
            ret->savedCursors[k].savedText = malloc(len+1);
            ret->savedCursors[k].savedText[len] = 0;
            memcpy(ret->savedCursors[k].savedText, c->savedCursors[k].savedText, len);
        }
    }

    return ret;
}

static TextEditorCommand *CreateCommand(const unsigned int binding[], const char *keys, int n, int scroll,
    void (*E)(TextEditor *, TextEditorCommand *), void (*U)(TextEditor *, TextEditorCommand *)){

    TextEditorCommand *res = (TextEditorCommand *)malloc(sizeof(TextEditorCommand));
    memset(res, 0, sizeof(TextEditorCommand));

    res->scroll = scroll;
    res->num = n;

    if(binding){

        int len = 0;
        while(binding[len] != 0) len++;

        res->keyBinding = (unsigned int *)malloc(len+1);
        res->keyBinding[len+1] = 0;
        memcpy(res->keyBinding, binding, len * sizeof(unsigned int));
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

    if(t->file->historyPos == 0 || t->file->historyPos - num < 0)
        return;

    int k;
    for(k = 0; k < num; k++){
        if(t->file->history[(t->file->historyPos-1)-k]->Undo){
            t->file->history[(t->file->historyPos-1)-k]->Undo(t, t->file->history[(t->file->historyPos-1)-k]);
            if(t->file->history[(t->file->historyPos-1)-k]->scroll == SCR_CENT)
                UpdateScrollCenter(t);
            else
                UpdateScroll(t);
        }
    }

    t->file->historyPos -= num;
}

static void RedoCommands(TextEditor *t, int num){

    if(t->file->historyPos + num > t->file->sHistory)
        num = t->file->sHistory - t->file->historyPos;

    if(num <= 0) return;


    int k;
    for(k = 0; k < num; k++){
        t->file->history[(t->file->historyPos)+k]->Execute(t, t->file->history[(t->file->historyPos)+k]);
        if(t->file->history[(t->file->historyPos)+k]->scroll == SCR_CENT)
            UpdateScrollCenter(t);
        else
            UpdateScroll(t);
    }

    t->file->historyPos += num;

}

static void RemoveExtraCursors(TextEditor *t){
    int k;
    for(k = 1; k < t->nCursors; k++){
        if(t->cursors[k].savedText) free(t->cursors[k].savedText);
        t->cursors[k].savedText = NULL;
    }
    t->cursors = (TextEditorCursor *)realloc(t->cursors, sizeof(TextEditorCursor));
    t->nCursors = 1;
    UpdateScrollCenter(t);
}

static void ExecuteCommand(TextEditor *t, TextEditorCommand *c){

    if(c->Undo == NULL || t->logging){
        c->Execute(t, c);
        if(c->scroll == SCR_CENT)
            UpdateScrollCenter(t);
        else
            UpdateScroll(t);
        return;
    }


    if(t->file->historyPos <= t->file->sHistory){

        int k;
        for(k = t->file->historyPos; k < t->file->sHistory; k++){
            FreeCommand(t->file->history[k]);
        }

        t->file->sHistory = t->file->historyPos;
    }

    t->file->history = (TextEditorCommand **)realloc(t->file->history, ++t->file->sHistory * sizeof(TextEditorCommand *));
    t->file->history[t->file->sHistory-1] = CopyCommand(c);
    t->file->history[t->file->sHistory-1]->Execute(t,t->file->history[t->file->sHistory-1]);
    if(c->scroll == SCR_CENT)
        UpdateScrollCenter(t);
    else
        UpdateScroll(t);
    t->file->historyPos++;
}

static void AddCommand(TextEditor *t, TextEditorCommand *c){
    t->commands = (TextEditorCommand **)realloc(t->commands, ++t->nCommands * sizeof(TextEditorCommand *));
    t->commands[t->nCommands-1] = c;
}

static int AddFile(TextEditor *t, TextEditorFile *f){

    if(strlen(f->path) > 0){
        int k;
        for(k = 0; k < t->nFiles; k++){
            if(strlen(t->files[k]->path) > 0 && strcmp(t->files[k]->path, f->path) == 0)
                return k;

        }
    }

    t->files = (TextEditorFile **)realloc(t->files, ++t->nFiles * sizeof(TextEditorFile *));
    t->files[t->nFiles-1] = f;
    return -1;
}

static void FreeFile(TextEditorFile *f){
    if(f->text) free(f->text);
    int k;
    for(k = 0; k < f->sHistory; k++)
        FreeCommand(f->history[k]);

    if(f->history) free(f->history);
    free(f);
}

static TextEditorFile *CreateTextEditorFile(char *path){
    TextEditorFile *ret = (TextEditorFile *)malloc(sizeof(TextEditorFile));
    memset(ret, 0, sizeof(TextEditorFile));
    if(path != NULL){
        strcpy(ret->path, path);
        int k = strlen(path);
        for(; k >= 0; k--){
#ifdef LINUX_COMPILE
            if(path[k] == '/'){
#endif
#ifdef WINDOWS_COMPILE
            if(path[k] == '\\'){
#endif
                k++;
                break;
            }
        }
        if(k < 0) k++;
        strcpy(ret->name, &path[k]);
    }
    return ret;
}

void TextEditor_LoadFile(TextEditor *t, char *pathRel){

    if(t->file && t->cursors)
        t->file->cursorPos = t->cursors[0].pos;

    
    if(pathRel == NULL){
        t->file = CreateTextEditorFile(NULL);
        t->file->text = malloc(1);
        t->file->text[0] = 0;
        strcpy(t->file->name, "new file");
        return;
    }

    char path[MAX_PATH_LEN] = {0};
#ifdef WINDOWS_COMPILE
    GetFullPathNameA(pathRel, MAX_PATH_LEN, path, NULL);
#endif
#ifdef LINUX_COMPILE
    char *tmp = realpath(pathRel, NULL);
    if(tmp){
        strcpy(path, tmp);
        free(tmp);
    }
#endif

    TextEditorFile *file = CreateTextEditorFile(path);

    int exists = -1;
    if((exists = AddFile(t, file)) >= 0){
        FreeFile(file);
        t->file = t->files[exists];
        // check to reload?
        return;
    } else {
        t->file = file;
    }


    FILE *fp = fopen(path, "r");

    if(!fp){
        t->file->text = malloc(1);
        t->file->text[0] = 0;
        return;
    }

    fseek(fp, 0, SEEK_END);

    int len = ftell(fp);

    rewind(fp);

    // 1 mb
    if(len > (1 << 20) * 1) len = (1 << 20) * 1; // not enough checking. but it'll save it from opening images on accident and crashing.

    t->file->text = (char *)malloc(len + 1);
    t->file->text[len] = 0;

    t->file->textLen = len;

    fread(t->file->text, sizeof(char), len, fp);

    fclose(fp);
}


void TextEditor_Init(TextEditor *t){
    memset(t, 0, sizeof(TextEditor));

    Graphics_init_pair(COLOR_SIDE_NUMBERS, COLOR_WHITE, COLOR_BLACK);
    Graphics_init_pair(COLOR_NORMAL, COLOR_WHITE, COLOR_BLACK);
    Graphics_init_pair(COLOR_KEYWORD, COLOR_CYAN, COLOR_BLACK);
    Graphics_init_pair(COLOR_COMMENT, COLOR_GREY, COLOR_BLACK);
    Graphics_init_pair(COLOR_TOKEN, COLOR_BLUE, COLOR_BLACK);
    Graphics_init_pair(COLOR_NUM, COLOR_RED, COLOR_BLACK);
    Graphics_init_pair(COLOR_FUNCTION, COLOR_YELLOW, COLOR_BLACK);
    Graphics_init_pair(COLOR_STRING, COLOR_MAGENTA, COLOR_BLACK);

    Graphics_init_pair(COLOR_SELECTED, COLOR_BLACK ,COLOR_CYAN);
    Graphics_init_pair(COLOR_SELECTED_DIRECTORY, COLOR_RED ,COLOR_CYAN);
    Graphics_init_pair(COLOR_UNSELECTED_DIRECTORY, COLOR_RED ,COLOR_WHITE);
    Graphics_init_pair(COLOR_AUTO_COMPLETE, COLOR_BLACK, COLOR_WHITE);
    Graphics_init_pair(COLOR_LOG_UNSELECTED, COLOR_BLACK, COLOR_WHITE);
    Graphics_init_pair(COLOR_CURSOR, COLOR_BLACK ,COLOR_MAGENTA);
    Graphics_init_pair(COLOR_FIND, COLOR_BLACK ,COLOR_WHITE);
    Graphics_init_pair(COLOR_LINE_NUM, COLOR_GREY ,COLOR_BLACK);

    Graphics_init_pair(TE_COLOR_BLACK, COLOR_BLACK ,COLOR_WHITE);
    Graphics_init_pair(TE_COLOR_WHITE, COLOR_WHITE ,COLOR_BLACK);
    Graphics_init_pair(TE_COLOR_CYAN, COLOR_CYAN ,COLOR_BLACK);
    Graphics_init_pair(TE_COLOR_RED, COLOR_RED ,COLOR_BLACK);
    Graphics_init_pair(TE_COLOR_YELLOW, COLOR_YELLOW ,COLOR_BLACK);
    Graphics_init_pair(TE_COLOR_BLUE, COLOR_BLUE ,COLOR_BLACK);
    Graphics_init_pair(TE_COLOR_GREEN, COLOR_GREEN ,COLOR_BLACK);
    Graphics_init_pair(TE_COLOR_MAGENTA, COLOR_MAGENTA ,COLOR_BLACK);

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|EDIT_ARROW_UP  , 0}, "", -1, SCR_NORM, MoveLinesText, UndoMoveLinesText));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|EDIT_ARROW_DOWN  , 0}, "", 1, SCR_NORM, MoveLinesText, UndoMoveLinesText));

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|'o'  , 0}, "", 0, SCR_NORM, OpenFileBrowser, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'o'  , 0}, "", 0, SCR_NORM, OpenFile, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'n'  , 0}, "", 0, SCR_NORM, NewFile, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'w'  , 0}, "", 0, SCR_NORM, CloseFile, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'p'  , 0}, "", 0, SCR_NORM, SwitchFile, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|'s'  , 0}, "", 0, SCR_NORM, SaveAsFile, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'s'  , 0}, "", 0, SCR_NORM, SaveFile, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'/'  , 0}, "", 0, SCR_NORM, ToggleComment, ToggleComment));
    // AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|'/'  , 0}, "", 0, ToggleComment, ToggleComment));

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'m'  , 0}, "", 0, SCR_NORM, MoveBrackets, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|'j'  , 0}, "", 0, SCR_NORM, SelectBrackets, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'g'  , 0}, "", 0, SCR_CENT, GotoLine, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'f'  , 0}, "", 0, SCR_CENT, FindTextInsensitive, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|'f'  , 0}, "", 0, SCR_CENT, FindText, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ENTER_KEY|EDIT_CTRL_KEY  , 0}, "", 0, SCR_CENT, EventCtrlEnter, NULL));
    // AddCommand(t, CreateCommand((unsigned int[]){'d'|EDIT_CTRL_KEY  , 0}, "", 0, SelectNextWord, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){'d'|EDIT_CTRL_KEY  , 0}, "", 0, SCR_CENT, SelectNextWord, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_UP|EDIT_CTRL_KEY  , 0}, "", -1, SCR_NORM, AddCursorCommand, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_DOWN|EDIT_CTRL_KEY  , 0}, "", 1, SCR_NORM, AddCursorCommand, NULL));

    // AddCommand(t, CreateCommand((unsigned int[]){unbound, 0}, "", -1, ExpandSelectionChars, NULL));
    // AddCommand(t, CreateCommand((unsigned int[]){unbound, 0}, "", 1, ExpandSelectionChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'l'|EDIT_SHIFT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, SCR_NORM, ExpandSelectionLines, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'k'|EDIT_SHIFT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, SCR_NORM, DeleteLine, UndoDeleteLine));

    AddCommand(t, CreateCommand((unsigned int[]){'h'|EDIT_CTRL_KEY  , 0}, "", -1, SCR_NORM, MoveByChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'l'|EDIT_CTRL_KEY  , 0}, "", 1, SCR_NORM, MoveByChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'j'|EDIT_CTRL_KEY  , 0}, "", -1, SCR_NORM, MoveLines, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'k'|EDIT_CTRL_KEY  , 0}, "", 1, SCR_NORM, MoveLines, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){'h'|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", -1, SCR_NORM, MoveByWords, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'l'|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, SCR_NORM, MoveByWords, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){']'|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, SCR_NORM, IndentLine, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'['|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", -1, SCR_NORM, IndentLine, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_LEFT|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", -1, SCR_NORM, ExpandSelectionWords, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_RIGHT|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, SCR_NORM, ExpandSelectionWords, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_UP|EDIT_SHIFT_KEY  , 0}, "", -1, SCR_CENT, ScrollScreen, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_DOWN|EDIT_SHIFT_KEY  , 0}, "", 1, SCR_CENT, ScrollScreen, NULL));
    

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_LEFT  , 0}, "", -1, SCR_NORM, MoveByChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_RIGHT  , 0}, "", 1, SCR_NORM, MoveByChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_UP  , 0}, "", -1, SCR_NORM, MoveLines, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|'a'  , 0}, "", 0, SCR_NORM, SelectAll, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_DOWN  , 0}, "", 1, SCR_NORM, MoveLines, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){'z'|EDIT_CTRL_KEY  , 0}, "", 1, SCR_CENT, Undo, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'y'|EDIT_CTRL_KEY  , 0}, "", 1, SCR_CENT, Redo, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'x'|EDIT_CTRL_KEY  , 0}, "", 1, SCR_CENT, Cut, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'c'|EDIT_CTRL_KEY  , 0}, "", 1, SCR_CENT, Copy, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'v'|EDIT_CTRL_KEY  , 0}, "", 1, SCR_CENT, Paste, UndoPaste));

    InitCursors(t);
    FileBrowser_Init(&t->fileBrowser);
    FILE *fp = fopen(LOGFILE, "r");
    if(fp){
        fseek(fp, 0, SEEK_END);
        int len = ftell(fp);
        rewind(fp);
        if(len > sizeof(int)){
            int nFiles;
            fread(&nFiles, sizeof(int), 1, fp);
            len -= sizeof(int);
            int k;
            for(k = 0; k < nFiles; k++){
                if(len < sizeof(int)) break;
                len -= sizeof(int);
                int pathLen;
                char path[512];
                fread(&pathLen, sizeof(int), 1, fp);
                if(len < pathLen) break;
                if(pathLen > 512) break;
                len -= pathLen;
                fread(path,1,pathLen,fp);
                path[pathLen] = 0;
                TextEditor_LoadFile(t, path);
            }
            if(len > sizeof(int)){
                int directoryLen;
                fread(&directoryLen, sizeof(int), 1, fp);
                if(len >= directoryLen && directoryLen < 512){
                    len -= directoryLen;
                    fread(t->fileBrowser.directory,1,directoryLen,fp);
                    t->fileBrowser.directory[directoryLen] = 0;
                    FileBrowser_ChangeDirectory(&t->fileBrowser);
                }
            }
        }
        fclose(fp);
    }
}

void TextEditor_Draw(TextEditor *t){

    int screenHeight = Graphics_TextCollumns();
    int screenWidth = Graphics_TextRows();

    int logY = 0, logX = 4;
    if(t->logging) logY = 1;

    int k = 0;
    int y = 0;
    int x = 0;

    Graphics_attron(COLOR_LINE_NUM);        
    // draw line numbers
    for(k = 0; k < Graphics_TextCollumns(); k++){
        char buffer[10];
        sprintf(buffer, "%i", t->file->scroll+k);
        Graphics_mvprintw(0, logY+k, buffer, strlen(buffer));
    }

    // logs

    if(t->logging && t->logging != LOGMODE_CONSOLE){

        Graphics_attron(COLOR_FIND);
        char buffer[] = "ERR\0";
        if(t->logging == LOGMODE_NUM){ sprintf(buffer, "g: "); }        
        else if(t->logging == LOGMODE_TEXT){ sprintf(buffer, "F: "); }        
        else if(t->logging == LOGMODE_TEXT_INSENSITIVE){ sprintf(buffer, "f: "); }        
        else if(t->logging == LOGMODE_OPEN){ sprintf(buffer, "o: "); }        
        else if(t->logging == LOGMODE_SAVE){ sprintf(buffer, "w: "); }        
        else if(t->logging == LOGMODE_SWITCH_FILE){ sprintf(buffer, "p: "); }        
        else if(t->logging == LOGMODE_FILEBROWSER){ sprintf(buffer, "O: "); }        
        else if(t->logging == LOGMODE_ALERT){ sprintf(buffer, "A: "); }        
        Graphics_mvprintw(0, 0, buffer, strlen(buffer));
    
        if(t->loggingText && strlen(t->loggingText) > 0){
            Graphics_mvprintw(3, 0, t->loggingText, strlen(t->loggingText));
        }

        if(t->logging == LOGMODE_SWITCH_FILE || t->logging == LOGMODE_FILEBROWSER){

            logY = 1;

            int nFiles = t->logging == LOGMODE_FILEBROWSER ? t->fileBrowser.nFiles : t->nFiles;
            
            k = 0;
            if(nFiles > Graphics_TextCollumns()-(logY+1)){
                k = t->logIndex-(Graphics_TextCollumns()-(logY+1));
                if(k < 0) k = 0;
            }

            int index = k;
            for(; k < nFiles; k++){
                char *name = t->logging == LOGMODE_FILEBROWSER ? t->fileBrowser.files[k].name : t->files[k]->name;
                int nameLen = strlen(name);
                int logNameLen = t->loggingText ? strlen(t->loggingText) : 0;
                if(nameLen > 0 && logNameLen <= nameLen){
                    if(logNameLen == 0 || CaseLowerStrnCmp(t->loggingText, name, logNameLen)){

                        if(index == t->logIndex){
                            if(t->logging == LOGMODE_FILEBROWSER && t->fileBrowser.files[k].dir)
                                Graphics_attron(COLOR_SELECTED_DIRECTORY);
                            else
                                Graphics_attron(COLOR_SELECTED);
                        } else {
                            if(t->logging == LOGMODE_FILEBROWSER && t->fileBrowser.files[k].dir)
                                Graphics_attron(COLOR_UNSELECTED_DIRECTORY);
                            else
                                Graphics_attron(COLOR_LOG_UNSELECTED);
                        }
                        index++;
                        Graphics_mvprintw(3, logY++, name, strlen(name));
                    }
                }
            }
        }
    }

    if(t->logging == LOGMODE_CONSOLE){

#ifdef LINUX_COMPILE
        fd_set rfds;
        struct timeval tv = {0, 0};
        char buf[4097];

        int logLen = strlen(t->loggingText);

        // while (1) {
        // if (waitpid(t->ttyPid, NULL, WNOHANG) == pid) {
        //   break;
        // }

        FD_ZERO(&rfds);
        FD_SET(t->ttyMaster, &rfds);
        if (select(t->ttyMaster + 1, &rfds, NULL, NULL, &tv)) {
          int size = read(t->ttyMaster, buf, 4096);

          t->loggingText = realloc(t->loggingText, logLen+size);
          memcpy(&t->loggingText[logLen], buf, size);

          logLen += size;
          t->loggingText[logLen] = 0;
        }
#endif
#ifdef WINDOWS_COMPILE
        FILE *fp = fopen(LOGCOMPILEFILE, "r");

        if(t->loggingText) free(t->loggingText);

        fseek(fp, 0, SEEK_END);
        int logLen = ftell(fp);
        rewind(fp);

        t->loggingText = malloc(logLen+1);
        fread(t->loggingText, 1, logLen, fp);
        t->loggingText[logLen] = 0;
        fclose(fp);
#endif

        Graphics_attron(COLOR_NORMAL);

        int last = 0;
        for(k = 0; k < logLen; k++){

            if(t->loggingText[k] == 0x1b){ // ansi

                if(t->loggingText[k+1] == '['){
                    k += 2;

                    last = k;
                    // params
                    int params[2] = {-1,-1};
                    int nParams = 0;
                    int lastParam = k;
                    while(k < logLen && t->loggingText[k] >= 0x30 && t->loggingText[k] <= 0x3F){
                        if(t->loggingText[k] == ';'){
                            t->loggingText[k] = 0;
                            params[nParams++] = atoi(&t->loggingText[lastParam]);
                            t->loggingText[k] = ';';
                            lastParam = k+1;
                        }
                        k++;
                    }

                    if(last != k){
                        char tmp = t->loggingText[k];
                        t->loggingText[k] = 0;
                        params[nParams++] = atoi(&t->loggingText[lastParam]);
                        t->loggingText[k] = tmp;
                    }


                    while(k < logLen && t->loggingText[k] >= 0x20 && t->loggingText[k] <= 0x2F){ k++; }
                    
                    last = k;
                    // ending

                    if(k < logLen && t->loggingText[k] >= 0x40 && t->loggingText[k] <= 0x7e){ k++; }

                    if(t->loggingText[k-1] == 'm'){
                        int m;
                        for(m = 0; m < nParams; m++){
                            
                            if(params[m] >= 30){
                                int ansi = params[m]-30;
                                if(ansi >= 7) ansi = 7;
                                Graphics_attron(TE_COLOR_BLACK + ansi);
                            } 
                            // if(params[m] == 01){
                            //     Graphics_attron(BOLD);
                            // }
                        }

                        if(nParams == 0){
                            Graphics_attron(TE_COLOR_WHITE);
                        }
                    }

                }


                last = k;
                k--;
                continue;
            }


            if(x >= Graphics_TextRows() || t->loggingText[k] == '\n'){
                last = k;
                y++;
                x = 0;
            // if(Graphics_TextRows() != x) k++;
                if(x < Graphics_TextRows())
                    continue;
            }

            Graphics_mvprintw(x, y, &t->loggingText[k], 1);
            x++;
        }
        // fclose(logfp);
        // free(t->loggingText);
        // t->loggingText = NULL;

        logY = y;
    }

    if(!t->file->text){
        Graphics_RenderNCurses();
        return;
    }

    // end logs

    t->file->textLen = strlen(t->file->text);

    int ctOffset = 0;
    x = 0;
    y = 0;
    // cant modulus x with width, \n might be farther out.

    k = 0;

    int scrollPos = 0;
    int scrollPosMax = 0;

    if(t->file->scroll > 0){
        for(k = 0; k < t->file->textLen; k++){
            if(t->file->text[k] == '\n'){
                y++;
                if(y == t->file->scroll) { k++; break; }
            }
        }
    }
    scrollPos = k;
    ctOffset = k;
    for(; k < t->file->textLen; k++){
        if(t->file->text[k] == '\n'){
            y++;
            if(y == t->file->scroll+screenHeight-logY) { k++; break; }
        }
    }
    scrollPosMax = k;

    y = 0;

    // int cur;
    // for(cur = 0; cur < t->nCursors; cur++){
    //     TextEditorCursor *c = &t->cursors[cur];
    
    // int renderTo = 0, renderStart = 0;
        int renderTo = scrollPosMax, renderStart = scrollPos;

    // if(c->selection.len == 0){
        // renderStart = c->pos - GetCharsIntoLine(t->file->text, c->pos);
        // renderTo = GetStartOfNextLine(t->file->text, t->file->textLen, c->pos);
        // if(renderTo < scrollPos) continue;
        for(k = scrollPos; k < renderStart; k++){
            if(t->file->text[k] == '\n'){
                y++;
            }
        }
        // if(renderStart < scrollPosMax) continue;
        ctOffset = k;
    
        for(; k < renderTo; ){


            // if(x > screenHeight){
                // for(; k < t->file->textLen; k++){
                //     if(t->file->text[k] == '\n'){
                //         y++;
                //         x = 0;
                //         ctOffset = ++k;
                //     }
                // }
            // }

            // printf("%i %i\n",y, screenHeight );

            int comment = 0;
            int string = 0;

            char c = t->file->text[k];

            char token = IsToken(c);
            if(!token){

                c = t->file->text[k];
                // number not like COLOR_FUNCTION but 23
                if((k - ctOffset) == 1 && IsDigit(c)){

                    Graphics_attron(COLOR_NUM);

                    for(k = k+1; k < renderTo && IsDigit(t->file->text[k]); k++);

                    Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], k - ctOffset);
                    x += k - ctOffset;
                    ctOffset = k;

                    continue;
                }

                k++;
                // add to temp str

            } else {

                if(k - ctOffset > 0){

                    if((k - ctOffset) == 1 && IsDigit(t->file->text[k-1])){
                        Graphics_attron(COLOR_NUM);
                        Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], 1);
                        x++;
                        ctOffset = k;
                        goto addedStr;
                    }

                    for(int m = 0; m < (int)(sizeof(keywords)/sizeof(char *)); m++){
                        if(strlen(keywords[m]) == (k - ctOffset) && 
                            memcmp(&t->file->text[ctOffset], keywords[m], (k - ctOffset)) == 0) {
                            
                            Graphics_attron(COLOR_KEYWORD);
                            Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], k - ctOffset);

                            x += k - ctOffset;
                            ctOffset = k;
                            goto addedStr;

                        }
                    }
                
                    if(c == '('){
                        // function def example(), k - ctOffset, ctOffset stops at beginning of non tokens, unless a digit or a keyword
                        // so that we can change colors at the token after the def

                        Graphics_attron(COLOR_FUNCTION);
                        Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], k - ctOffset); 
                        x += k - ctOffset;
                        Graphics_attron(COLOR_FUNCTION);
                        Graphics_mvprintw(logX+x, logY+y, &t->file->text[k], 1);
                        x++;
                        k++;
                        ctOffset = k;
                        continue;
                    }
        
                    Graphics_attron(COLOR_NORMAL);
                    Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], k - ctOffset);
                    x += k - ctOffset;

                    ctOffset = k;
                }

                addedStr:

                if(c =='\n'){
            
                    y++;
                    x = 0;
                    ctOffset = ++k;
                    continue;
                }

                if(c =='\t'){
                    x += 4;
                    ctOffset = ++k;
                    continue;
                }

                if(c ==' '){
                    x++;
                    ctOffset = ++k;
                    continue;
                }

                if(c == '/' && t->file->text[k+1] == '/') comment = 1;
                else if(c == '/' && t->file->text[k+1] == '*') comment = 2;
                else if(c == '"') string = 1;
                else if(c == '\'') string = 2;
        
        
                if(c == ')' || c == '('){
                    Graphics_attron(COLOR_FUNCTION);
                    Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], 1);
                    x++;
                    ctOffset = ++k;
                    continue;
                }
                
                if(c != ' ' && c != '(' && c != ')' && token && !comment && !string){
                    Graphics_attron(COLOR_TOKEN);
                    Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], 1);
                    x++;
                    ctOffset = ++k;
                    continue;
                }
                

                // if(!comment && !string) k++;

                if(comment){


                    k += 2;

                    if(comment == 1){
                        for (; k < renderTo; k++){
                            if(t->file->text[k] == '\n'){  break; }
                        }


                    } else { /* comment */


                        for(;k < (renderTo-1) && !(t->file->text[k] == '*' && t->file->text[k+1] == '/'); k++);

                        if(k < renderTo-1){
                            k++; // will run to end of file otherwise. 
                        }
                    }

                    if(k > 0 && t->file->text[k-1] == '*' && t->file->text[k] == '/') k++;

                    Graphics_attron(COLOR_COMMENT);
                    Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], k - ctOffset);
                    x += k - ctOffset;
                    ctOffset = k;
                    comment = 0;
                    continue;
                }


                if(string){


                    int escaped = 0;

                    while(k < renderTo){

                        k++;
                        if(k == renderTo) break;


                        if(t->file->text[k] == '\n') { break; }
                        if(t->file->text[k] == '"' && string == 1 && !escaped) { break; }
                        if(t->file->text[k] == '\'' && string == 2 && !escaped) { break; }


                        if(t->file->text[k] == '\\' && !escaped)
                            escaped = 1;
                        else
                            escaped = 0;


                    }
                    if(t->file->text[k] != '\n')
                        k++;
                    
                    if(k == renderTo) break;

                    Graphics_attron(COLOR_STRING);
                    Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], k - ctOffset);
                                    
                    x += k - ctOffset;
                    ctOffset = k;
                    string = 0;

                    continue;
                }

                x++;
                ctOffset = ++k;
            }
        }

        if(k - ctOffset > 0){
            Graphics_attron(COLOR_NORMAL);
            Graphics_mvprintw(logX+x, logY+y, &t->file->text[ctOffset], k - ctOffset);
        }

    // } else { // easy selections have no highlighting
    // }
    int cur;
    for(cur = 0; cur < t->nCursors; cur++){
        TextEditorCursor *c = &t->cursors[cur];
        
        TextEditorSelection *selection = &c->selection;

        if(selection->len > 0){
            int renderStart = selection->startCursorPos;
            int renderTo = selection->startCursorPos + selection->len;

            if(renderTo < scrollPos) continue;
            if(renderStart > scrollPosMax) continue;

            Graphics_attron(COLOR_SELECTED);

            x = 0; 
            y = 0;
            for(k = scrollPos; k < renderTo; k++){
                if(t->file->text[k] == '\n'){
                    y++;
                    x = 0;
                    continue;
                } else if(t->file->text[k] == '\t'){
                    if(k >= renderStart)
                        Graphics_mvprintw(logX+x, logY+y, "    ", 4);
                    x += 4;
                    continue;
                }

                if(k >= renderStart && x < screenWidth && y < screenHeight){
                    Graphics_mvprintw(logX+x, logY+y, &t->file->text[k], 1);
                }
                x++;
            }
        }
        // render cursor

        if(scrollPos > c->pos) continue;

        k = scrollPos;
        y = 0;
        x = 0;

        for(; k < c->pos && k < scrollPosMax; k++){
            if(t->file->text[k] == '\n'){
                x = 0;
                y++;
                continue;
            } else if(t->file->text[k] == '\t'){
                x+=4;
                continue;
            }
            x++;
        }

        Graphics_attron(COLOR_CURSOR);

        if((y >= screenHeight) ||
            x >= screenWidth) continue;

        if(t->file->text[c->pos] == '\n' || t->file->text[c->pos] == '\t')
            Graphics_mvprintw(logX+x, logY+y, " ", 1);
        else
            Graphics_mvprintw(logX+x, logY+y, &t->file->text[c->pos], 1);
    
    }

    int j;

    Graphics_RenderNCurses();

    k = 0;
    y = 0;

    for(j = 0; j < t->autoCompleteLen; j++){

        if(j == t->autoCompleteIndex)
            Graphics_attron(COLOR_SELECTED);
        else 
            Graphics_attron(COLOR_AUTO_COMPLETE);

        y = 0;
        x = 0;
        for(k = scrollPos; k < t->cursors[t->nCursors-1].pos; k++){

            if(t->file->text[k] == '\n'){
                x = 0;
                y++;
                continue;
            } else if(t->file->text[k] == '\t'){
                x+=4;
                continue;
            }
            x++;
            
            if(y < 0 || y+j+1 >= screenHeight) break;
            if(x >= screenWidth) continue;
        }

        if(k == t->cursors[t->nCursors-1].pos){

            char buffer[MAX_AUTO_COMPLETE_STRLEN];
            memset(buffer, ' ', MAX_AUTO_COMPLETE_STRLEN);
            memcpy(buffer, &t->file->text[t->autoComplete[j].offset],t->autoComplete[j].len);
            // Graphics_mvprintw(logX+x, logY+y+j+1, &t->file->text[t->autoComplete[j].offset], t->autoComplete[j].len);
            Graphics_mvprintw(logX+x, logY+y+j+1, buffer, MAX_AUTO_COMPLETE_STRLEN);
        }

    }

    Graphics_RenderNCurses();
}



void TextEditor_Event(TextEditor *t, unsigned int key){

    if(key == 27){ // escape
        RemoveSelections(t);
        RemoveExtraCursors(t);
        EndLogging(t);
        t->autoCompleteIndex = 0;
        UpdateScrollCenter(t);
        return;
    }

    if(key == EDIT_ENTER_KEY){ // enter
        EventEnter(t);
        return;
    }

    if(key>>8){

        //ctrl/shift/alt or arrowkeys

        if(key == ((( unsigned int)'q') | EDIT_CTRL_KEY)){
            t->quit = 1;
            return;
        }
        if(key == ((( unsigned int)'b') | EDIT_CTRL_KEY)){
            
            if(t->loggingText) free(t->loggingText);
            char *buffer = "compiling:\n\0";
            t->loggingText = malloc(strlen(buffer)+1);
            memcpy(t->loggingText, buffer, strlen(buffer));
            t->loggingText[strlen(buffer)] = 0;
            
#ifdef LINUX_COMPILE
            openpty(&t->ttyMaster, &t->ttySlave, NULL, NULL, NULL);

            t->_stderr = dup(STDERR_FILENO);
            t->_stdout = dup(STDOUT_FILENO);
            dup2(t->ttySlave, STDERR_FILENO);
            dup2(t->ttySlave, STDOUT_FILENO);

            t->ttyPid = fork();
            if (t->ttyPid == 0) {
                char *args[] = {"make",NULL};
                execvp(args[0], args);
            }
#endif
#ifdef WINDOWS_COMPILE
            system("make > " LOGCOMPILEFILE " 2>&1 &");
#endif
            t->logging = LOGMODE_CONSOLE;
            return;
        }
        if(key == ((( unsigned int)'=') | EDIT_CTRL_KEY)){
            Graphics_Zoom(1);
            return;
        }
        if(key == ((( unsigned int)'-') | EDIT_CTRL_KEY)){
            Graphics_Zoom(-1);
            return;
        }

        int k;
        for(k = 0; k < t->nCommands; k++){

            if(t->commands[k]->keyBinding[0] == key){

                ExecuteCommand(t,t->commands[k]);
                break;
            }
        }
        return;
    }
    if(key == 9){ // tab
        if(t->logging) return;
        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, "\t", 0,SCR_CENT, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);
        return;
    }

    if(key == 127){ // backspace

        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, 1, SCR_CENT, RemoveCharacters, UndoRemoveCharacters);
        ExecuteCommand(t, command);
        FreeCommand(command);
        return;
    }

    if(key >= 32 && key <= 126){

        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, (const char[]){(char)key, 0}, 0, SCR_CENT, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);
        return;
    }

}

int TextEditor_Destroy(TextEditor *t){
    
    FILE *fp = fopen(LOGFILE, "w");
    int nFiles = 0;
    int k;

    if(fp){
        for(k = 0; k < t->nFiles; k++){
            int len = strlen(t->files[k]->path);
            if(len > 0) nFiles++;

        }
        fwrite(&nFiles,sizeof(int),1,fp);
    }

    // write file last so it opens first

    int len = strlen(t->file->path);
    if(len > 0){
        for(k = 0; k < t->nFiles; k++){
            if(t->files[k] == t->file) { t->files[k] = t->files[t->nFiles-1]; break; }
        }
        t->files[t->nFiles-1] = t->file;
    }    
    for(k = 0; k < t->nFiles; k++){
        int len = strlen(t->files[k]->path);
        if(len > 0){
            if(fp){
                fwrite(&len,sizeof(int),1,fp);
                fwrite(t->files[k]->path,1,len,fp);
            }
        } else if(strlen(t->files[k]->text) > 0){
            EndLogging(t);
            t->logging = LOGMODE_ALERT;
            char buffer[] = "Unsaved changes? ctrl+w to ignore unsaved, ctrl+s to save, ctrl+S to save as.\0";
            int bufferLen = strlen(buffer);
            t->loggingText = malloc(bufferLen+1);
            strcpy(t->loggingText, buffer);
            t->loggingText[bufferLen] = 0;
            t->file = t->files[k];
            RefreshFile(t);
            if(fp) fclose(fp);
            t->quit = 0;
            return 0;

        }

        if(t->nFiles > 1 && t->file == t->files[k]){
            t->file = t->file == t->files[0] ? t->files[1] : t->files[0];
        }

        FreeFile(t->files[k]);

        // pop stack
        int j;
        for(j = k; j < t->nFiles-1; j++)
            t->files[j] = t->files[j+1];

        k--; // stack[k] is now the stack[k+1] so dont let for loop inc k

        t->files = (TextEditorFile **)realloc(t->files,sizeof(TextEditorFile*) * t->nFiles--);

    }

    if(fp){
        int len = strlen(t->fileBrowser.directory);
        if(len > 0){
            fwrite(&len,sizeof(int),1,fp);
            fwrite(t->fileBrowser.directory,1,len,fp);
        }
        fclose(fp);
    }

    for(k = 0; k < t->nCommands; k++)
        FreeCommand(t->commands[k]);

    FreeCursors(t);
    FileBrowser_Free(&t->fileBrowser);
    return 1;
}
