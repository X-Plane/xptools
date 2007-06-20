#ifndef WED_VERTEXTOOL_H
#define WED_VERTEXTOOL_H

#include "WED_HandleToolBase.h"
#include "IControlHandles.h"
#include "IOperation.h"

class	IGISEntity;

class	WED_VertexTool : public WED_HandleToolBase, public virtual IControlHandles {
public:

						 WED_VertexTool(
										const char *			tool_name,
										GUI_Pane *				host,
										WED_MapZoomerNew *		zoomer,
										IResolver *				resolver,
										int						sel_verts);						 
	virtual				~WED_VertexTool();	

	// CONTROL HANDLE INTERFACE:	
	virtual		void	BeginEdit(void);
	virtual		void	EndEdit(void);

	virtual		int		CountEntities(void) const;
	virtual		int		GetNthEntityID(int n) const;

	virtual		int		CountControlHandles(int id						  ) const;
	virtual		void	GetNthControlHandle(int id, int n, int * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const;

	virtual		int		GetLinks		    (int id) const;
	virtual		void	GetNthLinkInfo		(int id, int n, int * active, LinkType_t * ltype) const;
	virtual		int		GetNthLinkSource   (int id, int n) const;
	virtual		int		GetNthLinkSourceCtl(int id, int n) const;	// -1 if no bezier ctl point!
	virtual		int		GetNthLinkTarget   (int id, int n) const;
	virtual		int		GetNthLinkTargetCtl(int id, int n) const;
	
	virtual		bool	PointOnStructure(int id, const Point2& p) const;
	
	virtual		void	ControlsMoveBy(int id, const Vector2& delta);			
	virtual		void	ControlsHandlesBy(int id, int c, const Vector2& delta);
	virtual		void	ControlsLinksBy	 (int id, int c, const Vector2& delta);

//	virtual	int			FindProperty(const char * in_prop) { return -1; }
//	virtual int			CountProperties(void) { return 0; }
//	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info) {} 
//	virtual	void		GetNthPropertyDict(int n, PropertyDict_t& dict) { }
//	virtual	void		GetNthPropertyDictItem(int n, int e, string& item) { }
	
//	virtual void		GetNthProperty(int n, PropertyVal_t& val) { }
//	virtual void		SetNthProperty(int n, const PropertyVal_t& val) { }

	virtual	const char *		GetStatusText(void) { return NULL; }

//	virtual void *		QueryInterface(const char * class_id);

private:

	virtual	EntityHandling_t	TraverseEntity(IGISEntity * ent);

			void		GetEntityInternal(void) const;
			void		AddEntityRecursive(IGISEntity * e, const Bbox2& bounds) const;
			
		int						mSelVerts;
		int						mInEdit;
		
		mutable vector<IGISEntity *>	mEntityCache;
		mutable long long				mCacheKeyArchive;
		mutable long long				mCacheKeyZoomer;
};


#endif /* WED_VERTEXTOOL_H */
