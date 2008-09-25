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
#include "ObjUtilsGL.h"
#include "XUtils.h"
#include "XObjReadWrite.h"
#include "CompGeomUtils.h"
#include "MatrixUtils.h"
#include <gl.h>
#include <glext.h>


static	void	EmitNorm(const vector<vec_tex>& coords, int a, int b, int c)
{
	double	v1[3], v2[3], n[3];

	v1[0] = coords[b].v[0] - coords[a].v[0];
	v1[1] = coords[b].v[1] - coords[a].v[1];
	v1[2] = coords[b].v[2] - coords[a].v[2];

	v2[0] = coords[b].v[0] - coords[c].v[0];
	v2[1] = coords[b].v[1] - coords[c].v[1];
	v2[2] = coords[b].v[2] - coords[c].v[2];

	vec3_cross(n, v1, v2);
	vec3_normalize(n);

	glNormal3dv(n);
}

#if __POWERPC__
#define __stdcall
#endif

void	ObjectToDL(int inList, const XObj& inObj, int texNum, double inDist, bool build, PFNGLMULTITEXCOORD2FARBPROC multitexproc)
{
	bool	tex_on = true;
	int		last_cmd = -1, this_cmd;
	if (build)
		glNewList(inList, GL_COMPILE);

	bool	enabled = true;

	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin(); cmd != inObj.cmds.end(); ++cmd)
	{
		switch(cmd->cmdType) {
		case type_PtLine:
			if (enabled)
			{
				if (tex_on)
				{
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_ALPHA_TEST);
					glDisable(GL_BLEND);
					tex_on = false;
				}
				switch(cmd->cmdID) {
				case obj_Line:	this_cmd = GL_LINES;		break;
				case obj_Light:	this_cmd = GL_POINTS;		break;
				}
				if (this_cmd != last_cmd)
				{
					if (last_cmd != -1) glEnd();
					glBegin(this_cmd);
					last_cmd = this_cmd;
				}
				for (vector<vec_rgb>::const_iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
				{
					glColor3f(fabs(rgb->rgb[0]) / 10.0, fabs(rgb->rgb[1]) / 10.0, fabs(rgb->rgb[2]) / 10.0);
					glVertex3fv(rgb->v);
				}
			}
			break;


		case type_Poly:
			if (enabled)
			{
				if (!tex_on)
				{
					glEnable(GL_TEXTURE_2D);
					glEnable(GL_ALPHA_TEST);
					glEnable(GL_BLEND);
					tex_on = true;
					glColor3f(1.0, 1.0, 1.0);
				}
				switch(cmd->cmdID) {
				case obj_Tri:				this_cmd = GL_TRIANGLES;		break;
				case obj_Quad:				this_cmd = GL_QUADS;			break;
				case obj_Quad_Hard:			this_cmd = GL_QUADS;			break;
				case obj_Movie:				this_cmd = GL_QUADS;			break;
				case obj_Polygon:			this_cmd = GL_POLYGON;			break;
				case obj_Quad_Strip:		this_cmd = GL_QUAD_STRIP;		break;
				case obj_Tri_Strip:			this_cmd = GL_TRIANGLE_STRIP;	break;
				case obj_Tri_Fan:			this_cmd = GL_TRIANGLE_FAN;		break;
				}
				if (this_cmd != last_cmd)
				{
					if (last_cmd != -1) glEnd();
					glBegin(this_cmd);
					last_cmd = this_cmd;
				}
				int i = 0;
				for (vector<vec_tex>::const_iterator st = cmd->st.begin(); st != cmd->st.end(); ++st, ++i)
				{
					switch(cmd->cmdID) {
					case obj_Tri:
					case obj_Quad:
					case obj_Quad_Hard:
					case obj_Movie:
					case obj_Polygon:
						if (i == 0)
							EmitNorm(cmd->st, 0, 1, 2);
						break;
					case obj_Quad_Strip:
						if ((i % 2) && i > 2)
							EmitNorm(cmd->st, i-3, i-2, i);
						break;
					case obj_Tri_Strip:
						if (i > 1)
						{
							if (i % 2)
								EmitNorm(cmd->st, i-1, i-2, i);
							else
								EmitNorm(cmd->st, i-2, i-1, i);
						}
						break;
					case obj_Tri_Fan:
						if (i > 1)
							EmitNorm(cmd->st, 0, i-1, i);
						break;
					}
//					glTexCoord2fv(st->st);
					multitexproc(GL_TEXTURE0_ARB + texNum, st->st[0], st->st[1]);
					glVertex3fv(st->v);
				}
				switch(cmd->cmdID) {
				case obj_Polygon:
				case obj_Quad_Strip:
				case obj_Tri_Strip:
				case obj_Tri_Fan:
					glEnd();
					last_cmd = -1;
					break;
				}
			}
			break;


		case type_Attr:
			switch(cmd->cmdID) {
			case attr_LOD:
				enabled = (inDist > cmd->attributes[0]) && (inDist < cmd->attributes[1]);
				break;
			case attr_Cull:
				if (last_cmd != -1)	{ glEnd(); last_cmd = -1; }
				glEnable(GL_CULL_FACE);
				break;
			case attr_NoCull:
				if (last_cmd != -1)	{ glEnd(); last_cmd = -1; }
				glDisable(GL_CULL_FACE);
				break;
			}

			break;
		}

	}	// For loop

	if (last_cmd != -1)
		glEnd();
	if (build)
		glEndList();
}
