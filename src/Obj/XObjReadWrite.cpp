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

 // TODO
 // - Clean up writing code a bit to handle decimals better.

#include "XObjReadWrite.h"
#include "XObjDefs.h"
#include "AssertUtils.h"
#include "MemFileUtils.h"
#include <math.h>

#ifndef CRLF
	#if APL
		#define CRLF "\n"
	#elif LIN
		#define CRLF "\n"
	#elif IBM
		#define CRLF "\n"
	#else
		#error no platform defined
	#endif
#endif

static const char * dataref_or_none(const char * d)
{
	if(d[0] == 0)
		return "none";
	return d;
}

/****************************************************************************************
 * OBJ 8 READ
 ****************************************************************************************/
bool	XObj8Read(const char * inFile, XObj8& outObj)
{
	outObj.texture.clear();
	outObj.texture_lit.clear();
	outObj.texture_normal_map.clear();
//	outObj.texture_nrm.clear();
	outObj.indices.clear();
	outObj.geo_tri.clear(8);
	outObj.geo_lines.clear(6);
	outObj.geo_lights.clear(6);
	outObj.animation.clear();
	outObj.lods.clear();
	outObj.use_metalness = 0;
	outObj.glass_blending = 0;
	outObj.fixed_heading = -1.0;
	outObj.viewpoint_height = -1.0;
	outObj.loadCenter_texSize = -1;

	auto objFile = MemFile_Open(inFile);
	if (!objFile) return false;

	MFScanner	s;
	MFS_init(&s, objFile);

	int versions[] = { 800, 0 };

	if (!MFS_xplane_header(&s, versions, "OBJ", NULL))
	{
		LOG_MSG("E/OBJ unsupported version or header in %s\n", inFile);
		MemFile_Close(objFile);
		return false;
	}

	/************************************************************
	 * READ GEOMETRIC COMMANDS
	 ************************************************************/

	int trimax = 0, linemax = 0, lightmax = 0, idxmax = 0;
	int tricount = 0, linecount = 0, lightcount = 0, idxcount = 0;
	int anims = 0;
	float	stdat[8];

	XObjCmd8	cmd;
	XObjAnim8	animation;

	outObj.lods.push_back(XObjLOD8());
	outObj.lods.back().lod_near = outObj.lods.back().lod_far = 0;

	while (!MFS_done(&s))
	{
		bool ate_eoln = false;
		// VT <x> <y> <z> <nx> <ny> <nz> <s> <t>
		if (MFS_string_match(&s, "VT", false))
		{
			if (tricount >= trimax) { tricount++; break; }
			for (int i = 0; i < 8; ++i)
				stdat[i] = MFS_double(&s);
			outObj.geo_tri.set(tricount++, stdat);
		}
		// IDX10 <n> x 10
		else if (MFS_string_match(&s, "IDX10", false))
		{
			if (idxcount + 9 >= idxmax)
			{
				LOG_MSG("E/Obj %s number of idx exceeds declared idx count %d\n", inFile, idxmax);
				break;
			}
			for (int n = 0; n < 10; ++n)
			{
				unsigned int idx = MFS_int(&s);
				if (idx < tricount)
					outObj.indices[idxcount++] = idx;
				else
				{
					LOG_MSG("E/Obj %s idx #%d points to index %d exceeding range of tris read %d\n", inFile, idxcount, idx, tricount);
					outObj.indices[idxcount++] = 0;
				}
			}
		}
		// IDX <n>
		else if (MFS_string_match(&s, "IDX", false))
		{
			if (idxcount >= idxmax)
			{
				LOG_MSG("E/Obj %s number of idx exceeds declared idx count %d\n", inFile, idxmax);
				break;
			}
			unsigned int idx = MFS_int(&s);
			if (idx < tricount)
				outObj.indices[idxcount++] = idx;
			else
			{
				LOG_MSG("E/Obj %s idx #%d points to index %d exceeding range of tris read %d\n", inFile, idxcount, idx, tricount);
				outObj.indices[idxcount++] = 0;
			}
		}
		else if (MFS_string_match(&s, "TEXTURE", false))
		{
			MFS_string(&s, &outObj.texture);
		}
		else if (MFS_string_match(&s, "TEXTURE_LIT", false))
		{
			MFS_string(&s, &outObj.texture_lit);
		}
		else if (MFS_string_match(&s, "TEXTURE_NORMAL", false))
		{
			MFS_string(&s, &outObj.texture_normal_map);
		}
		else if (MFS_string_match(&s, "TEXTURE_DRAPED", false))
		{
			MFS_string(&s, &outObj.texture_draped);
		}
		else if (MFS_string_match(&s, "POINT_COUNTS", false))
		{
			if(trimax || linemax || lightmax || idxmax)
				LOG_MSG("E/Obj more than one POINT_COUNTS line in %s\n",inFile);
			trimax = MFS_int(&s);
			linemax = MFS_int(&s);
			lightmax = MFS_int(&s);
			idxmax = MFS_int(&s);
			outObj.indices.resize(idxmax);
			outObj.geo_tri.clear(8);
			outObj.geo_tri.resize(trimax);
			outObj.geo_lines.clear(6);
			outObj.geo_lines.resize(linemax);
			outObj.geo_lights.clear(6);
			outObj.geo_lights.resize(lightmax);
		}
		// VLINE <x> <y> <z> <r> <g> <b>
		else if (MFS_string_match(&s, "VLINE", false))
		{
			if (linecount >= linemax) { linecount++; break; }
			for (int i = 0; i < 6; ++i)
				stdat[i] = MFS_double(&s);
			outObj.geo_lines.set(linecount++, stdat);
		}
		// VLIGHT <x> <y> <z> <r> <g> <b>
		else if (MFS_string_match(&s, "VLIGHT", false))
		{
			if (lightcount >= lightmax) { lightcount++; break; }
			for (int i = 0; i < 6; ++i)
				stdat[i] = MFS_double(&s);
			outObj.geo_lights.set(lightcount++, stdat);
		}
		// TRIS offset count
		else if (MFS_string_match(&s, "TRIS", false))
		{
			cmd.cmd = obj8_Tris;
			cmd.idx_offset = MFS_int(&s);
			cmd.idx_count = MFS_int(&s);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// LINES offset count
		else if (MFS_string_match(&s, "LINES", false))
		{
			cmd.cmd = obj8_Lines;
			cmd.idx_offset = MFS_int(&s);
			cmd.idx_count = MFS_int(&s);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// LIGHTS offset count
		else if (MFS_string_match(&s, "LIGHTS", false))
		{
			cmd.cmd = obj8_Lights;
			cmd.idx_offset = MFS_int(&s);
			cmd.idx_count = MFS_int(&s);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ATTR_LOD near far
		else if (MFS_string_match(&s, "ATTR_LOD", false))
		{
			if (outObj.lods.back().lod_far != 0)	outObj.lods.push_back(XObjLOD8());
			outObj.lods.back().lod_near = MFS_double(&s);
			outObj.lods.back().lod_far = MFS_double(&s);
		}
		// ANIM_rotate x y z r1 r2 v1 v2 dref
		else if (MFS_string_match(&s, "ANIM_rotate", false))
		{
			animation.keyframes.clear();
			animation.cmd = anim_Rotate;
			animation.loop = 0.0f;
			cmd.cmd = anim_Rotate;
			cmd.idx_offset = outObj.animation.size();
			animation.loop = 0.0f;
			outObj.lods.back().cmds.push_back(cmd);
			animation.keyframes.push_back(XObjKey());
			animation.keyframes.push_back(XObjKey());
			animation.axis[0] = MFS_double(&s);
			animation.axis[1] = MFS_double(&s);
			animation.axis[2] = MFS_double(&s);
			animation.keyframes[0].v[0] = MFS_double(&s);
			animation.keyframes[1].v[0] = MFS_double(&s);
			animation.keyframes[0].key = MFS_double(&s);
			animation.keyframes[1].key = MFS_double(&s);
			MFS_string(&s, &animation.dataref);
			outObj.animation.push_back(animation);
		}
		// ANIM_trans x1 y1 z1 x2 y2 z2 v1 v2 dref
		else if (MFS_string_match(&s, "ANIM_trans", false))
		{
			animation.keyframes.clear();
			animation.cmd = anim_Translate;
			cmd.cmd = anim_Translate;
			animation.loop = 0.0f;
			cmd.idx_offset = outObj.animation.size();
			animation.loop = 0.0f;
			outObj.lods.back().cmds.push_back(cmd);
			animation.keyframes.push_back(XObjKey());
			animation.keyframes.push_back(XObjKey());
			animation.keyframes[0].v[0] = MFS_double(&s);
			animation.keyframes[0].v[1] = MFS_double(&s);
			animation.keyframes[0].v[2] = MFS_double(&s);
			animation.keyframes[1].v[0] = MFS_double(&s);
			animation.keyframes[1].v[1] = MFS_double(&s);
			animation.keyframes[1].v[2] = MFS_double(&s);
			animation.keyframes[0].key = MFS_double(&s);
			animation.keyframes[1].key = MFS_double(&s);
			MFS_string(&s, &animation.dataref);
			outObj.animation.push_back(animation);
		}
		// ANIM_begin
		else if (MFS_string_match(&s, "ANIM_begin", true))
		{
			anims++;
			cmd.cmd = anim_Begin;
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ANIM_end
		else if (MFS_string_match(&s, "ANIM_end", true))
		{
			anims--;
			cmd.cmd = anim_End;
			outObj.lods.back().cmds.push_back(cmd);
		}
/******************************************************************************************************************************/
		// LIGHT_CUSTOM <x> <y> <z> <r> <g> <b> <a> <s><s1> <t1> <s2> <t2> <dataref>
		else if (MFS_string_match(&s, "LIGHT_CUSTOM", false))
		{
			cmd.cmd = obj8_LightCustom;
			for (int n = 0; n < 12; ++n)
				cmd.params[n] = MFS_double(&s);
			MFS_string(&s, &cmd.name);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// LIGHT_NAMED <name> <x> <y> <z>
		else if (MFS_string_match(&s, "LIGHT_NAMED", false))
		{
			cmd.cmd = obj8_LightNamed;
			MFS_string(&s, &cmd.name);
			for (int n = 0; n < 3; ++n)
				cmd.params[n] = MFS_double(&s);
			cmd.idx_count = 0;
			outObj.lods.back().cmds.push_back(cmd);			
		}
		// LIGHT_PARAM <name> <x> <y> <z>
		else if (MFS_string_match(&s, "LIGHT_PARAM", false))
		{
			cmd.cmd = obj8_LightNamed;
			MFS_string(&s, &cmd.name);
			for (int n = 0; n < 3; ++n)
				cmd.params[n] = MFS_double(&s);
			cmd.idx_count = 0;

			// Ben says: some objs have comment on the param light line - x-plane ignores anything past the
			// known param count since it has the lights.txt file. For now, simply ignore anything past the end.
			while(MFS_has_word(&s) && cmd.idx_count < 9)
			{
				cmd.params[cmd.idx_count+3] = MFS_double(&s);
				++cmd.idx_count;
			}
			outObj.lods.back().cmds.push_back(cmd);			
		}
		// ATTR_layer_group <group name> <offset>
		else if (MFS_string_match(&s, "ATTR_layer_group", false))
		{
			cmd.cmd = attr_Layer_Group;
			MFS_string(&s, &cmd.name);
			cmd.params[0] = MFS_double(&s);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ATTR_hard [<type>]
		else if (MFS_string_match(&s, "ATTR_hard", false))
		{
			cmd.cmd = attr_Hard;
			MFS_string(&s, &cmd.name);
			if (cmd.name.empty()) cmd.name = "object";
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ATTR_hard_deck [<type>]
		else if (MFS_string_match(&s, "ATTR_hard_deck", false))
		{
			cmd.cmd = attr_Hard_Deck;
			MFS_string(&s, &cmd.name);
			if (cmd.name.empty()) cmd.name = "object";
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ATTR_no_blend <level>
		else if (MFS_string_match(&s, "ATTR_no_blend", false))
		{
			cmd.cmd = attr_No_Blend;
			cmd.params[0] = MFS_double(&s);
			if (cmd.params[0] == 0.0) cmd.params[0] = 0.5;
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ANIM_hide <v1> <v2> <dataref>
		else if (MFS_string_match(&s, "ANIM_hide", false))
		{
			animation.keyframes.clear();
			animation.cmd = anim_Hide;
			cmd.cmd = anim_Hide;
			animation.loop = 0.0f;
			cmd.idx_offset = outObj.animation.size();
			animation.loop = 0.0f;
			outObj.lods.back().cmds.push_back(cmd);
			animation.keyframes.push_back(XObjKey());
			animation.keyframes.push_back(XObjKey());
			animation.keyframes[0].key = MFS_double(&s);
			animation.keyframes[1].key = MFS_double(&s);
			MFS_string(&s, &animation.dataref);
			outObj.animation.push_back(animation);
		}
		// ANIM_show <v1> <v2> <dataref>
		else if (MFS_string_match(&s, "ANIM_show", false))
		{
			animation.keyframes.clear();
			animation.cmd = anim_Show;
			cmd.cmd = anim_Show;
			animation.loop = 0.0f;
			cmd.idx_offset = outObj.animation.size();
			animation.loop = 0.0f;
			outObj.lods.back().cmds.push_back(cmd);
			animation.keyframes.push_back(XObjKey());
			animation.keyframes.push_back(XObjKey());
			animation.keyframes[0].key = MFS_double(&s);
			animation.keyframes[1].key = MFS_double(&s);
			MFS_string(&s, &animation.dataref);
			outObj.animation.push_back(animation);
		}
/******************************************************************************************************************************/
		// ANIM_rotate_begin x y z dref
		else if (MFS_string_match(&s, "ANIM_rotate_begin", false))
		{
			animation.keyframes.clear();
			animation.cmd = anim_Rotate;
			cmd.cmd = anim_Rotate;
			animation.loop = 0.0f;
			cmd.idx_offset = outObj.animation.size();
			animation.loop = 0.0f;
			outObj.lods.back().cmds.push_back(cmd);
			animation.axis[0] = MFS_double(&s);
			animation.axis[1] = MFS_double(&s);
			animation.axis[2] = MFS_double(&s);
			MFS_string(&s, &animation.dataref);
			outObj.animation.push_back(animation);
		}
		// ANIM_trans_begin dref
		else if (MFS_string_match(&s, "ANIM_trans_begin", false))
		{
			animation.keyframes.clear();
			animation.cmd = anim_Translate;
			animation.loop = 0.0f;
			cmd.cmd = anim_Translate;
			cmd.idx_offset = outObj.animation.size();
			animation.loop = 0.0f;
			outObj.lods.back().cmds.push_back(cmd);
			MFS_string(&s, &animation.dataref);
			outObj.animation.push_back(animation);
		}
		// ANIM_rotate_key v r
		else if (MFS_string_match(&s, "ANIM_rotate_key", false))
		{
			outObj.animation.back().keyframes.push_back(XObjKey());
			outObj.animation.back().keyframes.back().key = MFS_double(&s);
			outObj.animation.back().keyframes.back().v[0] = MFS_double(&s);
		}
		// ANIM_trans_key v x y z
		else if (MFS_string_match(&s, "ANIM_trans_key", false))
		{
			outObj.animation.back().keyframes.push_back(XObjKey());
			outObj.animation.back().keyframes.back().key = MFS_double(&s);
			outObj.animation.back().keyframes.back().v[0] = MFS_double(&s);
			outObj.animation.back().keyframes.back().v[1] = MFS_double(&s);
			outObj.animation.back().keyframes.back().v[2] = MFS_double(&s);
		}
		// ANIM_rotate_end
		else if (MFS_string_match(&s, "ANIM_rotate_end", true))
		{
		}
		// ANIM_trans_end
		else if (MFS_string_match(&s, "ANIM_trans_end", true))
		{
		}
		// ANIM_keyframe_loop <loop>
		else if (MFS_string_match(&s, "ANIM_keyframe_loop", false))
		{
			outObj.animation.back().loop = MFS_double(&s);
		}
/******************************************************************************************************************************/
		// COCKPIT_REGION
/******************************************************************************************************************************/
		else if (MFS_string_match(&s, "COCKPIT_REGION", false))
		{
			outObj.regions.push_back(XObjPanelRegion8());
			outObj.regions.back().left   = MFS_int(&s);
			outObj.regions.back().bottom = MFS_double(&s);
			outObj.regions.back().right  = MFS_double(&s);
			outObj.regions.back().top    = MFS_double(&s);
		}
/******************************************************************************************************************************/
		// MANIPS (920)
/******************************************************************************************************************************/
		// ATTR_manip_none
		else if (MFS_string_match(&s, "ATTR_manip_none", true))
		{
			cmd.cmd = attr_Manip_None;
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ATTR_manip_drag_xy <cursor> <dx> <dy> <v1min> <v1max> <v2min> <v2max> <dref1> <dref> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_drag_xy", false))
		{
			cmd.cmd = attr_Manip_Drag_2d;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.axis[0] = MFS_double(&s);
			manip.axis[1] = MFS_double(&s);
			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			manip.v2_min = MFS_double(&s);
			manip.v2_max = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string(&s, &manip.dataref2);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_drag_axis <cursor> <dx> <dy> <dz> <v1> <v2> <dataref> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_drag_axis", false))
		{
			cmd.cmd = attr_Manip_Drag_Axis;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.axis[0] = MFS_double(&s);
			manip.axis[1] = MFS_double(&s);
			manip.axis[2] = MFS_double(&s);
			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_command <currsor> <cmnd> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_command", false))
		{
			cmd.cmd = attr_Manip_Command;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_command_axis <cursor> <dx> <dy> <dz> <positive cmnd> <negative cmnd> <tool tip>
		else if (MFS_string_match(&s, "ATTR_manip_command_axis", false))
		{
			cmd.cmd = attr_Manip_Command_Axis;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.axis[0] = MFS_double(&s);
			manip.axis[1] = MFS_double(&s);
			manip.axis[2] = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string(&s, &manip.dataref2);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_noop
		else if (MFS_string_match(&s, "ATTR_manip_noop", true))
		{
			cmd.cmd = attr_Manip_Noop;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;		
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);			
		}
/******************************************************************************************************************************/
		// LIGHT LEVEL (930)
/******************************************************************************************************************************/
		else if (MFS_string_match(&s, "ATTR_light_level", false))
		{
			cmd.cmd = attr_Light_Level;
			cmd.params[0] = MFS_double(&s);
			cmd.params[1] = MFS_double(&s);
			MFS_string(&s, &cmd.name);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ATTR_manip_push <cursor> <v1max> <v1min> <dref1> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_push", false))
		{
			cmd.cmd = attr_Manip_Push;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.v1_max = MFS_double(&s);
			manip.v1_min = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_radio <cursor> <v1max> <dref1> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_radio", false))
		{
			cmd.cmd = attr_Manip_Radio;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.v1_max = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_toggle <cursor> <v1max> <v1min> <dref1> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_toggle", false))
		{
			cmd.cmd = attr_Manip_Toggle;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.v1_max = MFS_double(&s);
			manip.v1_min = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}

		// ATTR_manip_delta <cursor> <v1max> <v1min> <dref1> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_delta", false))
		{
			cmd.cmd = attr_Manip_Delta;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			manip.v2_min = MFS_double(&s);
			manip.v2_max = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_wrap <cursor> <v1max> <v1min> <dref1> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_wrap", false))
		{
			cmd.cmd = attr_Manip_Wrap;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			manip.v2_min = MFS_double(&s);
			manip.v2_max = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
/******************************************************************************************************************************/
		// NEW MANIPS (1050)
/******************************************************************************************************************************/
		else if (MFS_string_match(&s, "ATTR_manip_wheel", false))
		{
			if(!outObj.manips.empty())
				outObj.manips.back().mouse_wheel_delta = MFS_double(&s);
		}
		// ATTR_manip_drag_axis_pix <cursor> <dx_pix> <step> <exp> <v1> <v2> <dataref> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_drag_axis_pix", false))
		{
			cmd.cmd = attr_Manip_Drag_Axis_Pix;;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			manip.axis[0] = MFS_double(&s);
			manip.axis[1] = MFS_double(&s);
			manip.axis[2] = MFS_double(&s);
			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_command_knob <cursor> <positive cmnd> <negative cmnd> <tool tip>
		else if (MFS_string_match(&s, "ATTR_manip_command_knob", false))
		{
			cmd.cmd = attr_Manip_Command_Knob;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			MFS_string(&s, &manip.dataref1);
			MFS_string(&s, &manip.dataref2);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln = true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_command_switch_up_down <cursor> <positive cmnd> <negative cmnd> <tool tip>
		else if (MFS_string_match(&s, "ATTR_manip_command_switch_up_down", false))
		{
			cmd.cmd = attr_Manip_Command_Switch_Up_Down;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			MFS_string(&s, &manip.dataref1);
			MFS_string(&s, &manip.dataref2);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln = true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_command_switch_left_right <cursor> <positive cmnd> <negative cmnd> <tool tip>
		else if (MFS_string_match(&s, "ATTR_manip_command_switch_left_right", false))
		{
			cmd.cmd = attr_Manip_Command_Switch_Left_Right;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			MFS_string(&s, &manip.dataref1);
			MFS_string(&s, &manip.dataref2);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln = true;
			outObj.manips.push_back(manip);
		}

		// ATTR_manip_axis_switch_left_right <cursor>  <v1> <v2> <click step> <hold step> <dref> <tool tip>
		else if (MFS_string_match(&s, "ATTR_manip_axis_knob", false))
		{
			cmd.cmd = attr_Manip_Axis_Knob;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);

			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			manip.axis[0] = MFS_double(&s);
			manip.axis[1] = MFS_double(&s);

			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln = true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_axis_switch_up_down <cursor>  <v1> <v2> <click step> <hold step> <dref> <tool tip>
		else if (MFS_string_match(&s, "ATTR_manip_axis_switch_up_down", false))
		{
			cmd.cmd = attr_Manip_Axis_Switch_Up_Down;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);

			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			manip.axis[0] = MFS_double(&s);
			manip.axis[1] = MFS_double(&s);

			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln = true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_axis_switch_left_right <cursor>  <v1> <v2> <click step> <hold step> <dref> <tool tip>
		else if (MFS_string_match(&s, "ATTR_manip_axis_switch_left_right", false))
		{
			cmd.cmd = attr_Manip_Axis_Switch_Left_Right;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);

			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			manip.axis[0] = MFS_double(&s);
			manip.axis[1] = MFS_double(&s);

			MFS_string(&s, &manip.dataref1);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln = true;
			outObj.manips.push_back(manip);
		}
/******************************************************************************************************************************/
		// PARTICLE SYSTEM
/******************************************************************************************************************************/
		// PARTICLE_SYSTEM <def name>
		else if (MFS_string_match(&s, "PARTICLE_SYSTEM", false))
		{
			MFS_string(&s, &outObj.particle_system);
		}
		// EMITTER name x y z psi the phi low high dref
		else if (MFS_string_match(&s, "EMITTER", false))
		{
			cmd.cmd = attr_Emitter;
			cmd.idx_offset = outObj.emitters.size();
			outObj.lods.back().cmds.push_back(cmd);
		
			XObjEmitter8	em;
			MFS_string(&s, &em.name);

			em.x = MFS_double(&s);
			em.y = MFS_double(&s);
			em.z = MFS_double(&s);
			em.psi = MFS_double(&s);
			em.the = MFS_double(&s);
			em.phi = MFS_double(&s);
			em.v_min = MFS_double(&s);
			em.v_max = MFS_double(&s);

			MFS_string(&s, &em.dataref);

			outObj.emitters.push_back(em);
		}
/******************************************************************************************************************************/
		// V11 NEW STUFF
/******************************************************************************************************************************/
		// ATTR_cockpit_device <device> <bus> <rheostat> <auto_adjust>
		else if (MFS_string_match(&s, "ATTR_cockpit_device", false))
		{
			cmd.cmd = attr_Cockpit_Device;
			MFS_string(&s, &cmd.name);
			cmd.params[0] = MFS_double(&s);
			cmd.params[1] = MFS_double(&s);
			cmd.params[2] = MFS_double(&s);
			outObj.lods.back().cmds.push_back(cmd);
		}
		else if (MFS_string_match(&s, "NORMAL_METALNESS", true))
		{
			outObj.use_metalness = 1;
		}
		else if (MFS_string_match(&s, "BLEND_GLASS", true))
		{
			outObj.glass_blending = 1;
		}
		// ATTR_axis_detented <dx> <dy> <dz> <v1_min> <v1_max> <dref>
		else if (MFS_string_match(&s, "ATTR_axis_detented", false))
		{
			XObjManip8& manip(outObj.manips.back());
			
			manip.centroid[0] = MFS_double(&s);	// centroid gets orthgonal lift axis
			manip.centroid[1] = MFS_double(&s);
			manip.centroid[2] = MFS_double(&s);
			manip.v2_min = MFS_double(&s);
			manip.v2_max = MFS_double(&s);
			
			MFS_string(&s, &manip.dataref2);
		}
		// ATTR_manip_drag_rotate <cursor> <x> <y> <z> <dx> <dy> <dz> <ange1> <angle2> <lift> <v1min> <v1max> <v2min> <v2max> <dataref1> <dataref2> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_drag_rotate", false))
		{
			cmd.cmd = attr_Manip_Drag_Rotate;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;

			MFS_string(&s, &manip.cursor);
			manip.centroid[0] = MFS_double(&s);
			manip.centroid[1] = MFS_double(&s);
			manip.centroid[2] = MFS_double(&s);
			manip.axis[0] = MFS_double(&s);
			manip.axis[1] = MFS_double(&s);
			manip.axis[2] = MFS_double(&s);
			manip.angle_min = MFS_double(&s);
			manip.angle_max = MFS_double(&s);
			manip.lift		= MFS_double(&s);
			manip.v1_min = MFS_double(&s);
			manip.v1_max = MFS_double(&s);
			manip.v2_min = MFS_double(&s);
			manip.v2_max = MFS_double(&s);
			
			MFS_string(&s, &manip.dataref1);
			MFS_string(&s, &manip.dataref2);
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_keyframe dref angle
		else if (MFS_string_match(&s, "ATTR_manip_keyframe", false))
		{
			XObjKey k;
			k.key  = MFS_double(&s);
			k.v[0] = MFS_double(&s);
			outObj.manips.back().rotation_key_frames.push_back(k);
		}
		// ATTR_axis_detent_range <lo> <hi> <height>
		else if (MFS_string_match(&s, "ATTR_axis_detent_range", false))
		{
			XObjDetentRange d;
			d.lo = MFS_double(&s);
			d.hi = MFS_double(&s);
			d.height = MFS_double(&s);
			outObj.manips.back().detents.push_back(d);
		}
		// ATTR_manip_command_switch_left_right2 <currsor> <cmnd> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_command_switch_left_right2", false))
		{
			cmd.cmd = attr_Manip_Command_Switch_Left_Right2;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			MFS_string(&s, &manip.dataref1);
			manip.dataref2 = manip.dataref1;
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_command_switch_up_down2 <cursor> <cmnd> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_command_switch_up_down2", false))
		{
			cmd.cmd = attr_Manip_Command_Switch_Up_Down2;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			MFS_string(&s, &manip.dataref1);
			manip.dataref2 = manip.dataref1;
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		// ATTR_manip_command_knob2 <currsor> <cmnd> <tooltip>
		else if (MFS_string_match(&s, "ATTR_manip_command_knob2", false))
		{
			cmd.cmd = attr_Manip_Command_Knob2;
			cmd.idx_offset = outObj.manips.size();
			outObj.lods.back().cmds.push_back(cmd);
			XObjManip8 manip;
			manip.mouse_wheel_delta = 0.0f;
			MFS_string(&s, &manip.cursor);
			MFS_string(&s, &manip.dataref1);
			manip.dataref2 = manip.dataref1;
			MFS_string_eol(&s, &manip.tooltip);
			ate_eoln=true;
			outObj.manips.push_back(manip);
		}
		else if (MFS_string_match(&s, "MAGNET", false))
		{
			cmd.cmd = attr_Magnet;
			// SKIP magnet name - we always write 'magnet'
			MFS_string(&s, &cmd.name);
			// Scan a second time to pick up type, e.g. ipad
			MFS_string(&s, &cmd.name);
	
			for(int i = 0; i < 6; ++i)
				cmd.params[i] = MFS_double(&s);

			outObj.lods.back().cmds.push_back(cmd);
		}
		else if (MFS_string_match(&s, "LOAD_CENTER", false))
		{
			outObj.loadCenter_latlon[0] = MFS_double(&s);
			outObj.loadCenter_latlon[1] = MFS_double(&s);
			MFS_double(&s);
			outObj.loadCenter_texSize = MFS_double(&s);
			outObj.fixed_heading = 0.0;
		}
		else if (MFS_string_match(&s, "#fixed_heading", false))
		{
			outObj.fixed_heading = MFS_double(&s);
		}
		else if (MFS_string_match(&s, "#viewpoint_height", false))
		{
			outObj.viewpoint_height = MFS_double(&s);
		}
		else if (MFS_string_match(&s, "#wed_text", false))
		{
			MFS_string_eol(&s, &outObj.description);
			ate_eoln=true;
		}

/******************************************************************************************************************************/
		// DEFAULT
/******************************************************************************************************************************/
		else
		// Common attribute handling:
		{
			string	attr_name;
			MFS_string(&s, &attr_name);
			int cmd_idx = FindObjCmd(attr_name.c_str(), true);
			if (cmd_idx != gCmdCount)
			{
				cmd.cmd = gCmds[cmd_idx].cmd_id;
				for (int n = 0; n < gCmds[cmd_idx].elem_count; ++n)
				{
					cmd.params[n] = MFS_double(&s);
				}
				outObj.lods.back().cmds.push_back(cmd);
			}

		}

		if(!ate_eoln)
			MFS_string_eol(&s, NULL);
	} // While loop
	MemFile_Close(objFile);

	if(trimax != tricount)
		LOG_MSG("E/Obj number of %s do not match POINT_COUNTS in %s\n", "VT", inFile);
	if(linemax != linecount)
		LOG_MSG("E/Obj number of %s do not match POINT_COUNTS in %s\n", "VLINE", inFile);
	if(lightmax != lightcount)
		LOG_MSG("E/Obj number of %s do not match POINT_COUNTS in %s\n", "VLIGHT", inFile);
	if(idxmax != idxcount)
		LOG_MSG("E/Obj number of %s do not match POINT_COUNTS in %s\n", "IDX", inFile);
	if(anims != 0)
		LOG_MSG("E/Obj imbalanced # ANIM_begin/end commands in %s\n", inFile);
	outObj.geo_tri.get_minmax(outObj.xyz_min,outObj.xyz_max);
	
	return true;
}


/****************************************************************************************
 * OBJ 8 WRITE
 ****************************************************************************************/
bool	XObj8Write(const char * inFile, const XObj8& outObj, const char * comment)
{
	int n;

	FILE * fi = fopen(inFile, "wb");
	if (fi == NULL) return false;
	const float * v;

	// HEADER
	fprintf(fi, "%c" CRLF "800 %s" CRLF "OBJ" CRLF CRLF, APL ? 'A' : 'I', comment ? comment : "");

	if (outObj.loadCenter_texSize)
	{
		fprintf(fi, "LOAD_CENTER %.5lf %.5lf %.1lf %d" CRLF,
			outObj.loadCenter_latlon[0],
			outObj.loadCenter_latlon[1],
			outObj.loadCenter_size,
			outObj.loadCenter_texSize);
	}

	// TEXTURES
	fprintf(fi, "TEXTURE %s" CRLF, outObj.texture.c_str());
	if (!outObj.texture_lit.empty())fprintf(fi, "TEXTURE_LIT %s" CRLF, outObj.texture_lit.c_str());
	if (!outObj.texture_normal_map.empty())fprintf(fi, "TEXTURE_NORMAL %s" CRLF, outObj.texture_normal_map.c_str());

	if(outObj.use_metalness)
		fprintf(fi,"NORMAL_METALNESS" CRLF);
	if(outObj.glass_blending)
		fprintf(fi,"BLEND_GLASS" CRLF);

	if(!outObj.particle_system.empty())
		fprintf(fi,"PARTICLE_SYSTEM %s" CRLF, outObj.particle_system.c_str());

	// SUBREGIONS
	for (int r = 0; r < outObj.regions.size(); ++r)
	{
		fprintf(fi,"COCKPIT_REGION %d %d %d %d" CRLF,
			outObj.regions[r].left,
			outObj.regions[r].bottom,
			outObj.regions[r].right,
			outObj.regions[r].top);
	}

	// POINT POOLS
	fprintf(fi, "POINT_COUNTS %d %d %d %llu" CRLF, outObj.geo_tri.count(), outObj.geo_lines.count(), outObj.geo_lights.count(), (unsigned long long)outObj.indices.size());

	bool hi_res =
		outObj.xyz_max[0] - outObj.xyz_min[0] < 30.0f &&
		outObj.xyz_max[1] - outObj.xyz_min[1] < 30.0f &&
		outObj.xyz_max[2] - outObj.xyz_min[2] < 30.0f;

	for (n = 0; n < outObj.geo_tri.count(); ++n)
	{
		v = outObj.geo_tri.get(n);
		fprintf(fi, hi_res ? "VT %.3f %.3f %.3f %.4f %.4f %.4f %.4f %.4f" CRLF
			               : "VT %.2f %.2f %.2f %.4f %.4f %.4f %.4f %.4f" CRLF, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
	}

	for (n = 0; n < outObj.geo_lines.count(); ++n)
	{
		v = outObj.geo_lines.get(n);
		fprintf(fi, hi_res ? "VLINE %.3f %.3f %.3f %f %f %f" CRLF
		                   : "VLINE %.2f %.2f %.2f %f %f %f" CRLF, v[0], v[1], v[2], v[3], v[4], v[5]);
	}	

	for (n = 0; n < outObj.geo_lights.count(); ++n)
	{
		v = outObj.geo_lights.get(n);
		fprintf(fi, hi_res ? "VLIGHT %.3f %.3f %.3f %f %f %f" CRLF
		                   : "VLIGHT %.2f %.2f %.2f %f %f %f" CRLF, v[0], v[1], v[2], v[3], v[4], v[5]);
	}

	int extra = outObj.indices.size() % 10;
	int trans = outObj.indices.size() - extra;

	for (n = 0; n < (int)outObj.indices.size(); ++n)
	{
		if (n >= trans) fprintf(fi, "IDX");
		else if ((n % 10) == 0) fprintf(fi, "IDX10");
		fprintf(fi, " %d", outObj.indices[n]);
		if (n >= trans) fprintf(fi, CRLF);
		else if ((n % 10) == 9) fprintf(fi, CRLF);
	}

	// CMDS

	for (vector<XObjLOD8>::const_iterator lod = outObj.lods.begin(); lod != outObj.lods.end(); ++lod)
	{
		if (lod->lod_far != 0.0)
		{
			fprintf(fi, "ATTR_LOD %.1f %.1f" CRLF, lod->lod_near, lod->lod_far);
		}

		for (vector<XObjCmd8>::const_iterator cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
		{
			switch(cmd->cmd) {
			case anim_Rotate:
				if (outObj.animation[cmd->idx_offset].keyframes.size() == 2)
					fprintf(fi, "ANIM_rotate %f %f %f %f %f %f %f %s" CRLF,
						outObj.animation[cmd->idx_offset].axis[0],
						outObj.animation[cmd->idx_offset].axis[1],
						outObj.animation[cmd->idx_offset].axis[2],
						outObj.animation[cmd->idx_offset].keyframes[0].v[0],
						outObj.animation[cmd->idx_offset].keyframes[1].v[0],
						outObj.animation[cmd->idx_offset].keyframes[0].key,
						outObj.animation[cmd->idx_offset].keyframes[1].key,
						outObj.animation[cmd->idx_offset].dataref.c_str());
				else
				{
					fprintf(fi, "ANIM_rotate_begin %f %f %f %s" CRLF,
						outObj.animation[cmd->idx_offset].axis[0],
						outObj.animation[cmd->idx_offset].axis[1],
						outObj.animation[cmd->idx_offset].axis[2],
						outObj.animation[cmd->idx_offset].dataref.c_str());
					for(n = 0; n < outObj.animation[cmd->idx_offset].keyframes.size(); ++n)
						fprintf(fi, "ANIM_rotate_key %f %f" CRLF,
							outObj.animation[cmd->idx_offset].keyframes[n].key,
							outObj.animation[cmd->idx_offset].keyframes[n].v[0]);
					fprintf(fi, "ANIM_rotate_end" CRLF);
				}
				if(outObj.animation[cmd->idx_offset].loop)
					fprintf(fi,"ANIM_keyframe_loop %f" CRLF, outObj.animation[cmd->idx_offset].loop);
				break;
			case anim_Translate:
				if (outObj.animation[cmd->idx_offset].keyframes.size() == 2)
					fprintf(fi, "ANIM_trans %f %f %f %f %f %f %f %f %s" CRLF,
						outObj.animation[cmd->idx_offset].keyframes[0].v[0],
						outObj.animation[cmd->idx_offset].keyframes[0].v[1],
						outObj.animation[cmd->idx_offset].keyframes[0].v[2],
						outObj.animation[cmd->idx_offset].keyframes[1].v[0],
						outObj.animation[cmd->idx_offset].keyframes[1].v[1],
						outObj.animation[cmd->idx_offset].keyframes[1].v[2],
						outObj.animation[cmd->idx_offset].keyframes[0].key,
						outObj.animation[cmd->idx_offset].keyframes[1].key,
						outObj.animation[cmd->idx_offset].dataref.c_str());
				else
				{
					fprintf(fi, "ANIM_trans_begin %s" CRLF,
						outObj.animation[cmd->idx_offset].dataref.c_str());
					for(n = 0; n < outObj.animation[cmd->idx_offset].keyframes.size(); ++n)
						fprintf(fi, "ANIM_trans_key %f %f %f %f" CRLF,
							outObj.animation[cmd->idx_offset].keyframes[n].key,
							outObj.animation[cmd->idx_offset].keyframes[n].v[0],
							outObj.animation[cmd->idx_offset].keyframes[n].v[1],
							outObj.animation[cmd->idx_offset].keyframes[n].v[2]);
					fprintf(fi, "ANIM_trans_end" CRLF);
				}
				if(outObj.animation[cmd->idx_offset].loop)
					fprintf(fi,"ANIM_keyframe_loop %f" CRLF, outObj.animation[cmd->idx_offset].loop);
				break;
			case obj8_Tris:
				fprintf(fi, "TRIS %d %d" CRLF, cmd->idx_offset, cmd->idx_count);
				break;
			case obj8_Lines:
				fprintf(fi, "LINES %d %d" CRLF, cmd->idx_offset, cmd->idx_count);
				break;
			case obj8_Lights:
				fprintf(fi, "LIGHTS %d %d" CRLF, cmd->idx_offset, cmd->idx_count);
				break;
			// OBJ 850 crap
			case obj8_LightCustom:
				fprintf(fi, "LIGHT_CUSTOM %f %f %f %f %f %f %f %f %f %f %f %f %s" CRLF,
					cmd->params[0], cmd->params[1 ], cmd->params[2 ],
					cmd->params[3], cmd->params[4 ], cmd->params[5 ],
					cmd->params[6], cmd->params[7 ], cmd->params[8 ],
					cmd->params[9], cmd->params[10], cmd->params[11], cmd->name.c_str());
				break;
			case obj8_LightNamed:
				if(cmd->idx_count == 0)
					fprintf(fi,"LIGHT_NAMED %s %f %f %f" CRLF, cmd->name.c_str(),
						cmd->params[0], cmd->params[1 ], cmd->params[2 ]);
				else
				{
					fprintf(fi,"LIGHT_PARAM %s %f %f %f", cmd->name.c_str(),
						cmd->params[0], cmd->params[1 ], cmd->params[2 ]);
					for(int p = 0; p < cmd->idx_count; ++p)
						fprintf(fi," %.2f",cmd->params[3+p]);
					fprintf(fi,CRLF);
				}
				break;
			case attr_Layer_Group:
				fprintf(fi,"ATTR_layer_group %s %d" CRLF, cmd->name.c_str(), (int) cmd->params[0]);
				break;
			case anim_Hide:
				if (outObj.animation[cmd->idx_offset].keyframes.size() == 2)
					fprintf(fi, "ANIM_hide %f %f %s" CRLF,
						outObj.animation[cmd->idx_offset].keyframes[0].key,
						outObj.animation[cmd->idx_offset].keyframes[1].key,
						outObj.animation[cmd->idx_offset].dataref.c_str());
				if(outObj.animation[cmd->idx_offset].loop)
					fprintf(fi,"ANIM_keyframe_loop %f" CRLF, outObj.animation[cmd->idx_offset].loop);
				break;
			case anim_Show:
				if (outObj.animation[cmd->idx_offset].keyframes.size() == 2)
					fprintf(fi, "ANIM_show %f %f %s" CRLF,
						outObj.animation[cmd->idx_offset].keyframes[0].key,
						outObj.animation[cmd->idx_offset].keyframes[1].key,
						outObj.animation[cmd->idx_offset].dataref.c_str());
				if(outObj.animation[cmd->idx_offset].loop)
					fprintf(fi,"ANIM_keyframe_loop %f" CRLF, outObj.animation[cmd->idx_offset].loop);
				break;
			case attr_Hard:
				if (cmd->name == "object")
					fprintf(fi,"ATTR_hard" CRLF);
				else
					fprintf(fi,"ATTR_hard %s" CRLF, cmd->name.c_str());
				break;
			case attr_Hard_Deck:
				if (cmd->name == "object")
					fprintf(fi,"ATTR_hard_deck" CRLF);
				else
					fprintf(fi,"ATTR_hard_deck %s" CRLF, cmd->name.c_str());
				break;
			case attr_No_Blend:
				if (cmd->params[0] == 0.5)
					fprintf(fi,"ATTR_no_blend" CRLF);
				else
					fprintf(fi,"ATTR_no_blend %f" CRLF, cmd->params[0]);
				break;
			case attr_Shadow:
				fprintf(fi, "ATTR_shadow" CRLF);
				break;
			case attr_No_Shadow:
					fprintf(fi, "ATTR_no_shadow" CRLF);
				break;
			case attr_Tex_Cockpit_Subregion:
				fprintf(fi,"ATTR_cockpit_region %d" CRLF, (int) cmd->params[0]);
				break;

			case attr_Manip_Drag_2d:
				fprintf(fi,"ATTR_manip_drag_xy %s %f %f %f %f %f %f %s %s %s" CRLF,
					outObj.manips[cmd->idx_offset].cursor.c_str(),
					outObj.manips[cmd->idx_offset].axis[0],
					outObj.manips[cmd->idx_offset].axis[1],
					outObj.manips[cmd->idx_offset].v1_min,
					outObj.manips[cmd->idx_offset].v1_max,
					outObj.manips[cmd->idx_offset].v2_min,
					outObj.manips[cmd->idx_offset].v2_max,
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref2.c_str()),
					outObj.manips[cmd->idx_offset].tooltip.c_str());
				break;
			case attr_Manip_Drag_Axis:
				fprintf(fi,"ATTR_manip_drag_axis %s %f %f %f %f %f %s %s" CRLF,
					outObj.manips[cmd->idx_offset].cursor.c_str(),
					outObj.manips[cmd->idx_offset].axis[0],
					outObj.manips[cmd->idx_offset].axis[1],
					outObj.manips[cmd->idx_offset].axis[2],
					outObj.manips[cmd->idx_offset].v1_min,
					outObj.manips[cmd->idx_offset].v1_max,
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
					outObj.manips[cmd->idx_offset].tooltip.c_str());

				if(!outObj.manips[cmd->idx_offset].dataref2.empty())
				{
					fprintf(fi,"ATTR_axis_detented %f %f %f\t%f %f %s" CRLF,
					outObj.manips[cmd->idx_offset].centroid[0],
					outObj.manips[cmd->idx_offset].centroid[1],
					outObj.manips[cmd->idx_offset].centroid[2],
					outObj.manips[cmd->idx_offset].v2_min,
					outObj.manips[cmd->idx_offset].v2_max,
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref2.c_str()));
				}
				for(vector<XObjDetentRange>::const_iterator di = outObj.manips[cmd->idx_offset].detents.begin();
					di != outObj.manips[cmd->idx_offset].detents.end(); ++di)
				{
					fprintf(fi,"ATTR_axis_detent_range %f %f %f" CRLF, di->lo, di->hi, di->height);
				}
					
				if(outObj.manips[cmd->idx_offset].mouse_wheel_delta != 0)
					fprintf(fi,"ATTR_manip_wheel %f" CRLF, outObj.manips[cmd->idx_offset].mouse_wheel_delta);
				break;
			case attr_Manip_Command:
				fprintf(fi,"ATTR_manip_command %s %s %s" CRLF,
					outObj.manips[cmd->idx_offset].cursor.c_str(),
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
					outObj.manips[cmd->idx_offset].tooltip.c_str());
				break;
			case attr_Manip_Command_Axis:
				fprintf(fi,"ATTR_manip_command_axis %s %f %f %f %s %s %s" CRLF,
					outObj.manips[cmd->idx_offset].cursor.c_str(),
					outObj.manips[cmd->idx_offset].axis[0],
					outObj.manips[cmd->idx_offset].axis[1],
					outObj.manips[cmd->idx_offset].axis[2],
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref2.c_str()),
					outObj.manips[cmd->idx_offset].tooltip.c_str());
				break;
			case attr_Manip_Noop:
				fprintf(fi,"ATTR_manip_noop %s %s" CRLF,
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
					outObj.manips[cmd->idx_offset].tooltip.c_str());
				break;
			case attr_Light_Level:
				fprintf(fi,"ATTR_light_level %f %f %s" CRLF, cmd->params[0], cmd->params[1],cmd->name.c_str());
				break;
				
			case attr_Manip_Push:
					fprintf(fi,"ATTR_manip_push %s %f %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].v1_max,
						outObj.manips[cmd->idx_offset].v1_min,
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					if(outObj.manips[cmd->idx_offset].mouse_wheel_delta != 0)
						fprintf(fi,"ATTR_manip_wheel %f" CRLF, outObj.manips[cmd->idx_offset].mouse_wheel_delta);
					break;
			case attr_Manip_Radio:
					fprintf(fi,"ATTR_manip_radio %s %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].v1_max,
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					if(outObj.manips[cmd->idx_offset].mouse_wheel_delta != 0)
						fprintf(fi,"ATTR_manip_wheel %f" CRLF, outObj.manips[cmd->idx_offset].mouse_wheel_delta);
					break;
			case attr_Manip_Toggle:
					fprintf(fi,"ATTR_manip_toggle %s %f %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].v1_max,
						outObj.manips[cmd->idx_offset].v1_min,
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					if(outObj.manips[cmd->idx_offset].mouse_wheel_delta != 0)
						fprintf(fi,"ATTR_manip_wheel %f" CRLF, outObj.manips[cmd->idx_offset].mouse_wheel_delta);
					break;
					
			case attr_Manip_Delta:
					fprintf(fi,"ATTR_manip_delta %s %f %f %f %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].v1_min,
						outObj.manips[cmd->idx_offset].v1_max,
						outObj.manips[cmd->idx_offset].v2_min,
						outObj.manips[cmd->idx_offset].v2_max,
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					if(outObj.manips[cmd->idx_offset].mouse_wheel_delta != 0)
						fprintf(fi,"ATTR_manip_wheel %f" CRLF, outObj.manips[cmd->idx_offset].mouse_wheel_delta);
					break;

			case attr_Manip_Wrap:
					fprintf(fi,"ATTR_manip_wrap %s %f %f %f %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].v1_min,
						outObj.manips[cmd->idx_offset].v1_max,
						outObj.manips[cmd->idx_offset].v2_min,
						outObj.manips[cmd->idx_offset].v2_max,
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					if(outObj.manips[cmd->idx_offset].mouse_wheel_delta != 0)
						fprintf(fi,"ATTR_manip_wheel %f" CRLF, outObj.manips[cmd->idx_offset].mouse_wheel_delta);
					break;
			case attr_Manip_Drag_Axis_Pix:
					fprintf(fi,"ATTR_manip_drag_axis_pix %s %f %f %f %f %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].axis[0],
						outObj.manips[cmd->idx_offset].axis[1],
						outObj.manips[cmd->idx_offset].axis[2],
						outObj.manips[cmd->idx_offset].v1_min,
						outObj.manips[cmd->idx_offset].v1_max,
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					if(outObj.manips[cmd->idx_offset].mouse_wheel_delta != 0)
						fprintf(fi,"ATTR_manip_wheel %f" CRLF, outObj.manips[cmd->idx_offset].mouse_wheel_delta);
					break;		
			case attr_Manip_Command_Knob:
					fprintf(fi,"ATTR_manip_command_knob %s %s %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref2.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					break;
			case attr_Manip_Command_Switch_Up_Down:
					fprintf(fi,"ATTR_manip_command_switch_up_down %s %s %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref2.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					break;
			case attr_Manip_Command_Switch_Left_Right:
					fprintf(fi,"ATTR_manip_command_switch_left_right %s %s %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref2.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					break;
			case attr_Manip_Axis_Knob:
					fprintf(fi,"ATTR_manip_axis_knob %s %f %f %f %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].v1_min,
						outObj.manips[cmd->idx_offset].v1_max,
						outObj.manips[cmd->idx_offset].axis[0],
						outObj.manips[cmd->idx_offset].axis[1],
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					break;
			case attr_Manip_Axis_Switch_Up_Down:
					fprintf(fi,"ATTR_manip_axis_switch_up_down %s %f %f %f %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].v1_min,
						outObj.manips[cmd->idx_offset].v1_max,
						outObj.manips[cmd->idx_offset].axis[0],
						outObj.manips[cmd->idx_offset].axis[1],
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					break;
			case attr_Manip_Axis_Switch_Left_Right:
					fprintf(fi,"ATTR_manip_axis_switch_left_right %s %f %f %f %f %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].v1_min,
						outObj.manips[cmd->idx_offset].v1_max,
						outObj.manips[cmd->idx_offset].axis[0],
						outObj.manips[cmd->idx_offset].axis[1],
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					break;
			case attr_Emitter:
					fprintf(fi,"EMITTER %s %f %f %f %f %f %f %f %f %s" CRLF,
						outObj.emitters[cmd->idx_offset].name.c_str(),
						outObj.emitters[cmd->idx_offset].x,
						outObj.emitters[cmd->idx_offset].y,
						outObj.emitters[cmd->idx_offset].z,
						outObj.emitters[cmd->idx_offset].psi,
						outObj.emitters[cmd->idx_offset].the,
						outObj.emitters[cmd->idx_offset].phi,
						outObj.emitters[cmd->idx_offset].v_min,
						outObj.emitters[cmd->idx_offset].v_max,
						outObj.emitters[cmd->idx_offset].dataref.c_str());
					break;
			case attr_Cockpit_Device:
					fprintf(fi,"ATTR_cockpit_device %s %d %d %d" CRLF,
						cmd->name.c_str(),
						(int) cmd->params[0],
						(int) cmd->params[1],
						(int) cmd->params[2]);
					break;
			case attr_Manip_Drag_Rotate:
					fprintf(fi,"ATTR_manip_drag_rotate %s %f %f %f\t%f %f %f\t%f %f %f\t%f %f  %f %f %s %s %s" CRLF,
						outObj.manips[cmd->idx_offset].cursor.c_str(),
						outObj.manips[cmd->idx_offset].centroid[0],
						outObj.manips[cmd->idx_offset].centroid[1],
						outObj.manips[cmd->idx_offset].centroid[2],
						outObj.manips[cmd->idx_offset].axis[0],
						outObj.manips[cmd->idx_offset].axis[1],
						outObj.manips[cmd->idx_offset].axis[2],
						outObj.manips[cmd->idx_offset].angle_min,
						outObj.manips[cmd->idx_offset].angle_max,
						outObj.manips[cmd->idx_offset].lift,
						outObj.manips[cmd->idx_offset].v1_min,
						outObj.manips[cmd->idx_offset].v1_max,
						outObj.manips[cmd->idx_offset].v2_min,
						outObj.manips[cmd->idx_offset].v2_max,
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
						dataref_or_none(outObj.manips[cmd->idx_offset].dataref2.c_str()),
						outObj.manips[cmd->idx_offset].tooltip.c_str());
					
					for(vector<XObjKey>::const_iterator ki = outObj.manips[cmd->idx_offset].rotation_key_frames.begin();
						ki != outObj.manips[cmd->idx_offset].rotation_key_frames.end(); ++ki)
					{
						fprintf(fi,"ATTR_manip_keyframe %f %f" CRLF, ki->key, ki->v[0]);
					}
					for(vector<XObjDetentRange>::const_iterator di = outObj.manips[cmd->idx_offset].detents.begin();
						di != outObj.manips[cmd->idx_offset].detents.end(); ++di)
					{
						fprintf(fi,"ATTR_axis_detent_range %f %f %f" CRLF, di->lo, di->hi, di->height);
					}
					if(outObj.manips[cmd->idx_offset].mouse_wheel_delta != 0)
						fprintf(fi,"ATTR_manip_wheel %f" CRLF, outObj.manips[cmd->idx_offset].mouse_wheel_delta);
					
					break;
			case attr_Manip_Command_Switch_Left_Right2:
				fprintf(fi,"ATTR_manip_command_switch_left_right2 %s %s %s" CRLF,
					outObj.manips[cmd->idx_offset].cursor.c_str(),
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
					outObj.manips[cmd->idx_offset].tooltip.c_str());
				break;
			case attr_Manip_Command_Switch_Up_Down2:
				fprintf(fi,"ATTR_manip_command_switch_up_down2 %s %s %s" CRLF,
					outObj.manips[cmd->idx_offset].cursor.c_str(),
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
					outObj.manips[cmd->idx_offset].tooltip.c_str());
				break;
			case attr_Manip_Command_Knob2:
				fprintf(fi,"ATTR_manip_command_knob2 %s %s %s" CRLF,
					outObj.manips[cmd->idx_offset].cursor.c_str(),
					dataref_or_none(outObj.manips[cmd->idx_offset].dataref1.c_str()),
					outObj.manips[cmd->idx_offset].tooltip.c_str());
				break;
			case attr_Magnet:
				fprintf(fi,"MAGNET magnet %s %f %f %f  %f %f %f" CRLF,
					cmd->name.c_str(),
					cmd->params[0],
					cmd->params[1],
					cmd->params[2],

					cmd->params[3],
					cmd->params[4],
					cmd->params[5]);
				break;
			default:
				{
					int idx = FindIndexForCmd(cmd->cmd);
					if (idx != gCmdCount)
					{
						fprintf(fi, "%s", gCmds[idx].name);
						for (n = 0; n < gCmds[idx].elem_count; ++n)
							fprintf(fi, " %f", cmd->params[n]);
						fprintf(fi, CRLF);
					}
				}
				break;
			}
		}
	}
	fclose(fi);
	return true;
}

