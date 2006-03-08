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
 
#include "XObjBuilder.h"
#include "XObjDefs.h"

XObjBuilder::XObjBuilder(XObj8 * inObj) : obj(inObj), lod(NULL)
{
	tex_repeat_s = 1.0;
	tex_repeat_t = 1.0;
	tex_offset_s = 0.0;
	tex_offset_t = 0.0;

	SetDefaultState();
}

XObjBuilder::~XObjBuilder()
{
	if (lod)	
		EndLOD();
}
 	
void	XObjBuilder::BeginLOD(float inNear, float inFar)
{
	if (lod) 
		EndLOD();
	obj->lods.push_back(XObjLOD8());
	lod = &obj->lods.back();
	lod->lod_near = inNear;
	lod->lod_far = inFar;
	SetDefaultState();
}

void	XObjBuilder::EndLOD(void)
{
	SetDefaultState();
	lod = NULL;
}

void	XObjBuilder::SetAttribute(int attr)
{
	switch(attr) {
	case attr_Shade_Flat:	flat = 1;		break;
	case attr_Shade_Smooth:	flat = 0;		break;
	case attr_NoCull:		two_sided = 1;	break;
	case attr_Cull:			two_sided = 0;	break;
	case attr_Tex_Cockpit:	cockpit = 1;	break;
	case attr_Tex_Normal:	cockpit = 0;	break;
	case attr_No_Blend:		no_blend = 1;	break;
	case attr_Blend:		no_blend = 0;	break;
	case attr_Hard:			hard = 1;		break;
	case attr_No_Hard:		hard = 0;		break;
	}
}

void	XObjBuilder::SetAttribute1(int attr, float v)
{
	switch(attr) {
	case attr_Offset: 	offset = v; break;
	case attr_Shiny_Rat:shiny  = v; break;
	}
}

void	XObjBuilder::SetAttribute3(int attr, float v[3])
{
	switch(attr) {
	case attr_Emission_RGB: 
		emission[0] = v[0]; emission[1] = v[1]; emission[2] = v[2];
		break;
	case attr_Diffuse_RGB: 
		diffuse[0] = v[0]; diffuse[1] = v[1]; diffuse[2] = v[2];
		break;
	}
}

void	XObjBuilder::AccumTri(float inTri[24])
{
	inTri[6 ] = inTri[6 ] * tex_repeat_s + tex_offset_s;
	inTri[7 ] = inTri[7 ] * tex_repeat_t + tex_offset_t;
	inTri[14] = inTri[14] * tex_repeat_s + tex_offset_s;
	inTri[15] = inTri[15] * tex_repeat_t + tex_offset_t;
	inTri[22] = inTri[22] * tex_repeat_s + tex_offset_s;
	inTri[23] = inTri[23] * tex_repeat_t + tex_offset_t;

	int		idx1 = obj->geo_tri.accumulate(inTri   );
	int		idx2 = obj->geo_tri.accumulate(inTri+8 );
	int		idx3 = obj->geo_tri.accumulate(inTri+16);

	int		start_i = obj->indices.size();

	obj->indices.push_back(idx1);
	obj->indices.push_back(idx2);	
	obj->indices.push_back(idx3);
	
	int		end_i = obj->indices.size();
	
	AssureLOD();
	SyncAttrs();
		
	if (lod->cmds.empty() || 
		lod->cmds.back().cmd != obj8_Tris ||
		lod->cmds.back().idx_count + lod->cmds.back().idx_offset != start_i)
	{
		XObjCmd8	cmd;
		cmd.cmd = obj8_Tris;
		cmd.idx_offset = start_i;
		cmd.idx_count = end_i - start_i;
		lod->cmds.push_back(cmd);
	} else {
		lod->cmds.back().idx_count += (end_i - start_i);
	}	
}

void	XObjBuilder::AccumLine(float inLine[12])
{
	int idx1 = obj->geo_lines.accumulate(inLine  );
	int idx2 = obj->geo_lines.accumulate(inLine+6);
	
	int		start_i = obj->indices.size();

	obj->indices.push_back(idx1);
	obj->indices.push_back(idx2);

	int		end_i = obj->indices.size();
	
	AssureLOD();
	
	if (lod->cmds.empty() || 
		lod->cmds.back().cmd != obj8_Lines ||
		lod->cmds.back().idx_count + lod->cmds.back().idx_offset != start_i)
	{
		XObjCmd8	cmd;
		cmd.cmd = obj8_Lines;
		cmd.idx_offset = start_i;
		cmd.idx_count = end_i - start_i;
		lod->cmds.push_back(cmd);
	} else {
		lod->cmds.back().idx_count += (end_i - start_i);
	}

}

void	XObjBuilder::AccumLight(float inPoint[6])
{
	int idx = obj->geo_lights.accumulate(inPoint);
	
	AssureLOD();
	if (!lod->cmds.empty() &&
		 lod->cmds.back().cmd == obj8_Lights &&
		 (lod->cmds.back().idx_offset + lod->cmds.back().idx_count) == idx)
	{
		lod->cmds.back().idx_count++;
	} else {
		lod->cmds.push_back(XObjCmd8());
		lod->cmds.back().cmd = obj8_Lights;
		lod->cmds.back().idx_offset = idx;
		lod->cmds.back().idx_count = 1;
	}	
}

void	XObjBuilder::AccumAnimBegin(void)
{
	AssureLOD(); 
	lod->cmds.push_back(XObjCmd8());
	lod->cmds.back().cmd = anim_Begin;	
}

void	XObjBuilder::AccumAnimEnd(void)
{
	AssureLOD(); 
	lod->cmds.push_back(XObjCmd8());
	lod->cmds.back().cmd = anim_End;
}

void	XObjBuilder::AccumTranslate(float xyz1[3], float xyz2[3], float v1, float v2, const char * ref)
{
	AssureLOD(); 
	XObjAnim8 anim;
	anim.dataref = ref;
	anim.xyzrv1[0] = xyz1[0];	anim.xyzrv2[0] = xyz2[0];
	anim.xyzrv1[1] = xyz1[1];   anim.xyzrv2[1] = xyz2[1];
	anim.xyzrv1[2] = xyz1[2];   anim.xyzrv2[2] = xyz2[2];
	anim.xyzrv1[4] = v1     ;   anim.xyzrv2[4] = v2     ;
	
	obj->animation.push_back(anim);
	lod->cmds.push_back(XObjCmd8());
	lod->cmds.back().cmd = anim_Translate;
	lod->cmds.back().idx_offset = obj->animation.size()-1;	
}

void	XObjBuilder::AccumRotate(float axis[3], float r1, float r2, float v1, float v2, const char * ref)
{
	AssureLOD(); 
	XObjAnim8 anim;
	anim.dataref = ref;
	anim.xyzrv1[0] = axis[0];	
	anim.xyzrv1[1] = axis[1];
	anim.xyzrv1[2] = axis[2];
	anim.xyzrv1[3] = r1     ;   anim.xyzrv2[3] = r2     ;
	anim.xyzrv1[4] = v1     ;   anim.xyzrv2[4] = v2     ;
	
	obj->animation.push_back(anim);
	lod->cmds.push_back(XObjCmd8());
	lod->cmds.back().cmd = anim_Rotate;
	lod->cmds.back().idx_offset = obj->animation.size()-1;	
}

void	XObjBuilder::AssureLOD(void)
{
	if (lod == NULL)
	{
		obj->lods.push_back(XObjLOD8());
		obj->lods.back().lod_near = 0;
		obj->lods.back().lod_far  = 0;	
		lod = &obj->lods.back();
	}

}

void	XObjBuilder::SetDefaultState(void)
{
	o_hard = hard = 0;
	o_flat = flat = 0;
	o_two_sided = two_sided = 0;
	o_no_blend = no_blend = 0;
	o_cockpit = cockpit = 0;
	o_offset = offset = 0.0;

	diffuse[0] = 1.0; diffuse[1] = 1.0; diffuse[2] = 1.0;
	o_diffuse[0] = 1.0; o_diffuse[1] = 1.0; o_diffuse[2] = 1.0;
	emission[0] = 0.0; emission[1] = 0.0; emission[2] = 0.0;
	o_emission[0] = 0.0; o_emission[1] = 0.0; o_emission[2] = 0.0;
	o_shiny = shiny = 0.0;
}

void XObjBuilder::SyncAttrs(void)
{
	if (flat != o_flat)
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = flat ? attr_Shade_Flat : attr_Shade_Smooth;
		o_flat = flat;
	}
	
	if (hard != o_hard)
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = hard ? attr_Hard : attr_No_Hard;
		o_hard = hard;
	}
	
	if (two_sided != o_two_sided)
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = two_sided ? attr_NoCull : attr_Cull;
		o_two_sided = two_sided;
	}
	
	if (cockpit != o_cockpit)
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = cockpit ? attr_Tex_Cockpit: attr_Tex_Normal;
		o_cockpit = cockpit;
	}
	
	if (no_blend != o_no_blend)
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = no_blend ? attr_No_Blend : attr_Blend;
		o_no_blend = no_blend;
	}

	if (shiny != o_shiny)
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = attr_Shiny_Rat;
		lod->cmds.back().params[0] = shiny;
		o_shiny = shiny;
	}

	if (offset != o_offset)
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = attr_Offset;
		lod->cmds.back().params[0] = offset;
		o_offset = offset;
	}

	if (emission[0] != o_emission[0] || emission[1] != o_emission[1] || emission[2] != o_emission[2])
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = attr_Emission_RGB;
		lod->cmds.back().params[0] = emission[0];
		lod->cmds.back().params[1] = emission[1];
		lod->cmds.back().params[2] = emission[2];
		o_emission[0] = emission[0];
		o_emission[1] = emission[1];
		o_emission[2] = emission[2];
	}

	if (diffuse[0] != o_diffuse[0] || diffuse[1] != o_diffuse[1] || diffuse[2] != o_diffuse[2])
	{
		lod->cmds.push_back(XObjCmd8()); 
		lod->cmds.back().cmd = attr_Diffuse_RGB;
		lod->cmds.back().params[0] = diffuse[0];
		lod->cmds.back().params[1] = diffuse[1];
		lod->cmds.back().params[2] = diffuse[2];
		o_diffuse[0] = diffuse[0];
		o_diffuse[1] = diffuse[1];
		o_diffuse[2] = diffuse[2];
	}
}

void	XObjBuilder::SetTexRepeatParams(float repeat_s, float repeat_t, float offset_s, float offset_t)
{
	tex_repeat_s = repeat_s;
	tex_repeat_t = repeat_t;
	tex_offset_s = offset_s;
	tex_offset_t = offset_t;
}

