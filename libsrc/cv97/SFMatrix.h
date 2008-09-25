/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFMatrix.h
*
******************************************************************/

#ifndef _CV97_SFMATRIX_H_
#define _CV97_SFMATRIX_H_

#include <math.h>
#include "Field.h"
#include "SFVec3f.h"
#include "SFRotation.h"

class SFRotation;

class SFMatrix : public Field {

	float mValue[4][4];

public:

	SFMatrix();
	SFMatrix(float value[4][4]);
	SFMatrix(SFMatrix *value);
	SFMatrix(SFRotation *rot);
	SFMatrix(float x, float y, float z, float angle);
	SFMatrix(float x, float y, float z);

	////////////////////////////////////////////////
	//	set value
	////////////////////////////////////////////////

	void setValue(float value[4][4]);
	void setValue(SFMatrix *matrix);

	////////////////////////////////////////////////
	//	set as scaling value
	////////////////////////////////////////////////

	void setScaling(SFVec3f *vector);
	void setScaling(float value[]);
	void setScaling(float x, float y, float z);

	////////////////////////////////////////////////
	//	set as translation value
	////////////////////////////////////////////////

	void setTranslation(SFVec3f *vector);
	void setTranslation(float value[]);
	void setTranslation(float x, float y, float z);

	////////////////////////////////////////////////
	//	set as direction value
	////////////////////////////////////////////////

	void setDirection(SFVec3f *vector);
	void setDirection(float value[]);
	void setDirection(float x, float y, float z);

	////////////////////////////////////////////////
	//	set as rotation value
	////////////////////////////////////////////////

	void setRotation(SFRotation *rotation);
	void setRotation(float value[]);
	void setRotation(float x, float y, float z, float rot);

	////////////////////////////////////////////////
	//	get value
	////////////////////////////////////////////////

	void getValue(float value[4][4]);

	////////////////////////////////////////////////
	//	get value only translation
	////////////////////////////////////////////////

	void getTranslation(float value[]);

	////////////////////////////////////////////////
	//	add
	////////////////////////////////////////////////

	void add(SFMatrix *matrix);

	////////////////////////////////////////////////
	//	multi
	////////////////////////////////////////////////

	void multi(float vector[]);
	void multi(float *x, float *y, float *z);
	void multi(SFVec3f *vector);

	////////////////////////////////////////////////
	//	convert
	////////////////////////////////////////////////

	void getSFRotation(SFRotation *rotation);

	////////////////////////////////////////////////
	//	toString
	////////////////////////////////////////////////

	void setValue(char *value);
	char *getValue(char *buffer, int bufferLen);

	////////////////////////////////////////////////
	//	other
	////////////////////////////////////////////////

	void invert();
	float determinant();
	void init();

	////////////////////////////////////////////////
	//	Java
	////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

	jobject toJavaObject(int bConstField = 0);
	void setValue(jobject field, int bConstField = 0);
	void getValue(jobject field, int bConstField = 0);

#endif
};

#endif