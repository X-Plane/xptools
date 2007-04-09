#if APL
	#if defined(__POWERPC__)
		#define LIL 0
		#define BIG 1
	#else
		#define LIL 1
		#define BIG 0
	#endif
#else
	#define LIL 1
	#define BIG 0
#endif

#include "xdefs.h"