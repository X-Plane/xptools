#include "prefs.h"
#include <string.h>
#include <ac_plugin.h>

#define DEFINE_PREF_STRING(n,d)	\
	PrefSpec *	spec_##n;	\
	char *		def_##n = STRING(d); \
	const char * get_##n(void) {return *((char **) spec_##n->addr); }

#define DEFINE_PREF_INT(n,d)	\
	PrefSpec *	spec_##n;	\
	int			def_##n = d; \
	int get_##n(void) {return *((int *) spec_##n->addr); }

#define DEFINE_PREF_DOUBLE(n,d)	\
	PrefSpec *	spec_##n;	\
	double			def_##n = d; \
	double get_##n(void) {return *((double *) spec_##n->addr); }

PREFS_LIST

#undef DEFINE_PREF_STRING
#undef DEFINE_PREF_INT
#undef DEFINE_PREF_DOUBLE

void	prefs_init(void)
{
	#define DEFINE_PREF_STRING(n,d)	spec_##n = ac_create_and_link_pref("x-plane_" #n, PREF_STRING, &def_##n);
	#define DEFINE_PREF_INT(n,d)	spec_##n = ac_create_and_link_pref("x-plane_" #n, PREF_INT, &def_##n);
	#define DEFINE_PREF_DOUBLE(n,d)	spec_##n = ac_create_and_link_pref("x-plane_" #n, PREF_DOUBLE, &def_##n);

PREFS_LIST

#undef DEFINE_PREF_STRING
#undef DEFINE_PREF_INT
#undef DEFINE_PREF_DOUBLE
}
