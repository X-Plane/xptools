/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef WED_CREATEPOLYGONTOOL_H
#define WED_CREATEPOLYGONTOOL_H

#include "WED_CreateToolBase.h"

class	WED_Thing;

enum CreateTool_t {

	create_Taxi = 0,
	create_Boundary,
	create_Marks,
	create_Hole,

	create_Facade,
	create_Forest,
	create_String,
	create_Line,
	create_Polygon

};

class	WED_CreatePolygonTool : public WED_CreateToolBase {
public:

						 WED_CreatePolygonTool(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer,
									IResolver *			resolver,
									WED_Archive *		archive,
									CreateTool_t		tool_type);
	virtual				~WED_CreatePolygonTool();

			void				SetResource(const string& r);

	// WED_MapToolNew
	virtual	const char *		GetStatusText(void);
//	virtual void *		QueryInterface(const char * class_id);

protected:

		WED_PropIntEnum					mPavement;
		WED_PropDoubleText				mRoughness;
		WED_PropDoubleText				mHeading;
		WED_PropIntEnumSet				mMarkings;
		WED_PropIntEnumSetFilter		mMarkingsLines;
		WED_PropIntEnumSetFilter		mMarkingsLights;

		WED_PropStringText				mResource;
		WED_PropDoubleText				mHeight;
		WED_PropDoubleText				mDensity;
		WED_PropDoubleText				mSpacing;

	virtual	void		AcceptPath(
							const vector<Point2>&	pts,
							const vector<Point2>&	dirs_lo,
							const vector<Point2>&	dirs_hi,
							const vector<int>		has_dirs,
							const vector<int>		has_split,
							int						closed);
	virtual	bool		CanCreateNow(void);


			WED_Thing *	GetHost(int& idx);

		CreateTool_t	mType;

};

#endif /* WED_CREATEPOLYGONTOOL_H */
