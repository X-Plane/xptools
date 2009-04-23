/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "prefs.h"
#include <string.h>
#include <ac_plugin.h>

//extern "C" void ui_update_linked_var(char * addr);

#define DEFINE_PREF_STRING(n,d)	\
	PrefSpec *	spec_##n;	\
	char *		def_##n = STRING(d); \
	const char * get_##n(void) {return *((char **) spec_##n->addr); }

#define DEFINE_PREF_INT(n,d)	\
	PrefSpec *	spec_##n;	\
	int			def_##n = d; \
	int get_##n(void) {return *((int *) spec_##n->addr); } \
	void set_##n(int v){ *((int *) spec_##n->addr) = v; \
	ui_update_linked_var((char *) spec_##n->addr); }

#define DEFINE_PREF_DOUBLE(n,d)	\
	PrefSpec *	spec_##n;	\
	double			def_##n = d; \
	double get_##n(void) {return *((double *) spec_##n->addr); } \
	void set_##n(double v){ *((double *) spec_##n->addr) = v; \
	ui_update_linked_var((char *) spec_##n->addr); }


PREFS_LIST

#undef DEFINE_PREF_STRING
#undef DEFINE_PREF_INT
#undef DEFINE_PREF_DOUBLE

void	prefs_init(void)
{
	#define DEFINE_PREF_STRING(n,d)	spec_##n = ac_create_and_link_pref((char*)"xplane_" #n, PREF_STRING, &def_##n);
	#define DEFINE_PREF_INT(n,d)	spec_##n = ac_create_and_link_pref((char*)"xplane_" #n, PREF_INT, &def_##n);
	#define DEFINE_PREF_DOUBLE(n,d)	spec_##n = ac_create_and_link_pref((char*)"xplane_" #n, PREF_DOUBLE, &def_##n);

PREFS_LIST

#undef DEFINE_PREF_STRING
#undef DEFINE_PREF_INT
#undef DEFINE_PREF_DOUBLE
}
