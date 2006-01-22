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

#include "ac_plugin.h"
#include "undoable.h"

#ifdef Boolean
#undef Boolean
#endif

#define BONES 0

#include "obj8_import.h"
#include "ac_utils.h"

#include "XObjDefs.h"
#include "XObjReadWrite.h"
#include "ObjConvert.h"

#include <list>
using std::list;


// Set this to 1 and each change in culling or flat/smooth shading pops another object.
#define	OBJ_FOR_ALL_ATTRS	0
// Set this 1 to provide _DEPTH attribute to support depth
#define SUPPORT_NO_DEPTH 0


#if BONES

extern "C"
{
Prototype ACJoint *ac_new_joint();

Prototype void ac_object_set_local_position(ACObject *b, Point3 *p);
Prototype void ac_object_set_local_rotation(ACObject *b, Point3 *p);
Prototype void ac_object_init_transform(ACObject *ob); // call this after setting local pos/rot
Prototype void ac_object_add_position_key(ACObject *b, float ftime, Point3 *p);
Prototype void ac_object_add_rotation_key(ACObject *b, float ftime, Point3 *p);

Prototype void animator_set_max_time(float t);

Prototype void ac_vertex_set_joint(Vertex *v, int b);
Prototype int ac_vertex_get_joint(Vertex *v);

Prototype void ac_object_init_transform(ACObject *ob); // create the animation matrix from local pos and rot
Prototype void ac_object_init_transform_recurse(ACObject *ob); // create the animation matrix from local pos and rot
}

#endif


/***************************************************************************************************
 * OBJ8 IMPORT AND EXPORT
 ***************************************************************************************************/


ACObject *	do_obj8_load(char *filename)
{
#if BONES
		animator_set_max_time(60.0);
#endif

		ACObject * 	group_obj = NULL;
		ACObject * 	lod_obj = NULL;
		ACObject * 	stuff_obj = NULL;
		list<ACObject *>	anim_obj;
#if BONES
		list<ACJoint *>		anim_joint;
		vector<ACJoint *>	top_joints;
#endif
		
		char *		tex_full_name = NULL;
		int			tex_id = -1;
		char *		panel_full_name = NULL;
		int			panel_id = -1;

		char		anim_cmd[1024];
		string		anim_dat;
		char *		anim_old_cmd;

		Point3		key1, key2;
					int i;

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
		
    group_obj = new_object(OBJECT_NORMAL);	
    
    string	fname(filename);
    string::size_type p = fname.find_last_of("\\/");	
    string justName = (p == fname.npos) ? fname : fname.substr(p+1);
    string justPath = fname.substr(0,p+1);
	string::size_type p2 = obj8.texture.find_last_of("\\/:");
	string texName = (p2 == obj8.texture.npos) ? obj8.texture : obj8.texture.substr(p2+1);    
	if (texName.size() > 4)
		texName.erase(texName.size()-4);
    string texNameBmp = texName + ".bmp";
    string texNamePng = texName + ".png";    
    string panelNamePng = "cockpit/-PANELS-/panel.png";
    string panelNameBmp = "cockpit/-PANELS-/panel.bmp";
        
    bool	has_cockpit_cmd = false;
    for(vector<XObjLOD8>::iterator lod = obj8.lods.begin(); lod != obj8.lods.end(); ++lod)
	for(vector<XObjCmd8>::iterator cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
	{
    	if (cmd->cmd == attr_Tex_Cockpit)
    	{
    		has_cockpit_cmd = true;
    		break;
    	}
    }
    object_set_name(group_obj,(char *) justName.c_str());
    
    if (!texName.empty())
    {
    	printf("Pgn tex path = %s\n", texNamePng.c_str());
		if (tex_id == -1)			tex_full_name = search_texture(filename, (char *) texNamePng.c_str());
		printf("full search = %s\n", tex_full_name ? tex_full_name : NULL);
		if (tex_full_name != NULL)	tex_id = add_new_texture_opt(tex_full_name,tex_full_name);
		printf("Tex id = %d\n", tex_id);
		if (tex_id == -1)			tex_full_name = search_texture(filename, (char *) texNameBmp.c_str());
		if (tex_full_name != NULL)	tex_id = add_new_texture_opt(tex_full_name,tex_full_name);
	}	
	if (has_cockpit_cmd)
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
        
    for(vector<XObjLOD8>::iterator lod = obj8.lods.begin(); lod != obj8.lods.end(); ++lod)
    {
    	lod_obj = new_object(OBJECT_NORMAL);
    	if (lod->lod_far != 0.0)
			sprintf(strbuf, "LOD %f/%f",lod->lod_near, lod->lod_far);
		else
			strcpy(strbuf, "Default LOD");
		object_set_name(lod_obj, strbuf);

		object_add_child(group_obj, lod_obj);
    	
    	bool	shade_flat = false;
    	bool	two_side = false;
    	bool	panel_tex = false;

    	bool	no_blend = false;
    	bool	hard_poly = false;
#if SUPPORT_NO_DEPTH
    	bool	no_depth = false;
#endif    	
    	float	offset = 0;
    	
		for(vector<XObjCmd8>::iterator cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
		{
			switch(cmd->cmd) {
			case obj8_Tris:			
			case obj8_Lines:
				if (stuff_obj == NULL)
				{
					stuff_obj = new_object(OBJECT_NORMAL);
					if (panel_tex)
					{
						if (panel_id != -1) object_texture_set(stuff_obj, panel_id);
					} else 
					{
						if (tex_id != -1) object_texture_set(stuff_obj, tex_id);
					}
					object_add_child(anim_obj.empty() ? lod_obj : anim_obj.back(), stuff_obj);
#if SUPPORT_NO_DEPTH
					sprintf(strbuf, "_POLY_OS=%d _HARD=%d _BLEND=%d _DEPTH=%d",
						(int) offset, hard_poly ? 1 : 0, no_blend ? 0 : 1, no_depth ? 0 : 1);
#else
					sprintf(strbuf, "_POLY_OS=%d _HARD=%d _BLEND=%d",
						(int) offset, hard_poly ? 1 : 0, no_blend ? 0 : 1);
#endif						
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
#if BONES						
						if (!anim_joint.empty())
							ac_vertex_set_joint(verts.back(), (int) anim_joint.back());
#endif							
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
#if BONES						
						if (!anim_joint.empty())
							ac_vertex_set_joint(verts.back(), (int) anim_joint.back());
#endif							
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
				if (panel_tex)	stuff_obj = NULL;
				panel_tex = false;
				break;
			case attr_Tex_Cockpit:
				if (!panel_tex)	stuff_obj = NULL;
				panel_tex = true;
				break;
			case attr_No_Blend:
				if (!no_blend) stuff_obj = NULL;
				no_blend = true;
				break;
			case attr_Blend:
				if (no_blend) stuff_obj = NULL;
				no_blend = false;
				break;
			case attr_Hard:
				if (!hard_poly) stuff_obj = NULL;
				hard_poly = true;
				break;
			case attr_No_Hard:
				if (hard_poly) stuff_obj = NULL;
				hard_poly = false;
				break;
			case attr_Offset:
				if (offset != cmd->params[0]) stuff_obj = NULL;
				offset = cmd->params[0];
				break;
#if SUPPORT_NO_DEPTH
			case attr_No_Depth:			
				if (!no_depth) stuff_obj = NULL;
				no_depth = true;
				break;
			case attr_Depth:
				if (no_depth) stuff_obj = NULL;
				no_depth = false;
				break;
#endif				
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
				
			case anim_Begin:
				{
					stuff_obj = NULL;
					ACObject * parent = anim_obj.empty() ? lod_obj : anim_obj.back();
					anim_obj.push_back(new_object(OBJECT_NORMAL));
					object_add_child(parent, anim_obj.back());
					object_set_name(anim_obj.back(), "ANIMATION");
#if BONES
					ACJoint * parent_joint = anim_joint.empty() ? group_obj : anim_joint.back();
					anim_joint.push_back(ac_new_joint());
					object_add_child(parent_joint, anim_joint.back());
					if (parent_joint == group_obj)
						top_joints.push_back(anim_joint.back());
					object_set_name(anim_joint.back(), "JOINT");
#endif					

					}
				break;
			case anim_End:
				stuff_obj = NULL;
				anim_obj.pop_back();
				break;
			case anim_Translate:
				anim_old_cmd = ac_object_get_data(anim_obj.back());
				if (anim_old_cmd)
					anim_dat = anim_old_cmd;
				else
					anim_dat.clear();
				sprintf(anim_cmd, "TRANSLATE %f %f %f %f %f %f %f %f %s\n",
					obj8.animation[cmd->idx_offset].xyzrv1[0],
					obj8.animation[cmd->idx_offset].xyzrv1[1],
					obj8.animation[cmd->idx_offset].xyzrv1[2],

					obj8.animation[cmd->idx_offset].xyzrv2[0],
					obj8.animation[cmd->idx_offset].xyzrv2[1],
					obj8.animation[cmd->idx_offset].xyzrv2[2],

					obj8.animation[cmd->idx_offset].xyzrv1[4],
					obj8.animation[cmd->idx_offset].xyzrv2[4],
					obj8.animation[cmd->idx_offset].dataref.c_str());
				anim_dat += anim_cmd;
				object_set_userdata(anim_obj.back(), (char *) anim_dat.c_str());
				
				key1.x = obj8.animation[cmd->idx_offset].xyzrv1[0];
				key1.y = obj8.animation[cmd->idx_offset].xyzrv1[1];
				key1.z = obj8.animation[cmd->idx_offset].xyzrv1[2];

				key2.x = obj8.animation[cmd->idx_offset].xyzrv2[0];
				key2.y = obj8.animation[cmd->idx_offset].xyzrv2[1];
				key2.z = obj8.animation[cmd->idx_offset].xyzrv2[2];
#if BONES				
				ac_object_add_position_key(anim_joint.back(), 0,  &key1);
				ac_object_add_position_key(anim_joint.back(), 60, &key2);
				key1.x = key1.y = key2.y = 0.0;
				ac_object_add_rotation_key(anim_joint.back(), 0,  &key1);
				ac_object_add_rotation_key(anim_joint.back(), 60, &key1);
#endif				
				break;
			case anim_Rotate:
				anim_old_cmd = ac_object_get_data(anim_obj.back());
				if (anim_old_cmd)
					anim_dat = anim_old_cmd;
				else
					anim_dat.clear();
				sprintf(anim_cmd, "ROTATE %f %f %f %f %f %f %f %s\n",
					obj8.animation[cmd->idx_offset].xyzrv1[0],
					obj8.animation[cmd->idx_offset].xyzrv1[1],
					obj8.animation[cmd->idx_offset].xyzrv1[2],

					obj8.animation[cmd->idx_offset].xyzrv1[3],
					obj8.animation[cmd->idx_offset].xyzrv2[3],

					obj8.animation[cmd->idx_offset].xyzrv1[4],
					obj8.animation[cmd->idx_offset].xyzrv2[4],
					obj8.animation[cmd->idx_offset].dataref.c_str());
				anim_dat += anim_cmd;
				object_set_userdata(anim_obj.back(), (char *) anim_dat.c_str());
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
					
					sprintf(strbuf, "_RGB=%f,%f,%f", dat[3], dat[4], dat[5]);
					ac_entity_set_point_value(light, "diffuse", &col_ac3);
					object_set_name(light, strbuf);
					object_add_child(anim_obj.empty() ? lod_obj : anim_obj.back(), light);
				}
				break;
			case obj_Smoke_Black:
			case obj_Smoke_White:
			case attr_Ambient_RGB:	
			case attr_Diffuse_RGB:
			case attr_Emission_RGB:
			case attr_Specular_RGB:
			case attr_Shiny_Rat:
			case attr_Reset:
#if !SUPPORT_NO_DEPTH
			case attr_No_Depth:			
			case attr_Depth:
#endif			
				break;
			}	
		}
	}

#if BONES    
    for (vector<ACJoint *>::iterator joint = top_joints.begin(); joint != top_joints.end(); ++joint)
    	ac_object_init_transform_recurse(*joint);
#endif
    
	object_calc_normals_force(group_obj);

    return group_obj;
}
