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

#include "obj_tools.h"
#include "ac_utils.h"
#include "Undoable.h"
#include "bitmap_match.h"
#include "obj_radius.h"
#include "XObjDefs.h"
#include "obj_model.h"
#include "ac_utils.h"
#include "obj8_export.h"
#include "prefs.h"
#include "obj8_import.h"

#include <math.h>
#include <ac_plugin.h>
#include <set>
#include <vector>
#include <algorithm> // AC added for Linux

using std::set;
using std::vector;
/***************************************************************************************************
 * TOOL COMMANDS
 ***************************************************************************************************/

void do_show_night(void)
{
	vector <ACObject *> all, need_new_tex;
	set<int>			textures;
	find_all_selected_objects(all);
	for (vector<ACObject *>::iterator i = all.begin(); i != all.end(); ++i)
	{
		if (ac_object_has_texture(*i))
			textures.insert(ac_object_get_texture_index(*i));
	}
	for (set<int>::iterator ii = textures.begin(); ii != textures.end(); ++ii)
	{
		ACImage * image = texture_id_to_image(*ii);
		if (image)
		{
			char * strp;
			ac_entity_get_string_value(image, (char*)"name", &strp);
			string	tname = strp;
			tname.insert(tname.length() - 4, "LIT");
			printf("Trying %s\n", tname.c_str());
			ACImage * lit = texture_id_to_image(add_new_texture_opt((char *) tname.c_str(), (char *) tname.c_str()));

			if (!lit)
			{
				tname = strp;
				tname.insert(tname.length() - 4, "_LIT");
				printf("Trying %s\n", tname.c_str());
				lit = texture_id_to_image(add_new_texture_opt((char *) tname.c_str(), (char *) tname.c_str()));
			}

			if (lit)
			{
				apply_lighting(image, lit);
			}
		}
	}
}

void do_show_transparent(void)
{
	vector <ACObject *> all, need_new_tex;
	set<int>			textures;
	find_all_selected_objects(all);
	for (vector<ACObject *>::iterator i = all.begin(); i != all.end(); ++i)
	{
		if (ac_object_has_texture(*i))
			textures.insert(ac_object_get_texture_index(*i));
	}
	for (set<int>::iterator ii = textures.begin(); ii != textures.end(); ++ii)
	{

		ACImage * image = texture_id_to_image(*ii);
		if (image)
			make_transparent(image);
	}
}

void do_change_tex(void)
{
	vector <ACObject *> all, need_new_tex;
	find_all_selected_objects(all);
	if (!all.empty())
	{
		const char *filter[] = { "PNG files", "*.png", "BMP files", "*.bmp", "All files", "*", NULL };
	    char *filename = ac_get_load_filename((char*)"Pick a bitman to use for your models...", (char**)filter);
	    if (STRINGISEMPTY(filename))
	        return;

		int new_texture_id = add_new_texture_opt/*add_new_texture_reload*/(filename, filename);
		ACImage * new_bitmap = texture_id_to_image(new_texture_id);
		if (new_bitmap == NULL)
		{
			message_dialog((char*)"Could not open bitmap file '%s'.", filename);
//			myfree(filename);
			return;
		}

		add_undoable_all((char*)"Merge textures");

		int total = 0, changed = 0;
		for (vector<ACObject *>::iterator i = all.begin(); i != all.end(); ++i)
		{
			if (ac_object_has_texture(*i))
			{
				ACImage * tex = texture_id_to_image(ac_object_get_texture_index(*i));
				if (tex)
				{
					int xoff, yoff;
					++total;
					if (tex != new_bitmap)
					if (bitmap_match(tex, new_bitmap, &xoff, &yoff))
					{
						printf("Matched %s at %d, %d.\n", ac_object_get_name(*i), xoff, yoff);
						int w,h,d, ow, oh, od;
						ac_image_get_dim(new_bitmap, &w, &h, &d);
						ac_image_get_dim(tex, &ow, &oh, &od);
						double	add_factor_x = (double) xoff / (double) w;
						double	add_factor_y = (double) yoff / (double) h;
						double	mult_factor_x = (double) ow / (double) w;
						double	mult_factor_y = (double) oh / (double) h;

						offset_object_textures(*i, add_factor_x, add_factor_y, mult_factor_x, mult_factor_y);
						need_new_tex.push_back(*i);
						++changed;
					}
				}
			} else {
				printf("Skipping %s - has no texture.\n", ac_object_get_name(*i));
			}
		}

		for (vector<ACObject *>::iterator j = need_new_tex.begin(); j != need_new_tex.end(); ++j)
		{
			object_texture_set(*j, new_texture_id);
		}

		message_dialog((char*)"Changed %d of %d objects with textures.", changed, total);

//		myfree(filename);

	} else
		message_dialog((char*)"Cannot substitute textures when no objects are selected.");


/*

	int  xoff, yoff;
	if (bitmap_match(new_bitmap, new_bitmap2, &xoff, &yoff))
		message_dialog("Got a match at %d, %d", xoff, yoff);
	else
		message_dialog("No match!");
*/
}

void do_select_tex(const char * config)
{
	float st[4];

	if (sscanf(config,"%f %f %f %f", st,st+1,st+2,st+3) != 4)
	{
		printf("Bad args %s to rescale tex.\n",config);
		return;
	}
	vector<ACObject *> all;
	vector<Surface *> who;
	clear_selection();
	find_all_objects(ac_get_world(),all);
	for(vector<ACObject *>::iterator i = all.begin(); i != all.end(); ++i)
		obj_sel_st(*i,st, who);

	List * surf_list = NULL;
	for(vector<Surface *>::iterator i = who.begin(); i != who.end(); ++i)
	{
		list_add_item_head(&surf_list, *i);
	}
	clear_selection();
	ac_selection_select_surfacelist(surf_list);

	list_free(&surf_list);

	redraw_all();
}

void do_rescale_tex(const char * config)
{
	float old_s1,  old_t1,  old_s2,  old_t2,
	 new_s1,  new_t1,  new_s2,  new_t2;

	if (sscanf(config,"%f %f %f %f %f %f %f %f",
		&old_s1, &old_t1, &old_s2, &old_t2,
		&new_s1, &new_t1, &new_s2, &new_t2) != 8)
	{
		printf("Bad args %s to rescalae tex.\n",config);
		return;
	}

	vector <ACObject *> all;
	find_all_selected_objects(all);

	float s_scale = (new_s2 - new_s1) / (old_s2 - old_s1);
	float s_offset = new_s1 - old_s1 * s_scale;

	float t_scale = (new_t2 - new_t1) / (old_t2 - old_t1);
	float t_offset = new_t1 - old_t1 * t_scale;

	if (!all.empty())
	{
		add_undoable_all((char*)"Remap textures");

		int total = 0, changed = 0;
		for (vector<ACObject *>::iterator i = all.begin(); i != all.end(); ++i)
		{
			if (ac_object_has_texture(*i))
			{
				offset_object_textures(*i, s_offset, t_offset, s_scale, t_scale);
			}
		}

	} else
		message_dialog((char*)"Cannot substitute textures when no objects are selected.");


}



void do_calc_lod(void)
{
	float minv[3], maxv[3];
	if (get_selection_bounds(minv, maxv))
	{
		float radius = GetObjectLesserRadius(minv, maxv);

		// resolution * radius / tan (half of field of view)
		float LOD = 1024.0 / 2.0 * radius / tan(70.0 / 2.0);
		message_dialog((char*)"Recommended LOD distance: %d meters.", (int) LOD);
	}


}

void do_named_group(char * str)
{
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
		message_dialog((char*)"Cannot animate these %d objects; they are not all part of the same group.", objs.size());
		return;
	}

	ACObject * new_obj = new_object(OBJECT_GROUP);

	object_set_name(new_obj, str);

	if (!objs.empty())
	for (int n = objs.size()-1; n >= 0 ; --n)
		object_reparent(objs[n], new_obj);

	object_add_child(*parents.begin(), new_obj);

	tcl_command((char*)"hier_update");
}

void do_tree_extrude(void)
{
	add_undoable_all((char*)"Make trees");

	List * objs_l = ac_selection_get_objects();
	for (List * iter = objs_l; iter != NULL; iter = iter->next)
	{
		ACObject * obj = (ACObject *) iter->data;

		List 	*vertices, *surfaces, *kids;
		int numvert, numsurf, numkids;

	    ac_object_get_contents(obj, &numvert, &numsurf, &numkids, &vertices, &surfaces, &kids);

		vector<Surface *>	svec;

		while(numsurf > 0 && surfaces != NULL)
		{
	    	Surface *s = (Surface *) surfaces->data;
			if (s->numvert == 4)
				svec.push_back(s);
			surfaces = surfaces->next;
		}

//	    	object_set_twosided_faces(obj, 0);
//	    	object_set_surface_shading(obj, 1);

		for (vector<Surface *>::iterator siter = svec.begin(); siter != svec.end(); ++siter)
		{
	    	Surface *s = *siter;
			Surface *	faces[4] = { s, NULL, NULL, NULL };
			SVertex * s1, * s2, *s3, *s4;

			s1 = ((SVertex *)s->vertlist->data);
			s2 = ((SVertex *)s->vertlist->next->data);
			s3 = ((SVertex *)s->vertlist->next->next->data);
			s4 = ((SVertex *)s->vertlist->next->next->next->data);

			for (int n = 1; n < 4; ++n)
			{
				Point3	p1 = { s1->v->x, s1->v->y, s1->v->z };
				Point3	p2 = { s2->v->x, s2->v->y, s2->v->z };
				Point3	p3 = { s3->v->x, s3->v->y, s3->v->z };
				Point3	p4 = { s4->v->x, s4->v->y, s4->v->z };
				Vertex * v1 = object_add_new_vertex(obj, &p1);
				Vertex * v2 = object_add_new_vertex(obj, &p2);
				Vertex * v3 = object_add_new_vertex(obj, &p3);
				Vertex * v4 = object_add_new_vertex(obj, &p4);

				faces[n] = new_surface();
				surface_add_vertex(faces[n], v1, s1->tx, s1->ty);
				surface_add_vertex(faces[n], v2, s2->tx, s2->ty);
				surface_add_vertex(faces[n], v3, s3->tx, s3->ty);
				surface_add_vertex(faces[n], v4, s4->tx, s4->ty);

				object_add_surface(obj, faces[n]);
			}

			float min_x = min(min(s1->v->x,s2->v->x),min(s3->v->x,s4->v->x));
			float max_x = max(max(s1->v->x,s2->v->x),max(s3->v->x,s4->v->x));
			float min_z = min(min(s1->v->z,s2->v->z),min(s3->v->z,s4->v->z));
			float max_z = max(max(s1->v->z,s2->v->z),max(s3->v->z,s4->v->z));

			float ctr_x = (min_x + max_x) * 0.5;
			float ctr_z = (min_z + max_z) * 0.5;

			double angle = rand() % 360;
			Point3	up = { 0.0, 1.0, 0.0 };
			for (int n = 0; n < 4; ++n)
			{
				surface_set_shading(faces[n], 1);
				surface_set_twosided(faces[n], 0);
				rotate_surface_y(faces[n], angle, ctr_x, ctr_z);
				surface_set_normals(faces[n], &up);
				angle += 90;
			}
		}

	}
	list_free(&objs_l);
	redraw_all();
}

void do_bulk_export(void)
{
//	char * objs[] = { "Object files", ".obj", NULL };
	char * fn = ac_get_export_folder((char*)"Please pick a bulk export folder...");
	if (fn == NULL) return;
	if (*fn == 0) return;

	ACObject * wrl = ac_get_world();

	int numvert, numsurf, numkids;
	List *vertices, *surfaces, *kids;
	List *iter;

    ac_object_get_contents(wrl, &numvert, &numsurf, &numkids,
        &vertices, &surfaces, &kids);

    for (iter = kids; iter != NULL; iter = iter->next)
    {
    	ACObject * child = (ACObject *)iter->data;
		char * exp_name = ac_object_get_name(child);
		if (exp_name && ac_object_is_visible(child))
		{
			char path[1024];
			strcpy(path, fn);
			strcat(path, "/");
//			strcat(path,g_export_prefix);
			strcat(path, exp_name);
			do_obj8_save_ex(path, child, 1, -1, 1);	// do prefix, all texes, do misc stuff
		}
	}

	myfree(fn);
}


void collect_objects_with_texture(ACObject *ob, int t, vector<ACObject *> &v)
{
	if ( (ac_object_has_texture(ob)) && (ac_object_get_texture_index(ob) ) )
		v.push_back(ob);

    for (List * p = ac_object_get_childrenlist(ob); p != NULL; p = p->next)
        collect_objects_with_texture((ACObject *)p->data, t, v);
}


void do_tex_export(void)
{
	set<int> texes;
	get_all_used_texes(ac_get_world(), texes);
	if (texes.empty())
	{
		message_dialog((char*)"Your model contains no textures - multi-export by texture is not useful here.");
		return;
	}

	char * fn = ac_get_export_folder((char*)"Please pick a bulk export folder...");
	if (fn == NULL) return;
	if (*fn == 0) return;

	char path[1024];

	for (set<int>::iterator i = texes.begin(); i != texes.end(); ++i)
	{
		int textureid = *i;
//		vector<ACObject *> obs;

//		collect_objects_with_texture(ac_get_world(), textureid, obs);

		string tname = texture_id_to_name(textureid);

//		if (obs.size() > 1)
//			printf("WARNING: there is more than one object with the same texture (%s). Suggest using Object->Merge.  Only the first object with texture will be output\n", tname.c_str() );

		string::size_type p = tname.find_last_of("/\\");

		strcpy(path, fn);
		strcat(path, "/");
		strcat(path, tname.c_str() + p + 1);
		strcpy(path+strlen(path)-3,"obj");
		// object_top_ancestor(*obs.begin)
		do_obj8_save_ex(path, ac_get_world(), 1, *i, i == texes.begin());
	}
}

void do_make_onesided(void)
{
	add_undoable_all((char*)"Make One-sided");

	List * surf_l = ac_selection_get_whole_surfaces_all();

	vector<Surface *>	surfs;

	for (List * iter = surf_l; iter != NULL; iter = iter->next)
	{
		Surface * surf = (Surface *) iter->data;
		if (surface_get_twosided(surf))
		if (surface_get_type(surf) == SURFACE_POLYGON)
			surfs.push_back(surf);
	}
	list_free(&surf_l);

	for (vector<Surface *>::iterator si = surfs.begin(); si != surfs.end(); ++si)
	{
		Surface * sold = *si;
		ACObject * obj = object_of_surface(sold);

		Surface * snew = new_surface();

		for (List * svi = sold->vertlist; svi; svi = svi->next)
		{
			SVertex * svo = (SVertex *) svi->data;
			SVertex * svn = surface_add_vertex_head(snew, svo->v, svo->tx, svo->ty);
			svn->normal.x = -svo->normal.x;
			svn->normal.y = -svo->normal.y;
			svn->normal.z = -svo->normal.z;
		}

		snew->normal.x = -sold->normal.x;
		snew->normal.y = -sold->normal.y;
		snew->normal.z = -sold->normal.z;

		object_add_surface(obj, snew);

		surface_set_twosided(sold, 0);
		surface_set_twosided(snew, 0);
		surface_set_shading(snew,surface_get_shading(sold));
		surface_set_col(snew, sold->col);
	}
}

void do_make_upnormal(void)
{
	add_undoable_all((char*)"Make Up Normals");

	List * surf_l = ac_selection_get_whole_surfaces_all();

	vector<Surface *>	surfs;

	for (List * iter = surf_l; iter != NULL; iter = iter->next)
	{
		Surface * surf = (Surface *) iter->data;
		if (surface_get_type(surf) == SURFACE_POLYGON)
			surfs.push_back(surf);
	}
	list_free(&surf_l);

	Point3 up = { 0.0, 1.0, 0.0 };
	for (vector<Surface *>::iterator si = surfs.begin(); si != surfs.end(); ++si)
	{
		surface_set_normals(*si, &up);
	}
}

void do_reload_all_texes(void)
{
	set<int> t;
	get_all_used_texes(ac_get_world(), t);
	for (set<int>::iterator i = t.begin(); i != t.end(); ++i)
	{
		tex_reload(*i);
	}
}

struct sort_by_ac_state {
	bool operator()(ACObject * lhs, ACObject *rhs) const {

		float poly_l = OBJ_get_poly_os(lhs);
		float poly_r = OBJ_get_poly_os(rhs);
		if (poly_l == 0.0f) poly_l += 1000.0f;			// hack to put poly_offset of 0 last.
		if (poly_r == 0.0f) poly_r += 1000.0f;
		if (poly_l != poly_r) return poly_l < poly_r;

		float blend_l = OBJ_get_blend(lhs);
		float blend_r = OBJ_get_blend(rhs);
		if (blend_l < 0.0f) blend_l = -1.0f;			// hack to ignore negative blends, which are a way of "remembering" what our blend was.
		if (blend_r < 0.0f) blend_r = -1.0f;
		if (blend_l != blend_r) return blend_l < blend_r;

		return false;
	}
};

void do_optimize_selection(float do_optimize)
{
	vector<ACObject *>	objs;
	find_all_selected_objects_stable(objs);

	if (objs.empty())
	{
		if (do_optimize)	message_dialog((char*)"Select one or more objects to optimize their order.");
		else				message_dialog((char*)"Select one or more objects to count their batches.");
		return;
	}
	ACObject * parent = do_optimize ? get_common_parent(objs) : NULL;
	if (parent == NULL && do_optimize)
	{
		message_dialog((char*)"Internal error - selected objects do not appear to be part of the hierarchy.");
		return;
	}

	sort_by_ac_state functor;
	int state_pre = 1, state_post = 1;

	for (int n = 1; n < objs.size(); ++n)
	{
		if (functor(objs[n-1],objs[n]) ||
			functor(objs[n],objs[n-1]))
				++ state_pre;
	}

	if (!do_optimize)
	{
		message_dialog((char*)"Selection will take %d batches to draw.",state_pre);
		return;
	}

	stable_sort(objs.begin(), objs.end(), functor);

	for (int n = 1; n < objs.size(); ++n)
	{
		if (functor(objs[n-1],objs[n]) ||
			functor(objs[n],objs[n-1]))
				++ state_post;

	}

	if (state_post == state_pre)
	{
		message_dialog((char*)"I cannot further optimize this selection.  It will require %d batches to draw.", state_pre);
		return;
	}

	add_undoable_all((char*)"Optimize Selection");

	for (vector<ACObject *>::iterator o = objs.begin(); o != objs.end(); ++o)
	{
		object_remove_child_nocleanup(object_parent(*o),*o);
	}
	for (vector<ACObject *>::iterator o = objs.begin(); o != objs.end(); ++o)
	{
		object_add_child(parent,*o);
	}

	message_dialog((char*)"X-Plane used to need %d batches for this set of objects, now we need %d batches.",state_pre,state_post);

	redraw_all();
	tcl_command((char*)"hier_update");
}

static void sel_if_light(ACObject * who)
{
	if(strcmp(ac_entity_get_class_name(who),AC_CLASS_LIGHT)==0)
		ac_select_object(who);

	List *kids = ac_object_get_childrenlist(who);

    for (List * p = kids; p != NULL; p = p->next)
        sel_if_light((ACObject *)p->data);
}


void do_sel_lights(void)
{
	add_undoable_change_selection((char*)"Select all lights");
	clear_selection();
	sel_if_light(ac_get_world());
	tcl_command((char*)"hier_update");
	redraw_all();
	display_status();

}
