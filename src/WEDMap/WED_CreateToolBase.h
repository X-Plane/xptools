#ifndef WED_CREATETOOLBASE_H
#define WED_CREATETOOLBASE_H

#include "WED_MapToolNew.h"
#include "CompGeomDefs2.h"
#include <vector>
#include "WED_HandleToolBase.h"
#include "IControlHandles.h"

class	WED_Archive;

using std::vector;

class	WED_CreateToolBase : public WED_HandleToolBase, public virtual IControlHandles {
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
									int					must_close,
									int					requires_airport);
	virtual				~WED_CreateToolBase();

//	virtual	int			HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
//	virtual	void		HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
//	virtual	void		HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers);
//	virtual	void		KillOperation(void);

//	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g); 

	virtual		void	BeginEdit(void);
	virtual		void	EndEdit(void);
	virtual		int		CountEntities(void) const;
	virtual		int		GetNthEntityID(int n) const;
	virtual		int		CountControlHandles(int id						  ) const;
	virtual		void	GetNthControlHandle(int id, int n, int * active, HandleType_t * con_type, Point2 * p, Vector2 * direction) const;
	virtual		int		GetLinks		    (int id) const;
	virtual		void	GetNthLinkInfo(int id, int n, int * active, LinkType_t * ltype) const;
	virtual		int		GetNthLinkSource   (int id, int n) const;
	virtual		int		GetNthLinkSourceCtl(int id, int n) const;	// -1 if no bezier ctl point!
	virtual		int		GetNthLinkTarget   (int id, int n) const;
	virtual		int		GetNthLinkTargetCtl(int id, int n) const;
	virtual		bool	PointOnStructure(int id, const Point2& p) const;
	virtual		void	ControlsHandlesBy(int id, int c, const Vector2& delta);			
	virtual		void	ControlsLinksBy	 (int id, int c, const Vector2& delta);			
	virtual		void	ControlsMoveBy	 (int id,        const Vector2& delta);			

	virtual	int					CreationDown(const Point2& start_pt);
	virtual	void				CreationDrag(const Point2& start_pt, const Point2& now_pt);
	virtual	void				CreationUp(const Point2& start_pt, const Point2& now_pt);

	virtual void		KillOperation(void);
	virtual	int			HandleKeyPress(char inKey, int inVK, GUI_KeyFlags inFlags			  );

	
protected:


	virtual	void		AcceptPath(
							const vector<Point2>&	pts,
							const vector<Point2>&	dirs_lo,
							const vector<Point2>&	dirs_hi,
							const vector<int>		has_dirs,
							const vector<int>		has_split,
							int						closed)=0;

		inline WED_Archive * GetArchive(void) { return mArchive; }

private:

			void		DoEmit(int close_it);

	int					mEditStarted;

	WED_Archive *		mArchive;

	vector<Point2>		mPts;
	vector<Point2>		mControlLo;
	vector<Point2>		mControlHi;
	vector<int>			mHasDirs;
	vector<int>			mIsSplit;
	
//	int		mStartX;
//	int		mStartY;
//	int		mNowX;
//	int		mNowY;

	float	mLastTime;
	Point2	mLastPt;
	

//	int		mDirOpen;
	int		mCreating;
	
	int		mMinPts;
	int		mMaxPts;
	int		mCanClose;
	int		mMustClose;
	int		mCanCurve;
	int		mMustCurve;
	
	int		mMustHaveApt;

};


#endif /* WED_CREATETOOLBASE_H */
