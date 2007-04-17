IS THIS USED?  NO!


#ifndef WED_AGGREGATELAYERS_H
#define WED_AGGREGATELAYERS_H

/*

	WED_AggregateLayers - THEORY OF OPERATION
	
	This class simply sequentially groups several sources of layers.
	For example, if you have layers that create the folders for objects
	and orthophotos, you could use this object to put one after the other.

*/

#include "WED_AbstractLayers.h"
#include "GUI_Listener.h"

class	WED_AggregateLayers : public WED_AbstractLayers, public GUI_Listener {
public:

								 WED_AggregateLayers();
	virtual						~WED_AggregateLayers();
	
			void				AppendLayers(WED_AbstractLayers * layers);

	virtual	int					CountLayers(void);
	virtual	int					GetIndent(int n);
	virtual	int					GetLayerCapabilities(int n);
	virtual	int					GetLayerAllowedTools(int n);
	virtual int					GetFlags(int n) const;
	virtual	string				GetName(int n) const;
	virtual float				GetOpacity(int n) const;	
	virtual	void				ToggleFlag(int n, int flag);
	virtual void				Rename(int n, const string& name);
	virtual	void				SetOpacity(int n, float opacity);

	virtual	void				DrawLayer(
									int						n,
									GUI_GraphState *		state,
									WED_MapZoomer *			zoomer,
									int						tool,
									int						selected,
									int						overlay);
	virtual	int					HandleMouseDown(
									int						n,
									WED_MapZoomer *			zoomer,
									int						tool,
									int						selected,
									int						x,
									int						y,
									int						button);
	virtual	void				HandleMouseDrag(
									int						n,
									WED_MapZoomer *			zoomer,
									int						tool,
									int						selected,
									int						x,
									int						y,
									int						button);
	virtual	void				HandleMouseUp(
									int						n,
									WED_MapZoomer *			zoomer,
									int						tool,
									int						selected,
									int						x,
									int						y,
									int						button);

	virtual	void				ReceiveMessage(
										GUI_Broadcaster *		inSrc,
										int						inMsg,
										int						inParam);


private:

			WED_AbstractLayers *	Translate(int& n) const;


	vector<WED_AbstractLayers *>	mLayers;

};

#endif