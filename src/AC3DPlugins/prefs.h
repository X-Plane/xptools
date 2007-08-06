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

#ifndef PREFS_H
#define PREFS_H

#define PREFS_LIST \
	DEFINE_PREF_STRING(default_layer_group, "none") \
	DEFINE_PREF_INT(default_layer_offset, 0) \
	DEFINE_PREF_INT(export_triangles, 1) \
	DEFINE_PREF_DOUBLE(default_LOD, 0.0f) \
	DEFINE_PREF_STRING(export_prefix, "") \
	DEFINE_PREF_STRING(texture_prefix, "") \

#define DEFINE_PREF_STRING(n,d)		const char *	get_##n(void);
#define DEFINE_PREF_INT(n,d)		int				get_##n(void);
#define DEFINE_PREF_DOUBLE(n,d)		double			get_##n(void);

PREFS_LIST

#undef DEFINE_PREF_STRING
#undef DEFINE_PREF_INT
#undef DEFINE_PREF_DOUBLE

void	prefs_init(void);



#endif