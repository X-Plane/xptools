/**************************************************************************\
 * 
 *  FILE: Linear.h
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

#ifndef DIME_LINEAR_H
#define DIME_LINEAR_H

#include <dime/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

class DIME_DLL_API dimeVec2f
{
public:
  dimeVec2f() {}
  dimeVec2f(const dimeVec2f &vec) {x = vec.x; y = vec.y;}
  dimeVec2f(dxfdouble _x, dxfdouble _y) {x = _x; y = _y;}
  void setValue(const dxfdouble _x, const dxfdouble _y) {x = _x; y = _y;}
 
  //dxfdouble operator [] (const int i) const         
  //{ return (i==0?x:y); }
  //  dxfdouble & operator [] (const int i)             
  //{ return(i==0?x:y);}
  void print() const { printf("Coord2: (%.3f, %.3f)\n", x,y);}
  void print(const char *s) {printf("%s: (%.3f, %.3f)\n", s,x,y);}
  dxfdouble x,y;

}; // class dimeVec2f

class DIME_DLL_API dimeVec3f
{ 
public:
  dxfdouble x, y, z;
  
  dimeVec3f(void) {};
  dimeVec3f(const dxfdouble X, const dxfdouble Y, const dxfdouble Z) 
  { x=X; y=Y; z=Z; };
  dimeVec3f(const dxfdouble *xyz)
  { x = xyz[0]; y = xyz[1]; z = xyz[2]; }
  dimeVec3f (const dimeVec3f& v) 
  { x=v.x; y=v.y; z=v.z; };
  dimeVec3f cross(const dimeVec3f &v) const
  { return dimeVec3f(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
  dxfdouble dot(const dimeVec3f &v) const
  { return x*v.x+y*v.y+z*v.z; }

  bool equals(const dimeVec3f &v)
  { return (x == v.x && y == v.y && z == v.z); }
  bool equals(const dimeVec3f &v, dxfdouble tol)
  { return (fabs(x-v.x) <= tol && fabs(y-v.y) <= tol && fabs(z-v.z) <= tol); }

  operator dxfdouble *() { return &x; } 
  const dxfdouble *getValue() const { return &x; }
  void getValue(dxfdouble &_x, dxfdouble &_y, dxfdouble &_z) const
  { _x = x; _y = y; _z = z;}
  dxfdouble length() const                    
  { return (dxfdouble) sqrt(x*x+y*y+z*z); }
  dxfdouble sqrLength(void) const
  { return x*x+y*y+z*z; }
  void negate(void)
  { x = -x; y = -y; z = -z; }
  void setValue(const dxfdouble *v) 
  { x = v[0]; y = v[1]; z = v[2]; } 
  void setValue(const dxfdouble X, const dxfdouble Y, const dxfdouble Z)
  { x=X; y=Y; z=Z; }

  dxfdouble operator [] (const int i) const
  { return( (i==0)?x:((i==1)?y:z) ); };
  dxfdouble & operator [] (const int i)             
  { return( (i==0)?x:((i==1)?y:z) ); };

  dimeVec3f &operator *= (const dxfdouble s)
  { x*=s; y*=s; z*=s; return *this; }
  dimeVec3f &operator /= (const dxfdouble s)
  { x/=s; y/=s; z/=s; return *this; }
  dimeVec3f &operator += (const dimeVec3f &v) 
  { x+=v.x; y+=v.y; z+=v.z; return *this; }
  dimeVec3f &operator -= (const dimeVec3f &v)
  { x-=v.x; y-=v.y; z-=v.z; return *this;}
  dimeVec3f operator -() const
  { return dimeVec3f(-x, -y, -z); }
  friend dimeVec3f operator *(const dimeVec3f &v, dxfdouble s)
  { return dimeVec3f(v.x*s, v.y*s, v.z*s); }
  friend dimeVec3f operator *(dxfdouble s, const dimeVec3f &v)
  { return dimeVec3f(v.x*s, v.y*s, v.z*s); }
  friend dimeVec3f operator / (const dimeVec3f &v, dxfdouble s)
  { return dimeVec3f(v.x/s, v.y/s, v.z/s); }
  friend dimeVec3f operator + (const dimeVec3f &v1, const dimeVec3f &v2)
  { return dimeVec3f(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); } 
  friend dimeVec3f operator - (const dimeVec3f &v1, const dimeVec3f &v2)
  { return dimeVec3f(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }

  friend bool operator ==(const dimeVec3f &v1, const dimeVec3f &v2)
  { return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z); }
  friend bool operator !=(const dimeVec3f &v1, const dimeVec3f &v2)
  { return (v1.x != v2.x || v1.y != v2.y || v1.z != v2.z); }

  dimeVec3f& operator = (const dimeVec3f &v)        // extra
  { x=v.x; y=v.y; z=v.z; return *this; } 
   
  void multMatrix(dxfdouble *matrix)       // extra
  {
    dxfdouble tx, ty, tz;
    tx = ty = tz = 0.0f;
    tx = matrix[0]*x+matrix[1]*y+matrix[2]*z;
    ty = matrix[4]*x+matrix[5]*y+matrix[6]*z;
    tz = matrix[8]*x+matrix[9]*y+matrix[10]*z;
    x = tx, y = ty, z = tz;
  } 

  void print() const // extra
  { printf("dimeVec3f: (%.3f, %.3f, %.3f)\n",x, y, z); }
  void print(const char *s) const
  { printf("%s: (%.3f, %.3f, %.3f)\n",s, x, y, z); }


  dimeVec3f multComponents(const dimeVec3f &v) const
  { return dimeVec3f(x*v.x, y*v.y, z*v.z); }

  dxfdouble angle(const dimeVec3f &v2);
  void normalize();

}; // class dimeVec3f

class DIME_DLL_API dimeMatrix
{
public:
  dimeMatrix() {}
  dimeMatrix(const dimeMatrix &matrix);
  // Constructor given all 16 elements in row-major order
  dimeMatrix(dxfdouble a11, dxfdouble a12, dxfdouble a13, dxfdouble a14,
	    dxfdouble a21, dxfdouble a22, dxfdouble a23, dxfdouble a24, 
	    dxfdouble a31, dxfdouble a32, dxfdouble a33, dxfdouble a34, 
	    dxfdouble a41, dxfdouble a42, dxfdouble a43, dxfdouble a44);
  void transpose();
  void makeIdentity();
  bool isIdentity() const;
  void setTransform(const dimeVec3f &translation,
		    const dimeVec3f &scalefactor,
		    const dimeVec3f &rotAngles);
  dimeMatrix &multRight(const dimeMatrix &m); // this = this * m
  dimeMatrix &multLeft(const dimeMatrix &m);   // this = m * this
  
  // Sets matrix to rotate by given rotation
  void setRotate(const dimeVec3f &rot);

  void setRotate(const dimeVec3f &x, const dimeVec3f &y, const dimeVec3f &z);

  // sets matrix to rotate around given vector
  void setRotation(const dimeVec3f &u, const dxfdouble angle);

  // Sets matrix to scale by given uniform factor
  void setScale(const dxfdouble s);

  // Sets matrix to scale by given vector
  void setScale(const dimeVec3f &s);

  // Sets matrix to translate by given vector
  void setTranslate(const dimeVec3f &t);

  // Multiplies matrix by given column vector, giving vector result
  void multMatrixVec(const dimeVec3f &src, dimeVec3f &dst) const;

  // transforms vector
  void multMatrixVec(dimeVec3f &vec) const;
  
  // Multiplies given row vector by matrix, giving vector result
  //void multVecMatrix(const dimeVec3f &src, dimeVec3f &dst) const;
  
  // Cast: returns pointer to storage of first element
  operator dxfdouble *() { return &matrix[0][0]; }

  // Make it look like a usual matrix (so you can do m[3][2])
  dxfdouble *operator [](int i) { return &matrix[i][0]; }
  const dxfdouble * operator [](int i) const { return &matrix[i][0];}

  dimeMatrix &operator =(const dimeMatrix &m);

  // Performs right multiplication with another matrix
  dimeMatrix &operator *=(const dimeMatrix &m)  { return multRight(m); }
  

  static dimeMatrix identity();
  bool inverse();
  bool inverse2();
  dxfdouble determinant(const int i=-2, const int j=-1);

  void operator *=(const dxfdouble val);

private:
  dxfdouble matrix[4][4];

}; // class dimeMatrix

#endif // ! DIME_LINEAR_H

