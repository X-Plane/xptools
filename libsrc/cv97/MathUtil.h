/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MathUtil.h
*
******************************************************************/

#ifndef _CV97_MATHUTIL_H_
#define _CV97_MATHUTIL_H_

#ifdef SUPPORT_OLDCPP
#include "OldCpp.h"
#endif

#ifndef PI
#define PI 3.1415926535897f

inline double Degree2Radian(double degree)
{
	return (degree / 180.0 * PI);
}

inline double Radian2Degree(double radian)
{
	return (radian / PI * 180.0);
}

#endif

void	VectorNormalize(float vector[3]);
float	VectorGetLength(float vector[3]);
void	VectorInverse(float vector[3]);
bool	VectorEquals(float vector1[3], float vector2[3]);
void	VectorGetDirection(float point1[3], float point2[3], float vector[3]);
float	VectorGetDot(float vector1[3], float vector2[3]);
void	VectorGetCross(float vector1[3], float vector2[3], float result[3]);
float	VectorGetAngle(float vector1[3], float vector2[3]);
bool	VectorEquals(float vector1[3], float vector2[3]);

void	GetNormalFromVertices(float vpoint[3][3], float vector[3]);
void	GetPlaneVectorFromTwoVectors(float vector1[3], float vector2[3], float resultVector[3]);

#endif
