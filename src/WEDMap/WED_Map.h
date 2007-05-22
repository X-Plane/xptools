#ifndef WED_MAP_H
#define WED_MAP_H

#include "CompGeomDefs2.h"
#include "GUI_Pane.h"
#include "WED_MapZoomerNew.h"
#include "GUI_Listener.h"

class	WED_MapLayer;
class	WED_MapToolNew;
class	IResolver;
class	IGISEntity;
class	ISelection;

class	WED_Map : public GUI_Pane, public WED_MapZoomerNew, public GUI_Listener {
public:
		
						 WED_Map(IResolver * in_resolver);
	virtual				~WED_Map();

			void		SetTool(WED_MapToolNew * tool);
			void		AddLayer(WED_MapLayer * layer);

	virtual void		SetBounds(int x1, int y1, int x2, int y2);
	virtual void		SetBounds(int inBounds[4]);

	virtual	void		Draw(GUI_GraphState * state);
	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	int			MouseMove(int x, int y);
	virtual	int			ScrollWheel(int x, int y, int dist, int axis);
	
	virtual	int			KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:

			void		DrawVisFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g);
			void		DrawStrFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g);

		IGISEntity *	GetGISBase();
		ISelection *	GetSel();
	

	vector<WED_MapLayer *>			mLayers;
	WED_MapToolNew *				mTool;
	IResolver *						mResolver;

	int				mIsToolClick;
	int				mIsMapDrag;
	int				mX;
	int				mY;

};


#endif

