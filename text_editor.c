#include "graphics.h"
#include "text_editor.h"
#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

enum {

    COLOR1 = 1,
    COLOR2, // keywords 
    COLOR3, // wildcard token
    COLOR4, // functios
    COLOR5, // number 
    COLOR6, // comment
    COLOR7, // string
    COLOR_SELECTED,
    COLOR_AUTO_COMPLETE,
    COLOR_CURSOR,
    COLOR_SIDE_NUMBERS,
};

static int MoveByWordsFunc(char *text, int len, int start, int dir);
static void SearchingRemoveChars(TextEditor *t, int num);
static void SearchingAddText(TextEditor *t, char *str);
static void FindCommand(TextEditor *t, TextEditorCommand *command);
static void AddSavedText(TextEditorCommand *command, char *str, int len, int index);
static void EraseAllSelectedText(TextEditor *t, int index, TextEditorCommand *command);
static void AutoComplete(TextEditor *t);
// static void DrawSearchBar(TextEditor *t);
static int IsToken(char c);
static int IsDigit(char c);
static int GetStartOfNextLine(char *text, int textLen, int cPos);
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
static void MoveLineUp(TextEditor *t, TextEditorCursor *c);
static void MoveLineDown(TextEditor *t, TextEditorCursor *c);
static void RemoveSelections(TextEditor *t);
static void SetCursorToSelection(TextEditorCursor *cursor, int n);
static void MoveByChars(TextEditor *t, TextEditorCommand *c);
static void MoveByWords(TextEditor *t, TextEditorCommand *c);
static void FreeCursors(TextEditor *t);
static void MoveCursorUpLine(TextEditor *t, TextEditorCursor *cursor);
static void MoveCursorDownLine(TextEditor *t, TextEditorCursor *cursor);
static void GetSelectionStartEnd(TextEditorSelection selection, int *s, int *e);
static void MoveLines(TextEditor *t, TextEditorCommand *c);
// static void ExpandSelectionChars(TextEditor *t,TextEditorCommand *c);
static void ExpandSelectionWords(TextEditor *t,TextEditorCommand *c);
static void AddCursorCommand(TextEditor *t, TextEditorCommand *c);
static TextEditorCursor *AddCursor(TextEditor *t);
static int Find(char *text, char *str, int len);
static void SelectNextWord(TextEditor *t, TextEditorCommand *c);
static void EventEnter(TextEditor *t);
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
static TextEditorCommand *CreateCommand(const unsigned int binding[], const char *keys, int n,
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

static int IsToken(char c){
    if(c == '(' || c == ' ' || c == '\n'|| c == ',' || c == '+' || c == '=' || c == '~' || c == '<' || 
        c == '>' || c == '*' || c == ')' || c == '/' || c == '{' || c == '}' || c == '%' || c == '^' || 
        c == ':' || c == '.' || c == ';' || c == ']' || c == '[' || c == '-' || c == ']' || c == '"' ||
        c == '\''|| c == '\t'|| c == '\0' || c == '#' || c == '&' || c == '!') return 1;

    return 0;
}


static void GetCursorPos(char *text, int cPos, int *x, int *y){

    *x = 0;
    *y = 0;

    int k;
    for(k = 0; k < cPos; k++){

        (*x)++;

        if(text[k] == '\n'){
            (*y)++;
            *x = 0;
        }
    }
}

static int fNextLine(char *text, int textLen, int cPos){

    int k;
    for(k = cPos; k < textLen; k++)
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

static void EventEnter(TextEditor *t){

    if(t->autoCompleteLen){
        AutoCompleteOffset *ac = &t->autoComplete[t->autoCompleteIndex];

        char buffer[MAX_AUTO_COMPLETE_STRLEN];
        int len = ac->len - t->autoCompleteSearchLen;
        int offset = ac->offset+t->autoCompleteSearchLen;
        memcpy(buffer, &t->text[offset], len);
        buffer[len] = 0;


        TextEditorCommand *command = 
        CreateCommand((const unsigned int[]){0}, buffer, 0, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);

        t->autoCompleteSearchLen = 0;
        t->autoCompleteLen = 0;
        t->autoCompleteIndex = 0;
        return;
    }

    TextEditorCommand *command = 
    CreateCommand((const unsigned int[]){0}, (const char[]){'\n', 0}, 0, AddCharacters, UndoAddCharacters);
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
    for(k=cPos; k > 0; k--)
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

    int end, start;
    GetSelectionStartEnd(cursor->selection, &start, &end);
    
    if(n < 0) cursor->pos = start;
    else cursor->pos = end;
}


static void MoveByChars(TextEditor *t, TextEditorCommand *c){

    int textLen = t->textLen;

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

static int MoveByWordsFunc(char *text, int len, int start, int dir){

    if((start > len && dir > 0) || (start == 0 && dir < 0)) return start;

    if(dir < 0){
        
        if(text[start-1] == '\n'){
            start -= 1;
        } else {

            // @#$|(a)sdff             
            if(!IsToken(text[start] && IsToken(text[start-1]))){
                --start;
            }
            
            if(IsToken(text[start])){
                while(start > 0){
                    char c = text[--start];
                    if(c == '\n') start++;
                    if(!IsToken(c) || c == '\n') break;
                }
            } else {
                start = GetWordStart(text, start); 
            }
        }

    } else {
        if(!IsToken(text[start] && IsToken(text[start+1])))
            ++start;

        if(IsToken(text[start])){
            while(start < len){
                char c = text[++start];
                if(!IsToken(c) || c == '\n') break;
            }
        } else {
            int  end = GetWordEnd(text, start);
            start = end;
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


        t->cursors[k].pos = MoveByWordsFunc(t->text, t->textLen,t->cursors[k].pos,c->num);
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

static void MoveLineUp(TextEditor *t, TextEditorCursor *c){
    if(t->text == NULL) return;

    int k;
    for(k = 0; k < t->nCursors; k++){
        
        TextEditorCursor *cursor = &t->cursors[k];            


        if(cursor->selection.len == 0){

            int lineStart = cursor->pos == 0 ? 0 : cursor->pos-1;
            for(; lineStart > 0 && t->text[lineStart] != '\n'; lineStart--);
            if(lineStart == 0) continue; // already top line
            lineStart++; // skip \n of prev line

            int charsOnLine = cursor->pos;
            for(; charsOnLine < t->textLen && t->text[charsOnLine] != '\n'; charsOnLine++);
            charsOnLine = (charsOnLine - lineStart);

            int startOfPrevLine = lineStart-2;
            for(; startOfPrevLine > 0 && t->text[startOfPrevLine] != '\n'; startOfPrevLine--);
            if(startOfPrevLine > 0) startOfPrevLine++;
            int prevLineLen = lineStart - startOfPrevLine;

            if(prevLineLen > charsOnLine){

                char *tmp = malloc(charsOnLine);
                memcpy(tmp, &t->text[lineStart], charsOnLine);

                memcpy(&t->text[startOfPrevLine+charsOnLine+1], &t->text[startOfPrevLine], prevLineLen);
                t->text[startOfPrevLine+charsOnLine] = '\n';
                memcpy(&t->text[startOfPrevLine], tmp, charsOnLine);

                free(tmp);
            } else {
                char *tmp = malloc(prevLineLen);
                memcpy(tmp, &t->text[startOfPrevLine], prevLineLen);

                memcpy(&t->text[startOfPrevLine], &t->text[lineStart], charsOnLine);
                t->text[startOfPrevLine+charsOnLine] = '\n';
                memcpy(&t->text[startOfPrevLine+charsOnLine+1], tmp, prevLineLen);
                free(tmp);
            }

            cursor->pos = startOfPrevLine + (cursor->pos - lineStart);
        } else {

            int lineStart = cursor->selection.startCursorPos == 0 ? 0 : cursor->selection.startCursorPos-1;
            for(; lineStart > 0 && t->text[lineStart] != '\n'; lineStart--);
            if(lineStart > 0) lineStart++; // skip \n of prev line

            int charsOnLine = cursor->selection.startCursorPos+cursor->selection.len-1;
            for(; charsOnLine < t->textLen && t->text[charsOnLine] != '\n'; charsOnLine++);
            if(charsOnLine >= t->textLen || t->text[charsOnLine] != '\n') continue; // end of file dont move down
            charsOnLine = (charsOnLine - lineStart);

            int startOfPrevLine = lineStart-2;
            for(; startOfPrevLine > 0 && t->text[startOfPrevLine] != '\n'; startOfPrevLine--);
            if(startOfPrevLine > 0) startOfPrevLine++;
            int prevLineLen = lineStart - startOfPrevLine;

            if(prevLineLen > charsOnLine){

                char *tmp = malloc(charsOnLine);
                memcpy(tmp, &t->text[lineStart], charsOnLine);

                memcpy(&t->text[startOfPrevLine+charsOnLine+1], &t->text[startOfPrevLine], prevLineLen);
                t->text[startOfPrevLine+charsOnLine] = '\n';
                memcpy(&t->text[startOfPrevLine], tmp, charsOnLine);

                free(tmp);

            } else {
                char *tmp = malloc(prevLineLen);
                memcpy(tmp, &t->text[startOfPrevLine], prevLineLen);

                memcpy(&t->text[startOfPrevLine], &t->text[lineStart], charsOnLine);
                t->text[startOfPrevLine+charsOnLine] = '\n';
                memcpy(&t->text[startOfPrevLine+charsOnLine+1], tmp, prevLineLen);
                free(tmp);

            }

            cursor->selection.startCursorPos = startOfPrevLine + (cursor->selection.startCursorPos - lineStart);
            cursor->pos = startOfPrevLine + (cursor->pos - lineStart);
    
        }
    }
}

static void MoveLineDown(TextEditor *t, TextEditorCursor *c){
    if(t->text == NULL) return;

    int k;
    for(k = 0; k < t->nCursors; k++){
        
        TextEditorCursor *cursor = &t->cursors[k];            


        if(cursor->selection.len == 0){

            int lineStart = cursor->pos == 0 ? 0 : cursor->pos-1;
            for(; lineStart > 0 && t->text[lineStart] != '\n'; lineStart--);
            if(lineStart > 0) lineStart++; // skip \n of prev line

            int charsOnLine = cursor->pos;
            for(; charsOnLine < t->textLen && t->text[charsOnLine] != '\n'; charsOnLine++);
            if(charsOnLine >= t->textLen || t->text[charsOnLine] != '\n') continue; // end of file dont move down
            charsOnLine = (charsOnLine - lineStart);

            int startOfNextLine = lineStart+charsOnLine+1;
            int nextLineLen = startOfNextLine;
            for(; nextLineLen < t->textLen && t->text[nextLineLen] != '\n'; nextLineLen++);

            nextLineLen = (nextLineLen - startOfNextLine);

            if(nextLineLen > charsOnLine){
                char *tmp = malloc(charsOnLine);
                memcpy(tmp, &t->text[lineStart], charsOnLine);
                memcpy(&t->text[lineStart], &t->text[startOfNextLine], nextLineLen);
                t->text[lineStart+nextLineLen] = '\n';
                memcpy(&t->text[lineStart+nextLineLen+1], tmp, charsOnLine);

                free(tmp);
            } else {
                char *tmp = malloc(nextLineLen);
                memcpy(tmp, &t->text[startOfNextLine], nextLineLen);

                memcpy(&t->text[lineStart+nextLineLen+1], &t->text[lineStart], charsOnLine);
                t->text[lineStart+nextLineLen] = '\n';
                memcpy(&t->text[lineStart], tmp, nextLineLen);
                free(tmp);
            }

            cursor->pos = lineStart+nextLineLen+1 + (cursor->pos - lineStart);
        } else {

            int lineStart = cursor->selection.startCursorPos == 0 ? 0 : cursor->selection.startCursorPos-1;
            for(; lineStart > 0 && t->text[lineStart] != '\n'; lineStart--);
            if(lineStart > 0) lineStart++; // skip \n of prev line

            int charsOnLine = cursor->selection.startCursorPos+cursor->selection.len-1;
            for(; charsOnLine < t->textLen && t->text[charsOnLine] != '\n'; charsOnLine++);
            if(charsOnLine >= t->textLen || t->text[charsOnLine] != '\n') continue; // end of file dont move down

            charsOnLine = (charsOnLine - lineStart);

            int startOfNextLine = lineStart+charsOnLine+1;
            int nextLineLen = startOfNextLine;
            for(; nextLineLen < t->textLen && t->text[nextLineLen] != '\n'; nextLineLen++);

            nextLineLen = (nextLineLen - startOfNextLine);

            if(nextLineLen > charsOnLine){
                char *tmp = malloc(charsOnLine);
                memcpy(tmp, &t->text[lineStart], charsOnLine);
                memcpy(&t->text[lineStart], &t->text[startOfNextLine], nextLineLen);
                t->text[lineStart+nextLineLen] = '\n';
                memcpy(&t->text[lineStart+nextLineLen+1], tmp, charsOnLine);

                free(tmp);
            } else {
                char *tmp = malloc(nextLineLen);
                memcpy(tmp, &t->text[startOfNextLine], nextLineLen);

                memcpy(&t->text[lineStart+nextLineLen+1], &t->text[lineStart], charsOnLine);
                t->text[lineStart+nextLineLen] = '\n';
                memcpy(&t->text[lineStart], tmp, nextLineLen);
                free(tmp);
            }

            cursor->pos = lineStart+nextLineLen+1 + (cursor->pos - lineStart);
            cursor->selection.startCursorPos = lineStart+nextLineLen+1 + (cursor->selection.startCursorPos - lineStart);

        }
    }
}

static void MoveLinesText(TextEditor *t, TextEditorCommand *c){

    int k;

    if(t->text == NULL || !t->textLen)
        return;

    for(k = 0; k < t->nCursors; k++){

        TextEditorCursor *cursor = &t->cursors[k];

        if(c->num > 0)
            MoveLineDown(t, cursor);
        else if(c->num < 0 && GetNumLinesToPos(t->text, cursor->pos) > 0)
            MoveLineUp(t, cursor);
    }

    ResolveCursorCollisions(t);
}

static void MoveCursorUpLine(TextEditor *t, TextEditorCursor *cursor){

    if(t->text == NULL) return;
    if(GetNumLinesToPos(t->text, cursor->pos) == 0) return;

    int charsIntoLine = GetCharsIntoLine(t->text, cursor->pos);
    int startOfPrevLine = GetStartOfPrevLine(t->text, cursor->pos);

    int k;
    for(k = startOfPrevLine; k < (int)t->textLen; k++)
        if(t->text[k] == '\n') break;

    int charsOnPrevLine = (k - startOfPrevLine);
    cursor->pos = charsIntoLine <= charsOnPrevLine ? startOfPrevLine + charsIntoLine: startOfPrevLine + charsOnPrevLine; 
}

static int GetStartOfNextLine(char *text, int textLen, int cPos){

    int k;
    for(k = cPos; k < textLen; k++)
        if(text[k] == '\n') { k++; break; }

    return k;
}

static void MoveCursorDownLine(TextEditor *t, TextEditorCursor *cursor){

    if(t->text == NULL) return;

    int charsIntoLine = GetCharsIntoLine(t->text, cursor->pos);
    int startOfNextLine = GetStartOfNextLine(t->text, t->textLen, cursor->pos);

    if(startOfNextLine == (int)t->textLen) return;

    int k;
    for(k = startOfNextLine; k < (int)t->textLen; k++)
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

    if(t->autoCompleteLen > 0){
        t->autoCompleteIndex += c->num;
        if(t->autoCompleteIndex < 0) t->autoCompleteIndex = 0;
        if(t->autoCompleteIndex > MAX_AUTO_COMPLETE-1) t->autoCompleteIndex = MAX_AUTO_COMPLETE-1;
        return;
    }

    int k;

    if(t->text == NULL || !t->textLen){
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

// static void ExpandSelectionChars(TextEditor *t,TextEditorCommand *c){

    // if(t->text == NULL) return;

    // int f;
    // for(f = 0; f < t->nCursors; f++){

    //     TextEditorCursor *cursor = &t->cursors[f];

    //     if(cursor->selection.len == 0){
    //         cursor->selection.startCursorPos = cursor->pos;
    //     }

    //     cursor->pos += c->num;

    //     if(cursor->pos < 0) cursor->pos = 0;
    //     if(cursor->pos > (int)t->textLen) cursor->pos = t->textLen;

    //     cursor->selection.len += c->num;

    //     if(cursor->selection.startCursorPos+cursor->selection.len < 0)
    //         cursor->selection.len -= c->num;

    //     if(cursor->selection.startCursorPos+cursor->selection.len > (int)t->textLen)
    //         cursor->selection.len -= c->num;
    // }

    // ResolveCursorCollisions(t);
// }

static void ExpandSelectionWords(TextEditor *t,TextEditorCommand *c){

    int k;
    for(k = 0; k < t->nCursors; k++){

        int prev = t->cursors[k].pos;
        t->cursors[k].pos = MoveByWordsFunc(t->text, t->textLen, t->cursors[k].pos,c->num);
        
        if(t->cursors[k].selection.len == 0)
            t->cursors[k].selection.startCursorPos = prev;


        if(t->cursors[k].pos > t->cursors[k].selection.startCursorPos+t->cursors[k].selection.len){

            t->cursors[k].selection.len += t->cursors[k].pos - prev;

        } else if(t->cursors[k].pos < t->cursors[k].selection.startCursorPos) {

            t->cursors[k].selection.len += t->cursors[k].selection.startCursorPos - t->cursors[k].pos;
            t->cursors[k].selection.startCursorPos = t->cursors[k].pos;
        }
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

}

static void ExpandSelectionLines(TextEditor *t, TextEditorCommand *c){

    if(t->text == NULL) return;

    RemoveExtraCursors(t);
    t->textLen = strlen(t->text);

    int f;
    for(f = 0; f < t->nCursors; f++){

        TextEditorCursor *cursor = &t->cursors[f];

        if(cursor->selection.len == 0){
            cursor->selection.startCursorPos = cursor->pos - GetCharsIntoLine(t->text, cursor->pos);
        }

        if(c->num < 0)
            cursor->pos = GetStartOfPrevLine(t->text, cursor->pos);
        else
            cursor->pos = GetStartOfNextLine(t->text, t->textLen, cursor->pos);         

        if(cursor->pos < 0) cursor->pos = 0;
        if(cursor->pos > (int)t->textLen) cursor->pos = t->textLen;

        cursor->selection.len = cursor->pos - cursor->selection.startCursorPos;
        if(cursor->selection.startCursorPos+cursor->selection.len < 0)
            cursor->selection.len -= c->num;

        if(cursor->selection.startCursorPos+cursor->selection.len > (int)t->textLen)
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
        int end = GetStartOfNextLine(t->text,t->textLen, cursor->pos);

        cursor->pos = end;

        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, end-start, RemoveCharacters, UndoRemoveCharacters);
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

    TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, 1, Copy, NULL);
    ExecuteCommand(t, command);
    FreeCommand(command);

    command = CreateCommand((const unsigned int[]){0}, 0, 0, RemoveCharacters, UndoRemoveCharacters);
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

    int textLen = t->textLen;

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
        textLen = t->textLen;

    int len = strlen(text);

    t->text = (char *)realloc(t->text, textLen + len + 1);
    t->text[textLen + len] = 0;

    if(textLen - pos > 0)
        memmove(&t->text[pos + len], &t->text[pos], textLen - pos);

    memcpy(&t->text[pos], text, len);

    MoveCursorsAndSelection(t, pos-1, len);
    t->textLen += len;
}

static void RemoveCharactersFromText(TextEditor *t, int pos, int len){

    if(t->text == NULL) return;

    int textLen = t->textLen;

    if(pos - len < 0) return;

    if(pos < textLen){
        // memmove(&t->text[pos - c->num], &t->text[pos], textLen - (pos+1));
        // memmove(&t->text[pos - c->num], &t->text[pos], textLen - (pos+c->num));
        memmove(&t->text[pos - len], &t->text[pos], textLen - pos);
    }


    t->text = (char *)realloc(t->text, (textLen - len) + 1);
    
    t->text[textLen - len] = 0;
    t->textLen -= len;

    MoveCursorsAndSelection(t, pos-1, -len);
}

static void UndoRemoveCharacters(TextEditor *t, TextEditorCommand *c){

    LoadCursors(t, c);

    int k;
    for(k = t->nCursors-1; k >= 0; k--){

        if(k < c->nSavedCursors && c->savedTexts[k])
            AddStrToText(t, t->cursors[k].pos, c->savedTexts[k]);
    }


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

            while(findEnd+t->autoCompleteSearchLen < t->textLen &&
                (res = Find(&t->text[findEnd], &t->text[search], t->autoCompleteSearchLen)) != -1){

                if(findEnd+res == search){
                    findEnd = c->pos;
                    continue;
                }

                int j;
                for(j = 0; findEnd + res + j < t->textLen && 
                    j < MAX_AUTO_COMPLETE_STRLEN && !IsToken(t->text[findEnd+res+j]); j++);


                int m;
                for(m = 0; m < t->autoCompleteLen; m++){
                    if(t->autoComplete[m].len == j && 
                        memcmp(&t->text[t->autoComplete[m].offset], &t->text[findEnd+res],j) == 0){
        
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

            // save for undo
            AddSavedText(c, &t->text[pos], len, k);
            RemoveCharactersFromText(t, t->cursors[k].pos, c->num);
        }

        if(t->text == NULL) break;

        EraseAllSelectedText(t, k, c);

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

    LoadCursors(t, c);

    int k;
    for(k = t->nCursors-1; k >= 0; k--){

        RemoveCharactersFromText(t, t->cursors[k].pos, c->addedLens[k]);

        if(k < c->nSavedCursors && c->savedTexts[k])
            AddStrToText(t, t->cursors[k].pos, c->savedTexts[k]);
    }



    SaveCursors(t, c);


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

static void IndentLine(TextEditor *t, TextEditorCommand *c){

    int k;
    for(k = 0; k < t->nCursors; k++){

        int prev = t->cursors[k].pos;
        if(t->cursors[k].selection.len == 0){

            if(t->text[prev] != '\n')
                t->cursors[k].pos = prev - GetCharsIntoLine(t->text, t->cursors[k].pos);
            else
                t->cursors[k].pos++;

            if(c->num > 0){
                TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, "\t", 0, AddCharacters, UndoAddCharacters);
                ExecuteCommand(t,command);
                FreeCommand(command);
                prev++;
            } else {
                if (t->text[t->cursors[k].pos] == '\t' || t->text[t->cursors[k].pos] == ' '){
                    t->cursors[k].pos++;
                    TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, 1, 
                        RemoveCharacters, UndoRemoveCharacters);
                    ExecuteCommand(t,command);
                    FreeCommand(command);
                    prev--;
                } 
            }

        } else {

            TextEditorSelection selection = t->cursors[k].selection;

            int startCursorPos=0;
            int endCursorPos=0;
            GetSelectionStartEnd(t->cursors[k].selection, &startCursorPos, &endCursorPos);
            RemoveSelections(t);

            int next = startCursorPos;

            if(t->text[next] != '\n')
                next = startCursorPos - GetCharsIntoLine(t->text, next);
            else
                next++;

            do {
                t->cursors[k].pos = next;

                if(c->num > 0){
                    if(next == selection.startCursorPos){
                        selection.startCursorPos++;
                    } else {
                        selection.len++;
                    }
                    TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 
                        "\t", 0, AddCharacters, UndoAddCharacters);
                    ExecuteCommand(t,command);
                    FreeCommand(command);
                    prev++;
                } else {
                    if (t->text[t->cursors[k].pos] == '\t' || t->text[t->cursors[k].pos] == ' '){
    
                        if(next == selection.startCursorPos){
                            selection.startCursorPos--;
                        } else {
                            selection.len--;
                        }
    
                        t->cursors[k].pos++;
                        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, 1, 
                            RemoveCharacters, UndoRemoveCharacters);
                        ExecuteCommand(t,command);
                        FreeCommand(command);
                        prev--;
                    } 
                }

                next = GetStartOfNextLine(t->text, t->textLen, next);

    
            } while(next < selection.startCursorPos+selection.len);

            // AddCharacters in Tab will delete everything;
            t->cursors[k].selection = selection;
        }


        t->cursors[k].pos = prev;
    }
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

    TextEditorCommand *ret = CreateCommand((const unsigned int *)c->keyBinding, c->keys, c->num, c->Execute, c->Undo);

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

static TextEditorCommand *CreateCommand(const unsigned int binding[], const char *keys, int n,
    void (*E)(TextEditor *, TextEditorCommand *), void (*U)(TextEditor *, TextEditorCommand *)){

    TextEditorCommand *res = (TextEditorCommand *)malloc(sizeof(TextEditorCommand));
    memset(res, 0, sizeof(TextEditorCommand));

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

    Graphics_init_pair(COLOR_SIDE_NUMBERS, COLOR_WHITE, COLOR_BLACK);
    Graphics_init_pair(COLOR1, COLOR_WHITE, COLOR_BLACK);
    Graphics_init_pair(COLOR2, COLOR_CYAN, COLOR_BLACK);
    Graphics_init_pair(COLOR6, COLOR_BLUE, COLOR_BLACK);
    Graphics_init_pair(COLOR3, COLOR_GREEN, COLOR_BLACK);
    Graphics_init_pair(COLOR5, COLOR_RED, COLOR_BLACK);
    Graphics_init_pair(COLOR4, COLOR_YELLOW, COLOR_BLACK);
    Graphics_init_pair(COLOR7, COLOR_MAGENTA, COLOR_BLACK);

    Graphics_init_pair(COLOR_SELECTED, COLOR_BLACK ,COLOR_YELLOW);
    Graphics_init_pair(COLOR_AUTO_COMPLETE, COLOR_BLACK, COLOR_WHITE);
    Graphics_init_pair(COLOR_CURSOR, COLOR_BLACK ,COLOR_MAGENTA);

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|EDIT_ARROW_UP  , 0}, "", -1, MoveLinesText, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_CTRL_KEY|EDIT_SHIFT_KEY|EDIT_ARROW_DOWN  , 0}, "", 1, MoveLinesText, NULL));
    // AddCommand(t, CreateCommand((unsigned int[]){'d'|EDIT_CTRL_KEY  , 0}, "", 0, SelectNextWord, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){'d'|EDIT_CTRL_KEY  , 0}, "", 0, SelectNextWord, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_SHIFT_KEY|EDIT_ARROW_UP|EDIT_CTRL_KEY  , 0}, "", -1, AddCursorCommand, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_SHIFT_KEY|EDIT_ARROW_DOWN|EDIT_CTRL_KEY  , 0}, "", 1, AddCursorCommand, NULL));

    // AddCommand(t, CreateCommand((unsigned int[]){unbound, 0}, "", -1, ExpandSelectionChars, NULL));
    // AddCommand(t, CreateCommand((unsigned int[]){unbound, 0}, "", 1, ExpandSelectionChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'l'|EDIT_SHIFT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, ExpandSelectionLines, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'k'|EDIT_SHIFT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, DeleteLine, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){'h'|EDIT_CTRL_KEY  , 0}, "", -1, MoveByChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'l'|EDIT_CTRL_KEY  , 0}, "", 1, MoveByChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'j'|EDIT_CTRL_KEY  , 0}, "", -1, MoveLines, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'k'|EDIT_CTRL_KEY  , 0}, "", 1, MoveLines, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){'h'|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", -1, MoveByWords, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'l'|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, MoveByWords, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){']'|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, IndentLine, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'['|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", -1, IndentLine, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_LEFT|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", -1, ExpandSelectionWords, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_RIGHT|EDIT_ALT_KEY|EDIT_CTRL_KEY  , 0}, "", 1, ExpandSelectionWords, NULL));
    

    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_LEFT  , 0}, "", -1, MoveByChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_RIGHT  , 0}, "", 1, MoveByChars, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_UP  , 0}, "", -1, MoveLines, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){EDIT_ARROW_DOWN  , 0}, "", 1, MoveLines, NULL));

    AddCommand(t, CreateCommand((unsigned int[]){'z'|EDIT_CTRL_KEY  , 0}, "", 1, Undo, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'r'|EDIT_CTRL_KEY  , 0}, "", 1, Redo, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'x'|EDIT_CTRL_KEY  , 0}, "", 1, Cut, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'c'|EDIT_CTRL_KEY  , 0}, "", 1, Copy, NULL));
    AddCommand(t, CreateCommand((unsigned int[]){'v'|EDIT_CTRL_KEY  , 0}, "", 1, Paste, UndoPaste));
    AddCommand(t, CreateCommand((unsigned int[]){31 /* ctrl + / */, 0}, "", 1, FindCommand, NULL));

    FILE *fp = fopen("text_editor.c", "r");

    fseek(fp, 0, SEEK_END);

    int len = ftell(fp);

    rewind(fp);

    t->text = (char *)malloc(len + 1);
    t->text[len] = 0;

    t->textLen = len;

    fread(t->text, sizeof(char), len, fp);

    fclose(fp);

    t->cursors = (TextEditorCursor *)malloc(sizeof(TextEditorCursor) * ++t->nCursors);
    memset(&t->cursors[0], 0, sizeof(TextEditorCursor));

}

// static void DrawSearchBar(TextEditor *t){
// }

void TextEditor_Draw(TextEditor *t){

    if(!t->text) return;

    int screenHeight = Graphics_TextCollumns();
    int screenWidth = Graphics_TextRows();
    t->textLen = strlen(t->text);

    int nLinesToLastCursor = 0;
    if(t->nCursors > 1)
        nLinesToLastCursor = GetNumLinesToPos(t->text,t->cursors[t->nCursors-1].pos);
    else
        nLinesToLastCursor = GetNumLinesToPos(t->text,t->cursors[0].pos);

    if(nLinesToLastCursor < t->scroll)
        t->scroll = nLinesToLastCursor;
    else if(nLinesToLastCursor >= t->scroll + (screenHeight-1)/2)
        t->scroll = nLinesToLastCursor - (screenHeight-1)/2;
    
    int ctOffset = 0;
    int x = 0, y = 0;
    // cant modulus x with width, \n might be farther out.

    int k = 0;
    int scrollPos = 0;
    int scrollPosMax = 0;

    if(t->scroll > 0){
        for(k = 0; k < t->textLen; k++){
            if(t->text[k] == '\n'){
                y++;
                if(y == t->scroll) { k++; break; }
            }
        }
    }
    scrollPos = k;
    ctOffset = k;
    for(; k < t->textLen; k++){
        if(t->text[k] == '\n'){
            y++;
            if(y == t->scroll+screenHeight) { k++; break; }
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
        // renderStart = c->pos - GetCharsIntoLine(t->text, c->pos);
        // renderTo = GetStartOfNextLine(t->text, t->textLen, c->pos);
        // if(renderTo < scrollPos) continue;
        for(k = scrollPos; k < renderStart; k++){
            if(t->text[k] == '\n'){
                y++;
            }
        }
        // if(renderStart < scrollPosMax) continue;
        ctOffset = k;
    
        for(; k < renderTo; ){

            // if(x > screenHeight){
                // for(; k < t->textLen; k++){
                //     if(t->text[k] == '\n'){
                //         y++;
                //         x = 0;
                //         ctOffset = ++k;
                //     }
                // }
            // }

            // printf("%i %i\n",y, screenHeight );

            int comment = 0;
            int string = 0;

            char c = t->text[k];

            char token = IsToken(c);
            if(!token){

                c = t->text[k];
                // number not like COLOR4 but 23
                if((k - ctOffset) == 1 && IsDigit(c)){

                    Graphics_attron(COLOR5);

                    for(k = k+1; k < renderTo && IsDigit(t->text[k]); k++);

                    Graphics_mvprintw(x, y, &t->text[ctOffset], k - ctOffset);
                    x += k - ctOffset;
                    ctOffset = k;

                    continue;
                }

                k++;
                // add to temp str

            } else {

                if(k - ctOffset > 0){

                    if((k - ctOffset) == 1 && IsDigit(t->text[k-1])){
                        Graphics_attron(COLOR5);
                        Graphics_mvprintw(x, y, &t->text[ctOffset], 1);
                        x++;
                        ctOffset = k;
                        goto addedStr;
                    }

                    for(int m = 0; m < (int)(sizeof(keywords)/sizeof(char *)); m++){
                        if(strlen(keywords[m]) == (k - ctOffset) && 
                            memcmp(&t->text[ctOffset], keywords[m], (k - ctOffset)) == 0) {
                            
                            Graphics_attron(COLOR2);
                            Graphics_mvprintw(x, y, &t->text[ctOffset], k - ctOffset);

                            x += k - ctOffset;
                            ctOffset = k;
                            goto addedStr;

                        }
                    }
                
                    if(c == '('){
                        // function def example(), k - ctOffset, ctOffset stops at beginning of non tokens, unless a digit or a keyword
                        // so that we can change colors at the token after the def

                        Graphics_attron(COLOR4);
                        Graphics_mvprintw(x, y, &t->text[ctOffset], k - ctOffset); 
                        x += k - ctOffset;
                        Graphics_attron(COLOR4);
                        Graphics_mvprintw(x, y, &t->text[k], 1);
                        x++;
                        k++;
                        ctOffset = k;
                        continue;
                    }
        
                    Graphics_attron(COLOR1);
                    Graphics_mvprintw(x, y, &t->text[ctOffset], k - ctOffset);
                    x += k - ctOffset;

                    ctOffset = k;
                }

                addedStr:

                if(c =='\n'){
            
                    y++;
                    x = 0;
                    ctOffset = ++k;

            //         //     Graphics_attron( COLOR_SIDE_NUMBERS);
            //         //     // for(j = 0; j < nDigits+1; j++)
                        //     Graphics_mvprintw( y, j, " ");
            //         //     // Graphics_attron( A_BOLD);
            //         //     sprintf(buffer, "%i", y+t->scroll);
                        // Graphics_mvprintw(x, y, 0, buffer);
            //         //     // Graphics_attroff(A_BOLD);
            //         //     Graphics_attron( currColor);
            //         // }

                    // x = nDigits+1;
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

                if(c == '/' && t->text[k+1] == '/') comment = 1;
                else if(c == '-' && t->text[k+1] == '-') comment = 1;
                else if(c == '/' && t->text[k+1] == '*') comment = 2;
                else if(c == '"') string = 1;
                else if(c == '\'') string = 2;
        
        
                if(c == ')' || c == '('){
                    Graphics_attron(COLOR4);
                    Graphics_mvprintw(x, y, &t->text[ctOffset], 1);
                    x++;
                    ctOffset = ++k;
                    continue;
                }
                
                if(c != ' ' && c != '(' && c != ')' && token && !comment && !string){
                    Graphics_attron(COLOR3);
                    Graphics_mvprintw(x, y, &t->text[ctOffset], 1);
                    x++;
                    ctOffset = ++k;
                    continue;
                }
                

                // if(!comment && !string) k++;

                if(comment){


                    k += 2;

                    if(comment == 1){
                        for (; k < renderTo; k++){
                            if(t->text[k] == '\n'){  break; }
                        }


                    } else { /* comment */


                        for(;k < (renderTo-1) && t->text[k] != '*' && t->text[k+1] != '/'; k++);

                        if(k < renderTo-1){
                            k++; // will run to end of file otherwise. 
                        }
                    }

                    Graphics_attron(COLOR6);
                    Graphics_mvprintw(x, y, &t->text[ctOffset], k - ctOffset);
                    x += k - ctOffset;
                    ctOffset = k;
                    comment = 0;
                    continue;
                }


                if(string){

                    ctOffset = k;
                    k++;

                    int escaped = 0;

                    while(k < renderTo){

                        k++;
                        if(k == renderTo) break;


                        if(t->text[k] == '\n') { k--; break; }
                        if(t->text[k] == '"' && string == 1 && !escaped) { k++; break; }
                        if(t->text[k] == '\'' && string == 2 && !escaped) { k++; break; }


                        if(t->text[k] == '\\' && !escaped)
                            escaped = 1;
                        else
                            escaped = 0;


                    }
                    
                    if(k == renderTo) break;

                    Graphics_attron(COLOR7);
                    Graphics_mvprintw(x, y, &t->text[ctOffset], k - ctOffset);
                    
                    x += k - ctOffset;
                    ctOffset = k;
                    string = 0;

                    continue;
                }

                x++;
                ctOffset = ++k;
            }
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

            x = 0; 
            y = 0;
            for(k = scrollPos; k < renderStart; k++){
                if(t->text[k] == '\n'){
                    y++;
                    x = 0;
                    continue;
                }
                x++;
            }

            Graphics_attron(COLOR_SELECTED);
            
            int renderStr = renderStart;
            for(; renderStart < renderTo; renderStart++){

                if(t->text[renderStart] == '\n'){
                    int len = (renderStart) - renderStr;
                    Graphics_mvprintw(x - len < 0 ? 0 : x - len, y, &t->text[renderStr], len);
                    renderStr = renderStart+1;
                    x = 0;
                    y++;
                    continue;
                } else if(t->text[renderStart] == '\t'){
                    int len = (renderStart) - renderStr;
                    Graphics_mvprintw(x - len < 0 ? 0 : x - len, y, &t->text[renderStr], len);
                    x+=4;
                    renderStr = renderStart+1;
                    continue;
                }

                x++;
            }

            if(renderStr < renderTo){
                if(x < screenWidth && y < screenHeight){
                    int len = renderStart - renderStr;
                    Graphics_mvprintw(x - len < 0 ? 0 : x - len, y, &t->text[renderStr], len);
                }
            }
        }
        // render cursor

        if(scrollPos > c->pos) continue;

        k = scrollPos;
        y = 0;
        x = 0;

        for(; k < c->pos && k < scrollPosMax; k++){
            if(t->text[k] == '\n'){
                x = 0;
                y++;
                continue;
            } else if(t->text[k] == '\t'){
                x+=4;
                continue;
            }
            x++;
        }

        Graphics_attron(COLOR_CURSOR);

        if((y >= screenHeight) ||
            x >= screenWidth) continue;

        if(t->text[c->pos] == '\n' || t->text[c->pos] == '\t')
            Graphics_mvprintw(x, y, " ", 1);
        else
            Graphics_mvprintw(x, y, &t->text[c->pos], 1);
    
    }

    int j;
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

            if(t->text[k] == '\n'){
                x = 0;
                y++;
                continue;
            } else if(t->text[k] == '\t'){
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
            memcpy(buffer, &t->text[t->autoComplete[j].offset],t->autoComplete[j].len);
            // Graphics_mvprintw(x, y+j+1, &t->text[t->autoComplete[j].offset], t->autoComplete[j].len);
            Graphics_mvprintw(x, y+j+1, buffer, MAX_AUTO_COMPLETE_STRLEN);
        }

    }
}



void TextEditor_Event(TextEditor *t, unsigned int key){

    if(key == 27){ // escape
        RemoveSelections(t);
        RemoveExtraCursors(t);
        return;
    }

    if(key == 10){ // enter
        EventEnter(t);
        return;
    }

    if(key == 9){ // tab
        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, "\t", 0, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);
        return;
    }

    if(key == 127){ // backspace

        if(t->searching){
            SearchingRemoveChars(t, 1);
            return;
        }

        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, 0, 1, RemoveCharacters, UndoRemoveCharacters);
        ExecuteCommand(t, command);
        FreeCommand(command);
        return;
    }

    if(key>>8){

        //ctrl/shift/alt or arrowkeys

        if(key == ((( unsigned int)'q') | EDIT_CTRL_KEY)){
            t->quit = 1;
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

    // if(key >= 32 && key <= 126){

        if(t->searching){
            SearchingAddText(t, (char[]){key, 0});
            return;
        }

        TextEditorCommand *command = CreateCommand((const unsigned int[]){0}, (const char[]){(char)key, 0}, 0, AddCharacters, UndoAddCharacters);
        ExecuteCommand(t,command);
        FreeCommand(command);
        // return;
    // }

}

void TextEditor_Destroy(TextEditor *t){

    if(t->text) free(t->text);

    int k;
    for(k = 0; k < t->nCommands; k++)
        FreeCommand(t->commands[k]);

    FreeCursors(t);
}
