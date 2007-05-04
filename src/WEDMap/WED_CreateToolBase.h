#ifndef WED_CREATETOOLBASE_H
#define WED_CREATETOOLBASE_H

#include "WED_MapToolNew.h"
#include "CompGeomDefs2.h"
#include <vector>

class	WED_Archive;

using std::vector;

class	WED_CreateToolBase : public WED_MapToolNew {
public:

						 WED_CreateToolBase(
									const char *		tool_name,
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer, 
									IResolver *			resolver,
									WED_Archive *		archive,
									int					min_num_pts,
									int					max_num_pts,
									int					can_curve,
									int					must_curve,
									int					can_close,
									int					must_close);
	virtual				~WED_CreateToolBase();

	virtual	int			HandleClickDown(int inX, int inY, int inButton);
	virtual	void		HandleClickDrag(int inX, int inY, int inButton);
	virtual	void		HandleClickUp  (int inX, int inY, int inButton);

	virtual	void		DrawVisualization		(int inCurrent, GUI_GraphState * g) { }
	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g); 
	virtual	void		DrawSelected			(int inCurrent, GUI_GraphState * g) { } 
	
	virtual	void		DrawEntityVisualization	(int inCurrent, IGISEntity * entity, GUI_GraphState * g) { } 
	virtual	void		DrawEntityStructure		(int inCurrent, IGISEntity * entity, GUI_GraphState * g) { } 
	virtual	void		DrawEntitySelected		(int inCurrent, IGISEntity * entity, GUI_GraphState * g) { } 
							
protected:

	virtual	void		AcceptPath(
							const vector<Point2>&	pts,
							const vector<int>		has_dirs,
							const vector<Point2>&	dirs,
							int						closed)=0;

		inline WED_Archive * GetArchive(void) { return mArchive; }

private:

			void		DoEmit(int close_it);

	WED_Archive *		mArchive;

	vector<Point2>		mPts;
	vector<Point2>		mDirs;
	vector<int>			mHasDirs;
	
	int		mStartX;
	int		mStartY;
	int		mNowX;
	int		mNowY;

	float	mLastTime;
	int		mLastX;
	int		mLastY;

	int		mDirOpen;
	int		mCreating;
	
	int		mMinPts;
	int		mMaxPts;
	int		mCanClose;
	int		mMustClose;
	int		mCanCurve;
	int		mMustCurve;

};


#endif /* WED_CREATETOOLBASE_H */
