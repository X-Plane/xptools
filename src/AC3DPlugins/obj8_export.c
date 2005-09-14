/** OBJ7 exporter for AC3D **/

/*
	
*/

#include "ac_plugin.h"
#include "undoable.h"

#ifdef Boolean
#undef Boolean
#endif

#include "obj8_export.h"
#include "ac_utils.h"

#include "XObjDefs.h"
#include "XObjReadWrite.h"


static float	gRepTexX = 1.0;
static float	gRepTexY = 1.0;
static float	gOffTexX = 0.0;
static float	gOffTexY = 0.0;

static XObj8	gObj8;
static bool		gSmooth;
static bool		gTwoSided;
static bool		gIsCockpit;
static bool		gWasCockpit;
static string	gTexName;
static int		gErrMissingTex;
static int		gHasTexNow;
static bool		gErrDoubleTex;
static List *	gBadObjects;


/* OBJ8 import and export */
static void obj8_reset_properties();
static void obj8_output_triangle(Surface *s);
static void obj8_output_polyline(Surface *s);
static void obj8_output_polygon(Surface *s);
static void obj8_output_object(ACObject *ob, set<ACObject *> * stopset);




/***************************************************************************************************
 * OBJ8 IMPORT AND EXPORT
 ***************************************************************************************************/

void obj8_reset_properties(void)
{
	// NOTE: in obj8 x-plaen resets props - just remember that we're reset!
	gSmooth = true;
	gTwoSided = false;
	gIsCockpit = false;
	gWasCockpit = false;
}

void obj8_output_triangle(Surface *s)
{
		Vertex *p1, *p2, *p3;
		SVertex * s1, * s2, *s3;

    p1 = SVERTEX(s->vertlist->data);
    p2 = SVERTEX(s->vertlist->next->data);
    p3 = SVERTEX(s->vertlist->next->next->data);

	s1 = ((SVertex *)s->vertlist->data);
	s2 = ((SVertex *)s->vertlist->next->data);
	s3 = ((SVertex *)s->vertlist->next->next->data);

	float	ds1[8] = { s1->v->x, s1->v->y, s1->v->z, s1->normal.x, s1->normal.y, s1->normal.z, s1->tx, s1->ty };
	float	ds2[8] = { s2->v->x, s2->v->y, s2->v->z, s2->normal.x, s2->normal.y, s2->normal.z, s2->tx, s2->ty };
	float	ds3[8] = { s3->v->x, s3->v->y, s3->v->z, s3->normal.x, s3->normal.y, s3->normal.z, s3->tx, s3->ty };

	float	df1[8] = { s1->v->x, s1->v->y, s1->v->z, s->normal.x, s->normal.y, s->normal.z, s1->tx, s1->ty };
	float	df2[8] = { s2->v->x, s2->v->y, s2->v->z, s->normal.x, s->normal.y, s->normal.z, s2->tx, s2->ty };
	float	df3[8] = { s3->v->x, s3->v->y, s3->v->z, s->normal.x, s->normal.y, s->normal.z, s3->tx, s3->ty };

	int		idx1 = gObj8.geo_tri.accumulate(gSmooth ? ds1 : df1);
	int		idx2 = gObj8.geo_tri.accumulate(gSmooth ? ds2 : df2);
	int		idx3 = gObj8.geo_tri.accumulate(gSmooth ? ds3 : df3);

	int		start_i = gObj8.indices.size();
	
	gObj8.indices.push_back(idx3);	// This is a CCW->CW conversion!
	gObj8.indices.push_back(idx2);
	gObj8.indices.push_back(idx1);
	
	int		end_i = gObj8.indices.size();
	
	if (gObj8.lods.empty())
	{
		gObj8.lods.push_back(XObjLOD8());
		gObj8.lods.back().lod_near = 0;
		gObj8.lods.back().lod_far  = 0;
	}
	
	if (gObj8.lods.back().cmds.empty() || 
		gObj8.lods.back().cmds.back().cmd != obj8_Tris ||
		gObj8.lods.back().cmds.back().idx_count + gObj8.lods.back().cmds.back().idx_offset != start_i)
	{
		XObjCmd8	cmd;
		cmd.cmd = obj8_Tris;
		cmd.idx_offset = start_i;
		cmd.idx_count = end_i - start_i;
		gObj8.lods.back().cmds.push_back(cmd);
	} else {
		gObj8.lods.back().cmds.back().idx_count += (end_i - start_i);
	}
}



void obj8_output_polyline(Surface *s)
{
	Vertex *p1, *p2;
	int n;

	int		start_i = gObj8.indices.size();

	float 	ds1[6] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
	float 	ds2[6] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
	int		idx1, idx2;

	for (n=0; n < s->numvert-1; n++)
	{
		p1 = SVERTEX(list_get_item(s->vertlist, n));
		p2 = SVERTEX(list_get_item(s->vertlist, n+1));

		ds1[0] = p1->x;	ds1[1] = p1->y;	ds1[2] = p1->z;
		ds2[0] = p2->x;	ds2[1] = p2->y;	ds2[2] = p2->z;
		
		idx1 = gObj8.geo_lines.accumulate(ds1);
		idx2 = gObj8.geo_lines.accumulate(ds2);
	
		gObj8.indices.push_back(idx1);
		gObj8.indices.push_back(idx2);
	}

	if (surface_get_type(s) == SURFACE_CLOSEDLINE)
	{
		p1 = SVERTEX(list_get_item(s->vertlist, s->numvert-1));
		p2 = SVERTEX(list_get_item(s->vertlist, 0));

		ds1[0] = p1->x;	ds1[1] = p1->y;	ds1[2] = p1->z;
		ds2[0] = p2->x;	ds2[1] = p2->y;	ds2[2] = p2->z;
		
		idx1 = gObj8.geo_lines.accumulate(ds1);
		idx2 = gObj8.geo_lines.accumulate(ds2);
	
		gObj8.indices.push_back(idx1);
		gObj8.indices.push_back(idx2);
	}	
	
	int		end_i = gObj8.indices.size();
	
	if (gObj8.lods.empty())
	{
		gObj8.lods.push_back(XObjLOD8());
		gObj8.lods.back().lod_near = 0;
		gObj8.lods.back().lod_far  = 0;
	}
	
	if (gObj8.lods.back().cmds.empty() || 
		gObj8.lods.back().cmds.back().cmd != obj8_Lines ||
		gObj8.lods.back().cmds.back().idx_count + gObj8.lods.back().cmds.back().idx_offset != start_i)
	{
		XObjCmd8	cmd;
		cmd.cmd = obj8_Lines;
		cmd.idx_offset = start_i;
		cmd.idx_count = end_i - start_i;
		gObj8.lods.back().cmds.push_back(cmd);
	} else {
		gObj8.lods.back().cmds.back().idx_count += (end_i - start_i);
	}
	
}

void obj8_output_polygon(Surface *s)
{
	if (!gHasTexNow && !gIsCockpit)
		++gErrMissingTex;

	bool	is_two_sided = surface_get_twosided(s);
	bool	is_smooth = surface_get_shading(s);
	
	XObjCmd8	cmd;

	if (gObj8.lods.empty())
	{
		gObj8.lods.push_back(XObjLOD8());
		gObj8.lods.back().lod_near = 0;
		gObj8.lods.back().lod_far = 0;
	}
		
	if (is_two_sided != gTwoSided)
	{
		cmd.cmd = is_two_sided ? attr_NoCull : attr_Cull;
		gObj8.lods.back().cmds.push_back(cmd);
		gTwoSided = is_two_sided;
	}
	
	if (is_smooth != gSmooth)
	{
		cmd.cmd = is_smooth ? attr_Shade_Smooth : attr_Shade_Flat;
		gObj8.lods.back().cmds.push_back(cmd);
		gSmooth = is_smooth;		
	}

	if (gIsCockpit != gWasCockpit)
	{
		cmd.cmd = gIsCockpit ? attr_Tex_Cockpit : attr_Tex_Normal;
		gObj8.lods.back().cmds.push_back(cmd);
		gIsCockpit = gWasCockpit;		
	}

	List *slist = (List *)surface_get_triangulations(s);
	if (slist != NULL)
	{
		List *t;

		for (t = slist; t != NULL; t = t->next)
		{
			obj8_output_triangle((Surface *)t->data); 
			surface_free((Surface *)t->data);
		}
		list_free(&slist);
	}
}



void obj8_output_object(ACObject *ob, set<ACObject *> * stopset)
{
		int 	numvert, numsurf, numkids;
		List 	*vertices, *surfaces, *kids;
		List 	*p;

    printf("outputing %s\n", ac_object_get_name(ob));

    ac_object_get_contents(ob, &numvert, &numsurf, &numkids,
        &vertices, &surfaces, &kids); 

	float	lod_start, lod_end;
	if (sscanf(ac_object_get_name(ob), "LOD %f/%f",&lod_start, &lod_end)==2)
	{
		obj8_reset_properties();

		gObj8.lods.push_back(XObjLOD8());
		gObj8.lods.back().lod_near = lod_start;
		gObj8.lods.back().lod_far = lod_end;
	}
	
	bool bad_obj = false;
	
	if (ac_object_has_texture(ob))
	{	
		string tex = texture_id_to_name(ac_object_get_texture_index(ob));
		gHasTexNow = true;
		if (strstrnocase(tex.c_str(), "cockpit/-PANELS-/panel."))
			gIsCockpit = true;
		else
			gIsCockpit = false;
		if (!gIsCockpit)
		{
			if (tex != gTexName && !gTexName.empty())
			{
				gErrDoubleTex = true;
				list_add_item_head(&gBadObjects, ob);
				bad_obj = true;
			} 
			gTexName = tex;
		}
	} else {
		gIsCockpit = false;
		gHasTexNow = false;
	}

	gRepTexX = ac_object_get_texture_repeat_x(ob);
	gRepTexY = ac_object_get_texture_repeat_y(ob);
	gOffTexX = ac_object_get_texture_offset_x(ob);
	gOffTexY = ac_object_get_texture_offset_y(ob);
        
    int no_tex_count = gErrMissingTex;
    for (p = surfaces; p != NULL; p = p->next)
    {
        Surface *s = (Surface *)p->data;
        if (surface_get_type(s) == SURFACE_POLYGON)
            obj8_output_polygon(s);
        else
            obj8_output_polyline(s);
    }
    
    if (no_tex_count < gErrMissingTex && !bad_obj)
    	list_add_item_head(&gBadObjects, ob);

    for (p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
    	if (stopset == NULL || stopset->find(child) == stopset->end())
	        obj8_output_object(child, stopset);
	}
}


int do_obj8_save(char * fname, ACObject * obj)
{
	
	gObj8.lods.clear();	
	gObj8.indices.clear();
	gObj8.texture.clear();
	gObj8.animation.clear();
	gObj8.geo_tri.clear(8);
	gObj8.geo_lines.clear(6);
	gObj8.geo_lights.clear(6);
	gObj8.texture_lit.clear();
	
	gTexName.clear();
	gErrMissingTex = 0;
	gHasTexNow = false;
	gErrDoubleTex = false;
	gBadObjects = NULL;

	gSmooth = true;
	gTwoSided = false;
	gIsCockpit = false;
	gWasCockpit = false;
    obj8_output_object(obj, NULL);
	obj8_reset_properties();
    
    string::size_type p = gTexName.find_last_of("\\/");
    if (p != gTexName.npos) gTexName.erase(0,p+1);
    gObj8.texture = gTexName;
    if (gObj8.texture.size() > 4)
	    gObj8.texture_lit = gObj8.texture.substr(0, gObj8.texture.size()-4) + "_lit" + gObj8.texture.substr(gObj8.texture.size()-4);

	if (!XObj8Write(fname, gObj8))
    {
        message_dialog("can't open file '%s' for writing", fname);
        return 0;
    }
    
    if (gErrMissingTex)
    	message_dialog("Warning: %d objects did not have texturse assigned.  You must assign a texture to every object for X-Plane output.", gErrMissingTex);
    if (gErrDoubleTex)
    	message_dialog("This model uses more than one texture.  You may only use one texture for an X-Plane OBJ.");
	if (gBadObjects)
    {
		clear_selection();
		ac_selection_select_objectlist(gBadObjects);
		list_free(&gBadObjects);
		gBadObjects = NULL;
		redraw_all();    	
    }
    
    return 1;
}


ACObject *	do_obj8_load(char *filename)
{
		ACObject * 	group_obj = NULL;
		ACObject * 	lod_obj = NULL;
		ACObject * 	stuff_obj = NULL;
		
		char *		tex_full_name = NULL;
		int			tex_id = -1;
		char *		panel_full_name = NULL;
		int			panel_id = -1;

	Point3	p3;
	char	strbuf[256];
	
	gObj8.lods.clear();
	if (!XObj8Read(filename, gObj8))
	{
		message_dialog("can't read OBJ7 file '%s'", filename);
		return NULL;
	}
	
    group_obj = new_object(OBJECT_NORMAL);	
    string	fname(filename);
    string::size_type p = fname.find_last_of("\\/");	
    string justName = (p == fname.npos) ? fname : fname.substr(p+1);
    string justPath = fname.substr(0,p+1);
	string::size_type p2 = gObj8.texture.find_last_of("\\/:");
	string texName = (p2 == gObj8.texture.npos) ? gObj8.texture : gObj8.texture.substr(p2+1);    
	if (texName.size() > 4)
		texName.erase(texName.size()-4);
    string texNameBmp = texName + ".bmp";
    string texNamePng = texName + ".png";    
    string panelNamePng = "cockpit/-PANELS-/panel.png";
    string panelNameBmp = "cockpit/-PANELS-/panel.bmp";
        
    bool	has_cockpit_cmd = false;
    for(vector<XObjLOD8>::iterator lod = gObj8.lods.begin(); lod != gObj8.lods.end(); ++lod)
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
        
    for(vector<XObjLOD8>::iterator lod = gObj8.lods.begin(); lod != gObj8.lods.end(); ++lod)
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
    	bool	no_depth = false;
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
					object_add_child(lod_obj, stuff_obj);						
				}

				if (cmd->cmd == obj8_Tris)
				{
					vector<Vertex *> verts;
					int i;
					for (i = 0; i < cmd->idx_count; ++i)
					{
						float * dat = gObj8.geo_tri.get(gObj8.indices[cmd->idx_offset + i]);
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
						}
						float * dat = gObj8.geo_tri.get(gObj8.indices[cmd->idx_offset + i]);					
				        surface_add_vertex_head(s, verts[i], dat[6], dat[7]);
				    }

				} else {

					vector<Vertex *> verts;
					int i;
					for (i = 0; i < cmd->idx_count; ++i)
					{
						float * dat = gObj8.geo_lines.get(gObj8.indices[cmd->idx_offset + i]);
						p3.x = dat[0];
						p3.y = dat[1];
						p3.z = dat[2];
						verts.push_back(object_add_new_vertex_head(stuff_obj, &p3));
					}
					
			        Surface * s = NULL;
					for (i = 0; i < cmd->idx_count; ++i)
					{
						if ((i % 2) == 0) 
						{
							s = new_surface();
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
			case attr_No_Depth:			
				if (!no_depth) stuff_obj = NULL;
				no_depth = true;
				break;
			case attr_Depth:
				if (no_depth) stuff_obj = NULL;
				no_depth = false;
				break;
				
			case anim_Begin:			stuff_obj = NULL;	break;	// TODO animation
			case anim_End:				stuff_obj = NULL;	break;
			case anim_Rotate:			stuff_obj = NULL;	break;
			case anim_Translate:		stuff_obj = NULL;	break;

			case attr_Shade_Flat:		shade_flat = true;	break;
			case attr_Shade_Smooth:		shade_flat = false;	break;
			case attr_Cull:				two_side = false;	break;
			case attr_NoCull:			two_side = true;	break;

			case obj8_Lights:			
			case obj_Smoke_Black:
			case obj_Smoke_White:
			case attr_Ambient_RGB:	
			case attr_Diffuse_RGB:
			case attr_Emission_RGB:
			case attr_Specular_RGB:
			case attr_Shiny_Rat:
			case attr_Reset:
				break;
			}	
		}
	}
    
	object_calc_normals_force(group_obj);

    return group_obj;
}

