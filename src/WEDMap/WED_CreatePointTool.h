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

#ifndef WED_CreatePointTool_H
#define WED_CreatePointTool_H

#include "WED_CreateToolBase.h"

enum CreatePoint_t {
	create_Beacon = 0,
	create_Sign,
	create_Helipad,
	create_Lights,
	create_RampStart,
	create_TowerViewpoint,
	create_Windsock,
	create_Object
};

class WED_CreatePointTool : public WED_CreateToolBase {
public:

						 WED_CreatePointTool(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer,
									IResolver *			resolver,
									WED_Archive *		archive,
									CreatePoint_t		tool_type);
	virtual				~WED_CreatePointTool();

			void				SetResource(const string& r);

	// WED_MapToolNew
	virtual	const char *		GetStatusText(void);

	// From IPropertyObject to customize sign text
	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info) const;
	virtual void		GetNthProperty(int n, PropertyVal_t& val) const;
	virtual void		SetNthProperty(int n, const PropertyVal_t& val);
	
protected:

		WED_PropIntEnum			beacon_kind;
		WED_PropIntEnum			sign_style;
		WED_PropIntEnum			sign_height;
		WED_PropIntEnum			heli_surface;
		WED_PropIntEnum			heli_markings;
		WED_PropIntEnum			heli_shoulder;
		WED_PropDoubleText		heli_roughness;
		WED_PropIntEnum			heli_edgelights;
		WED_PropIntEnum			light_kind;
		WED_PropDoubleText		light_angle;
		WED_PropDoubleText		tower_height;
		WED_PropBoolText		windsock_lit;
		WED_PropStringText		resource;
		WED_PropIntEnum			show_level;
		WED_PropIntEnum			ramp_type;
		WED_PropIntEnumBitfield	equip_type;
		WED_PropIntEnum			width;
		WED_PropIntEnum			ramp_op_type;
		WED_PropStringText		airlines;
		WED_PropStringText		sign_text;
		

	virtual	void		AcceptPath(
							const vector<Point2>&	pts,
							const vector<Point2>&	dirs_lo,
							const vector<Point2>&	dirs_hi,
							const vector<int>		has_dirs,
							const vector<int>		has_split,
							int						closed);
	virtual	bool		CanCreateNow(void);

		CreatePoint_t	mType;


};

#endif /* WED_CreatePointTool_H */
