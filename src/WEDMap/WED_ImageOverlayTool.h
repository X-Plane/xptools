#ifndef WED_IMAGEOVERLAYTOOL_H
#define WED_IMAGEOVERLAYTOOL_H

#include "WED_HandleToolBase.h"
#include "IControlHandles.h"

class	WED_ImageOverlayTool : public WED_HandleToolBase, public virtual IControlHandles {
public:

						 WED_ImageOverlayTool(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver, const char * root_path, const char * selection_path);
	virtual				~WED_ImageOverlayTool();

			void		PickFile(void);
			bool		CanShow(void) { return mBits; }
			bool		IsVisible(void) { return mVisible; }
			void		ToggleVisible(void);

	// WED_MapLayer
	virtual	void		DrawVisualization		(int inCurrent, GUI_GraphState * g);
	
	// IOperation
	virtual		void	BeginEdit(void);
	virtual		void	EndEdit(void);

	virtual		int		CountEntities(void) const;
	virtual		int		GetNthEntityID(int n) const;

	virtual		int		CountControlHandles(int id						  ) const;
	virtual		void	GetNthControlHandle(int id, int n,		 Point2& p) const;
	virtual		void	SetNthControlHandle(int id, int n, const Point2& p)		 ;

	virtual		int		GetLinks		    (int id) const;
	virtual		int		GetNthLinkSource   (int id, int n) const;
	virtual		int		GetNthLinkSourceCtl(int id, int n) const;	// -1 if no bezier ctl point!
	virtual		int		GetNthLinkTarget   (int id, int n) const;
	virtual		int		GetNthLinkTargetCtl(int id, int n) const;
	
	virtual		bool	PointOnStructure(int id, const Point2& p) const;
	
	virtual		void	ControlsMoveBy(int id, const Vector2& delta);			
	virtual		void	ControlsHandlesBy(int id, int c, const Vector2& delta);
	virtual		void	ControlsLinksBy	 (int id, int c, const Vector2& delta);

	// WED_MapToolNew
	virtual int			GetNumProperties(void) { return 0; }
	virtual	void		GetNthPropertyName(int, string&) { }
	virtual	double		GetNthPropertyValue(int) { return 0; }
	virtual	void		SetNthPropertyValue(int, double) { }
	
	virtual	int			GetNumButtons(void) { return 0; }
	virtual	void		GetNthButtonName(int, string&) { }
	virtual	void		NthButtonPressed(int) { }
	
	virtual	char *		GetStatusText(void) { return NULL; }

	// IUnknown
	virtual void *		QueryInterface(const char * class_id);

private:

	virtual	EntityHandling_t	TraverseEntity(IGISEntity * ent) { return ent_Skip; }

	unsigned long	mTexID;
	bool			mVisible;
	bool			mBits;
	string			mFile;
	Polygon2		mCoords;

};	

#endif
