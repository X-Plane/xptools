#ifndef WED_MARQUEETOOL_H
#define WED_MARQUEETOOL_H

#include "WED_HandleToolBase.h"
#include "IControlHandles.h"
#include "IOperation.h"

class	WED_MarqueeTool : public WED_HandleToolBase, public virtual IControlHandles {
public:

						 WED_MarqueeTool(
										GUI_Pane *				host,
										WED_MapZoomerNew *		zoomer,
										IResolver *				resolver,
										const char *			root_path,
										const char *			selection_path);						 
	virtual				~WED_MarqueeTool();
	

	// CONTROL HANDLE INTERFACE:	
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



	virtual int			GetNumProperties(void) { return 0; }
	virtual	void		GetNthPropertyName(int, string&) { }
	virtual	double		GetNthPropertyValue(int) { return 0; }
	virtual	void		SetNthPropertyValue(int, double) { }
	
	virtual	int			GetNumButtons(void) { return 0; }
	virtual	void		GetNthButtonName(int, string&) { }
	virtual	void		NthButtonPressed(int) { }
	
	virtual	char *		GetStatusText(void) { return NULL; }

	virtual void *		QueryInterface(const char * class_id);

private:

	virtual	EntityHandling_t	TraverseEntity(IGISEntity * ent) { return ent_AtomicOrContainer; }

				void	GetEntityInternal(vector<IGISEntity *>& e);
				bool	GetTotalBounds(Bbox2& b) const;
				void	ApplyRescale(const Bbox2& old_bounds, const Bbox2& new_bounds);

		IResolver *				mResolver;
		string					mSelection;

	
};

#endif /* WED_MARQUEETOOL_H */