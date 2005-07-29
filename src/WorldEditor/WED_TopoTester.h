#ifndef WED_TOPOTESTER_H
#define WED_TOPOTESTER_H

#include "WED_MapTool.h"
#include "MapDefs.h"

class	WED_TopoTester : public WED_MapTool {
public:

					WED_TopoTester(WED_MapZoomer * inZoomer);
	virtual			~WED_TopoTester();

	virtual	void	DrawFeedbackUnderlay(
							bool				inCurrent);
	virtual	void	DrawFeedbackOverlay(
							bool				inCurrent);
	virtual	bool	HandleClick(
							XPLMMouseStatus		inStatus,
							int 				inX, 
							int 				inY, 
							int 				inButton);

	virtual int		GetNumProperties(void);
	virtual	void	GetNthPropertyName(int, string&);
	virtual	double	GetNthPropertyValue(int);
	virtual	void	SetNthPropertyValue(int, double);
	
	virtual	int		GetNumButtons(void);
	virtual	void	GetNthButtonName(int, string&);
	virtual	void	NthButtonPressed(int);
	
	virtual	char *	GetStatusText(void);
	
private:

		Point2					mAnchor;
		Point2					mTarget;
		GISHalfedge *			mAnchorHint;
		Pmwx::Locate_type		mAnchorLoc;
		
		Point2					mFound;
		Pmwx::Locate_type		mFoundLoc;
		GISHalfedge *			mFoundHint;
		bool					mRayShoot;
		
};

#endif