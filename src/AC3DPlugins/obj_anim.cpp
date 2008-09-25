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

#include "obj_anim.h"
#include "obj_model.h"
#include "obj_editor.h"
#include "undoable.h"
#include "ac_plugin.h"
#include <ac_plugin.h>
#if APL
#include <OpenGL/gl.h>
#else
#include <gl.h>
#endif
#include <math.h>
#include <set>
#include <list>
#include <map>
#include "ac_utils.h"
#include "XObjDefs.h"
using std::swap;
using std::set;
using std::map;
using std::min;
using std::max;
using std::list;

struct dataref_info {
	float		min_v;
	float		max_v;
	float		now_v;
};

static map<string, float>			g_previous;
static map<string, dataref_info>	g_datarefs;
static map<string, string>			g_tcl_mapping;
static int							g_anim_inited = 0;
static int							g_anim_enabled = 0;

struct compare_key {
	bool operator()(const XObjKey& lhs, float rhs) const {
		return lhs.key < rhs;
	}
};

inline float slop_for_anim(int a)
{
	if (a == anim_show || a == anim_hide) 	return 1.0;
											return 0.0;
}

inline float	extrap(float x1, float y1, float x2, float y2, float x)
{
	if (x1 == x2) return (y1 + y2) * 0.5;
	return (x-x1) * (y2 - y1) / (x2 - x1) + y1;
}

inline float	key_extrap(float input, const vector<XObjKey>& table, int n)
{
	if (table.empty()) return 0.0f;
	if (table.size() == 1) return table.front().v[n];
	if (table.size() == 2) return extrap(table[0].key,table[0].v[n],table[1].key,table[1].v[n],input);

	vector<XObjKey>::const_iterator i = std::lower_bound(table.begin(), table.end(), input, compare_key());
	vector<XObjKey>::const_iterator p1, p2;

		 if (i == table.end())		{ p1 = i-2; p2 = i-1; }
	else if (i->key == input)		{ return i->v[n];	}
	else if (i == table.begin())	{ p1 = i; p2 = i+1; }
	else							{ p1 = i-1; p2 = i; }

	return extrap(p1->key,p1->v[n],p2->key,p2->v[n], input);
}

#pragma mark -

static void anim_add_any(
				ACObject *					obj,
				int							add_head,
				int							anim_kind,
				float						xyz1[3],	// For non-trans: this is the axis.
				float						xyz2[3],
				const vector<XObjKey>&		keys,
				const char *				dataref,
				const char *				name)
{
	ACObject * new_obj = new_object(OBJECT_NORMAL);
	object_set_name(new_obj, (char*) name);
	vector<XObjKey>::const_iterator i;
	vector<Vertex *>	verts;
	if(anim_kind==anim_trans)
	{
		for (i = keys.begin(); i != keys.end(); ++i)
		{
			Point3 p = { i->v[0], i->v[1], i->v[2] };
			verts.push_back(object_add_new_vertex(new_obj,&p));
		}
	}
	else
	{
		Point3	p1 = { xyz1[0], xyz1[1], xyz1[2] };
		Point3	p2 = { xyz2[0], xyz2[1], xyz2[2] };
		verts.push_back(object_add_new_vertex(new_obj,&p1));
		verts.push_back(object_add_new_vertex(new_obj,&p2));
	}

	Surface * s = new_surface();

	surface_set_rgb_long(s, rgb_floats_to_long(1.0, 1.0, 0.0));
	surface_set_type(s, SURFACE_LINE);
	surface_set_twosided(s, 1);
	surface_set_shading(s, 1);
	object_add_surface_head(new_obj, s);

	for (vector<Vertex *>::iterator vv = verts.begin(); vv != verts.end(); ++vv)
		surface_add_vertex(s, *vv, 0.0, 0.0);

	OBJ_set_anim_type(new_obj, anim_kind);
	OBJ_set_anim_keyframe_count(new_obj, keys.size());
	int n;
	for (n = 0, i = keys.begin(); i != keys.end(); ++i, ++n)
	{
		OBJ_set_anim_nth_value(new_obj, n, i->key);
		if (anim_kind==anim_rotate)
			OBJ_set_anim_nth_angle(new_obj, n, i->v[0]);
	}

	OBJ_set_anim_dataref(new_obj,dataref);

	object_add_child(obj, new_obj);
	if(add_head) move_child_to_head(obj, new_obj);

	if (g_anim_inited)
	{
		tcl_command("hier_update");
		redraw_all();
	}
}

void anim_add_translate(
				ACObject *					obj,
				int							add_head,
				const vector<XObjKey>&		key_table,
				const char *				dataref,
				const char *				name)
{
	anim_add_any(obj, add_head, anim_trans, NULL, NULL, key_table, dataref, name);
}


void anim_add_rotate(
				ACObject *					obj,
				int							add_head,
				float						center[3],
				float						axis[3],
				const vector<XObjKey>&		key_table,
				const char *				dataref,
				const char *				name)
{
	float	l1[3] = { center[0] - axis[0], center[1] - axis[1], center[2] - axis[2] };
	float	l2[3] = { center[0] + axis[0], center[1] + axis[1], center[2] + axis[2] };

	anim_add_any(obj, add_head, anim_rotate, l1, l2, key_table, dataref, name);
}

void anim_add_static(
				ACObject *					obj,
				int							add_head,
				float						xyz1[3],
				const char *				dataref,
				const char *				name)
{
	float origin[3] = { 0.0, 0.0, 0.0 };
	vector<XObjKey>	keys;
	anim_add_any(obj, add_head, anim_static, origin, xyz1, keys, dataref, name);
}

void anim_add_show(
				ACObject *					obj,
				int							add_head,
				const vector<XObjKey>&		key_table,
				const char *				dataref,
				const char *				name)
{
	float origin[3] = { 0.0, 0.0, 0.0 };
	anim_add_any(obj, add_head, anim_show, origin, origin, key_table, dataref, name);
}

void anim_add_hide(
				ACObject *					obj,
				int							add_head,
				const vector<XObjKey>&		key_table,
				const char *				dataref,
				const char *				name)
{
	float origin[3] = { 0.0, 0.0, 0.0 };
	anim_add_any(obj, add_head, anim_hide, origin, origin, key_table, dataref, name);
}


#pragma mark -

float *	axis_for_rotation(ACObject * obj, float buf[3])
{
	Vertex * v1 = surface_get_vertex(obj_get_first_surf(obj), 0);
	Vertex * v2 = surface_get_vertex(obj_get_first_surf(obj), 1);
	if (v1 == NULL || v2 == NULL)
	{
		buf[0] = 0.0;
		buf[1] = 1.0;
		buf[2] = 0.0;
		return buf;
	}
	buf[0] = (v2->x - v1->x);
	buf[1] = (v2->y - v1->y);
	buf[2] = (v2->z - v1->z);
	float len = sqrt(buf[0]*buf[0]+buf[1]*buf[1]+buf[2]*buf[2]);
	if (len != 0.0)
	{
		len = 1.0 / len;
		buf[0] *= len;
		buf[1] *= len;
		buf[2] *= len;
	}
	return buf;
}

float * center_for_rotation(ACObject * obj, float buf[3])
{
	Vertex * v1 = surface_get_vertex(obj_get_first_surf(obj), 0);
	Vertex * v2 = surface_get_vertex(obj_get_first_surf(obj), 1);
	if(v1 == NULL || v2 == NULL)
	{
		buf[0] = buf[1] = buf[2] = 0.0f;
		return buf;
	}
	buf[0] = (v2->x + v1->x) * 0.5;
	buf[1] = (v2->y + v1->y) * 0.5;
	buf[2] = (v2->z + v1->z) * 0.5;
	return buf;
}

float * center_for_rotation_negative(ACObject * obj, float buf[3])
{
	Vertex * v1 = surface_get_vertex(obj_get_first_surf(obj), 0);
	Vertex * v2 = surface_get_vertex(obj_get_first_surf(obj), 1);
	if(v1 == NULL || v2 == NULL)
	{
		buf[0] = buf[1] = buf[2] = 0.0f;
		return buf;
	}
	buf[0] = -(v2->x + v1->x) * 0.5;
	buf[1] = -(v2->y + v1->y) * 0.5;
	buf[2] = -(v2->z + v1->z) * 0.5;
	return buf;
}

float *	anim_trans_nth(ACObject * obj, int n, float buf[3])
{
	Vertex * v = surface_get_vertex(obj_get_first_surf(obj), n);
	if(v == NULL)
	{
		buf[0] = buf[1] = buf[2] = 0.0f;
		return buf;
	}
	buf[0] = v->x;
	buf[1] = v->y;
	buf[2] = v->z;
	return buf;
}

float *	anim_trans_nth_relative(ACObject * obj, int n, float buf[3])
{
	int root = OBJ_get_anim_keyframe_root(obj);
	float o[3];
	anim_trans_nth(obj, root, o);
	anim_trans_nth(obj, n, buf);
	buf[0] -= o[0];
	buf[1] -= o[1];
	buf[2] -= o[2];
	return buf;
}

#pragma mark -

void bake_static_transitions(ACObject * object)
{
	set<ACObject *> kill_set;
	Point3	diff = { 0.0, 0.0, 0.0 };
	List * p;
	List * kids = ac_object_get_childrenlist(object);
    for (p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
		if (OBJ_get_anim_type(child) == anim_static)
		{
			Vertex * v1 = surface_get_vertex(obj_get_first_surf(child), 0);
			Vertex * v2 = surface_get_vertex(obj_get_first_surf(child), 1);
			if(v1 != NULL && v2 != NULL)
			{
				diff.x += (v2->x - v1->x);
				diff.y += (v2->y - v1->y);
				diff.z += (v2->z - v1->z);
			}
			kill_set.insert(child);
		}
		else
		{
			translate_object(child, &diff);
		}
	}

	for (set<ACObject *>::iterator kill = kill_set.begin(); kill != kill_set.end(); ++kill)
		object_delete(*kill);

	kids = ac_object_get_childrenlist(object);
    for (p = kids; p != NULL; p = p->next)
		bake_static_transitions((ACObject *)p->data);
}

void	purge_datarefs(void)
{
	for (map<string,dataref_info>::iterator i = g_datarefs.begin(); i != g_datarefs.end(); ++i)
		g_previous[i->first] = i->second.now_v;
	g_datarefs.clear();
}

void	gather_datarefs(ACObject * obj)
{
	if (OBJ_get_anim_type(obj) != anim_none)
	{
		char dref[512];
		string dataref(OBJ_get_anim_dataref(obj,dref));
		dataref_info& i = g_datarefs[dataref];
		i.now_v = g_previous[dataref];
		for (int n = 0; n < OBJ_get_anim_keyframe_count(obj); ++n)
		{
			i.min_v = min(i.min_v, OBJ_get_anim_nth_value(obj, n)-slop_for_anim(OBJ_get_anim_type(obj)));
			i.max_v = max(i.max_v, OBJ_get_anim_nth_value(obj, n)+slop_for_anim(OBJ_get_anim_type(obj)));

		}
		i.now_v = min(i.max_v,max(i.min_v,i.now_v));
	}
	List * p;
	List * kids = ac_object_get_childrenlist(obj);
    for (p = kids; p != NULL; p = p->next)
		gather_datarefs((ACObject *)p->data);
}

static void quote_dref(string& s)
{
	string::size_type n = 0;
	while((n = s.find_first_of("[]",n)) != s.npos)
	{
		s.insert(n,"\\");
		n += 2;
	}
}

void	sync_datarefs()
{
	static int high_water = 0;
//	printf("Got sync dataref, anim=%d\n",g_anim_inited);
	if (!g_anim_inited) return;
	g_tcl_mapping.clear();
//	tcl_command("clean_anim");
	int n = 0;
	for (map<string,dataref_info>::iterator dref = g_datarefs.begin(); dref != g_datarefs.end(); ++dref)
	{
		char buf[10];
		sprintf(buf,"dr%d", n);
		string tcl_name(buf);
		string dref_q(dref->first);
		quote_dref(dref_q);
		tcl_command("sync_dataref %s %s %f %f %f", tcl_name.c_str(), dref_q.c_str(), dref->second.now_v, dref->second.min_v, dref->second.max_v);
		g_tcl_mapping[tcl_name] = dref->first;
		++n;
	}
	int k;
	for(k = n; k < high_water; ++k)
	{
		char buf[10];
		sprintf(buf,"dr%d", k);
		string tcl_name(buf);
		tcl_command("kill_dataref %s", tcl_name.c_str());
	}
	high_water = n;
}





#pragma mark -

static void make_anim_group(void)
{
	add_undoable_all("Make animation group");
	List * objs_l = ac_selection_get_objects();
	if (objs_l == NULL) return;
	vector<ACObject *> 	objs;
	set<ACObject *>		parents;
	while (objs_l)
	{
		objs.push_back((ACObject *) objs_l->data);
		objs_l = objs_l->next;
		ACObject * p = ac_object_get_parent(objs.back());
		if (p != NULL)
			parents.insert(p);
	}
	list_free(&objs_l);

	if (objs.empty())	return;

	if (parents.size() > 1)
	{
		message_dialog("Cannot animate these %d objects; they are not all part of the same group.", objs.size());
		return;
	}

	ACObject * new_obj = new_object(OBJECT_GROUP);

	object_set_name(new_obj, "Animation");

	if (!objs.empty())
	for (int n = objs.size()-1; n >= 0 ; --n)
		object_reparent(objs[n], new_obj);

	object_add_child(*parents.begin(), new_obj);
	OBJ_set_animation_group	(new_obj, 1);

	if (g_anim_inited)
	{
		tcl_command("hier_update");
		redraw_all();
	}
}

static void do_bake_selection(void)
{
	add_undoable_all("Bake selection");
	vector<ACObject *> obs;
	find_all_selected_objects_flat(obs);
	for (vector<ACObject *>::iterator o = obs.begin(); o != obs.end(); ++o)
	{
		bake_static_transitions(*o);
	}

	if (g_anim_inited)
	{
		redraw_all();
		tcl_command("hier_update");
	}
}



static void make_anim_of_type(int argc, char * argv[])
{
	add_undoable_all("Make Animation");

	List * objs_l = ac_selection_get_part_selected_objects();
	if (objs_l == NULL) return;
	vector<ACObject *> 	objs;
	set<ACObject *>		parents;
	while (objs_l)
	{
		objs.push_back((ACObject *) objs_l->data);
		objs_l = objs_l->next;
		ACObject * p = ac_object_get_parent(objs.back());
		if (p != NULL)
			parents.insert(p);
	}
	list_free(&objs_l);

	if (objs.empty())	return;

	if (parents.size() > 1)
	{
		message_dialog("Cannot animate these %d objects; they are not all part of the same group.", objs.size());
		return;
	}

	vector<XObjKey>	keys(2);
	keys[0].key = 0.0;
	keys[0].v[0] = 0.0;
	keys[0].v[1] = 0.0;
	keys[0].v[2] = 0.0;
	keys[1].key = 1.0;
	keys[1].v[0] = 0.0;
	keys[1].v[1] = 10.0;
	keys[1].v[2] = 0.0;

	float a1[3] = { 0.0, 0.0, 0.0 };
	float a2[3] = { 0.0, 10.0, 0.0 };

	Surface *	surf;
	ACObject *	obj;
	surf=find_single_selected_surface();

	if (surf==NULL && (obj=find_single_selected_object())!=NULL)
	{
		surf = obj_get_first_surf(obj);
	}

	if(surf!=NULL && surf->numvert > 0)
	{
		List * v;
		for(v=surf->vertlist;v;v=v->next)
		{
			a1[0] += SVERTEX(v->data)->x;
			a1[1] += SVERTEX(v->data)->y;
			a1[2] += SVERTEX(v->data)->z;
		}

		a1[0] /= (float) surf->numvert;
		a1[1] /= (float) surf->numvert;
		a1[2] /= (float) surf->numvert;

		float len=0;
		for(v=surf->vertlist;v;v=v->next)
		{
			len+=sqrt(sqr(a1[0]-SVERTEX(v->data)->x)+
					  sqr(a1[1]-SVERTEX(v->data)->y)+
					  sqr(a1[2]-SVERTEX(v->data)->z));
		}

		len /= (float) surf->numvert;

		if (strcmp(argv[1],"rotate")!=0)
		{
			keys[0].v[0] = a1[0];
			keys[0].v[1] = a1[1];
			keys[0].v[2] = a1[2];
			keys[1].v[0] = a1[0] + surf->normal.x * len;
			keys[1].v[1] = a1[1] + surf->normal.y * len;
			keys[1].v[2] = a1[2] + surf->normal.z * len;
		} else {
			a2[0] = surf->normal.x * len;
			a2[1] = surf->normal.y * len;
			a2[2] = surf->normal.z * len;
		}
	}

	if (strcmp(argv[1],"translate")==0)
		anim_add_translate(*parents.begin(), 1, keys, "none", "translation");

	if (strcmp(argv[1],"rotate")==0)
		anim_add_rotate(*parents.begin(), 1, a1, a2, keys, "none", "rotation");

	if (strcmp(argv[1],"show")==0)
		anim_add_show(*parents.begin(), 1, keys, "none", "show");

	if (strcmp(argv[1],"hide")==0)
		anim_add_hide(*parents.begin(), 1, keys, "none", "hide");
}

#pragma mark -

static void set_anim_enable(float n)
{
	g_anim_enabled = n;
	if (g_anim_inited)
		redraw_all();
}

// null dref for all anim!
static void sel_if_has(ACObject * who, const char *dref)
{
	char	buf[512];
	if (OBJ_get_anim_type(who) != anim_none)
	{
		OBJ_get_anim_dataref(who,buf);
		if(dref == NULL || strcmp(buf,dref)==0)
		{
			ac_select_object(who);
		}
	}

	List *kids = ac_object_get_childrenlist(who);

    for (List * p = kids; p != NULL; p = p->next)
        sel_if_has((ACObject *)p->data, dref);

}

static void set_anim_now(int argc, char * argv[])
{
	if (argc <= 2) return;
	map<string,string>::iterator who = g_tcl_mapping.find(argv[1]);
	if (who != g_tcl_mapping.end())
	{
		map<string,dataref_info>::iterator dref = g_datarefs.find(who->second);
		if (dref != g_datarefs.end())
		{
			dref->second.now_v = atof(argv[2]);
			if (g_anim_inited)
			{
				redraw_all();
			}
		}
	}
}

static void set_sel_now(int argc, char * argv[])
{
	if (argc <= 1) return;
	add_undoable_change_selection("Select animation");
	map<string,string>::iterator who = g_tcl_mapping.find(argv[1]);
	if (who != g_tcl_mapping.end())
	{
		string dref = who->second;
		if (!dref.empty())
		{
			sel_if_has(ac_get_world(),dref.c_str());
			tcl_command("hier_update");
			redraw_all();
			display_status();
		}
	}
}

static void select_all_anim(void)
{
	add_undoable_change_selection("Select all animations");
	clear_selection();
	sel_if_has(ac_get_world(),NULL);
	tcl_command("hier_update");
	redraw_all();
	display_status();
}


static void set_anim_for_sel_keyframe(int argc, char * argv[])
{
	if (argc > 2)
	{
		ACObject * obj = get_sel_single_obj(atoi(argv[2]));
		if (obj)
		{
			int n = atoi(argv[1]);
			if (n < OBJ_get_anim_keyframe_count(obj))
			{
				char buf[256];
				OBJ_get_anim_dataref(obj, buf);
				map<string,dataref_info>::iterator dref = g_datarefs.find(buf);
				if (dref != g_datarefs.end())
				{
					dref->second.now_v = OBJ_get_anim_nth_value(obj, n);
					if (g_anim_inited)
					{
						sync_datarefs();
						redraw_all();
					}
				}
			}
		}
	}
}

static void add_keyframe(int argc, char * argv[])
{
	if (argc > 2)
	{
		ACObject * obj = get_sel_single_obj(atoi(argv[2]));
		if (obj)
		{
			int n = atoi(argv[1]);
			int m = OBJ_get_anim_keyframe_count(obj);
			if (n < m && n >= (OBJ_get_anim_type(obj) == anim_trans ? 1 : 0))
			{
				add_undoable_all("Add Keyframe");
				if (OBJ_get_anim_type(obj) == anim_trans)
				{
					Boolean made;
					Vertex * v2 = surface_get_vertex(obj_get_first_surf(obj), n);
					Vertex * v1 = (n > 0) ? surface_get_vertex(obj_get_first_surf(obj), n-1) : v2;
					if (v1 == NULL || v2 == NULL)
						return;
					surface_insert_vertex(obj_get_first_surf(obj), v1, v2, &made);
				}
				int kf = n-1;
				int ks = n;
				if (kf < 0) kf = 0;
				float nv = 0.5 * (OBJ_get_anim_nth_value(obj, kf)+OBJ_get_anim_nth_value(obj, ks));
				float na = 0.5 * (OBJ_get_anim_nth_angle(obj, kf)+OBJ_get_anim_nth_angle(obj, ks));
				for (int k = m; k > n; --k)
				{
					OBJ_set_anim_nth_value(obj, k, OBJ_get_anim_nth_value(obj, k-1));
					OBJ_set_anim_nth_angle(obj, k, OBJ_get_anim_nth_angle(obj, k-1));
				}
				OBJ_set_anim_nth_value(obj, n, nv);
				OBJ_set_anim_nth_angle(obj, n, na);
				OBJ_set_anim_keyframe_count(obj,  1+OBJ_get_anim_keyframe_count(obj));
				if (g_anim_inited)
				{
					redraw_all();
					sync_datarefs();
				}
			}
		}
	}
}

static void delete_keyframe(int argc, char * argv[])
{
	if (argc > 2)
	{
		ACObject * obj = get_sel_single_obj(atoi(argv[2]));
		if (obj)
		{
			int n = atoi(argv[1]);
			int m = OBJ_get_anim_keyframe_count(obj);
			if (n < m && n >= 0 && m > 2)
			{
				add_undoable_all("Add Keyframe");
				if (OBJ_get_anim_type(obj) == anim_trans)
				{
					SVertex * dead = surface_get_svertex_at(obj_get_first_surf(obj), n);
					object_delete_vertex(obj, dead->v);
				}
				for (int k = n; k < (m-1); ++k)
				{
					OBJ_set_anim_nth_value(obj, k, OBJ_get_anim_nth_value(obj, k+1));
					OBJ_set_anim_nth_angle(obj, k, OBJ_get_anim_nth_angle(obj, k+1));
				}
				OBJ_set_anim_keyframe_count(obj,  OBJ_get_anim_keyframe_count(obj)-1);
				if (g_anim_inited)
				{
					redraw_all();
					sync_datarefs();
				}
			}
		}
	}
}

static void	obj_changed_cb(ACObject * obj)
{
	purge_datarefs();
	gather_datarefs(ac_get_world());
	sync_datarefs();
}

static void do_resync_anim(void)
{
	purge_datarefs();
	gather_datarefs(ac_get_world());
	sync_datarefs();
}

#pragma mark -

static void anim_exit_cb(void * data)
{
	g_anim_inited = 0;
}

static void build_key_table(ACObject * ob, vector<XObjKey>& table, int * root)
{
	int anim_type = OBJ_get_anim_type(ob);
	table.resize(OBJ_get_anim_keyframe_count(ob));
	for (int n = 0; n < OBJ_get_anim_keyframe_count(ob); ++n)
	{
		table[n].key = OBJ_get_anim_nth_value(ob, n);
		if(anim_type == anim_trans)
		{
			anim_trans_nth(ob, n, table[n].v);
		}
		else if (anim_type == anim_rotate)
			table[n].v[0] = OBJ_get_anim_nth_angle(ob, n);
	}
	if (table.size() > 1 && table.front().key > table.back().key)
	{
		if (root) *root = table.size()-*root-1;
		reverse(table.begin(),table.end());
	}
}

static float get_dataref_value(const char * dataref)
{
	string dref(dataref);
	map<string,dataref_info>::iterator i = g_datarefs.find(dref);
	if (i == g_datarefs.end()) return 0;
	return i->second.now_v;
}

static int vis = 1;
list<int>	push_stack;

static void anim_pre_func(ACObject * ob, Boolean is_primary_render)
{
	if (!g_anim_enabled) return;
	if (OBJ_get_animation_group(ob))
	{
		glMatrixMode(GL_MODELVIEW_MATRIX);
		glPushMatrix();
		push_stack.push_back(vis);
		// Ben says: vis is NOT reset to true jsut by starting anim group!
//		vis = 1;
	}
}

static void anim_post_func(ACObject * ob, Boolean is_primary_render)
{
	if (!g_anim_enabled) return;
	if (OBJ_get_animation_group(ob))
	{
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		if (vis != push_stack.back())
		{
			if(vis)	{ glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);	glDepthMask(GL_FALSE); }
			else	{ glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);	glDepthMask(GL_TRUE); }
		}
		vis = push_stack.back();
		push_stack.pop_back();
	}

	int anim_t = OBJ_get_anim_type(ob);

	if (anim_t != anim_none && ac_object_get_parent(ob) && OBJ_get_animation_group(ac_object_get_parent(ob)))
	{
		char dref[1024];
		float now_v = get_dataref_value(OBJ_get_anim_dataref(ob,dref));
		float k1, k2;

		float axis[3], offset[3];

		vector<XObjKey>	table;

		switch(anim_t) {
		case anim_trans:
			{
				int root = OBJ_get_anim_keyframe_root(ob);
				build_key_table(ob, table,&root);
				if (root >= table.size()) root = (table.size()-1);
				if (!table.empty())
					glTranslatef(key_extrap(now_v,table,0) - table[root].v[0],
								 key_extrap(now_v,table,1) - table[root].v[1],
								 key_extrap(now_v,table,2) - table[root].v[2]);
			}
			break;
		case anim_static:
			{
				float p[3];
				anim_trans_nth_relative(ob, 1, p);
				glTranslatef(p[0],
							 p[1],
							 p[2]);
			}
			break;
			break;
		case anim_rotate:
			{
				build_key_table(ob, table,NULL);
				if (!table.empty())
				{
					axis_for_rotation(ob, axis);
					center_for_rotation(ob, offset);
					glTranslatef(offset[0],offset[1],offset[2]);
					glRotatef(key_extrap(now_v, table, 0),
								axis[0],axis[1],axis[2]);
					glTranslatef(-offset[0],-offset[1],-offset[2]);
				}
			}
			break;
		case anim_hide:
			k1 = OBJ_get_anim_nth_value(ob, 0);
			k2 = OBJ_get_anim_nth_value(ob, 1);
			if (now_v >= k1 && now_v <= k2)
			{
				if (vis)
					{ glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE); glDepthMask(GL_FALSE); }
				vis = 0;
			}
			break;
		case anim_show:
			k1 = OBJ_get_anim_nth_value(ob, 0);
			k2 = OBJ_get_anim_nth_value(ob, 1);
			if (now_v >= k1 && now_v <= k2)
			{
				if (!vis)
					{ glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); glDepthMask(GL_TRUE); }
				vis = 1;
			}
			break;
		}
	}
}

void	rescale_keyframes			(ACObject * obj, float old_lo, float new_lo, float old_hi, float new_hi)
{
	int kk = OBJ_get_anim_keyframe_count(obj);
	for(int k = 0; k < kk; ++k)
		OBJ_set_anim_nth_value(obj,k,extrap(old_lo,new_lo,old_hi,new_hi,
			OBJ_get_anim_nth_value(obj,k)));
}

int		get_keyframe_range			(ACObject * obj, float& lo, float& hi)
{
	int at = OBJ_get_anim_type(obj);
	if(at != anim_trans && at != anim_rotate) return 0;
	int kk = OBJ_get_anim_keyframe_count(obj);
	if (kk < 1) return 0;

	lo = hi = OBJ_get_anim_nth_value(obj,0);
	for(int k = 1; k < kk; ++k)
	{
		lo = min(lo,OBJ_get_anim_nth_value(obj,k));
		hi = max(hi,OBJ_get_anim_nth_value(obj,k));
	}
	return 1;
}

static void	reverse_sel(void)
{
	vector<ACObject *>	objs;
	find_all_selected_objects(objs);
	int n,ctr=0;
	vector<float>	old_lo(objs.size()),old_hi(objs.size());

	for(n=0;n < objs.size(); ++n)
	if(get_keyframe_range(objs[n],old_lo[n],old_hi[n]))		++ctr;
	else													objs[n]=NULL;

	if (ctr==0) return;

	add_undoable_all("Reverse Keyframes");

	float lo=old_lo[0];
	float hi=old_hi[0];
	for(n=1;n<objs.size();++n)
	if(objs[n])
	{
		lo=min(lo,old_lo[n]);
		hi=max(hi,old_hi[n]);
	}

	for (n = 0; n < objs.size(); ++n)
	if(objs[n])
	{
		rescale_keyframes(objs[n],lo,hi,hi,lo);
	}
}

static void	rescale_sel(int argc, char * argv[])
{
	if (argc < 5) return;
	float old_lo = atof(argv[1]);
	float new_lo = atof(argv[2]);
	float old_hi = atof(argv[3]);
	float new_hi = atof(argv[4]);

	vector<ACObject *>	objs;
	find_all_selected_objects(objs);
	if (objs.empty()) return;

	add_undoable_all("Rescale Keyframes");
	for (int n = 0; n < objs.size(); ++n)
	{
		float l,h;
		if (get_keyframe_range(objs[n],l,h))
			rescale_keyframes(objs[n],old_lo,new_lo,old_hi,new_hi);
	}
}

void setup_obj_anim(void)
{
	g_anim_inited = 1;
	tcl_stubs.Tcl_CreateExitHandler(anim_exit_cb, NULL);
	ac_set_pre_render_object_callback(anim_pre_func);
	ac_set_post_render_object_callback(anim_post_func);
	ac_add_command_full("xplane_set_anim_enable", CAST_CMD(set_anim_enable), 1, "f", "ac3d xplane_set_anim_enable <0 or 1>", "set animation on or off");
	ac_add_command_full("xplane_set_anim_now", CAST_CMD(set_anim_now), 2, "argv", "ac3d xplane_set_anim_now <n> <t>", "set dataref n to time t");
	ac_add_command_full("xplane_anim_select", CAST_CMD(set_sel_now), 2, "argv", "ac3d xplane_anim_select<n>", "select all objects using dtaref n");
	ac_add_command_full("xplane_anim_select_all", CAST_CMD(select_all_anim), 0, NULL, "ac3d xplane_anim_select_all", "select all animated objects");
	ac_add_command_full("xplane_set_anim_keyframe", CAST_CMD(set_anim_for_sel_keyframe), 3, "argv", "ac3d xplane_set_anim_keyframe <kf index> <obj idx>", "set animation to this keyframe");
	ac_add_command_full("xplane_add_keyframe", CAST_CMD(add_keyframe), 3, "argv", "ac3d xplane_add_keyframe <kf index> <obj idx>", "set animation to this keyframe");
	ac_add_command_full("xplane_delete_keyframe", CAST_CMD(delete_keyframe), 3, "argv", "ac3d xplane_delete_keyframe <kf index> <obj idx>", "set animation to this keyframe");

	ac_add_command_full("xplane_reverse_keyframe", CAST_CMD(reverse_sel), 0, NULL, "ac3d xplane_reverse_keyframe", "reverse key frames of selection");
	ac_add_command_full("xplane_rescale_keyframe", CAST_CMD(rescale_sel), 5, "argv", "ac3d xplane_rescale_keyframe <old lo> <new lo> <old hi> <new hi>", "rescale key frames of selection");

	ac_add_command_full("xplane_resync_anim", CAST_CMD(do_resync_anim), 0, NULL, "ac3d xplane_resync_anim", "resync animation with model");

	ac_add_command_full("xplane_make_anim_group", CAST_CMD(make_anim_group),0, NULL, "ac3d xplane_make_anim_group", "make animation group");
	ac_add_command_full("xplane_make_anim_typed", CAST_CMD(make_anim_of_type),1, "argv", "ac3d make_anim_of_type rotate|transate|show|hide", "make animation");

	ac_add_command_full("xplane_bake_static", CAST_CMD(do_bake_selection),0,NULL, "ac3d xplane_bake_static", "bake down all static transitions");

	OBJ_register_change_cb(obj_changed_cb);
}

