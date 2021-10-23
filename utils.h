#ifndef UTILS_DEF
#define UTILS_DEF

#include "types.h"

typedef struct {
	u32 width;
	u32 height;
	float invW;
	float invH;
	u8 *pixels;
	u32 texture;
} Image_t;

u32 Utils_LoadImage(Image_t *tex, const char *path);

#endif