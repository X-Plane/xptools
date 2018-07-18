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
	DEFINE_PREF_INT(enable_regions, 0) \
	DEFINE_PREF_INT(region_count, 1) \
	DEFINE_PREF_INT(region_l0, 0) \
	DEFINE_PREF_INT(region_b0, 0) \
	DEFINE_PREF_INT(region_r0, 1024) \
	DEFINE_PREF_INT(region_t0, 1024) \
	DEFINE_PREF_INT(region_l1, 0) \
	DEFINE_PREF_INT(region_b1, 0) \
	DEFINE_PREF_INT(region_r1, 1024) \
	DEFINE_PREF_INT(region_t1, 1024) \
	DEFINE_PREF_INT(region_l2, 0) \
	DEFINE_PREF_INT(region_b2, 0) \
	DEFINE_PREF_INT(region_r2, 1024) \
	DEFINE_PREF_INT(region_t2, 1024) \
	DEFINE_PREF_INT(region_l3, 0) \
	DEFINE_PREF_INT(region_b3, 0) \
	DEFINE_PREF_INT(region_r3, 1024) \
	DEFINE_PREF_INT(region_t3, 1024)

#define DEFINE_PREF_STRING(n,d)		const char *	get_##n(void);
#define DEFINE_PREF_INT(n,d)		int				get_##n(void);			void	set_##n(int v);
#define DEFINE_PREF_DOUBLE(n,d)		double			get_##n(void);			void	set_##n(double v);

PREFS_LIST

#undef DEFINE_PREF_STRING
#undef DEFINE_PREF_INT
#undef DEFINE_PREF_DOUBLE

void	prefs_init(void);



#endif