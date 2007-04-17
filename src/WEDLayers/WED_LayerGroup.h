THIS IS NT USED
#ifndef WED_LAYERGROUP_H
#define WED_LAYERGROUP_H

/*
	
	WED_LayerGroup - THEORY OF OPERATION
	
	This class takes a set of layers and puts them all within a folder,
	indenting them.  It opens and closes the folder as needed.
	
	The entire contents of the folder must be one layer source, but if you
	did want to pack the folder, you could use the aggregate group to
	pack items sequentially into the folder.


*/

#include "WED_AbstractLayers.h"
#include "GUI_Listener.h"

class	WED_LayerGroup : public WED_AbstractLayers, public GUI_Listener {
public:

								 WED_LayerGroup(
								 		int						flags,
								 		int						caps,
								 		const string&			name,
								 		WED_AbstractLayers *	children);
	virtual						~WED_LayerGroup();

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

	WED_AbstractLayers *		mChildren;
	int							mFlags;
	int							mCaps;
	string						mName;	

};

#endif /* WED_ABSTRACTLAYER_H */
