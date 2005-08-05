#include "obj_export.h"
#include "obj_tools.h"
#include <ac_plugin.h>

/* Plugin main routines */
AC3D_PLUGIN_FUNC int AC3DPluginInit(AC3DPluginInitData *d);
AC3D_PLUGIN_FUNC int AC3DPluginExit();
AC3D_PLUGIN_FUNC char *AC3DPluginAbout();


/***************************************************************************************************
 * MAIN PLUGINJ
 ***************************************************************************************************/



AC3D_PLUGIN_FUNC int AC3DPluginInit(AC3DPluginInitData *d)
{
	/* This code is taken from an example...if we had TCL scripts to provide us with a 
	   user interface, we would use the below code to point AC3D's TCL interpreter at them. */

/*
char *path = (char *)strdup(d->plugin_dir_path);
#ifdef WINDOWS
ac_unbackslash_string(path);
#endif
    tcl_command("source \"%s/XPlaneSupport.tcl\"", path);
*/    

	double vers = ac_get_version_number();
//	printf("vers=%lf\n", vers);

	ac_register_file_exporter("OBJ7Save", ".obj", "X-Plane 7 Object File", do_obj7_save, "X-Plane 7 Object File Export Plugin"); 
	ac_register_file_importer("OBJ7Load", ".obj", "X-Plane 7 Object File", do_obj7_load, "X-Plane 7 Object File Import Plugin"); 

//	ac_register_file_exporter("XAutoCarSave", ".car", "X-Auto Car File", do_car_save, "X-Auto Car File Export Plugin"); 
//	ac_register_file_importer("XAutoCarLoad", ".car", "X-Auto Car File", do_car_load, "X-Auto Car File Import Plugin"); 



	ac_add_tools_menu_item("Calculate X-Plane LOD", "ac3d xplane_calc_lod", "Calculate vainishing point for x-plane model.");
	ac_add_tools_menu_item("Change Texture...", "ac3d xplane_change_texture", "Substitutes a new texture.");
	ac_add_tools_menu_item("Make Transparent", "ac3d xplane_make_transparent", "Makes BMP magenta transparent.");
	ac_add_tools_menu_item("Make Night Lighting", "ac3d xplane_make_night", "Shows LIT bitmap overlays.");


	ac_add_command_full("xplane_calc_lod", do_calc_lod, 0, NULL, "ac3d xplane_change_tex", "Calculates x-plane LOD params.");
	ac_add_command_full("xplane_change_texture", do_change_tex, 0, NULL, "ac3d xplane_change_texture", "Changes textures.");
	ac_add_command_full("xplane_make_transparent", do_show_transparent, 0, NULL, "ac3d xplane_make_night", "Makes selected object's textures transparent.");
	ac_add_command_full("xplane_make_night", do_show_night, 0, NULL, "ac3d xplane_make_night", "Show an object's night lighting.");

	return(0);
}


AC3D_PLUGIN_FUNC int AC3DPluginExit()
{
    return(0);
}

AC3D_PLUGIN_FUNC char *AC3DPluginAbout()
{
    return("OBJ7 Import/Export Plugin V1.1b2 - by Ben Supnik");
}

