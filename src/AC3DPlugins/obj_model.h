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

enum {
	manip_none = 0,
	manip_panel = 1,
	manip_axis = 2,
	manip_axis_2d = 3,
	manip_command = 4,
	manip_command_axis = 5,
	manip_noop = 6
};

void		OBJ_set_name(ACObject * obj, const char * name);
void		OBJ_set_hard(ACObject * obj, const char * surf);
void		OBJ_set_deck(ACObject * obj, int deck);
void		OBJ_set_blend(ACObject * obj, float cutoff);	// -1 for smooth
void		OBJ_set_poly_os(ACObject * obj, float offset);
void		OBJ_set_use_materials(ACObject * obj, int use);
void		OBJ_set_lit_dataref(ACObject * obj, const char * dataref);
void		OBJ_set_lit_v1(ACObject * obj, float v1);
void		OBJ_set_lit_v2(ACObject * obj, float v2);
void		OBJ_set_draw_disable(ACObject * obj, int dis);
void		OBJ_set_wall(ACObject * obj, int wall);
void		OBJ_set_mod_lit(ACObject * obj, int mod_lit);


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

void		OBJ_set_manip_type(ACObject * obj, int n);
void		OBJ_set_manip_dx(ACObject * obj, float dx);
void		OBJ_set_manip_dy(ACObject * obj, float dy);
void		OBJ_set_manip_dz(ACObject * obj, float dz);
void		OBJ_set_manip_v1_min(ACObject * obj, float v);
void		OBJ_set_manip_v1_max(ACObject * obj, float v);
void		OBJ_set_manip_v2_min(ACObject * obj, float v);
void		OBJ_set_manip_v2_max(ACObject * obj, float v);
void		OBJ_set_manip_dref1(ACObject * obj, const char * dref);
void		OBJ_set_manip_dref2(ACObject * obj, const char * dref);
void		OBJ_set_manip_tooltip(ACObject * obj, const char * dref);
void		OBJ_set_manip_cursor(ACObject * obj, const char * cursor);

void		OBJ_set_has_panel_regions(ACObject * obj, int e);
void		OBJ_set_num_panel_regions(ACObject * obj, int k);
void		OBJ_set_panel_left(ACObject * obj, int n, int l);
void		OBJ_set_panel_bottom(ACObject * obj, int n, int b);
void		OBJ_set_panel_right(ACObject * obj, int n, int r);
void		OBJ_set_panel_top(ACObject * obj, int n, int t);


// TODO: materials, show/hide animation, smoke puffs, custom lights, rgb lights

const char *	OBJ_get_name(ACObject * obj, char * buf);
const char *	OBJ_get_hard(ACObject * obj, char * buf);
int				OBJ_get_deck(ACObject * obj);
float			OBJ_get_blend(ACObject * obj);
float			OBJ_get_poly_os(ACObject * obj);
int				OBJ_get_use_materials(ACObject * obj);
const char *	OBJ_get_lit_dataref(ACObject * obj, char * buf);
float			OBJ_get_lit_v1(ACObject * obj);
float			OBJ_get_lit_v2(ACObject * obj);
int				OBJ_get_draw_disable(ACObject * obj);
int				OBJ_get_wall(ACObject * obj);
int				OBJ_get_mod_lit(ACObject * obj);


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

float			OBJ_get_anim_nth_value(ACObject * obj, int n);
float			OBJ_get_anim_nth_angle(ACObject * obj, int n);
int				OBJ_get_anim_keyframe_count(ACObject * obj);
int				OBJ_get_anim_keyframe_root(ACObject * obj);

int				OBJ_get_manip_type(ACObject * obj);
float			OBJ_get_manip_dx(ACObject * obj);
float			OBJ_get_manip_dy(ACObject * obj);
float			OBJ_get_manip_dz(ACObject * obj);
float			OBJ_get_manip_v1_min(ACObject * obj);
float			OBJ_get_manip_v1_max(ACObject * obj);
float			OBJ_get_manip_v2_min(ACObject * obj);
float			OBJ_get_manip_v2_max(ACObject * obj);
const char *	OBJ_get_manip_dref1(ACObject * obj, char * buf);
const char *	OBJ_get_manip_dref2(ACObject * obj, char * buf);
const char *	OBJ_get_manip_tooltip(ACObject * obj, char * buf);
const char *	OBJ_get_manip_cursor(ACObject * obj, char * buf);

int				OBJ_get_has_panel_regions(ACObject * obj);
int				OBJ_get_num_panel_regions(ACObject * obj);
int				OBJ_get_panel_left(ACObject * obj, int n);
int				OBJ_get_panel_bottom(ACObject * obj, int n);
int				OBJ_get_panel_right(ACObject * obj, int n);
int				OBJ_get_panel_top(ACObject * obj, int n);

typedef void (* OBJ_change_f)(ACObject * obj);
void		OBJ_register_change_cb(OBJ_change_f func);
void		OBJ_unregister_change_cb(OBJ_change_f func);

void		OBJ_register_datamodel_tcl_cmds(void);

#endif
