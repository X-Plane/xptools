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

//static XObj8	gObj8;

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
static void obj8_assure_one_lod(XObj8 * obj);
static void obj8_reset_properties();
static void obj8_output_triangle(XObj8 * obj, Surface *s);
static void obj8_output_polyline(XObj8 * obj, Surface *s);
static void obj8_output_polygon(XObj8 * obj, Surface *s);
static void obj8_output_light(XObj8 * obj, ACObject *ob);
static void obj8_output_object(XObj8 * obj, ACObject *ob, set<ACObject *> * stopset);

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

static void obj8_assure_one_lod(XObj8 * obj)
{
	if (obj->lods.empty())
	{
		obj->lods.push_back(XObjLOD8());
		obj->lods.back().lod_near = 0;
		obj->lods.back().lod_far  = 0;	
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

void obj8_output_triangle(XObj8 * obj, Surface *s)
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

	int		idx1 = obj->geo_tri.accumulate(gSmooth ? ds1 : df1);
	int		idx2 = obj->geo_tri.accumulate(gSmooth ? ds2 : df2);
	int		idx3 = obj->geo_tri.accumulate(gSmooth ? ds3 : df3);

	int		start_i = obj->indices.size();
	
	obj->indices.push_back(idx3);	// This is a CCW->CW conversion!
	obj->indices.push_back(idx2);
	obj->indices.push_back(idx1);
	
	int		end_i = obj->indices.size();
	
	obj8_assure_one_lod(obj);
		
	if (obj->lods.back().cmds.empty() || 
		obj->lods.back().cmds.back().cmd != obj8_Tris ||
		obj->lods.back().cmds.back().idx_count + obj->lods.back().cmds.back().idx_offset != start_i)
	{
		XObjCmd8	cmd;
		cmd.cmd = obj8_Tris;
		cmd.idx_offset = start_i;
		cmd.idx_count = end_i - start_i;
		obj->lods.back().cmds.push_back(cmd);
	} else {
		obj->lods.back().cmds.back().idx_count += (end_i - start_i);
	}
}



void obj8_output_polyline(XObj8 * obj, Surface *s)
{
	Vertex *p1, *p2;
	int n;

	int		start_i = obj->indices.size();

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
		
		idx1 = obj->geo_lines.accumulate(ds1);
		idx2 = obj->geo_lines.accumulate(ds2);
	
		obj->indices.push_back(idx1);
		obj->indices.push_back(idx2);
	}

	if (surface_get_type(s) == SURFACE_CLOSEDLINE)
	{
		p1 = SVERTEX(list_get_item(s->vertlist, s->numvert-1));
		p2 = SVERTEX(list_get_item(s->vertlist, 0));

		ds1[0] = p1->x;	ds1[1] = p1->y;	ds1[2] = p1->z;
		ds2[0] = p2->x;	ds2[1] = p2->y;	ds2[2] = p2->z;
		
		idx1 = obj->geo_lines.accumulate(ds1);
		idx2 = obj->geo_lines.accumulate(ds2);
	
		obj->indices.push_back(idx1);
		obj->indices.push_back(idx2);
	}	
	
	int		end_i = obj->indices.size();
	
	obj8_assure_one_lod(obj);
	
	if (obj->lods.back().cmds.empty() || 
		obj->lods.back().cmds.back().cmd != obj8_Lines ||
		obj->lods.back().cmds.back().idx_count + obj->lods.back().cmds.back().idx_offset != start_i)
	{
		XObjCmd8	cmd;
		cmd.cmd = obj8_Lines;
		cmd.idx_offset = start_i;
		cmd.idx_count = end_i - start_i;
		obj->lods.back().cmds.push_back(cmd);
	} else {
		obj->lods.back().cmds.back().idx_count += (end_i - start_i);
	}
	
}

void obj8_output_polygon(XObj8 * obj, Surface *s)
{
	if (!gHasTexNow && !gIsCockpit)
		++gErrMissingTex;

	bool	is_two_sided = surface_get_twosided(s);
	bool	is_smooth = surface_get_shading(s);
	
	XObjCmd8	cmd;

	obj8_assure_one_lod(obj);
		
	if (is_two_sided != gTwoSided)
	{
		cmd.cmd = is_two_sided ? attr_NoCull : attr_Cull;
		obj->lods.back().cmds.push_back(cmd);
		gTwoSided = is_two_sided;
	}
	
	if (is_smooth != gSmooth)
	{
		cmd.cmd = is_smooth ? attr_Shade_Smooth : attr_Shade_Flat;
		obj->lods.back().cmds.push_back(cmd);
		gSmooth = is_smooth;		
	}

	if (gIsCockpit != gWasCockpit)
	{
		cmd.cmd = gIsCockpit ? attr_Tex_Cockpit : attr_Tex_Normal;
		obj->lods.back().cmds.push_back(cmd);
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
			obj8_output_triangle(obj, (Surface *)t->data); 
			surface_free((Surface *)t->data);
		}
		list_free(&slist);
	}
}

static void obj8_output_light(XObj8 * obj, ACObject *ob)
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
	
	int idx = obj->geo_lights.accumulate(dat);
	
	obj8_assure_one_lod(obj);
	if (!obj->lods.back().cmds.empty() &&
		 obj->lods.back().cmds.back().cmd == obj8_Lights &&
		 (obj->lods.back().cmds.back().idx_offset + obj->lods.back().cmds.back().idx_count) == idx)
	{
		obj->lods.back().cmds.back().idx_count++;
	} else {
		obj->lods.back().cmds.push_back(XObjCmd8());
		obj->lods.back().cmds.back().cmd = obj8_Lights;
		obj->lods.back().cmds.back().idx_offset = idx;
		obj->lods.back().cmds.back().idx_count = 1;
	}
}






void obj8_output_object(XObj8 * obj, ACObject *ob, set<ACObject *> * stopset)
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

		obj->lods.push_back(XObjLOD8());
		obj->lods.back().lod_near = lod_start;
		obj->lods.back().lod_far = lod_end;
	}
	
	if (strstr(ac_object_get_name(ob), "ANIMATION") != NULL)
	{
		obj8_assure_one_lod(obj);
		obj->lods.back().cmds.push_back(XObjCmd8());
		obj->lods.back().cmds.back().cmd = anim_Begin;
		
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
					obj->animation.push_back(anim);
					obj->lods.back().cmds.push_back(XObjCmd8());
					obj->lods.back().cmds.back().cmd = anim_Translate;
					obj->lods.back().cmds.back().idx_offset = obj->animation.size()-1;
				}

				if (sscanf(startp, "ROTATE %f %f %f %f %f %f %f %s",
					&anim.xyzrv1[0],&anim.xyzrv1[1],&anim.xyzrv1[2],
					&anim.xyzrv1[3],&anim.xyzrv2[3],
					&anim.xyzrv1[4],&anim.xyzrv2[4],dataref) == 8)
				{
					anim.dataref = dataref;
					obj->animation.push_back(anim);
					obj->lods.back().cmds.push_back(XObjCmd8());
					obj->lods.back().cmds.back().cmd = anim_Rotate;
					obj->lods.back().cmds.back().idx_offset = obj->animation.size()-1;
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
		obj8_assure_one_lod(obj);
		gPolyOS = now_poly_os;
		obj->lods.back().cmds.push_back(XObjCmd8());
		obj->lods.back().cmds.back().cmd = attr_Offset;
		obj->lods.back().cmds.back().params[0] = gPolyOS;			
	}

	if (pull_int_attr(ob, "_HARD=", &now_hard) &&
		now_hard != gHardPoly)
	{
		obj8_assure_one_lod(obj);
		gHardPoly = now_hard;
		obj->lods.back().cmds.push_back(XObjCmd8());
		obj->lods.back().cmds.back().cmd = gHardPoly ? attr_Hard : attr_No_Hard;
	}

	if (pull_int_attr(ob, "_BLEND=", &now_blend) &&
		now_blend != gBlend)
	{
		obj8_assure_one_lod(obj);
		gBlend = now_blend;
		obj->lods.back().cmds.push_back(XObjCmd8());
		obj->lods.back().cmds.back().cmd = gBlend ? attr_Blend : attr_No_Blend;
	}

#if SUPPORT_NO_DEPTH
	if (pull_int_attr(ob, "_DEPTH=", &now_depth) &&
		now_depth != gDepth)
	{
		obj8_assure_one_lod();
		gDepth = now_depth;
		obj->lods.back().cmds.push_back(XObjCmd8());
		obj->lods.back().cmds.back().cmd = gDepth ? attr_Depth : attr_No_Depth;
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
            obj8_output_polygon(obj, s);
        else
            obj8_output_polyline(obj, s);
    }
    
    if (no_tex_count < gErrMissingTex && !bad_obj)
    	list_add_item_head(&gBadObjects, ob);

	if (ac_entity_is_class(ob, AC_CLASS_LIGHT))
	{
		obj8_output_light(obj, ob);
	}

    for (p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
    	if (stopset == NULL || stopset->find(child) == stopset->end())
	        obj8_output_object(obj, child, stopset);
	}
	
	if (strstr(ac_object_get_name(ob), "ANIMATION") != NULL)
	{
		obj->lods.back().cmds.push_back(XObjCmd8());
		obj->lods.back().cmds.back().cmd = anim_End;
	}

}


int do_obj8_save_common(char * fname, ACObject * obj, bool convert)
{
	XObj8	obj8;
	
	obj8.lods.clear();	
	obj8.indices.clear();
	obj8.texture.clear();
	obj8.animation.clear();
	obj8.geo_tri.clear(8);
	obj8.geo_lines.clear(6);
	obj8.geo_lights.clear(6);
	obj8.texture_lit.clear();
	
	gTexName.clear();
	gErrMissingTex = 0;
	gHasTexNow = false;
	gErrDoubleTex = false;
	gBadObjects = NULL;
	gBadSurfaces = NULL;
	gErrBadCockpit = false;
	gErrBadHard = false;

	obj8_reset_properties();
    obj8_output_object(&obj8, obj, NULL);
    
    string::size_type p = gTexName.find_last_of("\\/");
    if (p != gTexName.npos) gTexName.erase(0,p+1);
    obj8.texture = gTexName;
    if (obj8.texture.size() > 4)
	    obj8.texture_lit = obj8.texture.substr(0, obj8.texture.size()-4) + "_lit" + obj8.texture.substr(obj8.texture.size()-4);

	if (convert)
	{
		XObj	obj7;
		Obj8ToObj7(obj8, obj7);
		if (!XObjWrite(fname, obj7))
	    {
	        message_dialog("can't open file '%s' for writing", fname);
	        return 0;
	    }
		
	} else {
		if (!XObj8Write(fname, obj8))
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

int 		do_obj8_save(char * fname, ACObject * obj)
{
	return do_obj8_save_common(fname, obj, false);
}

int 		do_obj7_save_convert(char * fname, ACObject * obj)
{
	return do_obj8_save_common(fname, obj, true);
}