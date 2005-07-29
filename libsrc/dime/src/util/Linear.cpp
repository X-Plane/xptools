/**************************************************************************\
 * 
 *  FILE: Linear.cpp
 *
 *  This source file is part of DIME.
 *  Copyright (C) 1998-1999 by Systems In Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License (the accompanying file named COPYING) for more
 *  details.
 *
 **************************************************************************
 *
 *  If you need DIME for a non-GPL project, contact Systems In Motion
 *  to acquire a Professional Edition License:
 *
 *  Systems In Motion                                   http://www.sim.no/
 *  Prof. Brochs gate 6                                       sales@sim.no
 *  N-7030 Trondheim                                   Voice: +47 22114160
 *  NORWAY                                               Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class dimeVec2f dime/util/Linear.h
  \brief The dimeVec2f class is for containing and operating on a 2D vector /
  coordinate.
*/

/*!
  \class dimeVec3f dime/util/Linear.h
  \brief The dimeVec3f class is for containing and operating on a 3D vector /
  coordinate.
*/  

/*!
  \class dimeMatrix dime/util/Linear.h
  \brief The dimeMatrix class is for containing and operating on a four-by-four
  matrix.
*/

#include <dime/util/Linear.h>
#include <stdio.h>

#if 0 // OBSOLETED, was needed for old inverse() method

#if defined(__sgi) || defined (__sparc)
#include <ieeefp.h>
#endif

#ifdef _WIN32
#include "float.h"
#define fpclass _fpclass
#define FP_NZERO _FPCLASS_NZ
#define FP_PZERO _FPCLASS_PZ
#define M_PI 3.14159265357989
#elif defined(__hpux)
#define fpclass fpclassify
#define FP_NZERO FP_MINUS_ZERO
#define FP_PZERO FP_PLUS_ZERO
#endif

#ifdef macintosh
#include "float.h"
#define M_PI 3.14159265357989
#endif
#endif // OBSOLETED

void 
dimeVec3f::normalize(void) 
{
  dxfdouble dot = x*x + y*y + z*z;
  if (dot > 0.0) {
    dot = 1.0 / sqrt(dot);
    x *= dot; y *= dot; z *= dot;
  }
}

dxfdouble 
dimeVec3f::angle(const dimeVec3f &v2)
{
  dxfdouble cos,len;
  if ((len=length()*v2.length())==0.0f) return 0.0;
  cos = this->dot(v2) / len;
  if (cos > 1.0) cos = 1.0;
  if (cos < -1.0) cos = -1.0;
  return acos(cos);
}

dimeMatrix::dimeMatrix(const dimeMatrix &m)
{
  dxfdouble *p1 = &this->matrix[0][0];
  const dxfdouble *p2 = &m.matrix[0][0];
  int n = 16;
  while (n--) *p1++ = *p2++;
}

dimeMatrix::dimeMatrix(dxfdouble a11, dxfdouble a12, dxfdouble a13, dxfdouble a14,
		     dxfdouble a21, dxfdouble a22, dxfdouble a23, dxfdouble a24, 
		     dxfdouble a31, dxfdouble a32, dxfdouble a33, dxfdouble a34, 
		     dxfdouble a41, dxfdouble a42, dxfdouble a43, dxfdouble a44)
{
  this->matrix[0][0] = a11;
  this->matrix[0][1] = a12;
  this->matrix[0][2] = a13;
  this->matrix[0][3] = a14;
  this->matrix[1][0] = a21;
  this->matrix[1][1] = a22;
  this->matrix[1][2] = a23;
  this->matrix[1][3] = a24;
  this->matrix[2][0] = a31;
  this->matrix[2][1] = a32;
  this->matrix[2][2] = a33;
  this->matrix[2][3] = a34;
  this->matrix[3][0] = a41;
  this->matrix[3][1] = a42;
  this->matrix[3][2] = a43;
  this->matrix[3][3] = a44;
}

void
dimeMatrix::transpose()
{
  dxfdouble tmp;
  for (int i = 0; i < 3; i++) {
    for (int j = i+1; j < 4; j++) {
      tmp = this->matrix[i][j];
      this->matrix[i][j] = this->matrix[j][i];
      this->matrix[j][i] = tmp;
    }
  }
}

void 
dimeMatrix::makeIdentity()
{
  matrix[0][0]=matrix[1][1]=matrix[2][2]=matrix[3][3]=1.0f;
  matrix[0][1]=matrix[0][2]=matrix[0][3]=
    matrix[1][0]=matrix[1][2]=matrix[1][3]=
    matrix[2][0]=matrix[2][1]=matrix[2][3]=
    matrix[3][0]=matrix[3][1]=matrix[3][2]=0.0f;
}

bool 
dimeMatrix::isIdentity() const
{
  return 
    matrix[0][0] == 1.0f && 
    matrix[1][1] == 1.0f &&
    matrix[2][2] == 1.0f &&
    matrix[3][3] == 1.0f &&

    matrix[0][1] == 0.0f &&
    matrix[0][2] == 0.0f &&
    matrix[0][3] == 0.0f &&

    matrix[1][0] == 0.0f && 
    matrix[1][2] == 0.0f &&
    matrix[1][3] == 0.0f &&

    matrix[2][0] == 0.0f && 
    matrix[2][1] == 0.0f &&
    matrix[2][3] == 0.0f &&

    matrix[3][0] == 0.0f && 
    matrix[3][1] == 0.0f &&
    matrix[3][2] == 0.0f;
}

// FIXME: only 2D rotation is supported

void 
dimeMatrix::setTransform(const dimeVec3f &translation,
			const dimeVec3f &scalefactor,
			const dimeVec3f &rotAngles)
{
  /*
  fprintf(stderr,"set transform: %.3f %.3f %.3f - %.3f %.3f %.3f - %.3f\n",
	  translation[0], translation[1], translation[2],
	  scalefactor[0], scalefactor[1], scalefactor[2],
	  rotAngles[0]);*/
  this->makeIdentity();
  this->setTranslate(translation);
  dimeMatrix m2, m3;
  m2.makeIdentity();
  m2.setRotate(rotAngles);
  this->multRight(m2); // do rotate before translate
  m3.makeIdentity();
  m3.setScale(scalefactor);
  this->multRight(m3);    // do scale before rotate
  /*
  fprintf(stderr,
	  "%.3f %.3f %.3f %.3f\n"
	  "%.3f %.3f %.3f %.3f\n"
	  "%.3f %.3f %.3f %.3f\n"
	  "%.3f %.3f %.3f %.3f\n",
	  matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
	  matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
	  matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
	  matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
  */
}

dimeMatrix &
dimeMatrix::multRight(const dimeMatrix &m) // this = this * m
{
  dimeMatrix copy = *this;
  dxfdouble (*mat1)[4] = (dxfdouble (*)[4])copy.matrix;
  dxfdouble (*mat2)[4] = (dxfdouble (*)[4])m.matrix;
  
  int i, j, n;
  
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      matrix[i][j] = 0.0f;
      for (n = 0; n < 4; n++) matrix[i][j] += mat1[i][n] * mat2[n][j];
    }
  }
  return *this;
}

dimeMatrix &
dimeMatrix::multLeft(const dimeMatrix &m)   // this = m * this
{
  dimeMatrix copy = *this;
  dxfdouble (*mat1)[4] = (dxfdouble (*)[4])m.matrix;
  dxfdouble (*mat2)[4] = (dxfdouble (*)[4])copy.matrix;
  
  int i, j, n;
  
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      matrix[i][j] = 0.0f;
      for (n = 0; n < 4; n++) matrix[i][j] += mat1[i][n] * mat2[n][j];
    }
  }
  return *this;
}

// FIXME: only XY rotation supported

void 
dimeMatrix::setRotate(const dimeVec3f &rot)
{
  dxfdouble s = sin(DXFDEG2RAD(rot.z));
  dxfdouble c = cos(DXFDEG2RAD(rot.z));
  this->matrix[0][0] = c;
  this->matrix[0][1] = -s;
  this->matrix[1][0] = s;
  this->matrix[1][1] = c;
}

void 
dimeMatrix::setRotate(const dimeVec3f &x, const dimeVec3f &y, const dimeVec3f &z)
{
  this->matrix[0][0] = x.x;
  this->matrix[1][0] = x.y;
  this->matrix[2][0] = x.z;
  this->matrix[3][0] = 0.0f;

  this->matrix[0][1] = y.x;
  this->matrix[1][1] = y.y;
  this->matrix[2][1] = y.z;
  this->matrix[3][1] = 0.0f;

  this->matrix[0][2] = z.x;
  this->matrix[1][2] = z.y;
  this->matrix[2][2] = z.z;
  this->matrix[3][2] = 0.0f;

  this->matrix[0][3] = this->matrix[1][3] = this->matrix[2][3] = 0.0f;
  this->matrix[3][3] = 1.0f;
}

void 
dimeMatrix::setRotation(const dimeVec3f &u, const dxfdouble angle)
{
  dxfdouble cost,sint;
  
  cost=cos(angle);
  sint=sin(angle);

  matrix[3][0]=matrix[3][1]=matrix[3][2]=
    matrix[0][3]=matrix[1][3]=matrix[2][3]=0.0f;
  matrix[3][3]=1.0f;
  
  matrix[0][0]=u[0]*u[0]+cost*(1-u[0]*u[0]);
  matrix[0][1]=u[0]*u[1]*(1-cost)-u[2]*sint;
  matrix[0][2]=u[2]*u[0]*(1-cost)+u[1]*sint;
  matrix[1][0]=u[0]*u[1]*(1-cost)+u[2]*sint;
  matrix[1][1]=u[1]*u[1]+cost*(1-u[1]*u[1]);
  matrix[1][2]=u[1]*u[2]*(1-cost)-u[0]*sint;
  matrix[2][0]=u[2]*u[0]*(1-cost)-u[1]*sint;
  matrix[2][1]=u[1]*u[2]*(1-cost)+u[0]*sint;
  matrix[2][2]=u[2]*u[2]+cost*(1-u[2]*u[2]);
}

void 
dimeMatrix::setScale(const dxfdouble s)
{
  this->matrix[0][0] = this->matrix[1][1] = this->matrix[2][2] = s;  
}

void 
dimeMatrix::setScale(const dimeVec3f &s)
{
  this->matrix[0][0] = s.x;
  this->matrix[1][1] = s.y;
  this->matrix[2][2] = s.z;
}

void 
dimeMatrix::setTranslate(const dimeVec3f &t)
{
  this->matrix[0][3] = t.x;
  this->matrix[1][3] = t.y;
  this->matrix[2][3] = t.z;
}

void 
dimeMatrix::multMatrixVec(dimeVec3f &vec) const
{
  dimeVec3f copy = vec;
  multMatrixVec(copy, vec);
}

// Multiplies matrix by given column vector, giving vector result

void 
dimeMatrix::multMatrixVec(const dimeVec3f &src, dimeVec3f &dst) const
{
  dxfdouble W = 
    src.x*matrix[3][0]+
    src.y*matrix[3][1]+
    src.z*matrix[3][2]+
    matrix[3][3];
  dst.x =
    (src.x*matrix[0][0]+
     src.y*matrix[0][1]+
     src.z*matrix[0][2]+
     matrix[0][3])/W;
  dst.y =
    (src.x*matrix[1][0]+
     src.y*matrix[1][1]+
     src.z*matrix[1][2]+
     matrix[1][3])/W;
  dst.z =
    (src.x*matrix[2][0]+
     src.y*matrix[2][1]+
     src.z*matrix[2][2]+
     matrix[2][3])/W; 
}

dimeMatrix &
dimeMatrix::operator =(const dimeMatrix &m)
{
  dxfdouble *p1 = &this->matrix[0][0];
  const dxfdouble *p2 = &m.matrix[0][0];
  int n = 16;
  while (n--) *p1++ = *p2++;
  return *this;
}


dimeMatrix 
dimeMatrix::identity()
{
  return dimeMatrix(1.0f, 0.0f, 0.0f, 0.0f, 
		   0.0f, 1.0f, 0.0f ,0.0f, 
		   0.0f, 0.0f, 1.0f, 0.0f,
		   0.0f, 0.0f, 0.0f, 1.0f);
}

void
dimeMatrix::operator*=(const dxfdouble s)
{
  dxfdouble *ptr = &this->matrix[0][0];
  int n = 16;
  while (n--) *ptr++ *= s;
}

bool
dimeMatrix::inverse2()
{
#if 0 // OBSOLETED
  dimeMatrix A_;
  dxfdouble detA;

  A_.matrix[0][0]=determinant(0,0);
  A_.matrix[1][0]=-determinant(0,1);
  A_.matrix[2][0]=determinant(0,2);
  A_.matrix[3][0]=-determinant(0,3);
  A_.matrix[0][1]=-determinant(1,0);
  A_.matrix[1][1]=determinant(1,1);
  A_.matrix[2][1]=-determinant(1,2);
  A_.matrix[3][1]=determinant(1,3);
  A_.matrix[0][2]=determinant(2,0);
  A_.matrix[1][2]=-determinant(2,1);
  A_.matrix[2][2]=determinant(2,2);
  A_.matrix[3][2]=-determinant(2,3);
  A_.matrix[0][3]=-determinant(3,0);
  A_.matrix[1][3]=determinant(3,1);
  A_.matrix[2][3]=-determinant(3,2);
  A_.matrix[3][3]=determinant(3,3);

  detA=determinant();
#if defined(__linux__)
  if(isinf(1.0f/detA)) return false;
#elif defined(macintosh)
  if(isinf(1.0f/detA)) return false;
#elif !defined(__BEOS__)
  if(fpclass(detA) == FP_NZERO || fpclass(detA) == FP_PZERO) return false;
#endif
  A_*= 1 / detA;
  *this = A_;
  return true;
#else // OBSOLETED
  return false;
#endif // OBSOLETED
}

dxfdouble
dimeMatrix::determinant(const int i, const int j)
{
  int a,t;
  dxfdouble det=0.0f;

  if (i==-1 && j==-1) {   //4x4 determinant
    det+=matrix[0][0]*determinant(0,0);
    det-=matrix[1][0]*determinant(1,0);
    det+=matrix[2][0]*determinant(2,0);
    det-=matrix[3][0]*determinant(3,0);
    return det;
  }
  else { //3x3 determinant
    int x[3],y[3];
    for (a=0,t=0;a<3;a++) if (t!=i) x[a]=t++;else {t++;x[a]=t++;} 
    for (a=0,t=0;a<3;a++) if (t!=j) y[a]=t++;else {t++;y[a]=t++;} 
    
    det += matrix[x[0]][y[0]]*(matrix[x[1]][y[1]]*matrix[x[2]][y[2]] -
                               matrix[x[2]][y[1]]*matrix[x[1]][y[2]]);
    det -= matrix[x[1]][y[0]]*(matrix[x[0]][y[1]]*matrix[x[2]][y[2]] -
                               matrix[x[2]][y[1]]*matrix[x[0]][y[2]]);
    det += matrix[x[2]][y[0]]*(matrix[x[0]][y[1]]*matrix[x[1]][y[2]] -
                               matrix[x[1]][y[1]]*matrix[x[0]][y[2]]);
  }
  return det;
}

bool 
dimeMatrix::inverse()
{
  int n = 4;

  dxfdouble (*a)[4];
  a = (dxfdouble (*)[4])&this->matrix[0][0];
  
  dxfdouble max, s, h, q, pivot;
  int p[4];
  int i, j, k;

  for (k = 0; k < n; k++) { 
    max = 0.0f;
    p[k] = 0;

    for (i = k; i < n; i++) { 
      s = 0.0;
      for (j = k; j < n; j++)
        s += fabs(a[i][j]);
      q = fabs(a[i][k]) / s;
      if (q > max) { 
	max = q;  
        p[k] = i;
      }
    }

    if (max == 0.0f) return false;

    if (p[k] != k) {
      for (j = 0; j < n; j++) {
	h = a[k][j];  a[k][j] = a[p[k]][j];  a[p[k]][j] = h;
      }
    }

    pivot = a[k][k];
    for (j = 0; j < n; j++) {
      if (j != k) {
	a[k][j] = - a[k][j] / pivot;
        for (i = 0; i < n; i++) {
          if (i != k) a[i][j] += a[i][k] * a[k][j];
	}
      }
    }

    for (i = 0; i < n; i++) a[i][k] /= pivot;
    a[k][k] = 1 / pivot;
  }

  for (k = n-2; k >= 0; k--) {
    if (p [k] != k) {
      for (i = 0; i < n; i++) {
	h = a [i][k];  a [i][k] = a [i][p[k]];  a [i][p[k]] = h;
      }
    }
  }
  return true; 
}

