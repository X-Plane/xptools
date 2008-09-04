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


/* matrix.c - matrix routines for GeForce/Quadro shadow mapping demo */



/* Copyright NVIDIA Corporation, 2000. */




#include "hl_types.h"

#include "MatrixUtils.h"

#include <math.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

//#include <glut.h>











static GLdouble gIdentity[16] = {

   1.0, 0.0, 0.0, 0.0,

   0.0, 1.0, 0.0, 0.0,

   0.0, 0.0, 1.0, 0.0,

   0.0, 0.0, 0.0, 1.0

};



#include "MatrixUtils.h"



/*** MATRIX MANIPULATION ***/



void	setIdentityMatrix(GLdouble m[16])

{

	memcpy(m, gIdentity, sizeof(gIdentity));

}





/*

 * Compute inverse of 4x4 transformation matrix.

 * Code contributed by Jacques Leroy <jle@star.be>

 * Code lifted from Brian Paul's Mesa freeware OpenGL implementation.

 * Return GL_TRUE for success, GL_FALSE for failure (singular matrix)

 */

GLboolean

invertMatrix(GLdouble *out, const GLdouble *m)

{

/* NB. OpenGL Matrices are COLUMN major. */

#define SWAP_ROWS(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }

#define MAT(m,r,c) (m)[(c)*4+(r)]



 GLdouble wtmp[4][8];

 GLdouble m0, m1, m2, m3, s;

 GLdouble *r0, *r1, *r2, *r3;



 r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];



 r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),

 r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),

 r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,



 r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),

 r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),

 r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,



 r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),

 r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),

 r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,



 r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),

 r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),

 r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;



 /* choose pivot - or die */

 if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);

 if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);

 if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);

 if (0.0 == r0[0]) {

  return GL_FALSE;

 }



 /* eliminate first variable     */

 m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];

 s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;

 s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;

 s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;

 s = r0[4];

 if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }

 s = r0[5];

 if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }

 s = r0[6];

 if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }

 s = r0[7];

 if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }



 /* choose pivot - or die */

 if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);

 if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);

 if (0.0 == r1[1]) {

  return GL_FALSE;

 }



 /* eliminate second variable */

 m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];

 r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];

 r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];

 s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }

 s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }

 s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }

 s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }



 /* choose pivot - or die */

 if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);

 if (0.0 == r2[2]) {

  return GL_FALSE;

 }



 /* eliminate third variable */

 m3 = r3[2]/r2[2];

 r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],

 r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],

 r3[7] -= m3 * r2[7];



 /* last check */

 if (0.0 == r3[3]) {

  return GL_FALSE;

 }



 s = 1.0/r3[3];              /* now back substitute row 3 */

 r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;



 m2 = r2[3];                 /* now back substitute row 2 */

 s  = 1.0/r2[2];

 r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),

 r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);

 m1 = r1[3];

 r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,

 r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;

 m0 = r0[3];

 r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,

 r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;



 m1 = r1[2];                 /* now back substitute row 1 */

 s  = 1.0/r1[1];

 r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),

 r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);

 m0 = r0[2];

 r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,

 r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;



 m0 = r0[1];                 /* now back substitute row 0 */

 s  = 1.0/r0[0];

 r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),

 r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);



 MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],

 MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],

 MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],

 MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],

 MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],

 MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],

 MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],

 MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7]; 



 return GL_TRUE;



#undef MAT

#undef SWAP_ROWS

}



/* dst = transpose(src) */

void

transposeMatrix(GLdouble dst[16], GLdouble src[16])

{

  dst[0] = src[0];

  dst[1] = src[4];

  dst[2] = src[8];

  dst[3] = src[12];



  dst[4] = src[1];

  dst[5] = src[5];

  dst[6] = src[9];

  dst[7] = src[13];



  dst[8] = src[2];

  dst[9] = src[6];

  dst[10] = src[10];

  dst[11] = src[14];



  dst[12] = src[3];

  dst[13] = src[7];

  dst[14] = src[11];

  dst[15] = src[15];

}



/* dst = a + b */

void

addMatrices(GLdouble dst[16], GLdouble a[16], GLdouble b[16])

{

  dst[0] = a[0] + b[0];

  dst[1] = a[1] + b[1];

  dst[2] = a[2] + b[2];

  dst[3] = a[3] + b[3];



  dst[4] = a[4] + b[4];

  dst[5] = a[5] + b[5];

  dst[6] = a[6] + b[6];

  dst[7] = a[7] + b[7];



  dst[8] = a[8] + b[8];

  dst[9] = a[9] + b[9];

  dst[10] = a[10] + b[10];

  dst[11] = a[11] + b[11];



  dst[12] = a[12] + b[12];

  dst[13] = a[13] + b[13];

  dst[14] = a[14] + b[14];

  dst[15] = a[15] + b[15];

}



/* dst = a * b */

void

multMatrices(GLdouble dst[16], const GLdouble a[16], const GLdouble b[16])

{

	dst[0 ] = b[0 ]*a[0] + b[1 ]*a[4] + b[2 ]*a[8 ] + b[3 ]*a[12];

	dst[1 ] = b[0 ]*a[1] + b[1 ]*a[5] + b[2 ]*a[9 ] + b[3 ]*a[13];

	dst[2 ] = b[0 ]*a[2] + b[1 ]*a[6] + b[2 ]*a[10] + b[3 ]*a[14];

	dst[3 ] = b[0 ]*a[3] + b[1 ]*a[7] + b[2 ]*a[11] + b[3 ]*a[15];

	dst[4 ] = b[4 ]*a[0] + b[5 ]*a[4] + b[6 ]*a[8 ] + b[7 ]*a[12];

	dst[5 ] = b[4 ]*a[1] + b[5 ]*a[5] + b[6 ]*a[9 ] + b[7 ]*a[13];

	dst[6 ] = b[4 ]*a[2] + b[5 ]*a[6] + b[6 ]*a[10] + b[7 ]*a[14];

	dst[7 ] = b[4 ]*a[3] + b[5 ]*a[7] + b[6 ]*a[11] + b[7 ]*a[15];

	dst[8 ] = b[8 ]*a[0] + b[9 ]*a[4] + b[10]*a[8 ] + b[11]*a[12];

	dst[9 ] = b[8 ]*a[1] + b[9 ]*a[5] + b[10]*a[9 ] + b[11]*a[13];

	dst[10] = b[8 ]*a[2] + b[9 ]*a[6] + b[10]*a[10] + b[11]*a[14];

	dst[11] = b[8 ]*a[3] + b[9 ]*a[7] + b[10]*a[11] + b[11]*a[15];

	dst[12] = b[12]*a[0] + b[13]*a[4] + b[14]*a[8 ] + b[15]*a[12];

	dst[13] = b[12]*a[1] + b[13]*a[5] + b[14]*a[9 ] + b[15]*a[13];

	dst[14] = b[12]*a[2] + b[13]*a[6] + b[14]*a[10] + b[15]*a[14];

	dst[15] = b[12]*a[3] + b[13]*a[7] + b[14]*a[11] + b[15]*a[15];





#if 0

  int i, j;



  for (i = 0; i < 4; i++) {

    for (j = 0; j < 4; j++) {

      dst[i * 4 + j] =

        b[i * 4 + 0] * a[0 * 4 + j] +

        b[i * 4 + 1] * a[1 * 4 + j] +

        b[i * 4 + 2] * a[2 * 4 + j] +

        b[i * 4 + 3] * a[3 * 4 + j];

    }

  }

#endif  

}



void 

multMatrixVec(GLdouble dst[4], const GLdouble m[16], const GLdouble v[4])

{

	dst[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];

	dst[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];

	dst[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];

	dst[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];

}



void

applyMatrixVec(GLdouble vec[4], GLdouble m[16])

{

	GLdouble	t[4];

	multMatrixVec(t, m, vec);

	vec[0] = t[0];

	vec[1] = t[1];

	vec[2] = t[2];

	vec[3] = t[3];

}





void

copyMatrix(GLdouble dst[16], GLdouble src[16])

{

	memcpy(dst, src, sizeof(GLdouble) * 16);



#if 0

  int i;



  for (i=0; i<16; i++) {

    dst[i] = src[i];

  }

#endif  

}



void

buildFrustumMatrix(GLdouble m[16],

                   GLdouble l, GLdouble r, GLdouble b, GLdouble t,

                   GLdouble n, GLdouble f)

{

  m[0] = (2.0*n) / (r-l);

  m[1] = 0.0;

  m[2] = 0.0;

  m[3] = 0.0;



  m[4] = 0.0;

  m[5] = (2.0*n) / (t-b);

  m[6] = 0.0;

  m[7] = 0.0;



  m[8] = (r+l) / (r-l);

  m[9] = (t+b) / (t-b);

  m[10] = -(f+n) / (f-n);

  m[11] = -1.0;



  m[12] = 0.0;

  m[13] = 0.0;

  m[14] = -(2.0*f*n) / (f-n);

  m[15] = 0.0;

}



void

buildPerspectiveMatrix(GLdouble m[16],

                       GLdouble fovy, GLdouble aspect,

                       GLdouble zNear, GLdouble zFar)

{

  GLdouble xmin, xmax, ymin, ymax;



  ymax = zNear * tan(fovy * M_PI / 360.0);

  ymin = -ymax;



  xmin = ymin * aspect;

  xmax = ymax * aspect;



  buildFrustumMatrix(m, xmin, xmax, ymin, ymax, zNear, zFar);

}





void

buildOrthoMatrix( GLdouble m[16],

		    GLdouble left, GLdouble right,

		    GLdouble bottom, GLdouble top,

		    GLdouble nearval, GLdouble farval )

{

   GLdouble x, y, z;

   GLdouble tx, ty, tz;



   x = 2.0 / (right-left);

   y = 2.0 / (top-bottom);

   z = -2.0 / (farval-nearval);

   tx = -(right+left) / (right-left);

   ty = -(top+bottom) / (top-bottom);

   tz = -(farval+nearval) / (farval-nearval);



#define M(row,col)  m[col*4+row]

   M(0,0) = x;     M(0,1) = 0.0;  M(0,2) = 0.0;  M(0,3) = tx;

   M(1,0) = 0.0;  M(1,1) = y;     M(1,2) = 0.0;  M(1,3) = ty;

   M(2,0) = 0.0;  M(2,1) = 0.0;  M(2,2) = z;     M(2,3) = tz;

   M(3,0) = 0.0;  M(3,1) = 0.0;  M(3,2) = 0.0;  M(3,3) = 1.0;

#undef M



}







/* Build a 4x4 matrix transform based on the parameters for gluLookAt.

 * Code lifted from Brian Paul's MesaGLU.

 */

void

buildLookAtMatrix(GLdouble m[16],

                  GLdouble eyex, GLdouble eyey, GLdouble eyez,

                  GLdouble centerx, GLdouble centery, GLdouble centerz,

                  GLdouble upx, GLdouble upy, GLdouble upz)

{

   GLdouble x[3], y[3], z[3];

   GLdouble mag;



   /* Make rotation matrix */



   /* Z vector */

   z[0] = eyex - centerx;

   z[1] = eyey - centery;

   z[2] = eyez - centerz;

   mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );

   if (mag) {  /* mpichler, 19950515 */

      z[0] /= mag;

      z[1] /= mag;

      z[2] /= mag;

   }



   /* Y vector */

   y[0] = upx;

   y[1] = upy;

   y[2] = upz;



   /* X vector = Y cross Z */

   x[0] =  y[1]*z[2] - y[2]*z[1];

   x[1] = -y[0]*z[2] + y[2]*z[0];

   x[2] =  y[0]*z[1] - y[1]*z[0];



   /* Recompute Y = Z cross X */

   y[0] =  z[1]*x[2] - z[2]*x[1];

   y[1] = -z[0]*x[2] + z[2]*x[0];

   y[2] =  z[0]*x[1] - z[1]*x[0];



   /* mpichler, 19950515 */

   /* cross product gives area of parallelogram, which is < 1.0 for

    * non-perpendicular unit-length vectors; so normalize x, y here

    */



   mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );

   if (mag) {

      x[0] /= mag;

      x[1] /= mag;

      x[2] /= mag;

   }



   mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );

   if (mag) {

      y[0] /= mag;

      y[1] /= mag;

      y[2] /= mag;

   }



#define M(row,col)  m[col*4+row]

   M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = -x[0]*eyex + -x[1]*eyey + -x[2]*eyez;

   M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = -y[0]*eyex + -y[1]*eyey + -y[2]*eyez;

   M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = -z[0]*eyex + -z[1]*eyey + -z[2]*eyez;

   M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;

#undef M

}



/* For debugging purposes. */

void

printMatrix(char *msg, GLdouble m[16])

{

  printf("%s\n", msg);

  printf(" [ %f %f %f %f ]\n", m[0], m[4], m[8], m[12]);

  printf(" [ %f %f %f %f ]\n", m[1], m[5], m[9], m[13]);

  printf(" [ %f %f %f %f ]\n", m[2], m[6], m[10], m[14]);

  printf(" [ %f %f %f %f ]\n\n", m[3], m[7], m[11], m[15]);

}



#pragma mark -





void

buildRotation( GLdouble	m[16],

		     GLdouble angle, GLdouble x, GLdouble y, GLdouble z )

{

   GLdouble xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;

   GLboolean optimized;



   s = sin( angle * DEG2RAD );

   c = cos( angle * DEG2RAD );



   memcpy(m, gIdentity, sizeof(gIdentity));

   optimized = GL_FALSE;



#define M(row,col)  m[col*4+row]



   if (x == 0.0) {

      if (y == 0.0) {

         if (z != 0.0) {

            optimized = GL_TRUE;

            /* rotate only around z-axis */

            M(0,0) = c;

            M(1,1) = c;

            if (z < 0.0) {

               M(0,1) = s;

               M(1,0) = -s;

            }

            else {

               M(0,1) = -s;

               M(1,0) = s;

            }

         }

      }

      else if (z == 0.0) {

         optimized = GL_TRUE;

         /* rotate only around y-axis */

         M(0,0) = c;

         M(2,2) = c;

         if (y < 0.0) {

            M(0,2) = -s;

            M(2,0) = s;

         }

         else {

            M(0,2) = s;

            M(2,0) = -s;

         }

      }

   }

   else if (y == 0.0) {

      if (z == 0.0) {

         optimized = GL_TRUE;

         /* rotate only around x-axis */

         M(1,1) = c;

         M(2,2) = c;

         if (x < 0.0) {

            M(1,2) = s;

            M(2,1) = -s;

         }

         else {

            M(1,2) = -s;

            M(2,1) = s;

         }

      }

   }



   if (!optimized) {

      const GLdouble mag = sqrt(x * x + y * y + z * z);



      if (mag <= 1.0e-4) {

         /* no rotation, leave mat as-is */

         return;

      }



      x /= mag;

      y /= mag;

      z /= mag;





      /*

       *     Arbitrary axis rotation matrix.

       *

       *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied

       *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation

       *  (which is about the X-axis), and the two composite transforms

       *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary

       *  from the arbitrary axis to the X-axis then back.  They are

       *  all elementary rotations.

       *

       *  Rz' is a rotation about the Z-axis, to bring the axis vector

       *  into the x-z plane.  Then Ry' is applied, rotating about the

       *  Y-axis to bring the axis vector parallel with the X-axis.  The

       *  rotation about the X-axis is then performed.  Ry and Rz are

       *  simply the respective inverse transforms to bring the arbitrary

       *  axis back to it's original orientation.  The first transforms

       *  Rz' and Ry' are considered inverses, since the data from the

       *  arbitrary axis gives you info on how to get to it, not how

       *  to get away from it, and an inverse must be applied.

       *

       *  The basic calculation used is to recognize that the arbitrary

       *  axis vector (x, y, z), since it is of unit length, actually

       *  represents the sines and cosines of the angles to rotate the

       *  X-axis to the same orientation, with theta being the angle about

       *  Z and phi the angle about Y (in the order described above)

       *  as follows:

       *

       *  cos ( theta ) = x / sqrt ( 1 - z^2 )

       *  sin ( theta ) = y / sqrt ( 1 - z^2 )

       *

       *  cos ( phi ) = sqrt ( 1 - z^2 )

       *  sin ( phi ) = z

       *

       *  Note that cos ( phi ) can further be inserted to the above

       *  formulas:

       *

       *  cos ( theta ) = x / cos ( phi )

       *  sin ( theta ) = y / sin ( phi )

       *

       *  ...etc.  Because of those relations and the standard trigonometric

       *  relations, it is pssible to reduce the transforms down to what

       *  is used below.  It may be that any primary axis chosen will give the

       *  same results (modulo a sign convention) using thie method.

       *

       *  Particularly nice is to notice that all divisions that might

       *  have caused trouble when parallel to certain planes or

       *  axis go away with care paid to reducing the expressions.

       *  After checking, it does perform correctly under all cases, since

       *  in all the cases of division where the denominator would have

       *  been zero, the numerator would have been zero as well, giving

       *  the expected result.

       */



      xx = x * x;

      yy = y * y;

      zz = z * z;

      xy = x * y;

      yz = y * z;

      zx = z * x;

      xs = x * s;

      ys = y * s;

      zs = z * s;

      one_c = 1.0 - c;



      /* We already hold the identity-matrix so we can skip some statements */

      M(0,0) = (one_c * xx) + c;

      M(0,1) = (one_c * xy) - zs;

      M(0,2) = (one_c * zx) + ys;

/*    M(0,3) = 0.0; */



      M(1,0) = (one_c * xy) + zs;

      M(1,1) = (one_c * yy) + c;

      M(1,2) = (one_c * yz) - xs;

/*    M(1,3) = 0.0; */



      M(2,0) = (one_c * zx) - ys;

      M(2,1) = (one_c * yz) + xs;

      M(2,2) = (one_c * zz) + c;

/*    M(2,3) = 0.0; */



/*

      M(3,0) = 0.0;

      M(3,1) = 0.0;

      M(3,2) = 0.0;

      M(3,3) = 1.0;

*/

   }

#undef M



}



extern	void applyTranslation( GLdouble m[16], GLdouble x, GLdouble y, GLdouble z )

{

   m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];

   m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];

   m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];

   m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];

}



void applyRotation( GLdouble m[16], GLdouble angle, GLdouble x, GLdouble y, GLdouble z )

{

	GLdouble	temp[16], rot[16];

	buildRotation(rot, angle, x, y, z);

	multMatrices(temp, m, rot);

	copyMatrix(m, temp);

}



#pragma mark -



void	vec3_assign(GLdouble vec[3], GLdouble x, GLdouble y, GLdouble z)

{

	vec[0] = x;

	vec[1] = y;

	vec[2] = z;

}	



GLdouble vec3_dot(GLdouble op1[3], GLdouble op2[3])

{

	return op1[0] * op2[0] + 

		   op1[1] * op2[1] + 

		   op1[2] * op2[2];

}



void	vec3_normalize(GLdouble vec[3])

{

	GLdouble	len=sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);

	if (len>0.0)

	{

		len = 1.0 / len;

		vec[0] *= len;

		vec[1] *= len;

		vec[2] *= len;

	}

}



double	vec3_length(GLdouble	vec[3])

{

	return sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);

}



void	vec3_cross(GLdouble dst[3], GLdouble a[3], GLdouble b[3])

{

  dst[0] = a[1] * b[2] - a[2] * b[1] ;

  dst[1] = a[2] * b[0] - a[0] * b[2] ;

  dst[2] = a[0] * b[1] - a[1] * b[0] ;

}



