#include "obj_tools.h"
#include "ac_utils.h"
#include "bitmap_match.h"
#include "obj_radius.h"
#include "obj_export.h"
#include "XObjDefs.h"

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
