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
#include <algorithm>
#include "MathUtils.h"

#ifndef CHECK_GL_ERR
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

struct compare_key {
	bool operator()(const XObjKey& lhs, float rhs) const {
		return lhs.key < rhs;
	}
};

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

// very simple conversion to 16 bit floating point, its not handling NAN, infinity, nor will it create subnormals

static uint16_t half(float f)
{
	if (-6E-5f < f && f < 6E-5f )
		return 0x0000;
	else if (f > 65504.0f)
		f = 65504.0f;
	else if (f < -65504.0f)
		f = -65504.0f;

	int x = *(int *) &f;
	return    ((x >> 16) & 0x8000)                               // mantissa sign
		  | ((((x & 0x7f800000) - 0x38000000) >> 13 ) & 0x7C00)  // exponent
		  |   ((x >> 13) & 0x03ff);                              // mantissa
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

#if XOBJ8_USE_VBO

	#define SHORT_IDX 1
	#define HALF_SIZE_VBO 1

	#if HALF_SIZE_VBO
		#define VBO_10b_NRML 0

		#define VBO_VEC_FMT  GL_SHORT
		#if VBO_10b_NRML
//			glEnable(GL_RESCALE_NORMAL);
			glEnable(GL_NORMALIZE);
		#endif
		#define VBO_ST_FMT   GL_HALF_FLOAT
		#define VBO_VEC_T    GLhalf

		glPushMatrix();
		#define MAX_SCALE 32766.9
		double scale = MAX_SCALE / fltmax6(-obj.xyz_min[0], obj.xyz_max[0], -obj.xyz_min[1], obj.xyz_max[1], -obj.xyz_min[2], obj.xyz_max[2]);
		glScalef(1.0 / scale, 1.0 / scale, 1.0 / scale);
	#else
		#define VBO_VEC_FMT  GL_FLOAT
		#define VBO_ST_FMT   GL_FLOAT
		#define VBO_VEC_T    GLfloat
	#endif
#endif
    #define VBO_OFFS1    (3*sizeof(VBO_VEC_T))
    #define VBO_OFFS2    (VBO_OFFS1 + 3*sizeof(VBO_VEC_T))
    #define VBO_STRIDE   (VBO_OFFS2 + 2*sizeof(VBO_VEC_T))

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
#if XOBJ8_USE_VBO
					if(obj.geo_VBO)
					{
						glBindBuffer(GL_ARRAY_BUFFER, obj.geo_VBO);          CHECK_GL_ERR
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.idx_VBO);  CHECK_GL_ERR
					}
					else
					{
						GLuint x[2];
						glGenBuffers(2, x);                                  CHECK_GL_ERR
						const_cast<unsigned int&>(obj.geo_VBO) = x[0];
						const_cast<unsigned int&>(obj.idx_VBO) = x[1];

						glBindBuffer(GL_ARRAY_BUFFER, obj.geo_VBO);          CHECK_GL_ERR
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.idx_VBO);  CHECK_GL_ERR
			#if SHORT_IDX
						*(const_cast<bool*>(&obj.short_idx)) = obj.geo_tri.count() < 65536;
			#endif

			#if HALF_SIZE_VBO
						{
							union ogl_vert { GLfloat f[4]; GLint i[4]; GLshort s[8]; GLushort u[8]; GLbyte b[16]; };
							vector<ogl_vert> tmp;
							int n = obj.geo_tri.count();
							tmp.reserve(n);
							const float * f = obj.geo_tri.get(0);
							for(int i = 0; i < n; ++i)
							{
								tmp.push_back(ogl_vert());
//                              Using integers & scale with model projection has 5 bits more resolution than halfs
//								tmp.back().u[0] = half(*f++);
//								tmp.back().u[1] = half(*f++);
//								tmp.back().u[2] = half(*f++);
								tmp.back().s[0] = *f++ * scale;
								tmp.back().s[1] = *f++ * scale;
								tmp.back().s[2] = *f++ * scale;
					#if VBO_10b_NRML
//								Free two bytes (and two bits) for future use ...
								tmp.back().i[2] = (max(-512, min((int) (*f++ * 511), 511)) & 0x3FF)
												| ((max(-512, min((int) (*f++ * 511), 511)) & 0x3FF) << 10)
												| ((max(-512, min((int) (*f++ * 511), 511)) & 0x3FF) << 20);
//								Or squeeze another byte - but its no good for colors (unsigned !) if this vert happens to be
//                              used for line or light drawing, as the normals are colors then. And by just looking at the
//                              vertex data one can't say - only by going though all LINE and LIGHT commands one could find out
//								tmp.back().b[8]  = max(-128, min(*f++ * 127, 127));
//								tmp.back().b[9]  = max(-128, min(*f++ * 127, 127));
//								tmp.back().b[10] = max(-128, min(*f++ * 127, 127));
					#else
								tmp.back().u[3] = half(*f++);
								tmp.back().u[4] = half(*f++);
								tmp.back().u[5] = half(*f++);
					#endif
								tmp.back().u[6] = half(*f++);
								tmp.back().u[7] = half(*f++);
//                              This requires using VertexAttributePointers & shaders, as OGL < 2 has no way to enable normalized ints
//								tmp.back().s[6] = max(INT_MIN_LL, min((long long) (*f++ * INT_MAX_LL), INT_MAX_LL));
//								tmp.back().s[7] = max(INT_MIN_LL, min((long long) (*f++ * INT_MAX_LL), INT_MAX_LL));
							}
							glBufferData(GL_ARRAY_BUFFER, obj.geo_tri.count()*VBO_STRIDE, tmp.data(), GL_STATIC_DRAW); CHECK_GL_ERR
						}
			#else
						glBufferData(GL_ARRAY_BUFFER, obj.geo_tri.count()*VBO_STRIDE, obj.geo_tri.get(0), GL_STATIC_DRAW); CHECK_GL_ERR
			#endif

			#if SHORT_IDX
						if(obj.short_idx)
						{
							vector<GLushort> tmp;
							tmp.reserve(obj.indices.size());
							for(auto i : obj.indices)
								tmp.push_back(i);

							glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.indices.size()*sizeof(GLushort), tmp.data(), GL_STATIC_DRAW); CHECK_GL_ERR
						}
						else
			#endif
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.indices.size()*sizeof(GLuint), &obj.indices[0], GL_STATIC_DRAW); CHECK_GL_ERR

						const_cast<XObj8&>(obj).geo_tri.clear(8);           // now that its all in VRAM - free the RAM !
						const_cast<vector<int>&>(obj.indices).clear();
						const_cast<vector<int>&>(obj.indices).shrink_to_fit();
					}
					const char * vert_ptr = nullptr;                        // offset into VBO
#else  // XOBJ8_USE_VBO
					const char * vert_ptr = obj.geo_tri.get(0);             // position in RAM
#endif
					glVertexPointer			(3, VBO_VEC_FMT,           VBO_STRIDE, vert_ptr                 );	CHECK_GL_ERR
			#if VBO_10b_NRML
					glNormalPointer			(	GL_INT_2_10_10_10_REV, VBO_STRIDE, vert_ptr + VBO_OFFS1 + 2 );	CHECK_GL_ERR
			#else		                                               // yes - we're leaving 2 bytes unused and go for aligned loads instead
					glNormalPointer			(	GL_HALF_FLOAT,         VBO_STRIDE, vert_ptr + VBO_OFFS1     );	CHECK_GL_ERR
			#endif
					funcs->TexCoordPointer_f(2, VBO_ST_FMT,			   VBO_STRIDE, vert_ptr + VBO_OFFS2, ref);	CHECK_GL_ERR

//                  thats how it can be done with normalized ints for the st coordinates, but needs writing & loading shaders
//					glVertexAttribPointer(0, 2, GL_SHORT,              GL_FALSE, VBO_STRIDE, vert_ptr             );
//					glVertexAttribPointer(1, 1, GL_INT_2_10_10_10_REV, GL_TRUE,  VBO_STRIDE, vert_ptr + VBO_OFFS1 );
//					glVertexAttribPointer(2, 2, GL_SHORT,              GL_TRUE,  VBO_STRIDE, vert_ptr + VBO_OFFS2 );
				}
				glEnableClientState(GL_VERTEX_ARRAY);				CHECK_GL_ERR
				glEnableClientState(GL_NORMAL_ARRAY);				CHECK_GL_ERR
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);		CHECK_GL_ERR
				glDisableClientState(GL_COLOR_ARRAY);				CHECK_GL_ERR
				glColor3fv(mat_col);								CHECK_GL_ERR
#if XOBJ8_USE_VBO
				glDrawElements(GL_TRIANGLES, cmd->idx_count, obj.short_idx ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
								(void *) ((obj.short_idx ? sizeof(GLushort) : sizeof(GLuint)) * cmd->idx_offset));   CHECK_GL_ERR
#else
				glDrawElements(GL_TRIANGLES, cmd->idx_count, GL_UNSIGNED_INT, &obj.indices[cmd->idx_offset]); CHECK_GL_ERR
#endif
				break;
			case obj8_Lines:
#if HALF_SIZE_VBO
				glPopMatrix();
#endif
				if (drawMode_Lin != drawMode) {
					funcs->SetupLine_f(ref);	CHECK_GL_ERR
					drawMode = drawMode_Lin;
				}
				if (arrayMode_Lin != arrayMode)
				{
					arrayMode = arrayMode_Lin;
#if XOBJ8_USE_VBO
					glBindBuffer(GL_ARRAY_BUFFER, 0);				CHECK_GL_ERR
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.idx_VBO); CHECK_GL_ERR
#endif
					glVertexPointer(3, GL_FLOAT, 6*sizeof(float), obj.geo_lines.get(0));								CHECK_GL_ERR
					glColorPointer(3, GL_FLOAT, 6*sizeof(float), ((const char *) obj.geo_lines.get(0)) + 3*sizeof(float)); CHECK_GL_ERR
				}
				glEnableClientState(GL_VERTEX_ARRAY);			CHECK_GL_ERR
				glDisableClientState(GL_NORMAL_ARRAY);			CHECK_GL_ERR
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);	CHECK_GL_ERR
				glEnableClientState(GL_COLOR_ARRAY);			CHECK_GL_ERR
#if XOBJ8_USE_VBO
				glDrawElements(GL_LINES, cmd->idx_count, obj.short_idx ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
								(void*)((obj.short_idx ? sizeof(GLushort) : sizeof(GLuint))* cmd->idx_offset));   CHECK_GL_ERR
#else
				glDrawElements(GL_LINES, cmd->idx_count, GL_UNSIGNED_INT, &obj.indices[cmd->idx_offset]);	CHECK_GL_ERR
#endif
#if HALF_SIZE_VBO
				glPushMatrix();
				glScalef(1.0 / scale, 1.0 / scale, 1.0 / scale);
#endif
				break;
			case obj8_Lights:
				if (drawMode_Lgt != drawMode) {
					funcs->SetupLight_f(ref);	CHECK_GL_ERR
					drawMode = drawMode_Lgt;
				}
				if (arrayMode_Lgt != arrayMode)
				{
					arrayMode = arrayMode_Lgt;
#if XOBJ8_USE_VBO
					glBindBuffer(GL_ARRAY_BUFFER, 0);				CHECK_GL_ERR
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.idx_VBO); CHECK_GL_ERR
#endif
					glVertexPointer(3, GL_FLOAT, 6*sizeof(float), obj.geo_lights.get(0));							CHECK_GL_ERR
					glColorPointer(3, GL_FLOAT, 6*sizeof(float), ((const char *) obj.geo_lights.get(0)) + 3*sizeof(float));	CHECK_GL_ERR
				}
				glEnableClientState(GL_VERTEX_ARRAY);				CHECK_GL_ERR
				glDisableClientState(GL_NORMAL_ARRAY);				CHECK_GL_ERR
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);		CHECK_GL_ERR
				glEnableClientState(GL_COLOR_ARRAY);				CHECK_GL_ERR
#if XOBJ8_USE_VBO
				glDrawElements(GL_POINTS, cmd->idx_count, obj.short_idx ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
						(void*)((obj.short_idx ? sizeof(GLushort) : sizeof(GLuint))* cmd->idx_offset));   CHECK_GL_ERR
#else
				glDrawElements(GL_POINTS, cmd->idx_count, GL_UNSIGNED_INT, &obj.indices[cmd->idx_offset]);	CHECK_GL_ERR
#endif
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
#if XOBJ8_USE_VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);				CHECK_GL_ERR
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);		CHECK_GL_ERR
	#if HALF_SIZE_VBO
	glPopMatrix();
	#endif
#endif
}
