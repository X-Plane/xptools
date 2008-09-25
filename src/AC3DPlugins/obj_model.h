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

#ifndef OBJ_MODEL_H
#define OBJ_MODEL_H


#include "TclStubs.h"
#include <ac_plugin.h>

enum {
	anim_none = 0,
	anim_rotate = 1,
	anim_trans = 2,
	anim_static = 3,
	anim_show = 4,
	anim_hide = 5
};

void		OBJ_set_name(ACObject * obj, const char * name);
void		OBJ_set_hard(ACObject * obj, const char * surf);
void		OBJ_set_deck(ACObject * obj, int deck);
void		OBJ_set_blend(ACObject * obj, float cutoff);	// -1 for smooth
void		OBJ_set_poly_os(ACObject * obj, float offset);
void		OBJ_set_use_materials(ACObject * obj, int use);

void		OBJ_set_light_named(ACObject * obj, const char * name);
void		OBJ_set_light_red(ACObject * obj, float v);
void		OBJ_set_light_green(ACObject * obj, float v);
void		OBJ_set_light_blue(ACObject * obj, float v);
void		OBJ_set_light_alpha(ACObject * obj, float v);
void		OBJ_set_light_size(ACObject * obj, float v);
void		OBJ_set_light_s1(ACObject * obj, float v);
void		OBJ_set_light_t1(ACObject * obj, float v);
void		OBJ_set_light_s2(ACObject * obj, float v);
void		OBJ_set_light_t2(ACObject * obj, float v);
void		OBJ_set_light_smoke_size(ACObject * obj, float v);
void		OBJ_set_light_dataref(ACObject * obj, const char * dataref);

void		OBJ_set_LOD_near(ACObject * obj, float near_dis);
void		OBJ_set_LOD_far(ACObject * obj, float far_dis);

void		OBJ_set_layer_group(ACObject * obj, const char * layer);
void		OBJ_set_layer_group_offset(ACObject * obj, int offset);

void		OBJ_set_animation_group(ACObject * obj, int is_group);
void		OBJ_set_anim_type(ACObject * obj, int anim_type);
void		OBJ_set_anim_dataref(ACObject * obj, const char * dataref);
//void		OBJ_set_anim_low_value(ACObject * obj, float v1);
//void		OBJ_set_anim_high_value(ACObject * obj, float v2);
//void		OBJ_set_anim_low_angle(ACObject * obj, float a1);
//void		OBJ_set_anim_high_angle(ACObject * obj, float a2);

void		OBJ_set_anim_nth_value(ACObject * obj, int n, float v);
void		OBJ_set_anim_nth_angle(ACObject * obj, int n, float a);
void		OBJ_set_anim_keyframe_count(ACObject * obj, int n);
void		OBJ_set_anim_keyframe_root(ACObject * obj, int n);



// TODO: materials, show/hide animation, smoke puffs, custom lights, rgb lights

const char *	OBJ_get_name(ACObject * obj, char * buf);
const char *	OBJ_get_hard(ACObject * obj, char * buf);
int				OBJ_get_deck(ACObject * obj);
float			OBJ_get_blend(ACObject * obj);
float			OBJ_get_poly_os(ACObject * obj);
int				OBJ_get_use_materials(ACObject * obj);

const char *	OBJ_get_light_named(ACObject * obj, char * buf);
float			OBJ_get_light_red(ACObject * obj);
float			OBJ_get_light_green(ACObject * obj);
float			OBJ_get_light_blue(ACObject * obj);
float			OBJ_get_light_alpha(ACObject * obj);
float			OBJ_get_light_size(ACObject * obj);
float			OBJ_get_light_s1(ACObject * obj);
float			OBJ_get_light_t1(ACObject * obj);
float			OBJ_get_light_s2(ACObject * obj);
float			OBJ_get_light_t2(ACObject * obj);
float			OBJ_get_light_smoke_size(ACObject * obj);
const char *	OBJ_get_light_dataref(ACObject * obj, char * buf);

float			OBJ_get_LOD_near(ACObject * obj);
float			OBJ_get_LOD_far(ACObject * obj);

const char *	OBJ_get_layer_group(ACObject * obj, char * buf);
int				OBJ_get_layer_group_offset(ACObject * obj);

int				OBJ_get_animation_group(ACObject * obj);
int				OBJ_get_anim_type(ACObject * obj);
const char *	OBJ_get_anim_dataref(ACObject * obj, char * buf);
//float			OBJ_get_anim_low_value(ACObject * obj);
//float			OBJ_get_anim_high_value(ACObject * obj);
//float			OBJ_get_anim_low_angle(ACObject * obj);
//float			OBJ_get_anim_high_angle(ACObject * obj);

float		OBJ_get_anim_nth_value(ACObject * obj, int n);
float		OBJ_get_anim_nth_angle(ACObject * obj, int n);
int			OBJ_get_anim_keyframe_count(ACObject * obj);
int			OBJ_get_anim_keyframe_root(ACObject * obj);


typedef void (* OBJ_change_f)(ACObject * obj);
void		OBJ_register_change_cb(OBJ_change_f func);
void		OBJ_unregister_change_cb(OBJ_change_f func);

void		OBJ_register_datamodel_tcl_cmds(void);

#endif
