#ifndef OBJ_ANIM_H
#define OBJ_ANIM_H

#include <string>
#include <vector>
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
void	rescale_keyframes			(ACObject * obj, float new_lo, float new_hi);
int		get_keyframe_range			(ACObject * obj, float& lo, float& hi);

void	setup_obj_anim				(void);

#endif


