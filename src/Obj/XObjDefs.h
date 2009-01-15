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
#ifndef XOBJDEFS_H
#define XOBJDEFS_H

#include <string>
#include <vector>
using namespace std;

#include "ObjPointPool.h"

/****************************************************************************************
 * OBJ 2/7
 ****************************************************************************************
 *
 * Notes: no end command is written to the command stream.
 * LODs are inline as attributes.  The absense of LOD attributes means only a default LOD.
 *
 * Multiple primitives like lines, quads and tris must only have 2 3 and 4 vertices,
 * respectively for file-write.
 *
 */

enum {

	type_None = 0,
	type_PtLine,	// OBJ 7
	type_Poly,		// OBJ 7
	type_Attr,		// OBJ 7 or 8
	type_Indexed,	// OBJ 8
	type_Anim,		// OBJ 8
	type_Cust		// OBJ8

};

enum {
	// OBJ7 commands
	obj_End = 0,
	obj_Light,
	obj_Line,
	obj_Tri,
	obj_Quad,
	obj_Quad_Hard,
	obj_Quad_Cockpit,
	obj_Movie,
	obj_Polygon,
	obj_Quad_Strip,
	obj_Tri_Strip,
	obj_Tri_Fan,

	// Shared commands
	attr_Shade_Flat,
	attr_Shade_Smooth,
	attr_Ambient_RGB,
	attr_Diffuse_RGB,
	attr_Emission_RGB,
	attr_Specular_RGB,
	attr_Shiny_Rat,
	attr_No_Depth,
	attr_Depth,
	attr_LOD,
	attr_Reset,
	attr_Cull,
	attr_NoCull,
	attr_Offset,
	obj_Smoke_Black,
	obj_Smoke_White,

	// OBJ8 commands
	obj8_Tris,
	obj8_Lines,
	obj8_Lights,

	attr_Tex_Normal,
	attr_Tex_Cockpit,
	attr_No_Blend,
	attr_Blend,
	attr_Hard,
	attr_Hard_Deck,
	attr_No_Hard,

	anim_Begin,
	anim_End,
	anim_Rotate,
	anim_Translate,

	// 850 commands
	obj8_LightCustom,			// all in name??  param is pos?
	obj8_LightNamed,			// name has light name, param is pos
	attr_Layer_Group,			// name has group name, param[0] has offset
	anim_Hide,					// only v1 and v2 are used
	anim_Show,

	// 900 commands
	attr_Tex_Cockpit_Subregion,
	// 920 commands
	attr_Manip_None,
	attr_Manip_Drag_2d,
	attr_Manip_Drag_Axis,
	attr_Manip_Command,
	attr_Manip_Command_Axis,
	attr_Manip_Noop,
	// 930 commands
	attr_Light_Level,
	attr_Light_Level_Reset,
	attr_Draw_Disable,
	attr_Draw_Enable,
	attr_Solid_Wall,
	attr_No_Solid_Wall,
	attr_Max
};

struct	cmd_info {
	int				cmd_id;
	int				cmd_type;
	const char *	name;
	int				elem_count;
	int				v7;
	int				v8;
};

extern	cmd_info	gCmds[];
extern	int			gCmdCount;

struct	vec_tex {
	float	v[3];
	float	st[2];
};

struct	vec_rgb {
	float	v[3];
	float	rgb[3];
};

struct XObjCmd {

	int				cmdType;	// Are we a line, poly or attribute?
	int				cmdID;		// What command are we?

	vector<float>	attributes;
	vector<vec_tex>	st;
	vector<vec_rgb>	rgb;

};

struct	XObj {

	string			texture;
	vector<XObjCmd>	cmds;

};

int	FindObjCmd(const char * inToken, bool obj_8);
int	FindIndexForCmd(int inCmd);

/****************************************************************************************
 * OBJ 8
 ****************************************************************************************
 *
 * Notes: if the object has only a default LOD, the LOD range will be 0.0 to 0.0.
 *
 * The library does not merge consecutive-indexed tri commands on read or write.
 *
 */

struct XObjKey {
	float					key;
	float					v[3];		// angle for rotation, XYZ for translation
};

struct	XObjAnim8 {
	string					dataref;
	float					axis[3];	// Used for rotations
	vector<XObjKey>			keyframes;
};

struct XObjManip8 {
	string					dataref1;				// Commands for, cmd manips!
	string					dataref2;
	float					axis[3];
	float					v1_min, v1_max;
	float					v2_min, v2_max;
	string					cursor;
	string					tooltip;
};

struct	XObjCmd8 {
	int						cmd;
	float					params[12];
	string					name;
	int						idx_offset;
	int						idx_count;
};

struct	XObjLOD8 {
	float					lod_near;
	float					lod_far;
	vector<XObjCmd8>		cmds;
};

struct XObjPanelRegion8 {
	int						left;
	int						bottom;
	int						right;
	int						top;
};

struct	XObj8 {
	string 					texture;
	string 					texture_lit;
	vector<XObjPanelRegion8>regions;
	vector<int>				indices;
	ObjPointPool			geo_tri;
	ObjPointPool			geo_lines;
	ObjPointPool			geo_lights;
	vector<XObjAnim8>		animation;
	vector<XObjManip8>		manips;
	vector<XObjLOD8>		lods;
};

#endif
