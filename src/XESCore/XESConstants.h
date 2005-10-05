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
#ifndef XESCONSTANTS_H
#define XESCONSTANTS_H

// Std lapse rate is 2 degrees C colder per 1000 feet higher, or per 304.8 meters.
const float 	kStdLapseRate = -2.0 / 304.8;

#define NM_TO_MTR			1852.0
#define MTR_TO_NM			0.00054
#define FT_TO_MTR			0.3048
#define MTR_TO_FT			3.28084
#define PI					3.14159265358979323846
#define PI2					6.283185307179586
#define DEG_TO_NM_LAT		60.0
#define NM_TO_DEG_LAT		0.01666666666666667
#if !defined(PROJ_API_H)
#define RAD_TO_DEG			57.29577951308232
#define DEG_TO_RAD			0.0174532925199432958
#endif
#define DEG_TO_MTR_LAT		111120.0
#define MTR_TO_DEG_LAT		0.000008999280057595392

#endif
