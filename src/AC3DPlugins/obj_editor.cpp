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

#include "obj_editor.h"
#include "ac_utils.h"
#include "tcl_utils.h"
#include "obj_model.h"
#include <math.h>
#include "XDefs.h"
#if IBM
	// fix this if there is a 64bit version of ac3d somewhen
	#define	intptr_t	int
#endif
#ifdef LIN
    #include <stdint.h>
#endif


const int NUM_KEYFRAMES = 50;
const int MAX_MULTI_COUNT = 5;

/****************************************************************************************************************/

static const char * remap_str(const char * v, const char * old, const char * rep);
const char * remap_str(const char * v, const char * old, const char * rep)
{
	if (strcmp(v,old)==0)	return rep;
							return   v;
}

inline float fsign(float x) { if (x > 0.0) return 1.0; if (x < 0.0) return -1.0; return 0.0; }

/****************************************************************************************************************/

static ACObject * editor_recursion = NULL;

TCL_linked_vari * multi_edit = NULL;

ACObject * get_sel_single_obj(int n)
{
	vector<ACObject *>		objs;
	ACObject *				obj;
	find_all_selected_objects_flat(objs);
	if (objs.size() <= n) return NULL;
	obj = objs[n];
	if (ac_entity_is_class(obj, (char*)AC_CLASS_LIGHT)) return NULL;
	if (ac_entity_is_class(obj, (char*)AC_CLASS_GROUP)) return NULL;
	return obj;
}

static ACObject * get_sel_single_grp(int n)
{
	vector<ACObject *>		objs;
	ACObject *				obj;
	find_all_selected_objects_flat(objs);
	if (objs.size() <= n) return NULL;
	obj = objs[n];
	if (ac_entity_is_class(obj, (char*)AC_CLASS_GROUP)) return obj;
	return NULL;
}


static ACObject * get_sel_single_light(int n)
{
	vector<ACObject *>		objs;
	ACObject *				obj;
	find_all_selected_objects_flat(objs);
	if (objs.size() <= n) return NULL;
	obj = objs[n];
	if (ac_entity_is_class(obj, (char*)AC_CLASS_LIGHT)) return obj;
												 return NULL;
}

/****************************************************************************************************************/

#define	SIMPLE_PROPERTY_MAPPINGS_ALL	\
	SIMPLE_PROPERTY_STR(obj_name,get_sel_single_obj,OBJ_set_name,OBJ_get_name,"","")

#define	SIMPLE_PROPERTY_MAPPINGS_OBJ	\
	SIMPLE_PROPERTY_STR(hard_surf,get_sel_single_obj,OBJ_set_hard,OBJ_get_hard,"","none") \
	SIMPLE_PROPERTY_INT(is_deck,get_sel_single_obj,OBJ_set_deck,OBJ_get_deck) \
	SIMPLE_PROPERTY_FLT(poly_os,get_sel_single_obj,OBJ_set_poly_os,OBJ_get_poly_os) \
	SIMPLE_PROPERTY_STR(anim_dataref,get_sel_single_obj,OBJ_set_anim_dataref,OBJ_get_anim_dataref,"","") \
	SIMPLE_PROPERTY_FLT(anim_loop,get_sel_single_obj,OBJ_set_anim_loop, OBJ_get_anim_loop) \
	SIMPLE_PROPERTY_INT(anim_keyframe_count, get_sel_single_obj,OBJ_set_anim_keyframe_count,OBJ_get_anim_keyframe_count) \
	SIMPLE_PROPERTY_INT(use_materials,get_sel_single_obj,OBJ_set_use_materials,OBJ_get_use_materials) \
	SIMPLE_PROPERTY_STR(lit_dataref,get_sel_single_obj,OBJ_set_lit_dataref, OBJ_get_lit_dataref,"","") \
	SIMPLE_PROPERTY_FLT(lit_v1,get_sel_single_obj,OBJ_set_lit_v1,OBJ_get_lit_v1) \
	SIMPLE_PROPERTY_FLT(lit_v2,get_sel_single_obj,OBJ_set_lit_v2,OBJ_get_lit_v2) \
	SIMPLE_PROPERTY_INT(mod_lit,get_sel_single_obj,OBJ_set_mod_lit,OBJ_get_mod_lit) \
	SIMPLE_PROPERTY_INT(wall,get_sel_single_obj,OBJ_set_wall,OBJ_get_wall) \
	SIMPLE_PROPERTY_INT(draw_disable,get_sel_single_obj,OBJ_set_draw_disable,OBJ_get_draw_disable) \
	SIMPLE_PROPERTY_INT(anim_keyframe_root,get_sel_single_obj,OBJ_set_anim_keyframe_root,OBJ_get_anim_keyframe_root) \
	SIMPLE_PROPERTY_INT(manip_type,get_sel_single_obj,OBJ_set_manip_type,OBJ_get_manip_type) \
	SIMPLE_PROPERTY_FLT(manip_dx,get_sel_single_obj,OBJ_set_manip_dx,OBJ_get_manip_dx) \
	SIMPLE_PROPERTY_FLT(manip_dy,get_sel_single_obj,OBJ_set_manip_dy,OBJ_get_manip_dy) \
	SIMPLE_PROPERTY_FLT(manip_dz,get_sel_single_obj,OBJ_set_manip_dz,OBJ_get_manip_dz) \
	SIMPLE_PROPERTY_FLT(manip_centroid_x,get_sel_single_obj,OBJ_set_manip_centroid_x,OBJ_get_manip_centroid_x) \
	SIMPLE_PROPERTY_FLT(manip_centroid_y,get_sel_single_obj,OBJ_set_manip_centroid_y,OBJ_get_manip_centroid_y) \
	SIMPLE_PROPERTY_FLT(manip_centroid_z,get_sel_single_obj,OBJ_set_manip_centroid_z,OBJ_get_manip_centroid_z) \
	SIMPLE_PROPERTY_FLT(manip_v1_min,get_sel_single_obj,OBJ_set_manip_v1_min,OBJ_get_manip_v1_min) \
	SIMPLE_PROPERTY_FLT(manip_v1_max,get_sel_single_obj,OBJ_set_manip_v1_max,OBJ_get_manip_v1_max) \
	SIMPLE_PROPERTY_FLT(manip_v2_min,get_sel_single_obj,OBJ_set_manip_v2_min,OBJ_get_manip_v2_min) \
	SIMPLE_PROPERTY_FLT(manip_v2_max,get_sel_single_obj,OBJ_set_manip_v2_max,OBJ_get_manip_v2_max) \
	SIMPLE_PROPERTY_FLT(manip_angle_min,get_sel_single_obj,OBJ_set_manip_angle_min,OBJ_get_manip_angle_min) \
	SIMPLE_PROPERTY_FLT(manip_angle_max,get_sel_single_obj,OBJ_set_manip_angle_max,OBJ_get_manip_angle_max) \
	SIMPLE_PROPERTY_FLT(manip_lift,get_sel_single_obj,OBJ_set_manip_lift,OBJ_get_manip_lift) \
	SIMPLE_PROPERTY_STR(manip_dref1,get_sel_single_obj,OBJ_set_manip_dref1,OBJ_get_manip_dref1,"", "") \
	SIMPLE_PROPERTY_STR(manip_dref2,get_sel_single_obj,OBJ_set_manip_dref2,OBJ_get_manip_dref2,"", "") \
	SIMPLE_PROPERTY_STR(manip_cmnd1,get_sel_single_obj,OBJ_set_manip_dref1,OBJ_get_manip_dref1,"", "") \
	SIMPLE_PROPERTY_STR(manip_cmnd2,get_sel_single_obj,OBJ_set_manip_dref2,OBJ_get_manip_dref2,"", "") \
	SIMPLE_PROPERTY_STR(manip_tooltip,get_sel_single_obj,OBJ_set_manip_tooltip,OBJ_get_manip_tooltip,"", "") \
	SIMPLE_PROPERTY_STR(manip_cursor,get_sel_single_obj,OBJ_set_manip_cursor,OBJ_get_manip_cursor,"", "") \
	SIMPLE_PROPERTY_FLT(manip_wheel,get_sel_single_obj,OBJ_set_manip_wheel,OBJ_get_manip_wheel)
//	SIMPLE_PROPERTY_FLT(anim_low_value,get_sel_single_obj,OBJ_set_anim_low_value,OBJ_get_anim_low_value) \
//	SIMPLE_PROPERTY_FLT(anim_low_angle,get_sel_single_obj,OBJ_set_anim_low_angle,OBJ_get_anim_low_angle) \
//	SIMPLE_PROPERTY_FLT(anim_high_value,get_sel_single_obj,OBJ_set_anim_high_value,OBJ_get_anim_high_value) \
//	SIMPLE_PROPERTY_FLT(anim_high_angle,get_sel_single_obj,OBJ_set_anim_high_angle,OBJ_get_anim_high_angle) \

#define SIMPLE_PROPERTY_MAPPINGS_GRP \
	SIMPLE_PROPERTY_FLT(lod_near,get_sel_single_grp,OBJ_set_LOD_near,OBJ_get_LOD_near) \
	SIMPLE_PROPERTY_FLT(lod_far,get_sel_single_grp,OBJ_set_LOD_far,OBJ_get_LOD_far) \
	SIMPLE_PROPERTY_STR(layer_group,get_sel_single_grp,OBJ_set_layer_group,OBJ_get_layer_group, "", "none") \
	SIMPLE_PROPERTY_INT(layer_group_offset,get_sel_single_grp,OBJ_set_layer_group_offset,OBJ_get_layer_group_offset)

#define	SIMPLE_PROPERTY_MAPPINGS_LGT	\
	SIMPLE_PROPERTY_STR(light_type,get_sel_single_light,OBJ_set_light_named, OBJ_get_light_named,"","none") \
	SIMPLE_PROPERTY_STR(light_dataref,get_sel_single_light,OBJ_set_light_dataref, OBJ_get_light_dataref,"","") \
	SIMPLE_PROPERTY_FLT(light_red,get_sel_single_light,OBJ_set_light_red,OBJ_get_light_red) \
	SIMPLE_PROPERTY_FLT(light_green,get_sel_single_light,OBJ_set_light_green,OBJ_get_light_green) \
	SIMPLE_PROPERTY_FLT(light_blue,get_sel_single_light,OBJ_set_light_blue,OBJ_get_light_blue) \
	SIMPLE_PROPERTY_FLT(light_alpha,get_sel_single_light,OBJ_set_light_alpha,OBJ_get_light_alpha) \
	SIMPLE_PROPERTY_FLT(light_size,get_sel_single_light,OBJ_set_light_size,OBJ_get_light_size) \
	SIMPLE_PROPERTY_FLT(light_s1,get_sel_single_light,OBJ_set_light_s1,OBJ_get_light_s1) \
	SIMPLE_PROPERTY_FLT(light_s2,get_sel_single_light,OBJ_set_light_s2,OBJ_get_light_s2) \
	SIMPLE_PROPERTY_FLT(light_t1,get_sel_single_light,OBJ_set_light_t1,OBJ_get_light_t1) \
	SIMPLE_PROPERTY_FLT(light_t2,get_sel_single_light,OBJ_set_light_t2,OBJ_get_light_t2) \
	SIMPLE_PROPERTY_FLT(light_smoke_size,get_sel_single_light,OBJ_set_light_smoke_size,OBJ_get_light_smoke_size) \
	SIMPLE_PROPERTY_STR(light_p1,get_sel_single_light,OBJ_set_light_p1,OBJ_get_light_p1,"","") \
	SIMPLE_PROPERTY_STR(light_p2,get_sel_single_light,OBJ_set_light_p2,OBJ_get_light_p2,"","") \
	SIMPLE_PROPERTY_STR(light_p3,get_sel_single_light,OBJ_set_light_p3,OBJ_get_light_p3,"","") \
	SIMPLE_PROPERTY_STR(light_p4,get_sel_single_light,OBJ_set_light_p4,OBJ_get_light_p4,"","") \
	SIMPLE_PROPERTY_STR(light_p5,get_sel_single_light,OBJ_set_light_p5,OBJ_get_light_p5,"","") \
	SIMPLE_PROPERTY_STR(light_p6,get_sel_single_light,OBJ_set_light_p6,OBJ_get_light_p6,"","") \
	SIMPLE_PROPERTY_STR(light_p7,get_sel_single_light,OBJ_set_light_p7,OBJ_get_light_p7,"","") \
	SIMPLE_PROPERTY_STR(light_p8,get_sel_single_light,OBJ_set_light_p8,OBJ_get_light_p8,"","") \
	SIMPLE_PROPERTY_STR(light_p9,get_sel_single_light,OBJ_set_light_p9,OBJ_get_light_p9,"","")


#define APPLY_SET_ONE_OR_MANY(_GET_ONE, _APPLY_FUNC) \
	ACObject * obj; \
	if(multi_edit->get()) { \
	vector<ACObject *> objs; \
	find_all_selected_objects_flat(objs); \
	for(vector<ACObject *>::iterator ob = objs.begin(); ob != objs.end(); ++ob) { \
		editor_recursion = _GET_ONE; obj = *ob; _APPLY_FUNC; editor_recursion = NULL; } \
	} else { \
		obj = _GET_ONE; if(obj) { editor_recursion = obj; _APPLY_FUNC; editor_recursion = NULL; } }

#define	SIMPLE_PROPERTY_STR(prop_name,obj_func,set_func,get_func,native_code,ui_code)	\
	static TCL_linked_varsv * prop_name##_var = NULL; \
	static void xplane_##prop_name##_cb(const char * value, int idx, void * ref, TCL_linked_varsv * who) { \
		APPLY_SET_ONE_OR_MANY(obj_func(idx),set_func(obj,remap_str(value, ui_code, native_code))) }

#define	SIMPLE_PROPERTY_INT(prop_name,obj_func,set_func,get_func)	\
	static TCL_linked_variv * prop_name##_var = NULL; \
	static void xplane_##prop_name##_cb(int value, int idx, void * ref, TCL_linked_variv * who) { \
		APPLY_SET_ONE_OR_MANY(obj_func(idx),set_func(obj,value))  }

#define	SIMPLE_PROPERTY_FLT(prop_name,obj_func,set_func,get_func)	\
	static TCL_linked_vardv * prop_name##_var = NULL; \
	static void xplane_##prop_name##_cb(double value, int idx, void * ref, TCL_linked_vardv * who) { \
		 APPLY_SET_ONE_OR_MANY(obj_func(idx),set_func(obj,value))  }

SIMPLE_PROPERTY_MAPPINGS_ALL
SIMPLE_PROPERTY_MAPPINGS_OBJ
SIMPLE_PROPERTY_MAPPINGS_LGT
SIMPLE_PROPERTY_MAPPINGS_GRP

#undef SIMPLE_PROPERTY_STR
#undef SIMPLE_PROPERTY_INT
#undef SIMPLE_PROPERTY_FLT

static const char * k_anim_names[6] = { "no animation", "rotate", "translate", "static", "show", "hide" };

static void	xplane_blend_enable_cb(int value, int idx, void * ref, TCL_linked_variv * who)
{
	APPLY_SET_ONE_OR_MANY(get_sel_single_obj(idx),OBJ_set_blend(obj, fabs(OBJ_get_blend(obj)) * (value ? -1.0 : 1.0)))
}

static void	xplane_blend_level_cb(double value, int idx, void * ref, TCL_linked_vardv * who)
{
	APPLY_SET_ONE_OR_MANY(get_sel_single_obj(idx),OBJ_set_blend(obj, value))
}

static void xplane_anim_type_cb(const char * value, int idx, void * ref, TCL_linked_varsv * who)
{
	ACObject * obj;
	if(multi_edit->get())
	{
		vector<ACObject *> objs;
		find_all_selected_objects_flat(objs);
		for(vector<ACObject *>::iterator ob = objs.begin(); ob != objs.end(); ++ob)
		{
			obj = *ob;
			editor_recursion = get_sel_single_obj(idx);
			int found = 0;
			for (int n = 0; n < 6; ++n)
			if (strcmp(value,k_anim_names[n])==0)
			{
				OBJ_set_anim_type(obj, n);
				found = 1;
			}
			if (!found)
				OBJ_set_anim_type(obj, 0);
			editor_recursion = NULL;
		}
	}
	else
	{
		obj = get_sel_single_obj(idx);
		if(obj)
		{
			editor_recursion = obj;
			int found = 0;
			for (int n = 0; n < 6; ++n)
			if (strcmp(value,k_anim_names[n])==0)
			{
				OBJ_set_anim_type(obj, n);
				found = 1;
			}
			if (!found)
				OBJ_set_anim_type(obj, 0);
			editor_recursion = NULL;
		}
	}
}

static void xplane_anim_value_cb(double value, int idx, void * ref, TCL_linked_vardv * who)
{
	APPLY_SET_ONE_OR_MANY(get_sel_single_obj(idx),OBJ_set_anim_nth_value(obj, (uintptr_t) ref, value))
}

static void xplane_anim_angle_cb(double value, int idx, void * ref, TCL_linked_vardv * who)
{
	APPLY_SET_ONE_OR_MANY(get_sel_single_obj(idx),OBJ_set_anim_nth_angle(obj, (uintptr_t) ref, value))
}

TCL_linked_variv * blend_enable_var = NULL;
TCL_linked_vardv * blend_level_var = NULL;
TCL_linked_varsv * anim_type_var = NULL;
TCL_linked_vardv * anim_value_vars[NUM_KEYFRAMES] = { 0 };
TCL_linked_vardv * anim_angle_vars[NUM_KEYFRAMES] = { 0 };


/****************************************************************************************************************/

enum {
	sel_none = 0,
	sel_light = 1,
	sel_obj = 2,
	sel_group = 3,
	sel_multi = 4,
	sel_unknown = 5
};

static int OBJ_get_single_type(ACObject * who)
{
		 if (ac_entity_is_class(who, (char*)AC_CLASS_LIGHT))			return (sel_light);
	else if (ac_entity_is_class(who, (char*)AC_CLASS_GROUP))			return (sel_group);
	else														return (sel_obj);
}

static int OBJ_get_sel_type(int n, ACObject ** out_obj)
{
	vector<ACObject *>		objs;
	ACObject *				ob;
	find_all_selected_objects_flat(objs);

	if(objs.empty() || n < 0 || n >= objs.size())
		return (sel_none);
	else if (objs.size() > MAX_MULTI_COUNT)
	{
		int tp = OBJ_get_single_type(objs[0]);
		for (int n = 1; n < objs.size(); ++n)
		{
			if (OBJ_get_single_type(objs[n])!=tp)
				return sel_multi;
		}
		if (out_obj) *out_obj = objs[0];
		return tp;
	}
	else
	{
		ob = objs[n];
		if (out_obj) *out_obj = ob;
		return OBJ_get_single_type(ob);
	}
}

static void OBJ_get_sel_count_tcl(void)
{
// use this to debug
//	command_result_append_int(MAX_MULTI_COUNT);
//	return;
	vector<ACObject *>		objs;
	find_all_selected_objects_flat(objs);
	if(objs.empty())						command_result_append_int(1);
	else if(objs.size() <= MAX_MULTI_COUNT) command_result_append_int(objs.size());
	else									command_result_append_int(1);
}


static void OBJ_get_sel_type_tcl(float n)
{
	command_result_append_int(OBJ_get_sel_type(n, NULL));
}

static void OBJ_can_animate_tcl(float n)
{
	int idx = n;
	int result = 0;
	ACObject * ob;
	if (OBJ_get_sel_type(n, &ob)==sel_obj)
	if (OBJ_get_animation_group(object_parent(ob)))
	{
		if (ac_object_get_num_vertices(ob)>=2 &&
			ac_object_get_num_surfaces(ob)==1 &&
			ac_object_get_num_children(ob)==0)
		{
			List * sl = ac_object_get_surfacelist(ob);
			if (sl && sl->data)
			{
				if (surface_get_type((Surface*)sl->data)==SURFACE_LINE||surface_get_type((Surface*)sl->data)==SURFACE_CLOSEDLINE)
					result = 1;
			}
		}
	}
	command_result_append_int(result);
}

static void OBJ_editor_sync(ACObject * changed)
{
	ACObject * obj;

	vector<ACObject *>		objs;
	find_all_selected_objects_flat(objs);

	int total = (objs.size() <= MAX_MULTI_COUNT) ? objs.size() : 1;

	for (int idx = 0; idx < total; ++idx)
	{
		int seltype = OBJ_get_sel_type(idx, &obj);
		if (obj != editor_recursion)
		{
		//	printf("Sync - changed = %d, obj = %d, sel_type = %d\n", changed, obj, seltype);

			char buf[1024];

			#define	SIMPLE_PROPERTY_STR(prop_name,obj_func,set_func,get_func,native_code,ui_code)	\
				prop_name##_var->set(idx, remap_str(get_func(obj,buf),native_code,ui_code));
			#define	SIMPLE_PROPERTY_INT(prop_name,obj_func,set_func,get_func)	\
				prop_name##_var->set(idx, get_func(obj));
			#define	SIMPLE_PROPERTY_FLT(prop_name,obj_func,set_func,get_func)	\
				prop_name##_var->set(idx, get_func(obj));

			if (seltype == sel_obj && (changed == obj || changed == NULL))
			{
				if (obj == NULL) message_dialog((char*)"internal err - null obj!\n");
				SIMPLE_PROPERTY_MAPPINGS_ALL
				SIMPLE_PROPERTY_MAPPINGS_OBJ
				blend_enable_var->set(idx, OBJ_get_blend(obj) <= 0.0);
				blend_level_var->set(idx, fabs(OBJ_get_blend(obj)));
				anim_type_var->set(idx, k_anim_names[OBJ_get_anim_type(obj)]);
				for(int n = 0; n < OBJ_get_anim_keyframe_count(obj); ++n)
				{
					anim_value_vars[n]->set(idx, OBJ_get_anim_nth_value(obj,n));
					anim_angle_vars[n]->set(idx, OBJ_get_anim_nth_angle(obj,n));
				}
			}
			if (seltype == sel_light && (changed == obj || changed == NULL))
			{
				if (obj == NULL) message_dialog((char*)"internal err - null obj!\n");
				SIMPLE_PROPERTY_MAPPINGS_ALL
				SIMPLE_PROPERTY_MAPPINGS_LGT
			}
			if (seltype == sel_group && (changed == obj || changed == NULL))
			{
				if (obj == NULL) message_dialog((char*)"internal err - null obj!\n");
				SIMPLE_PROPERTY_MAPPINGS_ALL
				SIMPLE_PROPERTY_MAPPINGS_GRP
			}

			#undef SIMPLE_PROPERTY_STR
			#undef SIMPLE_PROPERTY_INT
			#undef SIMPLE_PROPERTY_FLT
		}
	}
}

static void OBJ_editor_sync_tcl(void)
{
	OBJ_editor_sync(NULL);
}

/****************************************************************************************************************/

void	OBJ_editor_init(void)
{
	ac_add_command_full((char*)"xplane_can_animate", CAST_CMD(OBJ_can_animate_tcl), 1, (char*)"f", (char*)"ac3d xplane_can_animate", (char*)"can this object be an animation action?");
	ac_add_command_full((char*)"xplane_get_sel_type", CAST_CMD(OBJ_get_sel_type_tcl), 1, (char*)"f", (char*)"ac3d xplane_get_sel_type", (char*)"get the selection type.");
	ac_add_command_full((char*)"xplane_editor_sync", CAST_CMD(OBJ_editor_sync_tcl), 0, NULL, (char*)"ac3d xplane_editor_sync", (char*)"resync the editor.");
	ac_add_command_full((char*)"xplane_get_sel_count", CAST_CMD(OBJ_get_sel_count_tcl), 0, NULL, (char*)"ac3d xplane_get_sel_count", (char*)"get number of selected items to edit.");

	#define	SIMPLE_PROPERTY_STR(prop_name,obj_func,set_func,get_func,native_code,ui_code)	\
		prop_name##_var = new TCL_linked_varsv(ac_get_tcl_interp(),"xplane_" #prop_name, MAX_MULTI_COUNT, xplane_##prop_name##_cb, NULL, "none");
	#define	SIMPLE_PROPERTY_INT(prop_name,obj_func,set_func,get_func)	\
		prop_name##_var = new TCL_linked_variv(ac_get_tcl_interp(),"xplane_" #prop_name, MAX_MULTI_COUNT, xplane_##prop_name##_cb, NULL, 0);
	#define	SIMPLE_PROPERTY_FLT(prop_name,obj_func,set_func,get_func)	\
		prop_name##_var = new TCL_linked_vardv(ac_get_tcl_interp(),"xplane_" #prop_name, MAX_MULTI_COUNT, xplane_##prop_name##_cb, NULL, 0.0);

	SIMPLE_PROPERTY_MAPPINGS_ALL
	SIMPLE_PROPERTY_MAPPINGS_OBJ
	SIMPLE_PROPERTY_MAPPINGS_LGT
	SIMPLE_PROPERTY_MAPPINGS_GRP

	blend_enable_var = new TCL_linked_variv(ac_get_tcl_interp(), "xplane_blend_enable", MAX_MULTI_COUNT, xplane_blend_enable_cb, NULL, 0);
	blend_level_var = new TCL_linked_vardv(ac_get_tcl_interp(), "xplane_blend_level", MAX_MULTI_COUNT, xplane_blend_level_cb, NULL, 0.0);
	anim_type_var = new TCL_linked_varsv(ac_get_tcl_interp(),"xplane_anim_type",MAX_MULTI_COUNT, xplane_anim_type_cb,NULL,"no animation");

	for (int n = 0; n < NUM_KEYFRAMES; ++n)
	{
		char	buf[25];
		sprintf(buf,"xplane_anim_value%d", n);
		anim_value_vars[n] = new TCL_linked_vardv(ac_get_tcl_interp(), STRING(buf), MAX_MULTI_COUNT, xplane_anim_value_cb, (void *) n, 0);
		sprintf(buf,"xplane_anim_angle%d", n);
		anim_angle_vars[n] = new TCL_linked_vardv(ac_get_tcl_interp(), STRING(buf), MAX_MULTI_COUNT, xplane_anim_angle_cb, (void *) n, 0);
	}

	multi_edit =  new TCL_linked_vari(ac_get_tcl_interp(), "xplane_multi_edit", NULL, NULL, 0);

	#undef SIMPLE_PROPERTY_STR
	#undef SIMPLE_PROPERTY_INT
	#undef SIMPLE_PROPERTY_FLT


	OBJ_register_change_cb(OBJ_editor_sync);
}
