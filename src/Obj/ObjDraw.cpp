/*
 * Copyright (c) 2005, Laminar Research.
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
#include "ObjDraw.h"
#include "XObjDefs.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include "glew.h"
#endif
#include <math.h>
#include <stdio.h>
#include <algorithm>

#if DEV
	static void GL_ERR(int err)
	{
		if (err != 0)
			printf("GL ERROR: %d\n", err);
	}
	#define	CHECK_GL_ERR		GL_ERR(glGetError());
#else
	#define	CHECK_GL_ERR	
#endif

static void	Default_SetupPoly(void * ref)
{
	glEnable(GL_TEXTURE_2D);
//	glColor3f(1.0, 1.0, 1.0);
}
static void 	Default_SetupLine(void * ref)
{
	glDisable(GL_TEXTURE_2D);
}
static void	Default_SetupLight(void * ref)
{
	glDisable(GL_TEXTURE_2D);
}
static void	Default_SetupMovie(void * ref)
{
	glEnable(GL_TEXTURE_2D);
//	glColor3f(1.0, 1.0, 1.0);
}
static void	Default_SetupPanel(void * ref)
{
	glEnable(GL_TEXTURE_2D);
//	glColor3f(1.0, 1.0, 1.0);
}
static void	Default_TexCoord(const float * st, void * ref)
{
	glTexCoord2fv(st);
}

static void Default_TexCoordPointer(int size, unsigned long type, long stride, const void * pointer, void * ref)
{
	glTexCoordPointer(size, type, stride, pointer);
}

static float	Default_GetAnimParam(const char * string, float v1, float v2, void * ref)
{
	return 0.0;
}

static void	Default_SetDraped(void * ref) { }
static void	Default_SetNoDraped(void * ref) { }

static	ObjDrawFuncs10_t sDefault = {
	Default_SetupPoly, Default_SetupLine, Default_SetupLight,
	Default_SetupMovie, Default_SetupPanel, Default_TexCoord, Default_TexCoordPointer, Default_GetAnimParam, 
	Default_SetDraped, Default_SetNoDraped
};

enum {
	drawMode_Non,
	drawMode_Tri,
	drawMode_Lin,
	drawMode_Lgt,
	drawMode_Pan,
	drawMode_Mov
};

enum {
	arrayMode_Non,
	arrayMode_Tri,
	arrayMode_Lin,
	arrayMode_Lgt
};

void	ObjDraw(const XObj& obj, float dist, ObjDrawFuncs10_t * funcs, void * ref)
{
	if (funcs == NULL) funcs = &sDefault;
	int 	drawMode = drawMode_Non;
	bool	do_draw = true;
	float	mat_col[3] = { 1.0, 1.0, 1.0 };

	for (vector<XObjCmd>::const_iterator cmd = obj.cmds.begin(); cmd != obj.cmds.end(); ++cmd)
	{
		int	want_mode = drawMode_Non;

		if (cmd->cmdID == attr_LOD)
			do_draw = dist >= cmd->attributes[0] && dist <= cmd->attributes[1];

		if (do_draw)
		{
			switch(cmd->cmdID) {
			case obj_Light:				want_mode = drawMode_Lgt;	break;
			case obj_Line:				want_mode = drawMode_Lin;	break;
			case obj_Tri:
			case obj_Quad:
			case obj_Quad_Hard:
			case obj_Polygon:
			case obj_Quad_Strip:
			case obj_Tri_Strip:
			case obj_Tri_Fan:			want_mode = drawMode_Tri;	break;
			case obj_Quad_Cockpit:		want_mode = drawMode_Pan;	break;
			case obj_Movie:				want_mode = drawMode_Mov;	break;
			}
			if (want_mode != drawMode_Non)
			{
				if (want_mode != drawMode)
				{
					switch(want_mode) {
					case drawMode_Tri:		funcs->SetupPoly_f(ref);	break;
					case drawMode_Lin:		funcs->SetupLine_f(ref);	break;
					case drawMode_Lgt:		funcs->SetupLight_f(ref);	break;
					case drawMode_Pan:		funcs->SetupPanel_f(ref);	break;
					case drawMode_Mov:		funcs->SetupMovie_f(ref);	break;
					}
				}
				drawMode = want_mode;
			}

			switch(cmd->cmdType) {
			case type_Poly:
				glColor3fv(mat_col);
				switch(cmd->cmdID) {
				case obj_Tri:				glBegin(GL_TRIANGLES);	break;
				case obj_Quad:
				case obj_Quad_Hard:
				case obj_Quad_Cockpit:
				case obj_Movie:				glBegin(GL_QUADS);	break;
				case obj_Polygon:			glBegin(GL_POLYGON);	break;
				case obj_Quad_Strip:		glBegin(GL_QUAD_STRIP);	break;
				case obj_Tri_Strip:			glBegin(GL_TRIANGLE_STRIP);	break;
				case obj_Tri_Fan:			glBegin(GL_TRIANGLE_FAN);	break;
				default:					glBegin(GL_TRIANGLES);	break;
				}
				for (vector<vec_tex>::const_iterator st = cmd->st.begin(); st != cmd->st.end(); ++st)
				{
					funcs->TexCoord_f(st->st,ref);
					glVertex3fv(st->v);
				}
				glEnd();
				break;
			case type_PtLine:
				switch(cmd->cmdID) {
				case obj_Line:				glBegin(GL_LINES);	break;
				case obj_Light:
				default:					glBegin(GL_POINTS);	break;
				}
				for (vector<vec_rgb>::const_iterator rgb = cmd->rgb.begin(); rgb != cmd->rgb.end(); ++rgb)
				{
					glColor3f(fabs(rgb->rgb[0]) * 0.1, fabs(rgb->rgb[1]) * 0.1, fabs(rgb->rgb[2]) * 0.1);
					glVertex3fv(rgb->v);
				}
				glEnd();
				break;
			case type_Attr:
				switch(cmd->cmdID) {
				case attr_Shade_Flat:	glShadeModel(GL_FLAT); break;
				case attr_Shade_Smooth: glShadeModel(GL_SMOOTH); break;
//				case attr_Ambient_RGB: { float rgba[4] = { cmd->attributes[0],cmd->attributes[1],cmd->attributes[2],1.0 }; glMaterialfv(GL_FRONT,GL_AMBIENT ,rgba);	} break;
				case attr_Diffuse_RGB:  mat_col[0] = cmd->attributes[0]; mat_col[1] = cmd->attributes[1]; mat_col[2] = cmd->attributes[2]; glColor3fv(mat_col);	 break;
				case attr_Emission_RGB:{ float rgba[4] = { cmd->attributes[0],cmd->attributes[1],cmd->attributes[2],1.0 }; glMaterialfv(GL_FRONT,GL_EMISSION,rgba);	} break;
//				case attr_Specular_RGB:{ float rgba[4] = { cmd->attributes[0],cmd->attributes[1],cmd->attributes[2],1.0 }; glMaterialfv(GL_FRONT,GL_SPECULAR,rgba);	} break;
				case attr_Shiny_Rat:	{ float rgba[4] = { cmd->attributes[0], cmd->attributes[0], cmd->attributes[0], 1.0 };
											glLightfv	(GL_LIGHT0,GL_SPECULAR ,rgba);
											glMaterialfv (GL_FRONT, GL_SPECULAR, rgba);
											glMateriali (GL_FRONT,GL_SHININESS,128); } break;
				case attr_No_Depth:		glDisable(GL_DEPTH_TEST);
				case attr_Depth:		glEnable(GL_DEPTH_TEST);
				case attr_Reset: { float amb[4] = { 0.2, 0.2, 0.2, 1.0 }, zero[4] = { 0.0, 0.0, 0.0, 1.0 }; mat_col[0] = mat_col[1] = mat_col[2] = 1.0;
//									glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
//									glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
									glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
									glMaterialfv(GL_FRONT, GL_EMISSION, zero);
									glColor3fv(mat_col);
									glMateriali (GL_FRONT,GL_SHININESS,0); } break;
				case attr_Cull:		glEnable(GL_CULL_FACE);		break;
				case attr_NoCull:	glDisable(GL_CULL_FACE);	break;
				case attr_Offset:	if (cmd->attributes[0] != 0)
					{	glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(-5.0*cmd->attributes[0],-1.0);glDepthMask(GL_FALSE);	} else
					{	glDisable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(0.0, 0.0);glDepthMask(GL_TRUE);	}
					break;
				}
				break;
			} // Case for type of cmd
		} // Drwaing enabled
	} // While loop on cmds
}

//inline float rescale_float(float x1, float x2, float y1, float y2, float x)
//{
//	if (x1 == x2) return y1;
//	return y1 + ((y2 - y1) * (x - x1) / (x2 - x1));
//}

struct compare_key {
	bool operator()(const XObjKey& lhs, float rhs) const {
		return lhs.key < rhs;
	}
};

inline float	extrap(float x1, float y1, float x2, float y2, float x)
{
	if (x1 == x2) return (y1 + y2) * 0.5;
	return (x-x1) * (y2 - y1) / (x2 - x1) + y1;
}

inline float	key_extrap(float input, const vector<XObjKey>& table, int n)
{
	if (table.empty()) return 0.0f;
	if (table.size() == 1) return table.front().v[n];
	if (table.size() == 2) return extrap(table[0].key,table[0].v[n],table[1].key,table[1].v[n],input);

	vector<XObjKey>::const_iterator i = std::lower_bound(table.begin(), table.end(), input, compare_key());
	vector<XObjKey>::const_iterator p1, p2;

		 if (i == table.end())		{ p1 = i-2; p2 = i-1; }
	else if (i->key == input)		{ return i->v[n];	}
	else if (i == table.begin())	{ p1 = i; p2 = i+1; }
	else							{ p1 = i-1; p2 = i; }

	return extrap(p1->key,p1->v[n],p2->key,p2->v[n], input);
}


void	ObjDraw8(const XObj8& obj, float dist, ObjDrawFuncs10_t * funcs, void * ref)
{
	CHECK_GL_ERR
	if (funcs == NULL) funcs = &sDefault;
	int 	drawMode = drawMode_Non;
	int		arrayMode = arrayMode_Non;
	bool	tex_is_cockpit = false;
	int 	want_draw;
	const XObjAnim8 * anim;

	float v;

	for (vector<XObjLOD8>::const_iterator lod = obj.lods.begin(); lod != obj.lods.end(); ++lod)
	if ((lod->lod_near <= dist && dist < lod->lod_far) || obj.lods.size() == 1)	// lods = 1 - maybe no lod directive, lod dist could be 0,0.
	{
		float	mat_col[3] = { 1.0, 1.0, 1.0 };

		for (vector<XObjCmd8>::const_iterator cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
		{
			switch(cmd->cmd) {
			case obj8_Tris:
				want_draw = tex_is_cockpit ? drawMode_Pan : drawMode_Tri;
				if (want_draw != drawMode) {
					if (want_draw == drawMode_Pan)	funcs->SetupPanel_f(ref);
					else							funcs->SetupPoly_f(ref);
					CHECK_GL_ERR
					drawMode = want_draw;
				}
				if (arrayMode_Tri != arrayMode)
				{
					arrayMode = arrayMode_Tri;
					const float * offset = obj.geo_tri.get(0);
#if XOBJ8_USE_VBO
					if(obj.geo_VBO)
					{
						glBindBuffer(GL_ARRAY_BUFFER, obj.geo_VBO);				CHECK_GL_ERR
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.idx_VBO);		CHECK_GL_ERR
					}
					else
					{
						GLuint x[2];
						glGenBuffers(2, x);									CHECK_GL_ERR
						const_cast<unsigned int&>(obj.geo_VBO) = x[0];
						const_cast<unsigned int&>(obj.idx_VBO) = x[1];

						glBindBuffer(GL_ARRAY_BUFFER, obj.geo_VBO);			CHECK_GL_ERR
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.idx_VBO);	CHECK_GL_ERR

						// todo: reduce VBO size by storing normals as GL_INT_2_10_10_10_REV, uv's as GL_UNSIGNED_SHORT
						// or go all out and normalize vertices to GL_SHORT, then glScale/glRotate based on xyz_min[]/max[] data
						glBufferData(GL_ARRAY_BUFFER, obj.geo_tri.count()*sizeof(GLfloat)*8, obj.geo_tri.get(0), GL_STATIC_DRAW);	CHECK_GL_ERR
						// todo: use GL_UNSIGNED_SHORT indices for objects under 64k indices (as most ? frequently used objects are small)
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.indices.size()*sizeof(GLuint), &obj.indices[0], GL_STATIC_DRAW);	CHECK_GL_ERR

						// obj.geo_tri.delete();       // todo: free that now unused memory
					}
					offset = nullptr;
#endif
					glVertexPointer			(3, GL_FLOAT, 8*sizeof(GLfloat), offset	     );	CHECK_GL_ERR
					glNormalPointer			(	GL_FLOAT, 8*sizeof(GLfloat), offset + 3	 );	CHECK_GL_ERR
					funcs->TexCoordPointer_f(2, GL_FLOAT, 8*sizeof(GLfloat), offset + 6, ref);	CHECK_GL_ERR
				}
				glEnableClientState(GL_VERTEX_ARRAY);				CHECK_GL_ERR
				glEnableClientState(GL_NORMAL_ARRAY);				CHECK_GL_ERR
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);		CHECK_GL_ERR
				glDisableClientState(GL_COLOR_ARRAY);				CHECK_GL_ERR
				glColor3fv(mat_col);								CHECK_GL_ERR

#if XOBJ8_USE_VBO
				if(obj.geo_VBO)
					glDrawElements(GL_TRIANGLES, cmd->idx_count, GL_UNSIGNED_INT, (void *) (cmd->idx_offset * sizeof(GLuint)));
				else
#endif
					glDrawElements(GL_TRIANGLES, cmd->idx_count, GL_UNSIGNED_INT, &obj.indices[cmd->idx_offset]);
				CHECK_GL_ERR
				break;
			case obj8_Lines:
				if (drawMode_Lin != drawMode) {
					funcs->SetupLine_f(ref);	CHECK_GL_ERR
					drawMode = drawMode_Lin;
				}
				if (arrayMode_Lin != arrayMode)
				{
					arrayMode = arrayMode_Lin;
					glVertexPointer(3, GL_FLOAT, 24, obj.geo_lines.get(0));								CHECK_GL_ERR
					glColorPointer(3, GL_FLOAT, 24, ((const char *) obj.geo_lines.get(0)) + 12);		CHECK_GL_ERR
				}
				glEnableClientState(GL_VERTEX_ARRAY);			CHECK_GL_ERR
				glDisableClientState(GL_NORMAL_ARRAY);			CHECK_GL_ERR
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);	CHECK_GL_ERR
				glEnableClientState(GL_COLOR_ARRAY);			CHECK_GL_ERR
				glDrawElements(GL_LINES, cmd->idx_count, GL_UNSIGNED_INT, &obj.indices[cmd->idx_offset]);	CHECK_GL_ERR
				break;
			case obj8_Lights:
				if (drawMode_Lgt != drawMode) {
					funcs->SetupLight_f(ref);	CHECK_GL_ERR
					drawMode = drawMode_Lgt;
				}
				if (arrayMode_Lgt != arrayMode)
				{
					arrayMode = arrayMode_Lgt;
					glVertexPointer(3, GL_FLOAT, 24, obj.geo_lights.get(0));							CHECK_GL_ERR
					glColorPointer(3, GL_FLOAT, 24, ((const char *) obj.geo_lights.get(0)) + 12);		CHECK_GL_ERR
				}
				glEnableClientState(GL_VERTEX_ARRAY);				CHECK_GL_ERR
				glDisableClientState(GL_NORMAL_ARRAY);				CHECK_GL_ERR
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);		CHECK_GL_ERR
				glEnableClientState(GL_COLOR_ARRAY);				CHECK_GL_ERR
				glDrawArrays(GL_POINTS, cmd->idx_offset, cmd->idx_count);		CHECK_GL_ERR
				break;

			case attr_Tex_Normal:	tex_is_cockpit = false;	drawMode = drawMode_Non;  break;
			case attr_Tex_Cockpit:	tex_is_cockpit = true;	break;
			case attr_No_Blend:	glDisable(GL_BLEND); CHECK_GL_ERR break;
			case attr_Blend:	glEnable(GL_BLEND);	 CHECK_GL_ERR break;


			case anim_Begin:	glPushMatrix();	CHECK_GL_ERR break;
			case anim_End:		glPopMatrix(); CHECK_GL_ERR break;
			case anim_Rotate:
				anim = &obj.animation[cmd->idx_offset];
				v = funcs->GetAnimParam(anim->dataref.c_str(), anim->keyframes.front().key, anim->keyframes.back().key, ref);
				v = key_extrap(v, anim->keyframes, 0);
				glRotatef(v, anim->axis[0], anim->axis[1], anim->axis[2]);
				break;
			case anim_Translate:
				anim = &obj.animation[cmd->idx_offset];
				v = funcs->GetAnimParam(anim->dataref.c_str(), anim->keyframes.front().key, anim->keyframes.back().key, ref);
				glTranslatef(
					key_extrap(v, anim->keyframes, 0),
					key_extrap(v, anim->keyframes, 1),
					key_extrap(v, anim->keyframes, 2));
				break;

			case attr_Shade_Flat:	glShadeModel(GL_FLAT); 	 CHECK_GL_ERR break;
			case attr_Shade_Smooth: glShadeModel(GL_SMOOTH); CHECK_GL_ERR break;
//			case attr_Ambient_RGB: 	glMaterialfv(GL_FRONT,GL_AMBIENT, cmd->params); 	CHECK_GL_ERR break;
			case attr_Diffuse_RGB:  mat_col[0] = cmd->params[0]; mat_col[1] = cmd->params[1]; mat_col[2] = cmd->params[2];		glColor3fv(mat_col); 	CHECK_GL_ERR break;
			case attr_Emission_RGB: glMaterialfv(GL_FRONT,GL_EMISSION, cmd->params); 	CHECK_GL_ERR break;
//			case attr_Specular_RGB: glMaterialfv(GL_FRONT,GL_SPECULAR, cmd->params); 	CHECK_GL_ERR break;
			case attr_Shiny_Rat:	{ GLfloat spec[4] = { cmd->params[0], cmd->params[0], cmd->params[0], 1.0 };

											glLightfv	(GL_LIGHT0,GL_SPECULAR ,spec);
											glMaterialfv (GL_FRONT, GL_SPECULAR, spec);
											glMateriali (GL_FRONT,GL_SHININESS,128); } break;
			case attr_No_Depth:		glDisable(GL_DEPTH_TEST);	CHECK_GL_ERR	break;
			case attr_Depth:		glEnable(GL_DEPTH_TEST);	CHECK_GL_ERR	break;
			case attr_Reset: { float amb[4] = { 0.2, 0.2, 0.2, 1.0 }, diff[4] = { 0.8, 0.8, 0.8, 1.0 }, zero[4] = { 0.0, 0.0, 0.0, 1.0 };
								glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
								glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
								glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
								glMaterialfv(GL_FRONT, GL_EMISSION, zero);
								glColor3f(1.0, 1.0, 1.0);
								mat_col[0] = mat_col[1] = mat_col[2] = 1.0;
								glMateriali (GL_FRONT,GL_SHININESS,0); } CHECK_GL_ERR break;
			case attr_Cull:		glEnable(GL_CULL_FACE);		CHECK_GL_ERR break;
			case attr_NoCull:	glDisable(GL_CULL_FACE);	CHECK_GL_ERR break;
			case attr_Offset:	if (cmd->params[0] != 0)
				{	glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(-5.0*cmd->params[0],-1.0);glDepthMask(GL_FALSE);	} else
				{	glDisable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(0.0, 0.0);glDepthMask(GL_TRUE);	}
				CHECK_GL_ERR
				break;
			case attr_Draped:	funcs->SetupDraped_f(ref);	break;
			case attr_NoDraped:	funcs->SetupNoDraped_f(ref);	break;
			} // Case

		} // cmd loop
		
		funcs->SetupNoDraped_f(ref);
		{ float amb[4] = { 0.2, 0.2, 0.2, 1.0 }, diff[4] = { 0.8, 0.8, 0.8, 1.0 }, zero[4] = { 0.0, 0.0, 0.0, 1.0 };
			glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
			glMaterialfv(GL_FRONT, GL_EMISSION, zero);
			glColor3f(1.0, 1.0, 1.0);
			mat_col[0] = mat_col[1] = mat_col[2] = 1.0;
			glMateriali (GL_FRONT,GL_SHININESS,0); 
		}
		
	} // our LOD
}


