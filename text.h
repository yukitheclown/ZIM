#ifndef TEXT_DEF
#define TEXT_DEF


void Text_Init(void);
void Text_Draw(int x, int y, int hSpacing, int vSpacing, int maxWidth, const char *text);
void Text_Close(void);

#endif