/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	VRMLSetInfo.h
*
******************************************************************/

#ifndef _CV97_SETINFO_H_
#define _CV97_SETINFO_H_

void AddSFColor(float		color[3]);
void AddSFRotation(float	rotation[4]);
void AddSFVec3f(float		vector[3]);
void AddSFVec2f(float		vector[2]);
void AddSFInt32(int			value);
void AddSFFloat(float		value);
void AddSFString(char		*string);

#endif
