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
#include "Undoable.h"

#ifdef Boolean
#undef Boolean
#endif

#include "obj8_export.h"
#include "ac_utils.h"
#include "obj_anim.h"
#include "obj_model.h"
#include "obj_panel.h"

#include "XObjDefs.h"
#include "XObjReadWrite.h"
#include "XObjWriteEmbedded.h"
#include "ObjConvert.h"
#include "XObjBuilder.h"
#include "prefs.h"
#include <stdio.h>
#include <list>
#include <set>
using std::list;
using std::set;

static float	gRepTexX = 1.0;
static float	gRepTexY = 1.0;
static float	gOffTexX = 0.0;
static float	gOffTexY = 0.0;

//static int		gSmooth;
//static int		gTwoSided;
//static int		gIsCockpit;
//static int		gWasCockpit;

//static int		gHardPoly;
//static int		gPolyOS;
//static int		gBlend;

static string	gTexName;
static int		gErrMissingTex;
static int		gHasTexNow;
static bool		gErrDoubleTex;
static bool		gSubregionOOBErr;
static List *	gBadObjects;
static bool		gErrBadCockpit;
static bool		gErrBadHard;
static List *	gBadSurfaces;


/* OBJ8 import and export */
static void obj8_output_triangle(XObjBuilder * builder, Surface *s, bool is_smooth);
static void obj8_output_polyline(XObjBuilder * builder, Surface *s);
static void obj8_output_polygon(XObjBuilder * builder, Surface *s);
static void obj8_output_light(XObjBuilder * builder, ACObject *obj);
static void obj8_output_object(XObjBuilder * builder, ACObject *obj, ACObject * root, int tex_id, int do_misc);

enum convert_choice {
	convert_none,
	convert_7,
	convert_e
};


static int do_obj8_save_common(char * fname, ACObject * obj, convert_choice convert, int do_prefix, int tex_id, int do_misc);


/***************************************************************************************************
 * OBJ8 IMPORT AND EXPORT
 ***************************************************************************************************/

void obj8_output_triangle(XObjBuilder * builder, Surface *s, bool is_smooth)
{
	if (!get_export_triangles()) return;
		SVertex * s1, * s2, *s3;

	s3 = ((SVertex *)s->vertlist->data);
	s2 = ((SVertex *)s->vertlist->next->data);
	s1 = ((SVertex *)s->vertlist->next->next->data);

	float	ds1[24] = { s1->v->x, s1->v->y, s1->v->z, s1->normal.x, s1->normal.y, s1->normal.z, s1->tx, s1->ty,
						s2->v->x, s2->v->y, s2->v->z, s2->normal.x, s2->normal.y, s2->normal.z, s2->tx, s2->ty,
						s3->v->x, s3->v->y, s3->v->z, s3->normal.x, s3->normal.y, s3->normal.z, s3->tx, s3->ty };

	float	df1[24] = { s1->v->x, s1->v->y, s1->v->z, s->normal.x, s->normal.y, s->normal.z, s1->tx, s1->ty,
						s2->v->x, s2->v->y, s2->v->z, s->normal.x, s->normal.y, s->normal.z, s2->tx, s2->ty,
						s3->v->x, s3->v->y, s3->v->z, s->normal.x, s->normal.y, s->normal.z, s3->tx, s3->ty };

	builder->AccumTri(is_smooth ? ds1 : df1);
}



void obj8_output_polyline(XObjBuilder * builder, Surface *s)
{
	if (!get_export_triangles()) return;

	Vertex *p1, *p2;
	int n;

	float 	ds1[12] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
						0.0, 0.0, 0.0, 1.0, 1.0, 1.0 };

	index_to_3f(s->col, &ds1[3], &ds1[4 ], &ds1[5 ]);
	index_to_3f(s->col, &ds1[9], &ds1[10], &ds1[11]);

	for (n=0; n < s->numvert-1; n++)
	{
		p1 = SVERTEX(list_get_item(s->vertlist, n));
		p2 = SVERTEX(list_get_item(s->vertlist, n+1));

		ds1[0] = p1->x;	ds1[1] = p1->y;	ds1[2] = p1->z;
		ds1[6] = p2->x;	ds1[7] = p2->y;	ds1[8] = p2->z;

		builder->AccumLine(ds1);
	}

	if (surface_get_type(s) == SURFACE_CLOSEDLINE)
	{
		p1 = SVERTEX(list_get_item(s->vertlist, s->numvert-1));
		p2 = SVERTEX(list_get_item(s->vertlist, 0));

		ds1[0] = p1->x;	ds1[1] = p1->y;	ds1[2] = p1->z;
		ds1[6] = p2->x;	ds1[7] = p2->y;	ds1[8] = p2->z;

		builder->AccumLine(ds1);
	}
}

void obj8_output_polygon(XObjBuilder * builder, Surface *s)
{
	if (!gHasTexNow && !builder->IsCockpit() && builder->IsVisible())
		++gErrMissingTex;

	bool	is_two_sided = surface_get_twosided(s);
	bool	is_smooth = surface_get_shading(s);

	if (OBJ_get_use_materials(object_of_surface(s)))
	{
		ACMaterial * mat = ac_palette_get_material(s->col);
		if (mat)
		{
			ACrgb	diffuse, emissive, spec;
			if (ac_entity_get_rgb_value((ACEntity*) mat, (char*)"diffuse", &diffuse))
			{
				float diff[3] = { diffuse.r, diffuse.g, diffuse.b };
				builder->SetAttribute3(attr_Diffuse_RGB,diff);
			}
			if (ac_entity_get_rgb_value((ACEntity*) mat, (char*)"emissive", &emissive))
			{
				float emis[3] = { emissive.r, emissive.g, emissive.b };
				builder->SetAttribute3(attr_Emission_RGB,emis);
			}
			if (ac_entity_get_rgb_value((ACEntity*) mat, (char*)"specular", &spec))
			{
				builder->SetAttribute1(attr_Shiny_Rat,(spec.r + spec.g + spec.b) / 3.0);
			}
		}

	} else
		builder->SetAttribute(attr_Reset);


	builder->SetAttribute(is_two_sided ? attr_NoCull : attr_Cull);
//	builder->SetAttribute(is_smooth ? attr_Shade_Smooth : attr_Shade_Flat);
	// Ben says: smooth flag sets normals basde on face - no need to set flat shading!
	builder->SetAttribute(is_smooth ? attr_Shade_Smooth : attr_Shade_Smooth);


	if (s->numvert != 4)
	{
		if (builder->IsCockpit())
		{
			gErrBadCockpit = true;
			list_add_item_head(&gBadSurfaces, s);
		}
		if (!builder->IsHard().empty())
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
			obj8_output_triangle(builder, (Surface *)t->data, is_smooth);
			surface_free((Surface *)t->data);
		}
		list_free(&slist);
	}
}

static void obj8_output_light(XObjBuilder * builder, ACObject *obj)
{
	Point3	xyz;
	ac_entity_get_point_value(obj, (char*)"loc", &xyz);
	float pos[3] = { xyz.x, xyz.y, xyz.z };

	char lname[256], lref[256];
	OBJ_get_light_named(obj, lname);

	if (lname[0] == 0) return;

		 if (strcmp(lname,"rgb")==0)
	{
		float	dat[6] = { xyz.x, xyz.y, xyz.z,
			OBJ_get_light_red  (obj),
			OBJ_get_light_green(obj),
			OBJ_get_light_blue (obj) };
		builder->AccumLight(dat);
	}
	else if (strcmp(lname,"custom")==0)
	{
		float params[9] = {
			OBJ_get_light_red  (obj),
			OBJ_get_light_green(obj),
			OBJ_get_light_blue (obj),
			OBJ_get_light_alpha(obj),
			OBJ_get_light_size (obj),
			OBJ_get_light_s1   (obj),
			OBJ_get_light_t1   (obj),
			OBJ_get_light_s2   (obj),
			OBJ_get_light_t2   (obj) };
		OBJ_get_light_dataref(obj,lref);
		builder->AccumLightCustom(pos, params, lref);
	}
	else if (strcmp(lname,"white smoke")==0)
	{
		builder->AccumSmoke(obj_Smoke_White, pos, OBJ_get_light_smoke_size(obj));
	}
	else if (strcmp(lname,"black smoke")==0)
	{
		builder->AccumSmoke(obj_Smoke_Black, pos, OBJ_get_light_smoke_size(obj));
	}
	else
	{
		builder->AccumLightNamed(pos, lname);
	}
}


void obj8_output_object(XObjBuilder * builder, ACObject * obj, ACObject * root, int tex_id, int do_misc)
{
	char  buf[1024];
	if (!ac_object_is_visible(obj)) return;

		int 	numvert, numsurf, numkids;
		List 	*vertices, *surfaces, *kids;
		List 	*p;

//    printf("outputing %s\n", ac_object_get_name(obj));

    ac_object_get_contents(obj, &numvert, &numsurf, &numkids,
        &vertices, &surfaces, &kids);

	float	lod_start = OBJ_get_LOD_near(obj);
	float   lod_end = OBJ_get_LOD_far(obj);
	if (lod_start != 0.0 || lod_end != 0.0)
	{
		builder->EndLOD();
		builder->BeginLOD(lod_start, lod_end);
	}

	if (OBJ_get_layer_group(obj, buf))
	{
		if (buf[0] != 0 && strcmp(buf,"none"))
		builder->SetAttribute1Named(attr_Layer_Group, OBJ_get_layer_group_offset(obj), buf);
	}

	if (OBJ_get_animation_group(obj))
	{
		builder->AccumAnimBegin();
	}

	char dref[256];
	float xyz1[3];
	float xyz2[3];
	int k;

	switch(OBJ_get_anim_type(obj)) {
	case anim_rotate:
		builder->AccumTranslate(
			center_for_rotation(obj, xyz2),
			center_for_rotation(obj, xyz2),
			0.0, 0.0, "none");
		builder->AccumRotateBegin(axis_for_rotation(obj,xyz1),
							OBJ_get_anim_dataref(obj, dref));
		for(k = 0; k < OBJ_get_anim_keyframe_count(obj); ++k)
			builder->AccumRotateKey(OBJ_get_anim_nth_value(obj, k),
									OBJ_get_anim_nth_angle(obj, k));
		builder->AccumRotateEnd();
		builder->AccumTranslate(
			center_for_rotation_negative(obj, xyz2),
			center_for_rotation_negative(obj, xyz2),
			0.0, 0.0, "none");
		break;
	case anim_trans:
		{
			builder->AccumTranslateBegin(OBJ_get_anim_dataref(obj, dref));
			for(k = 0; k < OBJ_get_anim_keyframe_count(obj); ++k)
				builder->AccumTranslateKey(OBJ_get_anim_nth_value(obj,k),
											anim_trans_nth_relative(obj, k, xyz1));
			builder->AccumTranslateEnd();
		}
		break;
	case anim_static:
		builder->AccumTranslate(
							anim_trans_nth(obj,0,xyz1),
							anim_trans_nth(obj,1,xyz2),
							OBJ_get_anim_nth_value(obj,0),
							OBJ_get_anim_nth_value(obj,1),
							OBJ_get_anim_dataref(obj, dref));
		break;
	case anim_show:
		builder->AccumShow(
							OBJ_get_anim_nth_value(obj,0),
							OBJ_get_anim_nth_value(obj,1),
							OBJ_get_anim_dataref(obj, dref));
		break;
	case anim_hide:
		builder->AccumHide(
							OBJ_get_anim_nth_value(obj,0),
							OBJ_get_anim_nth_value(obj,1),
							OBJ_get_anim_dataref(obj, dref));
		break;
	case anim_none:
		{
			float now_poly_os, now_blend;

			now_poly_os = OBJ_get_poly_os(obj);						//  pull_int_attr_recursive(obj, "_POLY_OS=",0,root);
			builder->SetAttribute1(attr_Offset, now_poly_os);

			OBJ_get_hard(obj, buf);
			int deck = OBJ_get_deck(obj);
			if(buf[0] == 0)
				builder->SetAttribute(attr_No_Hard);
			else if (strcmp(buf,"object")==0)
				builder->SetAttribute(deck ? attr_Hard_Deck : attr_Hard);
			else
				builder->SetAttribute1Named(deck ? attr_Hard_Deck : attr_Hard, 0.0, buf);

			now_blend = OBJ_get_blend(obj);
			if (now_blend <= 0.0)
				builder->SetAttribute(attr_Blend);
			else
				builder->SetAttribute1(attr_No_Blend, now_blend);

			if(OBJ_get_mod_lit(obj))
			{
				OBJ_get_lit_dataref(obj, buf);
				builder->SetAttribute1Named(attr_Light_Level, 0.0f, buf);
			} else
				builder->SetAttribute(attr_Light_Level_Reset);

			if(OBJ_get_draw_disable(obj))
				builder->SetAttribute(attr_Draw_Disable);
			else
				builder->SetAttribute(attr_Draw_Enable);

			if(OBJ_get_wall(obj))
				builder->SetAttribute(attr_Solid_Wall);
			else
				builder->SetAttribute(attr_No_Solid_Wall);

			bool bad_obj = false;
			int panel_reg = -1;
			int has_real_tex = 0;

			if(!OBJ_get_draw_disable(obj))
			{
				if (ac_object_has_texture(obj))
				{
					string tex = texture_id_to_name(ac_object_get_texture_index(obj));
					gHasTexNow = true;

					if (is_panel_tex(ac_object_get_texture_index(obj)))
					{
						if(get_sub_panel_count())
						{
							panel_reg = get_sub_panel_for_mesh(obj);
							if(panel_reg >= 0)
								builder->SetAttribute1(attr_Tex_Cockpit_Subregion,panel_reg);
							else
							{
								printf("Subregion is out of bounds for: %s.\n", ac_object_get_name(obj));
								gSubregionOOBErr = true;
							}
						}
						else
							builder->SetAttribute(attr_Tex_Cockpit);
					}
					else {
						has_real_tex = 1;
						builder->SetAttribute(attr_Tex_Normal);
					}
					if (!builder->IsCockpit())
					if (tex_id == -1 || tex_id == ac_object_get_texture_index(obj))
					{
						if (tex != gTexName && !gTexName.empty())
						{
							gErrDoubleTex = true;
							list_add_item_head(&gBadObjects, obj);
							bad_obj = true;
						}
						gTexName = tex;
					}
				} else {
					builder->SetAttribute(attr_Tex_Normal);
					gHasTexNow = false;
				}
			}

			XObjManip8 m;
			OBJ_get_manip_cursor(obj,buf);
			m.cursor = buf;
			OBJ_get_manip_tooltip(obj,buf);
			m.tooltip = buf;
			OBJ_get_manip_dref1(obj,buf);
			m.dataref1 = buf;
			OBJ_get_manip_dref2(obj,buf);
			m.dataref2 = buf;
			m.v1_min = OBJ_get_manip_v1_min(obj);
			m.v1_max = OBJ_get_manip_v1_max(obj);
			m.v2_min = OBJ_get_manip_v2_min(obj);
			m.v2_max = OBJ_get_manip_v2_max(obj);
			m.axis[0] = OBJ_get_manip_dx(obj);
			m.axis[1] = OBJ_get_manip_dy(obj);
			m.axis[2] = OBJ_get_manip_dz(obj);

			switch(OBJ_get_manip_type(obj)) {
			case manip_panel:
				builder->AccumManip(attr_Tex_Cockpit,m);
				break;
			case manip_none:
				builder->AccumManip(attr_Manip_None,m);
				break;
			case manip_axis:
				builder->AccumManip(attr_Manip_Drag_Axis,m);
				break;
			case manip_axis_2d:
				builder->AccumManip(attr_Manip_Drag_2d,m);
				break;
			case manip_command:
				builder->AccumManip(attr_Manip_Command,m);
				break;
			case manip_command_axis:
				builder->AccumManip(attr_Manip_Command_Axis,m);
				break;
			case manip_noop:
				builder->AccumManip(attr_Manip_Noop,m);
				break;
			}

			int do_surf = has_real_tex ? (tex_id == -1 || tex_id == ac_object_get_texture_index(obj)) : do_misc;

			if(panel_reg >= 0)
				builder->SetTexRepeatParams(
					panel_get_texture_repeat_x(panel_reg,obj),
					panel_get_texture_repeat_y(panel_reg,obj),
					panel_get_texture_offset_x(panel_reg,obj),
					panel_get_texture_offset_y(panel_reg,obj));
			else
				builder->SetTexRepeatParams(
					ac_object_get_texture_repeat_x(obj),
					ac_object_get_texture_repeat_y(obj),
					ac_object_get_texture_offset_x(obj),
					ac_object_get_texture_offset_y(obj));

			int no_tex_count = gErrMissingTex;
			for (p = surfaces; p != NULL; p = p->next)
			{
				Surface *s = (Surface *)p->data;
				if (surface_get_type(s) == SURFACE_POLYGON)
				{
					if (do_surf)
						obj8_output_polygon(builder, s);
				} else {
					if (do_misc)
						obj8_output_polyline(builder, s);
				}
			}

			if (no_tex_count < gErrMissingTex && !bad_obj)
				list_add_item_head(&gBadObjects, obj);
		}
		break;
	}

	if (ac_entity_is_class(obj, (char*)AC_CLASS_LIGHT))
	if (do_misc)
	{
		obj8_output_light(builder, obj);
	}

    for (p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
	        obj8_output_object(builder, child, root, tex_id, do_misc);
	}

	if (OBJ_get_animation_group(obj))
	{
		builder->AccumAnimEnd();
	}
}



int do_obj8_save_common(char * fname, ACObject * obj, convert_choice convert, int do_prefix, int tex_id, int do_misc)
{
	printf("Saving file: %s.  We have %d panel regions enabled.\n",fname,get_sub_panel_count());
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
	gSubregionOOBErr = false;
	gBadObjects = NULL;
	gBadSurfaces = NULL;
	gErrBadCockpit = false;
	gErrBadHard = false;

	XObjBuilder		builder(&obj8);

	if (get_default_layer_group() && get_default_layer_group()[0] && strcmp(get_default_layer_group(),"none"))
		builder.SetAttribute1Named(attr_Layer_Group, get_default_layer_offset(), get_default_layer_group());

    obj8_output_object(&builder, obj, obj, tex_id, do_misc);

	if (get_default_LOD() > 0.0f)
	if (obj8.lods.size() == 1 && obj8.lods.front().lod_far == 0.0)
	{
		obj8.lods[0].lod_near = 0.0f;
		obj8.lods[0].lod_far = get_default_LOD();
	}

	// Texture path.  Ben says: users want the texture path to be relative to the ac3d file.  Doable I suppose.

	// tcl_get_string("acfilename")

	string export_path(fname);
	string::size_type export_filename_idx, tex_dir_idx;
	export_filename_idx = export_path.find_last_of("\\/");
	tex_dir_idx = gTexName.find_last_of("\\/");

	string tex_lit(gTexName);
	if (tex_lit.size() > 4)
	{
	    tex_lit.insert(tex_lit.length()-4,"_LIT");
		FILE * f = fopen(tex_lit.c_str(), "r");
		if (f)
		{
			fclose(f);
			if (tex_dir_idx != gTexName.npos)
			tex_lit.erase(0,tex_dir_idx+1);
			obj8.texture_lit = tex_lit;
		}
	}
	if (tex_dir_idx != gTexName.npos)
	gTexName.erase(0,tex_dir_idx+1);
    obj8.texture = gTexName;

	if (!obj8.texture.empty())		obj8.texture.insert(0,get_texture_prefix());
	if (!obj8.texture_lit.empty())	obj8.texture_lit.insert(0,get_texture_prefix());

	if (do_prefix)
		export_path.insert(export_filename_idx+1,string(get_export_prefix()));

	for (int n = 0; n < get_sub_panel_count(); ++n)
	{
		obj8.regions.push_back(XObjPanelRegion8());
		obj8.regions.back().left  = get_sub_panel_l(n);
		obj8.regions.back().right = get_sub_panel_r(n);
		obj8.regions.back().bottom= get_sub_panel_b(n);
		obj8.regions.back().top   = get_sub_panel_t(n);
	}
	builder.Finish();

	if (convert == convert_7)
	{
		XObj	obj7;
		Obj8ToObj7(obj8, obj7);
		if (!XObjWrite(export_path.c_str(), obj7))
	    {
	        message_dialog((char*)"can't open file '%s' for writing", export_path.c_str());
	        return 0;
	    }

	}
#if PHONE
	else if (convert == convert_e)
	{
		Obj8_Optimize(obj8);
		if (!XObjWriteEmbedded(export_path.c_str(), obj8, true))	// 16 bit!
	    {
	        message_dialog((char*)"can't open file '%s' for writing", export_path.c_str());
	        return 0;
	    }

	}
#endif
	else {
		if (!XObj8Write(export_path.c_str(), obj8))
	    {
	        message_dialog((char*)"can't open file '%s' for writing", export_path.c_str());
	        return 0;
	    }
	}
    if (gErrMissingTex)
    	message_dialog((char*)"Warning: %d objects did not have textures assigned.  You must assign a texture to every object for X-Plane output.", gErrMissingTex);
    if (gErrDoubleTex)
    	message_dialog((char*)"This model uses more than one texture.  You may only use one texture for an X-Plane OBJ.");
	if(gSubregionOOBErr)
		message_dialog((char*)"You have used panel sub-regions, but your texture mapping goes out of the bounds of the sub-regions.  Your panel may not have exported right.");

   if (gErrBadCockpit && convert == convert_7)
    	message_dialog((char*)"This model has non-quad surfaces that use the panel texture.  Only quad surfaces may use the panel texture in OBJ7.");
   if (gErrBadHard && convert == convert_7)
    	message_dialog((char*)"This model has non-quad surfaces that rae marked as hard.  Only quad surfaces may be hard in OBJ7.");

   if (gBadSurfaces)
    {
    	if (convert == convert_7) {
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
	return do_obj8_save_common(fname, obj, convert_none, false, -1, true);
}

int 		do_obje_save(char * fname, ACObject * obj)
{
	return do_obj8_save_common(fname, obj, convert_e, false, -1, true);
}


int 		do_obj8_save_ex(char * fname, ACObject * obj, int do_prefix, int tex_id, int do_misc)
{
	return do_obj8_save_common(fname, obj, convert_none, do_prefix, tex_id, do_misc);
}

int 		do_obj7_save_convert(char * fname, ACObject * obj)
{
	return do_obj8_save_common(fname, obj, convert_7, false, -1, true);
}
