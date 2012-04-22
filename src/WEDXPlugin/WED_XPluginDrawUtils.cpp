/*
 * Copyright (c) 2012, mroe.
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
#include "WED_XPluginDrawUtils.h"

#if APL
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#if !IBM
#define CALLBACK
#endif

static void CALLBACK TessBegin(GLenum mode)		   {glBegin(mode);}
static void CALLBACK TessEnd(void)				   {glEnd();}
static void CALLBACK TessVertex(const Point3 * p)  {glVertex3d(p->x,p->y,p->z);}
static void CALLBACK TessVertexUV(const Point3 * p){const Point3 * uv = p; ++uv; glTexCoord2f(uv->x, uv->y); glVertex2d(p->x,p->y);}

void glPolygon3(const Point3 * pts, bool has_uv, const int * contours, int n)
{
	GLUtesselator * tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN,	(void (CALLBACK *)(void))TessBegin);
	gluTessCallback(tess, GLU_TESS_END,		(void (CALLBACK *)(void))TessEnd);
	if(has_uv)
	gluTessCallback(tess, GLU_TESS_VERTEX,	(void (CALLBACK *)(void))TessVertexUV);
	else
	gluTessCallback(tess, GLU_TESS_VERTEX,	(void (CALLBACK *)(void))TessVertex);

	gluBeginPolygon(tess);

	while(n--)
	{
		if (contours && *contours++)	gluNextContour(tess, GLU_INTERIOR);

		double	xyz[3] = {pts->x, pts->y,pts->z};
		gluTessVertex(tess, xyz, (void*) pts++);
		if(has_uv)
			++pts;
	}

	gluEndPolygon (tess);
	gluDeleteTess(tess);
}

