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

#ifndef OBJ_ANIM_H
#define OBJ_ANIM_H

#include <string>
#include <vector>
#include "TclStubs.h"
#include <ac_plugin.h>
using std::string;
using std::vector;
#include "XObjDefs.h"

void anim_add_translate(
				ACObject *					obj,
				int							add_head,
				const vector<XObjKey>&		key_table,
				const char *				dataref,
				const char *				name);


void anim_add_rotate(
				ACObject *					obj,
				int							add_head,
				float						center[3],
				float						axis[3],
				const vector<XObjKey>&		key_table,
				const char *				dataref,
				const char *				name);

void anim_add_static(
				ACObject *					obj,
				int							add_head,
				float						offset[3],
				const char *				dataref,
				const char *				name);

void anim_add_show(
				ACObject *					obj,
				int							add_head,
				const vector<XObjKey>&		key_table,
				const char *				dataref,
				const char *				name);

void anim_add_hide(
				ACObject *					obj,
				int							add_head,
				const vector<XObjKey>&		key_table,
				const char *				dataref,
				const char *				name);


float *	axis_for_rotation			(ACObject * obj, float buf[3]);
float * center_for_rotation			(ACObject * obj, float buf[3]);
float * center_for_rotation_negative(ACObject * obj, float buf[3]);
float *	anim_trans_nth				(ACObject * obj, int n, float buf[3]);
float *	anim_trans_nth_relative		(ACObject * obj, int n, float buf[3]);

void	bake_static_transitions		(ACObject * object);
void	purge_datarefs				(void);
void	gather_datarefs				(ACObject * obj);
void	sync_datarefs				(void);
void	rescale_keyframes			(ACObject * obj, float old_lo, float new_lo, float old_hi, float new_hi);
int		get_keyframe_range			(ACObject * obj, float& lo, float& hi);

void	setup_obj_anim				(void);

#endif


