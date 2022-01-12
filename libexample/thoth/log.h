#ifndef LOG_DEF
#define LOG_DEF

enum {
	LOG_NORMAL = 0,
	LOG_RED = 31,
	LOG_GREEN = 32,
	LOG_YELLOW = 33,
	LOG_BLUE = 34,
	LOG_MAGENTA = 35,
	LOG_CYAN = 36,
};

void Log_Formatted(int color, const char *file, int line, const char *format, ...);

#define LOG(color, ...) Log_Formatted(color, __FILE__, __LINE__, __VA_ARGS__);

#endif