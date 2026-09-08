#ifdef ENABLE_DEBUG

#include <stdio.h>

extern char debug_depth;

#define DEBUG_BEGIN_FN() \
	do { debug_depth++; } while (0)

#define DEBUG_END_FN() \
	do { debug_depth--; } while (0)

#define DEBUG_PRINT(...) \
	do { \
		for (char _dbg_i = debug_depth; _dbg_i > 0; _dbg_i--) \
			printf("\t"); \
		printf(__VA_ARGS__); \
	} while (0)

#else //DISABLE_DEBUG

#define DEBUG_BEGIN_FN() \
	do { } while (0)

#define DEBUG_END_FN() \
	do { } while (0)

#  define DEBUG_PRINT(...) \
	do { } while (0)

#endif
