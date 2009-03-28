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

#include "dsf_export.h"
#include "obj8_export.h"
#include "obj8_import.h"
#include "obj_tools.h"
#include "obj_anim.h"
#include "uv_mapper.h"
#include "obj_update.h"
#include "obj_editor.h"
#include "TclStubs.h"
#include <ac_plugin.h>
#include <stdlib.h>
#include <stdio.h>
#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif
#include <string.h>
#include "prefs.h"
#include "obj_model.h"
#include "obj_panel.h"

/* Plugin main routines */
AC3D_PLUGIN_FUNC int AC3DPluginInit(AC3DPluginInitData *d);
AC3D_PLUGIN_FUNC int AC3DPluginExit();
AC3D_PLUGIN_FUNC char *AC3DPluginAbout();


#define CAST_CMD(x) reinterpret_cast<void*>(x)
/***************************************************************************************************
 * MAIN PLUGIN
 ***************************************************************************************************/

AC3D_PLUGIN_FUNC int AC3DPluginInit(AC3DPluginInitData *d)
{
	const char * emsg = TCL_init_stubs();
	if(emsg)
	{
		message_dialog("X-Plane export plugin cannot load: %s", emsg);
		return -1;
	}
	prefs_init();

	/* This code is taken from an example...if we had TCL scripts to provide us with a
	   user interface, we would use the below code to point AC3D's TCL interpreter at them. */

	register_panel_vars();

	char *path = (char *)strdup(d->plugin_dir_path);
	#ifdef WINDOWS
	ac_unbackslash_string(path);
	#endif
		tcl_command("source \"%s/XPlaneSupport.tcl\"", path);

//	double vers = ac_get_version_number();
//	printf("vers=%lf\n", vers);


	ac_register_file_exporter("OBJ7Save", ".obj", "X-Plane 7 Object File", do_obj7_save_convert, "X-Plane 7 Object File Export Plugin");
	ac_register_file_exporter("OBJ8Save", ".obj", "X-Plane 8 Object File", do_obj8_save, "X-Plane 8 Object File Export Plugin");
#if PHONE
	ac_register_file_exporter("OBJeSave", ".obe", "X-Plane 8 Embedded Object File", do_obje_save, "X-Plane 8 Embedded Object File Export Plugin");
#endif	
	ac_register_file_importer("OBJ8Load", ".obj", "X-Plane 8 Object File", do_obj8_load, "X-Plane 7/8 Object File Import Plugin"); 

//	ac_register_file_exporter("DSFSave", ".dsf", "X-Plane DSF (scenery) File", do_dsf_save, "X-Plane 8 DSF Export Plugin");
//	ac_register_file_importer("DSFLoad", ".dsf", "X-Plane DSF (scenery) File", do_dsf_load, "X-Plane 8 DSF Import Plugin");

//	ac_register_file_exporter("XAutoCarSave", ".car", "X-Auto Car File", do_car_save, "X-Auto Car File Export Plugin");
//	ac_register_file_importer("XAutoCarLoad", ".car", "X-Auto Car File", do_car_load, "X-Auto Car File Import Plugin");

	ac_add_command_full("xplane_rescale_tex", CAST_CMD(do_rescale_tex), 1, "s", "string of <os1> <ot1> <os2> <ot2> <ns1> <nt1> <ns2> <nt2>", "rescale selected obj texes");
	ac_add_command_full("xplane_select_tex", CAST_CMD(do_select_tex), 1, "s", "string of <s1> <t1> <s2> <t2>", "select surface by tex coodrs");

	ac_add_command_full("xplane_calc_lod", CAST_CMD(do_calc_lod), 0, NULL, "ac3d xplane_change_tex", "Calculates x-plane LOD params.");
	ac_add_command_full("xplane_change_texture", CAST_CMD(do_change_tex), 0, NULL, "ac3d xplane_change_texture", "Changes textures.");
	ac_add_command_full("xplane_make_transparent", CAST_CMD(do_show_transparent), 0, NULL, "ac3d xplane_make_night", "Makes selected object's textures transparent.");
	ac_add_command_full("xplane_make_night", CAST_CMD(do_show_night), 0, NULL, "ac3d xplane_make_night", "Show an object's night lighting.");
	ac_add_command_full("xplane_make_named_group", CAST_CMD(do_named_group), 1, "s", "ac3d xplane_make_named_group <name>", "Make a named group.");
	ac_add_command_full("xplane_make_tree", CAST_CMD(do_tree_extrude), 0, NULL, "ac3d xplane_make_tree", "Make a tree from a quad.");
	ac_add_command_full("xplane_bulk_export", CAST_CMD(do_bulk_export), 0, NULL, "ac3d xplane_bulk_export", "Export many objects by object name.");
	ac_add_command_full("xplane_tex_export", CAST_CMD(do_tex_export), 0, NULL, "ac3d xplane_tex_export", "Export many objects by texture.");
	ac_add_command_full("xplane_make_onesided", CAST_CMD(do_make_onesided), 0, NULL, "ac3d xplane_make_onesided", "Make all surfaces one-sided.");
	ac_add_command_full("xplane_make_upnormal", CAST_CMD(do_make_upnormal), 0, NULL, "ac3d xplane_make_upnormal", "Make all normals go up.");
//	ac_add_command_full("xplane_select_down_surfaces", CAST_CMD(do_select_downfacing), 0, NULL, "ac3d xplane_select_down_surfaces", "Select down-facing surfaces.");

	ac_add_command_full("xplane_do_uvmap", CAST_CMD(do_uv_map), 0, NULL, "ac3d xplane_do_uvmap", "Make a UV map.");
	ac_add_command_full("xplane_reload_texes", CAST_CMD(do_reload_all_texes), 0, NULL, "ac3d xplane_reload_texes", "Reload all texes.");
	ac_add_command_full("xplane_uv_copy", CAST_CMD(do_uv_copy), 0, NULL, "ac3d xplane_uv_copy", "Copy UV map from an object.");
	ac_add_command_full("xplane_uv_paste", CAST_CMD(do_uv_paste), 0, NULL, "ac3d xplane_uv_paste", "Project UV map onto a new object.");

	ac_add_command_full("xplane_sel_lights", CAST_CMD(do_sel_lights), 0, NULL, "ac3d xplane_sel_lights", "Select lights.");

//	ac_add_command_full("xplane_make_subpanel", CAST_CMD(do_make_panel_subtexes), 0, NULL, "ac3d xplane_make_subpanel", "Make sub-panel texes from main panel.");

	ac_add_command_full("xplane_optimize_selection", CAST_CMD(do_optimize_selection), 1, "f", "ac3d xplane_optimize_selection", "Optimize selection for x-plane.");

	setup_obj_anim();
	register_updater();
	OBJ_register_datamodel_tcl_cmds();
	OBJ_editor_init();
	return(0);
}


AC3D_PLUGIN_FUNC int AC3DPluginExit()
{
    return(0);
}

AC3D_PLUGIN_FUNC char *AC3DPluginAbout()
{
    return("OBJ8 Import/Export Plugin v3.2a1 - by Ben Supnik");
}

