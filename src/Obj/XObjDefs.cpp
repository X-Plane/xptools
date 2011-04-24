/*
 * Copyright (c) 2004, Laminar Research.
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
#include "XObjDefs.h"

cmd_info	gCmds[] = {

{	obj_End,			type_None,		"end",				0, 1, 0 },
{	obj_Light,			type_PtLine,	"light",			1, 1, 0 },
{   obj_Line,			type_PtLine,    "line",				2, 1, 0 },
{   obj_Tri,			type_Poly,		"tri",				3, 1, 0 },
{   obj_Quad,			type_Poly,		"quad",				4, 1, 0 },
{   obj_Quad_Cockpit,	type_Poly,		"quad_cockpit",		4, 1, 0 },
{   obj_Quad_Hard,		type_Poly,		"quad_hard",		4, 1, 0 },
{   obj_Smoke_Black,	type_Attr,		"smoke_black",		4, 1, 1 },
{   obj_Smoke_White,	type_Attr,		"smoke_white",		4, 1, 1 },
{   obj_Movie,			type_Poly,		"quad_movie",		4, 1, 0 },
{   obj_Polygon,		type_Poly,		"polygon",			0, 1, 0 },
{   obj_Quad_Strip,		type_Poly,		"quad_strip",		0, 1, 0 },
{   obj_Tri_Strip,		type_Poly,		"tri_strip",		0, 1, 0 },
{   obj_Tri_Fan,		type_Poly,		"tri_fan",			0, 1, 0 },
{   attr_Shade_Flat,	type_Attr,		"ATTR_shade_flat",	0, 1, 1 },
{   attr_Shade_Smooth,	type_Attr,		"ATTR_shade_smooth",0, 1, 1 },
{   attr_Shade_Flat,	type_Attr,		"shade_flat",		0, 1, 0 },
{   attr_Shade_Smooth,	type_Attr,		"shade_smooth",		0, 1, 0 },
{   attr_Ambient_RGB,	type_Attr,		"ATTR_ambient_rgb",	3, 1, 1 },
{   attr_Diffuse_RGB,	type_Attr,		"ATTR_diffuse_rgb",	3, 1, 1 },
{   attr_Diffuse_RGB,	type_Attr,		"ATTR_difuse_rgb",	3, 1, 1 },
{   attr_Emission_RGB,	type_Attr,		"ATTR_emission_rgb",3, 1, 1 },
{	attr_Specular_RGB,	type_Attr,		"ATTR_specular_rgb",3, 1, 1 },
{   attr_Shiny_Rat,		type_Attr,		"ATTR_shiny_rat",	1, 1, 1 },
{   attr_No_Depth,		type_Attr,		"ATTR_no_depth",	0, 1, 1 },
{	attr_Depth,			type_Attr,		"ATTR_depth",		0, 1, 1 },
{   attr_LOD,			type_Attr,		"ATTR_LOD",			2, 1, 1 },
{	attr_Reset,			type_Attr,		"ATTR_reset",		0, 1, 1 },
{	attr_Cull,			type_Attr,		"ATTR_cull",		0, 1, 1 },
{	attr_NoCull,		type_Attr,		"ATTR_no_cull",		0, 1, 1 },
{	attr_Offset,		type_Attr,		"ATTR_poly_os",		1, 1, 1 },

{	obj8_Tris,			type_Indexed,	"TRIS",				2,0,1},
{	obj8_Lines,			type_Indexed,	"LINES",			2,0,1},
{	obj8_Lights,		type_Indexed,	"LIGHTS",			2,0,1},
{	attr_Tex_Normal,	type_Attr,		"ATTR_no_cockpit",	0,0,1},
{	attr_Tex_Cockpit,	type_Attr,		"ATTR_cockpit",		0,0,1},
{	attr_No_Blend,		type_Attr,		"ATTR_no_blend",	0,0,1},
{	attr_Blend,			type_Attr,		"ATTR_blend",		0,0,1},
{	attr_No_Hard,		type_Attr,		"ATTR_no_hard",		0,0,1},
{	attr_Hard,			type_Attr,		"ATTR_hard",		0,0,1},
{	attr_Hard_Deck,		type_Attr,		"ATTR_hard_deck",	0,0,1},
{	anim_Begin,			type_Anim,		"ANIM_begin",		0,0,1},
{	anim_End,			type_Anim,		"ANIM_end",			0,0,1},
{	anim_Rotate,		type_Anim,		"ANIM_rotate",		7,0,1},
{	anim_Translate,		type_Anim,		"ANIM_trans",		8,0,1},

{	obj8_LightCustom,	type_Cust,		"LIGHT_CUSTOM",		14,0,1},
{	obj8_LightNamed,	type_Cust,		"LIGHT_NAMED",		4 ,0,1},
{	attr_Layer_Group,	type_Cust,		"ATTR_layer_group",	2 ,0,1},
{	anim_Hide,			type_Anim,		"ANIM_hide",		3,0,1},
{	anim_Show,			type_Anim,		"ANIM_show",		3,0,1},

// 900
{	attr_Tex_Cockpit_Subregion,	type_Attr,	"ATTR_cockpit_region",	1,0,1},
// 920
{	attr_Manip_None,		type_Attr,	"ATTR_manip_none",			0,0,1},
{	attr_Manip_Drag_2d,		type_Attr,	"ATTR_manip_drag_xy",		1,0,1},
{	attr_Manip_Drag_Axis,	type_Attr,	"ATTR_manip_drag_axis",		1,0,1},
{	attr_Manip_Command,		type_Attr,	"ATTR_manip_command",		1,0,1},
{	attr_Manip_Command_Axis,type_Attr,	"ATTR_manip_command_axis",	1,0,1},
{	attr_Manip_Noop,		type_Attr,	"ATTR_manip_noop",			0,0,1},
// 930
{	attr_Light_Level,		type_Attr,	"ATTR_light_level",			1,0,1},
{	attr_Light_Level_Reset,	type_Attr,	"ATTR_light_level_reset",	0,0,1},
{	attr_Draw_Disable,		type_Attr,	"ATTR_draw_disable",		0,0,1},
{	attr_Draw_Enable,		type_Attr,	"ATTR_draw_enable",			0,0,1},
{	attr_Solid_Wall,		type_Attr,	"ATTR_solid_camera",		0,0,1},
{	attr_No_Solid_Wall,		type_Attr,	"ATTR_no_solid_camera",		0,0,1},
// 10000
{	attr_Draped,			type_Attr,	"ATTR_draped",				0,0,1},
{	attr_NoDraped,			type_Attr,	"ATTR_no_draped",			0,0,1},



{	attr_Manip_Push,		type_Attr,	"ATTR_manip_push",			1,0,1},
{	attr_Manip_Radio,		type_Attr,	"ATTR_manip_radio",			1,0,1},
{	attr_Manip_Toggle,		type_Attr,	"ATTR_manip_toggle",		1,0,1},
{	attr_Manip_Delta,		type_Attr,	"ATTR_manip_delta",			1,0,1},
{	attr_Manip_Wrap,		type_Attr,	"ATTR_manip_wrap",			1,0,1},

{   attr_Max,			type_None,		NULL,				0, 0, 0 }

};

int gCmdCount = sizeof(gCmds) / sizeof(gCmds[0]);


int	FindObjCmd(const char * inToken, bool obj_8)
{
	int n = 0;
	while (gCmds[n].name)
	{
		if (!strcmp(inToken, gCmds[n].name))
		if ((!obj_8 && gCmds[n].v7) || (obj_8 && gCmds[n].v8))
			return n;
		++n;
	}

	return sizeof(gCmds) / sizeof(gCmds[0]);
}

int	FindIndexForCmd(int inCmd)
{
	int n = 0;
	while (gCmds[n].name)
	{
		if (gCmds[n].cmd_id == inCmd)
			return n;
		++n;
	}
	return sizeof(gCmds) / sizeof(gCmds[0]);
}
