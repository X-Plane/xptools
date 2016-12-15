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

/* matrix.h - matrix routines for GeForce/Quadro shadow mapping demo */

/* Copyright NVIDIA Corporation, 2000. */

#ifndef __MATRIX_H__
#define __MATRIX_H__

//#include <glut.h>
#if APL
#include <OpenGL/gl.h>
#elif LIN
#include <GL/gl.h>
#else
#include <gl/gl.h>
#endif

/* Some <math.h> files do not define M_PI... */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD	(M_PI / 180.0)



extern void	setIdentityMatrix(GLdouble m[16]);
extern GLboolean invertMatrix(GLdouble *out, const GLdouble *m);
extern void transposeMatrix(GLdouble dst[16], GLdouble src[16]);
extern void copyMatrix(GLdouble dst[16], GLdouble src[16]);
extern void addMatrices(GLdouble dst[16], GLdouble a[16], GLdouble b[16]);
extern void multMatrices(GLdouble dst[16], const GLdouble a[16], const GLdouble b[16]);
extern void multMatrixVec(GLdouble dst[4], const GLdouble m[16], const GLdouble v[4]);
extern void applyMatrixVec(GLdouble vec[4], GLdouble m[16]);

extern void buildFrustumMatrix(GLdouble m[16],
                               GLdouble l, GLdouble r, GLdouble b, GLdouble t,
                               GLdouble n, GLdouble f);
extern void buildPerspectiveMatrix(GLdouble m[16],
                                   GLdouble fovy, GLdouble aspect,
                                   GLdouble zNear, GLdouble zFar);
extern void buildOrthoMatrix( GLdouble m[16],
							    GLdouble left, GLdouble right,
							    GLdouble bottom, GLdouble top,
							    GLdouble nearval, GLdouble farval );

extern void buildLookAtMatrix(GLdouble m[16],
                              GLdouble eyex, GLdouble eyey, GLdouble eyez,
                              GLdouble centerx, GLdouble centery, GLdouble centerz,
                              GLdouble upx, GLdouble upy, GLdouble upz);


extern	void buildRotation( GLdouble m[16],
		     GLdouble angle, GLdouble x, GLdouble y, GLdouble z );
extern	void applyTranslation( GLdouble m[16], GLdouble x, GLdouble y, GLdouble z );
extern	void applyRotation( GLdouble m[16], GLdouble angle, GLdouble x, GLdouble y, GLdouble z );

extern void printMatrix(char *msg, GLdouble m[16]);


void	 vec3_assign(GLdouble vec[3], GLdouble x, GLdouble y, GLdouble z);
GLdouble vec3_dot(GLdouble op1[3], GLdouble op2[3]);
void	 vec3_normalize(GLdouble vec[3]);
double	 vec3_length(GLdouble vec[3]);
void	 vec3_cross(GLdouble out[3], GLdouble op1[3], GLdouble op2[3]);

#endif  /* __MATRIX_H__ */
