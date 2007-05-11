#ifndef WED_CREATEPOLYGONTOOL_H
#define WED_CREATEPOLYGONTOOL_H

#include "WED_CreateToolBase.h"

enum CreateTool_t {

	create_Taxi = 0,
	create_Boundary,
	create_Marks

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

	// WED_MapToolNew
	virtual	const char *		GetStatusText(void);
//	virtual void *		QueryInterface(const char * class_id);

protected:

		WED_PropIntEnum			mPavement;
		WED_PropDoubleText		mRoughness;
		WED_PropDoubleText		mHeading;
		WED_PropIntEnumSet		mMarkings;

	virtual	void		AcceptPath(
							const vector<Point2>&	pts,
							const vector<int>		has_dirs,
							const vector<Point2>&	dirs,
							int						closed);

		CreateTool_t	mType;

};

#endif /* WED_CREATEPOLYGONTOOL_H */
