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

/** OBJ7 exporter for AC3D **/

/*

ANDY SEZ ABOUT MATERIALS:
There's:

Prototype int ac_palette_get_new_material_index(ACMaterialTemplate *m)
 if this material exists then return it's index
    otherwise, allocate a new one, copy the contents from m
    and return it's index
This calls:

Prototype Boolean material_compare(ACMaterial *m, ACMaterialTemplate *m2)

That checks each part of the material.  So- you'll get a whole new material
if there's the slightest difference.

If you only need RGB, use:

Prototype long rgb_to_index(long rgbcol)

This only checks the rgb of existing materials - not all the other
attributes.


*/

#include "TclStubs.h"
#include "ac_plugin.h"
#include "undoable.h"

#ifdef Boolean
#undef Boolean
#endif

#include "obj8_import.h"
#include "ac_utils.h"

#include "XObjDefs.h"
#include "XObjReadWrite.h"
#include "ObjConvert.h"
#include "obj_model.h"
#include "obj_anim.h"
#include "obj_panel.h"

#include <list>
using std::list;


// Set this to 1 and each change in culling or flat/smooth shading pops another object.
#define	OBJ_FOR_ALL_ATTRS	0



/***************************************************************************************************
 * OBJ8 IMPORT AND EXPORT
 ***************************************************************************************************/


ACObject *	do_obj8_load(char *filename)
{
		ACObject * 	group_obj = NULL;
		ACObject * 	lod_obj = NULL;
		ACObject * 	stuff_obj = NULL;
		list<ACObject *>	anim_obj;
		char *		tex_full_name = NULL;
		int			tex_id = -1;
		char *		panel_full_name = NULL;
		int			panel_id = -1;
		int			panel_reg_id[4] = { -1, -1, -1, -1 };

//		char		anim_cmd[1024];
		string		anim_dat;
//		char *		anim_old_cmd;

//		Point3		key1, key2;
					int i;

		material_template_t	current_material;
		current_material.rgb.r = 1.0;
		current_material.rgb.g = 1.0;
		current_material.rgb.b = 1.0;

		current_material.ambient.r = 0.2;
		current_material.ambient.g = 0.2;
		current_material.ambient.b = 0.2;

		current_material.specular.r = 0.0;
		current_material.specular.g = 0.0;
		current_material.specular.b = 0.0;

		current_material.emissive.r = 0.0;
		current_material.emissive.g = 0.0;
		current_material.emissive.b = 0.0;

		current_material.shininess = 128;
		current_material.transparency = 0.0;
		
		int default_material = ac_palette_get_new_material_index(&current_material);
		
	Point3	p3;
	char	strbuf[256];
	
//	obj8.lods.clear();
	XObj8	obj8;
	if (!XObj8Read(filename, obj8))
	{
		XObj	obj7;

		if (!XObjRead(filename, obj7))
			return NULL;
	
		Obj7ToObj8(obj7, obj8);
	} 

	set_std_panel();
	for (int n = 0; n < obj8.regions.size(); ++n)
	{
		add_sub_panel(obj8.regions[n].left,obj8.regions[n].bottom,obj8.regions[n].right,obj8.regions[n].top);
	}
		
    group_obj = new_object(OBJECT_GROUP);	
    
    string	fname(filename);
    string::size_type p = fname.find_last_of("\\/");	
    string justName = (p == fname.npos) ? fname : fname.substr(p+1);
    string justPath = fname.substr(0,p+1);
	string::size_type p2 = obj8.texture.find_last_of("\\/:");
	string texName = (p2 == obj8.texture.npos) ? obj8.texture : obj8.texture.substr(p2+1);    
	if (texName.size() > 4)
		texName.erase(texName.size()-4);
	string texPath;
	if(p2 != obj8.texture.npos) texPath = obj8.texture.substr(0,p2+1);
    string texNameBmp = texName + ".bmp";
    string texNamePng = texName + ".png";    
    string texNameDds = texName + ".dds";   
	string texPathBmp = texPath + texNameBmp; 
	string texPathPng = texPath + texNamePng; 
	string texPathDds = texPath + texNameDds; 
    string panelNamePng = "cockpit/-PANELS-/panel.png";
    string panelNameBmp = "cockpit/-PANELS-/panel.bmp";
        
    bool	has_cockpit_cmd = false;
	bool	has_cockpit_reg = false;
    for(vector<XObjLOD8>::iterator lod = obj8.lods.begin(); lod != obj8.lods.end(); ++lod)
	for(vector<XObjCmd8>::iterator cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
	{
    	if (cmd->cmd == attr_Tex_Cockpit)    		has_cockpit_cmd = true;
		if (cmd->cmd == attr_Tex_Cockpit_Subregion)	has_cockpit_reg = true;
    }
    object_set_name(group_obj,(char *) justName.c_str());
    
    if (!texName.empty())
    {
		if (tex_id == -1)			tex_full_name = search_texture(filename, (char *) texNamePng.c_str());
		if (tex_full_name != NULL)	tex_id = add_new_texture_opt(tex_full_name,tex_full_name);
		if (tex_id == -1)			tex_full_name = search_texture(filename, (char *) texNameBmp.c_str());
		if (tex_full_name != NULL)	tex_id = add_new_texture_opt(tex_full_name,tex_full_name);
		if (tex_id == -1)			tex_full_name = search_texture(filename, (char *) texNameDds.c_str());
		if (tex_full_name != NULL)	tex_id = add_new_texture_opt(tex_full_name,tex_full_name);

		if (tex_id == -1)			tex_full_name = search_texture(filename, (char *) texPathPng.c_str());
		if (tex_full_name != NULL)	tex_id = add_new_texture_opt(tex_full_name,tex_full_name);
		if (tex_id == -1)			tex_full_name = search_texture(filename, (char *) texPathBmp.c_str());
		if (tex_full_name != NULL)	tex_id = add_new_texture_opt(tex_full_name,tex_full_name);
		if (tex_id == -1)			tex_full_name = search_texture(filename, (char *) texPathDds.c_str());
		if (tex_full_name != NULL)	tex_id = add_new_texture_opt(tex_full_name,tex_full_name);

	}	
	if (has_cockpit_cmd || has_cockpit_reg)
	{
		printf("Trying cockpit cmds.\n");
		printf("panel name = %s\n", panelNamePng.c_str());
		if (panel_id == -1)			panel_full_name = search_texture(filename, (char *) panelNamePng.c_str());
		printf("Panel full name %s\n", panel_full_name);		
		if (panel_full_name != NULL)panel_id = add_new_texture_opt(panel_full_name,panel_full_name);
		printf("tex id = %d\n", panel_id);
		if (panel_id == -1)			panel_full_name = search_texture(filename, (char *) panelNameBmp.c_str());
		if (panel_full_name != NULL)panel_id = add_new_texture_opt(panel_full_name,panel_full_name);
	}
	
	if (has_cockpit_reg && panel_id != -1)
	{	
		do_make_panel_subtexes_auto(panel_id,panel_reg_id);
	}
        
    for(vector<XObjLOD8>::iterator lod = obj8.lods.begin(); lod != obj8.lods.end(); ++lod)
    {
    	lod_obj = new_object(OBJECT_GROUP);
    	if (lod->lod_far != 0.0)
			sprintf(strbuf, "LOD %f/%f",lod->lod_near, lod->lod_far);
		else
			strcpy(strbuf, "Default LOD");
		object_set_name(lod_obj, strbuf);
		if (lod->lod_far != 0.0)
		{
			OBJ_set_LOD_near(lod_obj, lod->lod_near);
			OBJ_set_LOD_far (lod_obj, lod->lod_far );
		}

		object_add_child(group_obj, lod_obj);
    	
    	bool	shade_flat = false;
    	bool	two_side = false;
    	int		panel_tex = tex_id;

    	float	no_blend = -1.0;
    	string	hard_poly;
		int		deck = 0;
    	float	offset = 0;
    	
		for(vector<XObjCmd8>::iterator cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
		{
			switch(cmd->cmd) {
			case obj8_Tris:			
			case obj8_Lines:
				if (stuff_obj == NULL)
				{
					stuff_obj = new_object(OBJECT_NORMAL);
					if (panel_tex != -1)object_texture_set(stuff_obj, panel_tex);

					object_add_child(anim_obj.empty() ? lod_obj : anim_obj.back(), stuff_obj);
					OBJ_set_poly_os(stuff_obj, offset);
					OBJ_set_blend(stuff_obj, no_blend);
					OBJ_set_hard(stuff_obj, hard_poly.c_str());
					OBJ_set_deck(stuff_obj,deck);
					sprintf(strbuf, "POLY_OS=%d HARD=%s BLEND=%s",
						(int) offset, hard_poly.c_str(), no_blend >= 0.0 ? "no":"yes");
					object_set_name(stuff_obj, strbuf);
				}

				if (cmd->cmd == obj8_Tris)
				{
					vector<Vertex *> verts;
					for (i = 0; i < cmd->idx_count; ++i)
					{
						float * dat = obj8.geo_tri.get(obj8.indices[cmd->idx_offset + i]);
						p3.x = dat[0];
						p3.y = dat[1];
						p3.z = dat[2];
						verts.push_back(object_add_new_vertex_head(stuff_obj, &p3));
					}
					
			        Surface * s = NULL;
					for (i = 0; i < cmd->idx_count; ++i)
					{
						if ((i % 3) == 0) 
						{
							s = new_surface();
						    surface_set_type(s, SURFACE_POLYGON);
						    surface_set_twosided(s, two_side);
						    surface_set_shading(s, !shade_flat);
					        object_add_surface_head(stuff_obj, s);
							
							int our_material = ac_palette_get_new_material_index(&current_material);
							surface_set_col(s, our_material);
							if (our_material != default_material)
								OBJ_set_use_materials(stuff_obj, 1);
						}
						float * dat = obj8.geo_tri.get(obj8.indices[cmd->idx_offset + i]);					
				        surface_add_vertex_head(s, verts[i], dat[6], dat[7]);
				    }

				} else {

					vector<Vertex *> verts;
					int i;
					for (i = 0; i < cmd->idx_count; ++i)
					{
						float * dat = obj8.geo_lines.get(obj8.indices[cmd->idx_offset + i]);
						p3.x = dat[0];
						p3.y = dat[1];
						p3.z = dat[2];
						verts.push_back(object_add_new_vertex_head(stuff_obj, &p3));
					}
					
			        Surface * s = NULL;
					for (i = 0; i < cmd->idx_count; ++i)
					{
						float * dat = obj8.geo_lines.get(obj8.indices[cmd->idx_offset + i]);
						if ((i % 2) == 0) 
						{
							s = new_surface();
							surface_set_rgb_long(s, rgb_floats_to_long(dat[3], dat[4], dat[5]));							
						    surface_set_type(s, SURFACE_LINE);
						    surface_set_twosided(s, two_side);
						    surface_set_shading(s, !shade_flat);
					        object_add_surface_head(stuff_obj, s);				
						}
				        surface_add_vertex_head(s, verts[i], 0.0, 0.0);
				    }

				}
				break;
			
			case attr_Tex_Normal:
				if (panel_tex != tex_id)	stuff_obj = NULL;
				panel_tex = tex_id;
				break;
			case attr_Tex_Cockpit:
				if (panel_tex != panel_id)	stuff_obj = NULL;
				panel_tex = panel_id;
				break;
			case attr_Tex_Cockpit_Subregion:
				if (panel_tex != panel_reg_id[(int) cmd->params[0]])	stuff_obj = NULL;
				panel_tex = panel_reg_id[(int) cmd->params[0]];
				break;
			case attr_No_Blend:
				if (!no_blend != cmd->params[0]) stuff_obj = NULL;
				no_blend = cmd->params[0];
				break;
			case attr_Blend:
				if (no_blend != -1.0) stuff_obj = NULL;
				no_blend = -1.0;
				break;
			case attr_Hard:
				if (hard_poly != cmd->name) stuff_obj = NULL;
				if(deck == 1) stuff_obj = NULL;
				hard_poly = cmd->name;
				deck = 0;
				break;
			case attr_Hard_Deck:
				if (hard_poly != cmd->name) stuff_obj = NULL;
				if(deck == 0) stuff_obj = NULL;
				hard_poly = cmd->name;
				deck = 1;
				break;
			case attr_No_Hard:
				if (!hard_poly.empty()) stuff_obj = NULL;
				hard_poly.clear();
				deck = 0;
				break;				
			case attr_Offset:
				if (offset != cmd->params[0]) stuff_obj = NULL;
				offset = cmd->params[0];
				break;
			case attr_Shade_Flat:		
#if OBJ_FOR_ALL_ATTRS
				if (!shade_flat)	stuff_obj = NULL;
#endif				
				shade_flat = true;	
				break;
			case attr_Shade_Smooth:		
#if OBJ_FOR_ALL_ATTRS
				if (shade_flat)	stuff_obj = NULL;
#endif				
				shade_flat = false;	
				break;
			case attr_Cull:	
#if OBJ_FOR_ALL_ATTRS
				if (two_side)	stuff_obj = NULL;
#endif							
				two_side = false;	
				break;
			case attr_NoCull:			
#if OBJ_FOR_ALL_ATTRS
				if (!two_side)	stuff_obj = NULL;
#endif							
				two_side = true;	
				break;
			
			case attr_Layer_Group:
				OBJ_set_layer_group(group_obj, cmd->name.c_str());
				OBJ_set_layer_group_offset(group_obj, cmd->params[0]);
				break;
						
			case anim_Begin:
				{
					stuff_obj = NULL;
					ACObject * parent = anim_obj.empty() ? lod_obj : anim_obj.back();
					anim_obj.push_back(new_object(OBJECT_GROUP));
					object_add_child(parent, anim_obj.back());
					object_set_name(anim_obj.back(), "ANIMATION");
					OBJ_set_animation_group(anim_obj.back(),1);

					}
				break;
			case anim_End:
				stuff_obj = NULL;
				anim_obj.pop_back();
				break;
			case anim_Translate:
				{
					int root = 0;
					if (obj8.animation[cmd->idx_offset].keyframes[root].v[0] != 0.0 ||
						obj8.animation[cmd->idx_offset].keyframes[root].v[1] != 0.0 ||
						obj8.animation[cmd->idx_offset].keyframes[root].v[2] != 0.0)
						
						anim_add_static(anim_obj.back(), 0, obj8.animation[cmd->idx_offset].keyframes[root].v, obj8.animation[cmd->idx_offset].dataref.c_str(), "static translate");
				
					if (obj8.animation[cmd->idx_offset].keyframes.size() > 2 || 
						(obj8.animation[cmd->idx_offset].keyframes.size() == 2 && (
							obj8.animation[cmd->idx_offset].keyframes[0].v[0] != obj8.animation[cmd->idx_offset].keyframes[1].v[0] ||
							obj8.animation[cmd->idx_offset].keyframes[0].v[1] != obj8.animation[cmd->idx_offset].keyframes[1].v[1] ||
							obj8.animation[cmd->idx_offset].keyframes[0].v[2] != obj8.animation[cmd->idx_offset].keyframes[1].v[2])))
						anim_add_translate(anim_obj.back(), 0, obj8.animation[cmd->idx_offset].keyframes, obj8.animation[cmd->idx_offset].dataref.c_str(), "translate");
				}
				break;	
			case anim_Rotate:
				{
					float center[3] = { 0.0, 0.0, 0.0 };

					anim_add_rotate(anim_obj.back(), 0, center, obj8.animation[cmd->idx_offset].axis, 
										obj8.animation[cmd->idx_offset].keyframes,
										obj8.animation[cmd->idx_offset].dataref.c_str(), "rotate");
				}
				break;
			case anim_Show:
				anim_add_show(anim_obj.back(), 0,
									obj8.animation[cmd->idx_offset].keyframes,
									obj8.animation[cmd->idx_offset].dataref.c_str(), "show");
				break;
			case anim_Hide:
				anim_add_hide(anim_obj.back(), 0,
									obj8.animation[cmd->idx_offset].keyframes,
									obj8.animation[cmd->idx_offset].dataref.c_str(), "show");
				break;
			case obj8_Lights:			
				for (i = 0; i < cmd->idx_count; ++i)
				{
					stuff_obj = NULL;
					ACObject * light = new_object(OBJECT_LIGHT);
					Point3	pt_ac3, col_ac3 = { 0.0, 0.0, 0.0 };
					float * dat = obj8.geo_lights.get(cmd->idx_offset + i);
					pt_ac3.x = dat[0];
					pt_ac3.y = dat[1];
					pt_ac3.z = dat[2];
					ac_entity_set_point_value(light, "loc", &pt_ac3);
					
					sprintf(strbuf, "RGB (%f,%f,%f)", dat[3], dat[4], dat[5]);
					ac_entity_set_point_value(light, "diffuse", &col_ac3);
					object_set_name(light, strbuf);
					object_add_child(anim_obj.empty() ? lod_obj : anim_obj.back(), light);
					OBJ_set_light_red(light, dat[3]);
					OBJ_set_light_green(light, dat[4]);
					OBJ_set_light_blue(light, dat[5]);
					OBJ_set_light_named(light, "rgb");
				}
				break;
			case obj8_LightNamed:
				{
					stuff_obj = NULL;
					ACObject * light = new_object(OBJECT_LIGHT);
					Point3	pt_ac3, col_ac3 = { 0.0, 0.0, 0.0 };
					pt_ac3.x = cmd->params[0];
					pt_ac3.y = cmd->params[1];
					pt_ac3.z = cmd->params[2];
					ac_entity_set_point_value(light, "loc", &pt_ac3);					
					ac_entity_set_point_value(light, "diffuse", &col_ac3);
					object_set_name(light, (char*) cmd->name.c_str());
					object_add_child(anim_obj.empty() ? lod_obj : anim_obj.back(), light);				
					OBJ_set_light_named(light, cmd->name.c_str());
				}				
				break;				
			case obj8_LightCustom:
				{
					stuff_obj = NULL;
					ACObject * light = new_object(OBJECT_LIGHT);
					Point3	pt_ac3, col_ac3 = { 0.0, 0.0, 0.0 };
					pt_ac3.x = cmd->params[0];
					pt_ac3.y = cmd->params[1];
					pt_ac3.z = cmd->params[2];
					ac_entity_set_point_value(light, "loc", &pt_ac3);
					ac_entity_set_point_value(light, "diffuse", &col_ac3);
					object_add_child(anim_obj.empty() ? lod_obj : anim_obj.back(), light);				
					OBJ_set_light_named(light, "custom");
					OBJ_set_light_dataref(light, cmd->name.c_str());
					OBJ_set_light_red	(light,cmd->params[3]);
					OBJ_set_light_green	(light,cmd->params[4]);
					OBJ_set_light_blue	(light,cmd->params[5]);
					OBJ_set_light_alpha	(light,cmd->params[6]);
					OBJ_set_light_size	(light,cmd->params[7]);
					OBJ_set_light_s1	(light,cmd->params[8]);
					OBJ_set_light_t1	(light,cmd->params[9]);
					OBJ_set_light_s2	(light,cmd->params[10]);
					OBJ_set_light_t2	(light,cmd->params[11]);
				}				
				break;
			case obj_Smoke_Black:
				{
					stuff_obj = NULL;
					ACObject * light = new_object(OBJECT_LIGHT);
					Point3	pt_ac3, col_ac3 = { 0.0, 0.0, 0.0 };
					pt_ac3.x = cmd->params[0];
					pt_ac3.y = cmd->params[1];
					pt_ac3.z = cmd->params[2];
					ac_entity_set_point_value(light, "loc", &pt_ac3);
					ac_entity_set_point_value(light, "diffuse", &col_ac3);					
					object_set_name(light, "Black Smoke");
					object_add_child(anim_obj.empty() ? lod_obj : anim_obj.back(), light);									
					OBJ_set_light_smoke_size(light, cmd->params[3]);
					OBJ_set_light_named(light, "black smoke");
				}
				break;
			case obj_Smoke_White:
				{
					stuff_obj = NULL;
					ACObject * light = new_object(OBJECT_LIGHT);
					Point3	pt_ac3, col_ac3 = { 0.0, 0.0, 0.0 };
					pt_ac3.x = cmd->params[0];
					pt_ac3.y = cmd->params[1];
					pt_ac3.z = cmd->params[2];
					ac_entity_set_point_value(light, "loc", &pt_ac3);
					ac_entity_set_point_value(light, "diffuse", &col_ac3);					
					object_set_name(light, "White Smoke");
					object_add_child(anim_obj.empty() ? lod_obj : anim_obj.back(), light);									
					OBJ_set_light_smoke_size(light, cmd->params[3]);
					OBJ_set_light_named(light, "white smoke");
				}
				break;
			case attr_Ambient_RGB:	
				break;
			case attr_Diffuse_RGB:
				current_material.rgb.r = cmd->params[0];
				current_material.rgb.g = cmd->params[1];
				current_material.rgb.b = cmd->params[2];
				break;
			case attr_Emission_RGB:
				current_material.emissive.r = cmd->params[0];
				current_material.emissive.g = cmd->params[1];
				current_material.emissive.b = cmd->params[2];
				break;
			case attr_Specular_RGB:
				break;
			case attr_Shiny_Rat:
				current_material.specular.r = cmd->params[0];
				current_material.specular.g = cmd->params[0];
				current_material.specular.b = cmd->params[0];
				break;
			case attr_Reset:
				current_material.rgb.r = 1.0;
				current_material.rgb.g = 1.0;
				current_material.rgb.b = 1.0;

				current_material.specular.r = 0.0;
				current_material.specular.g = 0.0;
				current_material.specular.b = 0.0;

				current_material.emissive.r = 0.0;
				current_material.emissive.g = 0.0;
				current_material.emissive.b = 0.0;				
				break;
			}	
		}
	}
    
	object_calc_normals_force(group_obj);

	bake_static_transitions(group_obj);
	purge_datarefs();
	gather_datarefs(group_obj);
	sync_datarefs();

    return group_obj;
}
