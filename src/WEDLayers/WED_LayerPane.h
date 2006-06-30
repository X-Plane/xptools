CURRENTLY NOT USED
#ifndef WED_LAYERPANE_H
#define WED_LAYERPANE_H

#include "GUI_Pane.h"
#include "GUI_Listener.h"
#include "GUI_Broadcaster.h"
#include "GUI_Table.h"

class	WED_AbstractLayers;

class	WED_LayerPane : public GUI_Pane, public GUI_Listener, public GUI_Broadcaster {
public:

						 WED_LayerPane();
	virtual				~WED_LayerPane();
	
			void		SetLayers(WED_AbstractLayers * inLayers);
			
			

	
	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);
							
private:

			void		RecalcSize(void);

	WED_AbstractLayers *		mLayers;

};

#endif
