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

#include "obj8_export.h"
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

static float	gRepTexX = 1.0;
static float	gRepTexY = 1.0;
static float	gOffTexX = 0.0;
static float	gOffTexY = 0.0;

static XObj8	gObj8;

static int		gSmooth;
static int		gTwoSided;
static int		gIsCockpit;
static int		gWasCockpit;

static int		gHardPoly;
static int		gPolyOS;
static int		gBlend;
#if SUPPORT_NO_DEPTH
static int		gDepth;
#endif

static string	gTexName;
static int		gErrMissingTex;
static int		gHasTexNow;
static bool		gErrDoubleTex;
static List *	gBadObjects;
static bool		gErrBadCockpit;
static bool		gErrBadHard;
static List *	gBadSurfaces;


/* OBJ8 import and export */
static void obj8_assure_one_lod(void);
static void obj8_reset_properties();
static void obj8_output_triangle(Surface *s);
static void obj8_output_polyline(Surface *s);
static void obj8_output_polygon(Surface *s);
static void obj8_output_light(ACObject *ob);
static void obj8_output_object(ACObject *ob, set<ACObject *> * stopset);

static int do_obj8_save_common(char * fname, ACObject * obj, bool convert);


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

static void obj8_assure_one_lod(void)
{
	if (gObj8.lods.empty())
	{
		gObj8.lods.push_back(XObjLOD8());
		gObj8.lods.back().lod_near = 0;
		gObj8.lods.back().lod_far  = 0;	
	}
}

void obj8_reset_properties(void)
{
	// NOTE: in obj8 x-plaen resets props - just remember that we're reset!
	gSmooth = 1;
	gTwoSided = 0;
	gIsCockpit = 0;
	gWasCockpit = 0;

#if SUPPORT_NO_DEPTH	
	gDepth = 1;
#endif
	gBlend = 1;
	gHardPoly = 0;
	gPolyOS = 0;
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
	
	obj8_assure_one_lod();
		
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

	index_to_3f(s->col, &ds1[3], &ds1[4], &ds1[5]);
	index_to_3f(s->col, &ds2[3], &ds2[4], &ds2[5]);

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
	
	obj8_assure_one_lod();
	
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

	obj8_assure_one_lod();
		
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
		gWasCockpit = gIsCockpit;		
	}

	if (s->numvert != 4)
	{
		if (gIsCockpit)
		{
			gErrBadCockpit = true;
			list_add_item_head(&gBadSurfaces, s);
		}
		if (gHardPoly)
		{
			gErrBadHard = true;
			list_add_item_head(&gBadSurfaces, s);
		}
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

static void obj8_output_light(ACObject *ob)
{
	Point3	xyz, rgb = { 1.0, 1.0, 1.0 };
	ac_entity_get_point_value(ob, "loc", &xyz);
	
	char * title = ac_object_get_name(ob);
	char * token = strstr(title, "_RGB=");
	if (token != NULL)
	{
		if (strlen(token) > 5)
		{
			token += 5;
			sscanf(token, "%f,%f,%f", &rgb.x, &rgb.y, &rgb.z);
		}
	}
	float	dat[6] = { xyz.x, xyz.y, xyz.z, rgb.x, rgb.y, rgb.z };
	
	int idx = gObj8.geo_lights.accumulate(dat);
	
	obj8_assure_one_lod();
	if (!gObj8.lods.back().cmds.empty() &&
		 gObj8.lods.back().cmds.back().cmd == obj8_Lights &&
		 (gObj8.lods.back().cmds.back().idx_offset + gObj8.lods.back().cmds.back().idx_count) == idx)
	{
		gObj8.lods.back().cmds.back().idx_count++;
	} else {
		gObj8.lods.back().cmds.push_back(XObjCmd8());
		gObj8.lods.back().cmds.back().cmd = obj8_Lights;
		gObj8.lods.back().cmds.back().idx_offset = idx;
		gObj8.lods.back().cmds.back().idx_count = 1;
	}
}






void obj8_output_object(ACObject *ob, set<ACObject *> * stopset)
{
	if (!ac_object_is_visible(ob)) return;
	
		int 	numvert, numsurf, numkids;
		List 	*vertices, *surfaces, *kids;
		List 	*p;

//    printf("outputing %s\n", ac_object_get_name(ob));

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
	
	if (strstr(ac_object_get_name(ob), "ANIMATION") != NULL)
	{
		obj8_assure_one_lod();
		gObj8.lods.back().cmds.push_back(XObjCmd8());
		gObj8.lods.back().cmds.back().cmd = anim_Begin;
		
		char * startp = ac_object_get_data(ob);
		if (startp)
		{
			char * endp = startp + strlen(startp);
			while (startp != endp)
			{
				char * stopp = startp;
				while (stopp < endp && *stopp != '\n')
					++stopp;
				if (stopp < endp) ++stopp;
				
				char		dataref[512];
				XObjAnim8	anim;
				if (sscanf(startp, "TRANSLATE %f %f %f %f %f %f %f %f %s",
					&anim.xyzrv1[0],&anim.xyzrv1[1],&anim.xyzrv1[2],
					&anim.xyzrv2[0],&anim.xyzrv2[1],&anim.xyzrv2[2],
					&anim.xyzrv1[4],&anim.xyzrv2[4],dataref) == 9)
				{
					anim.dataref = dataref;
					gObj8.animation.push_back(anim);
					gObj8.lods.back().cmds.push_back(XObjCmd8());
					gObj8.lods.back().cmds.back().cmd = anim_Translate;
					gObj8.lods.back().cmds.back().idx_offset = gObj8.animation.size()-1;
				}

				if (sscanf(startp, "ROTATE %f %f %f %f %f %f %f %s",
					&anim.xyzrv1[0],&anim.xyzrv1[1],&anim.xyzrv1[2],
					&anim.xyzrv1[3],&anim.xyzrv2[3],
					&anim.xyzrv1[4],&anim.xyzrv2[4],dataref) == 8)
				{
					anim.dataref = dataref;
					gObj8.animation.push_back(anim);
					gObj8.lods.back().cmds.push_back(XObjCmd8());
					gObj8.lods.back().cmds.back().cmd = anim_Rotate;
					gObj8.lods.back().cmds.back().idx_offset = gObj8.animation.size()-1;
				}				
				
				startp = stopp;				
			}
		}
	}
	
		int	now_poly_os, now_hard, now_blend;
#if SUPPORT_NO_DEPTH
		int now_depth;
#endif		

	if (pull_int_attr(ob, "_POLY_OS=", &now_poly_os) &&
		now_poly_os != gPolyOS)
	{
		obj8_assure_one_lod();
		gPolyOS = now_poly_os;
		gObj8.lods.back().cmds.push_back(XObjCmd8());
		gObj8.lods.back().cmds.back().cmd = attr_Offset;
		gObj8.lods.back().cmds.back().params[0] = gPolyOS;			
	}

	if (pull_int_attr(ob, "_HARD=", &now_hard) &&
		now_hard != gHardPoly)
	{
		obj8_assure_one_lod();
		gHardPoly = now_hard;
		gObj8.lods.back().cmds.push_back(XObjCmd8());
		gObj8.lods.back().cmds.back().cmd = gHardPoly ? attr_Hard : attr_No_Hard;
	}

	if (pull_int_attr(ob, "_BLEND=", &now_blend) &&
		now_blend != gBlend)
	{
		obj8_assure_one_lod();
		gBlend = now_blend;
		gObj8.lods.back().cmds.push_back(XObjCmd8());
		gObj8.lods.back().cmds.back().cmd = gBlend ? attr_Blend : attr_No_Blend;
	}

#if SUPPORT_NO_DEPTH
	if (pull_int_attr(ob, "_DEPTH=", &now_depth) &&
		now_depth != gDepth)
	{
		obj8_assure_one_lod();
		gDepth = now_depth;
		gObj8.lods.back().cmds.push_back(XObjCmd8());
		gObj8.lods.back().cmds.back().cmd = gDepth ? attr_Depth : attr_No_Depth;
	}
#endif
	
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

	if (ac_entity_is_class(ob, AC_CLASS_LIGHT))
	{
		obj8_output_light(ob);
	}

    for (p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
    	if (stopset == NULL || stopset->find(child) == stopset->end())
	        obj8_output_object(child, stopset);
	}
	
	if (strstr(ac_object_get_name(ob), "ANIMATION") != NULL)
	{
		gObj8.lods.back().cmds.push_back(XObjCmd8());
		gObj8.lods.back().cmds.back().cmd = anim_End;
	}

}


int do_obj8_save_common(char * fname, ACObject * obj, bool convert)
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
	gBadSurfaces = NULL;
	gErrBadCockpit = false;
	gErrBadHard = false;

	obj8_reset_properties();
    obj8_output_object(obj, NULL);
    
    string::size_type p = gTexName.find_last_of("\\/");
    if (p != gTexName.npos) gTexName.erase(0,p+1);
    gObj8.texture = gTexName;
    if (gObj8.texture.size() > 4)
	    gObj8.texture_lit = gObj8.texture.substr(0, gObj8.texture.size()-4) + "_lit" + gObj8.texture.substr(gObj8.texture.size()-4);

	if (convert)
	{
		XObj	obj7;
		Obj8ToObj7(gObj8, obj7);
		if (!XObjWrite(fname, obj7))
	    {
	        message_dialog("can't open file '%s' for writing", fname);
	        return 0;
	    }
		
	} else {
		if (!XObj8Write(fname, gObj8))
	    {
	        message_dialog("can't open file '%s' for writing", fname);
	        return 0;
	    }
	}    
    if (gErrMissingTex)
    	message_dialog("Warning: %d objects did not have textures assigned.  You must assign a texture to every object for X-Plane output.", gErrMissingTex);
    if (gErrDoubleTex)
    	message_dialog("This model uses more than one texture.  You may only use one texture for an X-Plane OBJ.");

   if (gErrBadCockpit && convert)
    	message_dialog("This model has non-quad surfaces that use the panel texture.  Only quad surfaces may use the panel texture in OBJ7.");
   if (gErrBadHard && convert)
    	message_dialog("This model has non-quad surfaces that rae marked as hard.  Only quad surfaces may be hard in OBJ7.");
 
   if (gBadSurfaces)
    {
    	if (convert) {    	
			clear_selection();
			ac_selection_select_surfacelist(gBadSurfaces);
			redraw_all();    	
		}
		list_free(&gBadSurfaces);
		gBadSurfaces = NULL;
    }
	else if (gBadObjects)
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
	
	gObj8.lods.clear();
	if (!XObj8Read(filename, gObj8))
	{
		XObj	obj7;

		if (!XObjRead(filename, obj7))
			return NULL;
	
		Obj7ToObj8(obj7, gObj8);
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
						float * dat = gObj8.geo_tri.get(gObj8.indices[cmd->idx_offset + i]);
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
#if BONES						
						if (!anim_joint.empty())
							ac_vertex_set_joint(verts.back(), (int) anim_joint.back());
#endif							
					}
					
			        Surface * s = NULL;
					for (i = 0; i < cmd->idx_count; ++i)
					{
						float * dat = gObj8.geo_lines.get(gObj8.indices[cmd->idx_offset + i]);
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
					gObj8.animation[cmd->idx_offset].xyzrv1[0],
					gObj8.animation[cmd->idx_offset].xyzrv1[1],
					gObj8.animation[cmd->idx_offset].xyzrv1[2],

					gObj8.animation[cmd->idx_offset].xyzrv2[0],
					gObj8.animation[cmd->idx_offset].xyzrv2[1],
					gObj8.animation[cmd->idx_offset].xyzrv2[2],

					gObj8.animation[cmd->idx_offset].xyzrv1[4],
					gObj8.animation[cmd->idx_offset].xyzrv2[4],
					gObj8.animation[cmd->idx_offset].dataref.c_str());
				anim_dat += anim_cmd;
				object_set_userdata(anim_obj.back(), (char *) anim_dat.c_str());
				
				key1.x = gObj8.animation[cmd->idx_offset].xyzrv1[0];
				key1.y = gObj8.animation[cmd->idx_offset].xyzrv1[1];
				key1.z = gObj8.animation[cmd->idx_offset].xyzrv1[2];

				key2.x = gObj8.animation[cmd->idx_offset].xyzrv2[0];
				key2.y = gObj8.animation[cmd->idx_offset].xyzrv2[1];
				key2.z = gObj8.animation[cmd->idx_offset].xyzrv2[2];
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
					gObj8.animation[cmd->idx_offset].xyzrv1[0],
					gObj8.animation[cmd->idx_offset].xyzrv1[1],
					gObj8.animation[cmd->idx_offset].xyzrv1[2],

					gObj8.animation[cmd->idx_offset].xyzrv1[3],
					gObj8.animation[cmd->idx_offset].xyzrv2[3],

					gObj8.animation[cmd->idx_offset].xyzrv1[4],
					gObj8.animation[cmd->idx_offset].xyzrv2[4],
					gObj8.animation[cmd->idx_offset].dataref.c_str());
				anim_dat += anim_cmd;
				object_set_userdata(anim_obj.back(), (char *) anim_dat.c_str());
				break;
			case obj8_Lights:			
				for (i = 0; i < cmd->idx_count; ++i)
				{
					stuff_obj = NULL;
					ACObject * light = new_object(OBJECT_LIGHT);
					Point3	pt_ac3, col_ac3 = { 0.0, 0.0, 0.0 };
					float * dat = gObj8.geo_lights.get(cmd->idx_offset + i);
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

int 		do_obj8_save(char * fname, ACObject * obj)
{
	return do_obj8_save_common(fname, obj, false);
}

int 		do_obj7_save_convert(char * fname, ACObject * obj)
{
	return do_obj8_save_common(fname, obj, true);
}