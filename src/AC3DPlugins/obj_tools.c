#include "obj_tools.h"
#include "ac_utils.h"
#include "bitmap_match.h"
#include "obj_radius.h"
#include "obj_export.h"
#include "XObjDefs.h"
#include "obj8_export.h"

#include <ac_plugin.h>
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
			ac_entity_get_string_value(image, "name", &strp);
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
		char *filter[] = { "BMP files", "*.bmp", "PNG files", "*.png", "All files", "*", NULL };
	    char *filename = ac_get_load_filename("Pick a bitman to use for your models...", filter);
	    if (STRINGISEMPTY(filename))
	        return;

		int new_texture_id = add_new_texture_opt/*add_new_texture_reload*/(filename, filename);
		ACImage * new_bitmap = texture_id_to_image(new_texture_id);
		if (new_bitmap == NULL)
		{
			message_dialog("Could not open bitmap file '%s'.", filename);
//			myfree(filename);
			return;
		}
		
		add_undoable_all("Merge textures");
		
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

		message_dialog("Changed %d of %d objects with textures.", changed, total);

//		myfree(filename);

	} else
		message_dialog("Cannot substitute textures when no objects are selected.");


/*	

	int  xoff, yoff;
	if (bitmap_match(new_bitmap, new_bitmap2, &xoff, &yoff))
		message_dialog("Got a match at %d, %d", xoff, yoff);
	else
		message_dialog("No match!");
*/		
}

void do_calc_lod(void)
{
	gObj.cmds.clear();	
//	gTexName.clear();
//	gSmooth = true;
//	gTwoSided = false;

	// This gets all of the top level objects that are selected...no object will be
	// a child (removed or direct) of another...
	List *	objs = ac_selection_get_objects();
	if (objs)
	{	
		for (List * i = objs; i; i=i->next)
		{
			obj7_output_object((ACObject *) i->data, NULL);
		}
		list_free(&objs);
		
		if (!gObj.cmds.empty())
		{
			float radius = GetObjectLesserRadius(gObj);
			
			// resolution * radius / tan (half of field of view)
			float LOD = 1024.0 / 2.0 * radius / tan(70.0 / 2.0);		
			message_dialog("Recommended LOD distance: %d meters.", (int) LOD);
		}
	}
}

void do_animation_group(void)
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
		message_dialog("Cannot animate these %d objects; they are not all part of the same group.", objs.size());
		return;
	}	
	
	ACObject * new_obj = new_object(OBJECT_GROUP);
	object_set_name(new_obj, "ANIMATION");

	for (int n = 0; n < objs.size(); ++n)
		object_reparent(objs[n], new_obj);
	
	object_add_child(*parents.begin(), new_obj);	
	tcl_command("hier_update");
}

void do_tree_extrude(void)
{
	add_undoable_all("Make trees");

	List * objs_l = ac_selection_get_objects();
	for (List * iter = objs_l; iter != NULL; iter = iter->next)
	{
		ACObject * obj = (ACObject *) iter->data;

		List 	*vertices, *surfaces, *kids;
		int numvert, numsurf, numkids;

	    ac_object_get_contents(obj, &numvert, &numsurf, &numkids, &vertices, &surfaces, &kids); 
	    
	    if (numsurf == 1)
	    {
	    	Surface *s = (Surface *) surfaces->data;
			if (s->numvert == 4)
			{
				SVertex * s1, * s2, *s3, *s4;

				s1 = ((SVertex *)s->vertlist->data);
				s2 = ((SVertex *)s->vertlist->next->data);
				s3 = ((SVertex *)s->vertlist->next->next->data);
				s4 = ((SVertex *)s->vertlist->next->next->next->data);

				float min_x = min(min(s1->v->x,s2->v->x),min(s3->v->x,s4->v->x));
				float max_x = max(max(s1->v->x,s2->v->x),max(s3->v->x,s4->v->x));

				float min_y = min(min(s1->v->y,s2->v->y),min(s3->v->y,s4->v->y));
				float max_y = max(max(s1->v->y,s2->v->y),max(s3->v->y,s4->v->y));

				float min_z = min(min(s1->v->z,s2->v->z),min(s3->v->z,s4->v->z));
				float max_z = max(max(s1->v->z,s2->v->z),max(s3->v->z,s4->v->z));

				float smin = min(min(s1->tx, s2->tx),min(s3->tx,s4->tx));
				float smax = max(max(s1->tx, s2->tx),max(s3->tx,s4->tx));
				float tmin = min(min(s1->ty, s2->ty),min(s3->ty,s4->ty));
				float tmax = max(max(s1->ty, s2->ty),max(s3->ty,s4->ty));

				float ctr_x = (min_x + max_x) * 0.5;
				float ctr_z = (min_z + max_z) * 0.5;				
				float hwidth = (max_x - min_x) * 0.5;
				
				Point3 p1 = { ctr_x, min_y, ctr_z - hwidth };
				Point3 p2 = { ctr_x, max_y, ctr_z - hwidth };
				Point3 p3 = { ctr_x, max_y, ctr_z + hwidth };
				Point3 p4 = { ctr_x, min_y, ctr_z + hwidth };
				
				Vertex * v1 = object_add_new_vertex(obj, &p1);
				Vertex * v2 = object_add_new_vertex(obj, &p2);
				Vertex * v3 = object_add_new_vertex(obj, &p3);
				Vertex * v4 = object_add_new_vertex(obj, &p4);
				
				Surface * ns = new_surface();
				surface_add_vertex(ns, v1, smin, tmin);
				surface_add_vertex(ns, v2, smin, tmax);
				surface_add_vertex(ns, v3, smax, tmax);
				surface_add_vertex(ns, v4, smax, tmin);

				object_add_surface(obj, ns);
				double angle = rand() % 360;
				angle *= (3.1415926 / 180.0);				
				
				set_global_matrix_rotate(0.0, angle, 0.0);
				Point3 ndiff = { -ctr_x, 0, -ctr_z };				
				Point3 diff = { ctr_x, 0, ctr_z };				
				translate_object(obj, &ndiff);				
				
				rotate_object(obj);
				translate_object(obj, &diff);				
				
			}
	    }
	}
	objectlist_calc_normals(objs_l);
	list_free(&objs_l);
	redraw_all(); 
}

void do_bulk_export(void)
{
	char * objs[] = { "Object files", ".obj", NULL };
	char * fn = ac_get_export_folder("Please pick a bulk export folder...");
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
		char * exp_name = pull_str_attr (child, "_EXPORT=");
		if (exp_name && ac_object_is_visible(child))
		{
			char path[1024];
			strcpy(path, fn);
			strcat(path, "/");
			strcat(path, exp_name);
			do_obj8_save(path, child);
		}
	}
	
	myfree(fn);
}

typedef	set<Vertex *>					SurfSig;
typedef multimap<SurfSig, Surface *>	SurfMap;

static void make_surf_sig(Surface * surf, SurfSig& sig)
{
	sig.clear();
	List * iter;
	for (iter = surf->vertlist; iter != NULL; iter = iter->next)
	{
		SVertex * v = (SVertex *) iter->data;
		sig.insert(v->v);
	}
}

static void obj_accum_downface(ACObject * obj, SurfMap& down, SurfMap& up)
{
	if (!ac_object_is_visible(obj)) return;
	int numvert, numsurf, numkids;
	List *vertices, *surfaces, *kids;
	List *iter;

    ac_object_get_contents(obj, &numvert, &numsurf, &numkids,
        &vertices, &surfaces, &kids); 

	for (iter = surfaces; iter != NULL; iter = iter->next)
	{
 	 	Surface * surf = (Surface *)iter->data;
 	 	SurfSig sig;
 	 	make_surf_sig(surf, sig);
    	if (surf->normal.y < 0.0)
			down.insert(SurfMap::value_type(sig, surf));
		else
			up.insert(SurfMap::value_type(sig, surf));
	}
	
	for (iter = kids; iter != NULL; iter = iter->next)
		obj_accum_downface((ACObject *)iter->data, down, up);
	
}

void do_select_downfacing(void)
{
	ACObject * wrl = ac_get_world();
	
	SurfMap down, up;
	obj_accum_downface(wrl, down, up);
	List * bad = NULL;
	
	for (SurfMap::iterator i = down.begin(); i != down.end(); ++i)
	{
		if (up.count(i->first) > 0)
		list_add_item_head(&bad, i->second);
	}
	
	clear_selection();
	if (bad)
	{
		ac_selection_select_surfacelist(bad);
		list_free(&bad);	
	}	
}

