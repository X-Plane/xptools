/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	BoundingBox.h
*
******************************************************************/

#ifndef _CV97_BOUNDINGBOX_H_
#define _CV97_BOUNDINGBOX_H_

class BoundingBox {

	float	mMaxPosition[3];
	float	mMinPosition[3];
	int		mNPoints;

public:

	BoundingBox();
	BoundingBox(BoundingBox *bbox);
	BoundingBox(float center[3], float size[3]);

	~BoundingBox();

	void	initialize();

	void	addPoint(float point[3]);
	void	addPoint(float x, float y, float z);
	void	addBoundingBox(float center[3], float size[3]);
	void	addBoundingBox(BoundingBox *bbox);

	void	setNPoints(int npoints);
	int		getNPoints();

	void	setMinPosition(float pos[3]);
	void	setMaxPosition(float pos[3]);

	void	setMinPosition(float x, float y, float z);
	void	setMaxPosition(float x, float y, float z);

	void	getMinPosition(float pos[3]);
	void	getMaxPosition(float pos[3]);

	void	set(float center[3], float size[3]);
	void	set(BoundingBox *bbox);

	void	getCenter(float center[3]);
	void	getSize(float size[3]);

};

#endif
