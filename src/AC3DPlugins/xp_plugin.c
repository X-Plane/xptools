#include "dsf_export.h"
#include "obj8_export.h"
#include "obj8_import.h"
#include "obj_tools.h"
#include <ac_plugin.h>
#include <stdio.h>
#include <gl.h>
#include <string.h>
#include "prefs.h"

/* Plugin main routines */
AC3D_PLUGIN_FUNC int AC3DPluginInit(AC3DPluginInitData *d);
AC3D_PLUGIN_FUNC int AC3DPluginExit();
AC3D_PLUGIN_FUNC char *AC3DPluginAbout();


#define CAST_CMD(x) reinterpret_cast<void*>(x)
/***************************************************************************************************
 * MAIN PLUGIN
 ***************************************************************************************************/


typedef void (ac_render_object_callback_func)(
		ACObject *		ob, 
		Boolean			is_primary_render);
		
extern "C" Prototype void	ac_set_pre_render_object_callback(ac_render_object_callback_func *func);
extern "C" Prototype void	ac_set_post_render_object_callback(ac_render_object_callback_func *func);


static int gRotate = 0;

static void pre_func(ACObject * ob, Boolean is_primary_render);
static void post_func(ACObject * ob, Boolean is_primary_render);

void pre_func(ACObject * ob, Boolean is_primary_render)
{
	glMatrixMode(GL_MODELVIEW_MATRIX);
	glPushMatrix();
	glRotatef(gRotate,0,1,0);
}

void post_func(ACObject * ob, Boolean is_primary_render)
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	if (!is_primary_render && object_parent(ob)==NULL)
		gRotate += 1;
}



AC3D_PLUGIN_FUNC int AC3DPluginInit(AC3DPluginInitData *d)
{
	prefs_init();

	/* This code is taken from an example...if we had TCL scripts to provide us with a 
	   user interface, we would use the below code to point AC3D's TCL interpreter at them. */

	char *path = (char *)strdup(d->plugin_dir_path);
	#ifdef WINDOWS
	ac_unbackslash_string(path);
	#endif
		tcl_command("source \"%s/XPlaneSupport.tcl\"", path);

//	double vers = ac_get_version_number();
//	printf("vers=%lf\n", vers);

	ac_set_pre_render_object_callback(pre_func);
	ac_set_post_render_object_callback(post_func);

	ac_register_file_exporter("OBJ7Save", ".obj", "X-Plane 7 Object File", do_obj7_save_convert, "X-Plane 7 Object File Export Plugin"); 
	ac_register_file_exporter("OBJ8Save", ".obj", "X-Plane 8 Object File", do_obj8_save, "X-Plane 8 Object File Export Plugin");
	ac_register_file_importer("OBJ8Load", ".obj", "X-Plane 8 Object File", do_obj8_load, "X-Plane 7/8 Object File Import Plugin"); 

//	ac_register_file_exporter("DSFSave", ".dsf", "X-Plane DSF (scenery) File", do_dsf_save, "X-Plane 8 DSF Export Plugin"); 
//	ac_register_file_importer("DSFLoad", ".dsf", "X-Plane DSF (scenery) File", do_dsf_load, "X-Plane 8 DSF Import Plugin"); 

//	ac_register_file_exporter("XAutoCarSave", ".car", "X-Auto Car File", do_car_save, "X-Auto Car File Export Plugin"); 
//	ac_register_file_importer("XAutoCarLoad", ".car", "X-Auto Car File", do_car_load, "X-Auto Car File Import Plugin"); 

	ac_add_tools_menu_item("Calculate X-Plane LOD", "ac3d xplane_calc_lod", "Calculate vainishing point for x-plane model.");
	ac_add_tools_menu_item("Change Texture...", "ac3d xplane_change_texture", "Substitutes a new texture.");
	ac_add_tools_menu_item("Make Transparent", "ac3d xplane_make_transparent", "Makes BMP magenta transparent.");
	ac_add_tools_menu_item("Make Night Lighting", "ac3d xplane_make_night", "Shows LIT bitmap overlays.");
	ac_add_tools_menu_item("Make Animation Group", "ac3d xplane_make_named_group \\\"ANIMATION\\\"", "Make an animation group.");
	ac_add_tools_menu_item("Make LOD Group", "ac3d xplane_make_named_group \\\"LOD 0/5000\\\"", "Make an animation group.");
	ac_add_tools_menu_item("Make Trees", "ac3d xplane_make_tree", "Make trees.");
	ac_add_tools_menu_item("Bulk Export...", "ac3d xplane_bulk_export", "Do a bulk export.");
//	ac_add_tools_menu_item("Select downward surfaces...", "ac3d xplane_select_down_surfaces", "Select downward facing surfaces...");
	ac_add_tools_menu_item("Make One-sided", "ac3d xplane_make_onesided", "Make all surfaces one-sided.");
	ac_add_tools_menu_item("Make Up Normals", "ac3d xplane_make_upnormal", "Make all Normals Go Up.");
	
	ac_add_command_full("xplane_rescale_tex", CAST_CMD(do_rescale_tex), 1, "s",
		"string of <os1> <ot1> <os2> <ot2> <ns1> <nt1> <ns2> <nt2>", "rescale selected obj texes");

	ac_add_command_full("xplane_calc_lod", CAST_CMD(do_calc_lod), 0, NULL, "ac3d xplane_change_tex", "Calculates x-plane LOD params.");
	ac_add_command_full("xplane_change_texture", CAST_CMD(do_change_tex), 0, NULL, "ac3d xplane_change_texture", "Changes textures.");
	ac_add_command_full("xplane_make_transparent", CAST_CMD(do_show_transparent), 0, NULL, "ac3d xplane_make_night", "Makes selected object's textures transparent.");
	ac_add_command_full("xplane_make_night", CAST_CMD(do_show_night), 0, NULL, "ac3d xplane_make_night", "Show an object's night lighting.");
	ac_add_command_full("xplane_make_named_group", CAST_CMD(do_named_group), 1, "s", "ac3d xplane_make_named_group <name>", "Make a named group.");
	ac_add_command_full("xplane_make_tree", CAST_CMD(do_tree_extrude), 0, NULL, "ac3d xplane_make_tree", "Make a tree from a quad.");
	ac_add_command_full("xplane_bulk_export", CAST_CMD(do_bulk_export), 0, NULL, "ac3d xplane_bulk_export", "Export many objects.");
	ac_add_command_full("xplane_make_onesided", CAST_CMD(do_make_onesided), 0, NULL, "ac3d xplane_make_onesided", "Make all surfaces one-sided.");
//	ac_add_command_full("xplane_select_down_surfaces", CAST_CMD(do_select_downfacing), 0, NULL, "ac3d xplane_select_down_surfaces", "Select down-facing surfaces.");
	ac_add_command_full("xplane_make_upnormal", CAST_CMD(do_make_upnormal), 0, NULL, "ac3d xplane_make_upnormal", "Make all normals go up.");
	return(0);
}


AC3D_PLUGIN_FUNC int AC3DPluginExit()
{
    return(0);
}

AC3D_PLUGIN_FUNC char *AC3DPluginAbout()
{
    return("OBJ8 Import/Export Plugin v2.1b1 - by Ben Supnik");
}

