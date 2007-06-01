#ifndef WED_DOCUMENTWINDOW_H
#define WED_DOCUMENTWINDOW_H

#include "GUI_Window.h"
#include "GUI_Listener.h"

//class WED_ObjectLayers;
//class WED_LayerGroup;
//class WED_LayerTable;
//class WED_LayerTableGeometry;


class	WED_MapPane;
class	WED_Document;
class	WED_PropertyTable;
class	WED_PropertyTableHeader;

class	WED_DocumentWindow : public GUI_Window, public GUI_Listener {
public:

				 WED_DocumentWindow(
				 		const char * 	inTitle, 
				 		GUI_Commander * inCommander,
				 		WED_Document *	inDocument);
	virtual		~WED_DocumentWindow();
	
	virtual	int	KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags);
	virtual	int	HandleCommand(int command);
	virtual	int	CanHandleCommand(int command, string& ioName, int& ioCheck);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

	virtual	bool	Closed(void);

private:

	WED_Document *				mDocument;
	WED_MapPane *				mMapPane;

//	WED_PropertyTable *			mTestTable;	
//	WED_PropertyTableHeader *	mTestTableHeader;

//	WED_ObjectLayers *		mObjects;
//	WED_LayerGroup *		mObjectGroup;	
//	WED_LayerTable *		mLayerTable;
//	WED_LayerTableGeometry*	mLayerTableGeometry;
};

#endif
