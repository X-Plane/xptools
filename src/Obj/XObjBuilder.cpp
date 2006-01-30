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
	case attr_Shade_Flat:	if (!flat) { flat = 1; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	case attr_Shade_Smooth:	if ( flat) { flat = 0; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;		
	case attr_NoCull:		if (!two_sided) { two_sided = 1; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	case attr_Cull:			if ( two_sided) { two_sided = 0; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	case attr_Tex_Cockpit:	if (!cockpit) { cockpit = 1; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	case attr_Tex_Normal:	if ( cockpit) { cockpit = 0; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	case attr_No_Blend:		if (!no_blend) { no_blend = 1; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	case attr_Blend:		if ( no_blend) { no_blend = 0; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	case attr_Hard:			if (!hard) { hard = 1; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	case attr_No_Hard:		if ( hard) { hard = 0; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; }	break;
	}
}

void	XObjBuilder::SetAttribute1(int attr, float v)
{
	switch(attr) {
	case attr_Offset: 	if (v != offset) { offset = v; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; lod->cmds.back().params[0] = v; } break;
	case attr_Shiny_Rat:if (v != shiny ) { shiny = v; AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr; lod->cmds.back().params[0] = v; } break;
	}
}

void	XObjBuilder::SetAttribute3(int attr, float v[3])
{
	switch(attr) {
	case attr_Emission_RGB: 
		if (v[0] != emission[0] || v[1] != emission[1] || v[2] != emission[2])
		{
			emission[0] = v[0]; emission[1] = v[1]; emission[2] = v[2];
			AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr;
			lod->cmds.back().params[0] = v[0]; lod->cmds.back().params[1] = v[1]; lod->cmds.back().params[2] = v[2];
		}
		break;
	case attr_Diffuse_RGB: 
		if (v[0] != diffuse[0] || v[1] != diffuse[1] || v[2] != diffuse[2])
		{
			diffuse[0] = v[0]; diffuse[1] = v[1]; diffuse[2] = v[2];
			AssureLOD(); lod->cmds.push_back(XObjCmd8()); lod->cmds.back().cmd = attr;
			lod->cmds.back().params[0] = v[0]; lod->cmds.back().params[1] = v[1]; lod->cmds.back().params[2] = v[2];
		}
		break;
	}
}

void	XObjBuilder::AccumTri(float inTri[24])
{
	int		idx1 = obj->geo_tri.accumulate(inTri   );
	int		idx2 = obj->geo_tri.accumulate(inTri+8 );
	int		idx3 = obj->geo_tri.accumulate(inTri+16);

	int		start_i = obj->indices.size();

	obj->indices.push_back(idx1);
	obj->indices.push_back(idx2);	
	obj->indices.push_back(idx3);
	
	int		end_i = obj->indices.size();
	
	AssureLOD();
		
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

#if 0
 
 private:
 
 	void	AssureLOD(void);
 
 	XObj8 *		obj;
 	int			hard;
// 	int			no_depth;
 	int			flat;
 	int			two_sided;
 	int			no_blend;
 	int			cockpit;
 	float		offset;

//	float		ambient[3];			// Ambient not used - no ambient control in x-plane
	float		diffuse[3];			// Diffuse STRONGLY not recommended!  Use texture
	float		emission[3];		// Used for self-lit signs
//	float		specular[3];		// Specular not used - set automatically by shiny-rat!
	float		shiny;				// Used for metal

};
 
 #endif