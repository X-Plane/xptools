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

#ifndef WED_CreateLineTool_H
#define WED_CreateLineTool_H

#include "WED_CreateToolBase.h"

enum CreateLine_t {
	create_Runway = 0,
	create_Sealane
};

class WED_CreateLineTool : public WED_CreateToolBase {
public:

						 WED_CreateLineTool(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer, 
									IResolver *			resolver,
									WED_Archive *		archive,
									CreateLine_t		tool_type);
	virtual				~WED_CreateLineTool();

	// WED_MapToolNew
	virtual	const char *		GetStatusText(void);
//	virtual void *		QueryInterface(const char * class_id);

protected:

		WED_PropIntEnum				rwy_surface;
		WED_PropIntEnum				rwy_shoulder;
		WED_PropDoubleText			rwy_roughness;
		WED_PropBoolText			rwy_center_lites;
		WED_PropIntEnum				rwy_edge_lights;
		WED_PropBoolText			rwy_remaining_signs;

		WED_PropIntEnum				rwy_markings;
		WED_PropIntEnum				rwy_app_lights;
		WED_PropBoolText			rwy_tdzl;
		WED_PropIntEnum				rwy_reil;
		
		WED_PropBoolText			sea_buoys;

	virtual	void		AcceptPath(
							const vector<Point2>&	pts,
							const vector<int>		has_dirs,
							const vector<Point2>&	dirs,
							int						closed);

		CreateLine_t	mType;


};

#endif /* WED_CreateLineTool_H */
