#ifndef WED_LAYERPANE_H
#define WED_LAYERPANE_H

#include "GUI_Pane.h"
#include "GUI_Listener.h"
#include "GUI_Broadcaster.h"

class	WED_AbstractLayers;

class	WED_LayerPane : public GUI_Pane, public GUI_Listener, public GUI_Broadcaster {
public:

						 WED_LayerPane(WED_AbstractLayers * layers);
	virtual				~WED_LayerPane();
	

	virtual	void		Draw(GUI_GraphState * state);
	
	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp(int x, int y, int button);
	
	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);
							
private:

			void		RecalcSize(void);

	WED_AbstractLayers *		mLayers;

};

#endif
