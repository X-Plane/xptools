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
#ifndef WED_XPLUGINDRAWUTILS_H
#define WED_XPLUGINDRAWUTILS_H

#include "CompGeomDefs3.h"

#if LIN || IBM
#include <GL/gl.h>
#else
#if __GNUC__
#include <OpenGL/Gl.h>
#else
#include <gl.h>
#endif
#endif

#define LINE_OFFSET 0.1

#define	BEZ_MIN_SEGS    3
#define BEZ_MAX_SEGS    100
#define	BEZ_MTR_PER_SEG 2

#include <math.h>

const float Pi = 3.14159265358979;
const float RadToDeg = 180.0 / Pi;
const float DegToRad = 1.0 / RadToDeg;

inline void rotate2d(const double deg,
                     const double pX,const double pY,
                     const double wX,const double wY,
                     double * outX,double * outY)
{
    double psi = deg * DegToRad;
    double aCos = cos(psi);
    double aSin = sin(psi);

    if (outX) *outX = (pX * aCos - pY * aSin) + wX;
    if (outY) *outY = (pX * aSin + pY * aCos) + wY;
}

const float gWallCols[24] = {   .5,  0,  0, 1,
                                .5, .5,  0, 1,
                                 0, .5,  0, 1,
                                 0, .5, .5, 1,
                                 0,  0, .5, 1,
                                .5,  0, .5, 1   };

void glPolygon3(const Point3 * pts,bool has_uv, const int * contours, int n);

#endif // WED_XPLUGINDRAWUTILS_H
