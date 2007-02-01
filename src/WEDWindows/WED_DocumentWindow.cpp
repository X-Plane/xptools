#include "WED_Document.h"
#include "WED_Progress.h"
#include "XESIO.h"
#include "AptIO.h"
#include "MapAlgs.h"
#include "WED_Messages.h"
#include "WED_DocumentWindow.h"
#include "WED_Thing.h"
#include "WED_PropertyTable.h"

#include "GUI_TextTable.h"
#include "GUI_ScrollerPane.h"
#include "GUI_Splitter.h"
#include "GUI_Table.h"

#include "WED_LayerGroup.h"
//#include "WED_ObjectLayers.h"
#include "WED_LayerTable.h"

#include "WED_MapPane.h"


WED_DocumentWindow::WED_DocumentWindow(
	 		const char * 	inTitle, 
	 		int 			inBounds[4],
	 		GUI_Commander * inCommander,
	 		WED_Document *	inDocument) :
	GUI_Window(inTitle, inBounds, inCommander),
	mDocument(inDocument)
{
	
	GUI_Window::SetDescriptor(mDocument->GetFilePath());

	mDocument->AddListener(this);
	
//	mObjects = new WED_ObjectLayers(mDocument->GetObjectRoot());
//	mObjectGroup = new WED_LayerGroup(
//							wed_Layer_Hide | wed_Layer_Rename | wed_Layer_Reorder,
//							wed_Flag_Visible | wed_Flag_Children,
//							"Objects",
//							mObjects);
				
	static const char * titles[] = { "name", "locked", "hidden", 0 };
	static int widths[] = { 100, 50, 50 };
	WED_Thing * root = SAFE_CAST(WED_Thing,mDocument->GetArchive()->Fetch(0));
	mTestTable = new WED_PropertyTable(root,titles, widths);

//	mLayerTable = new WED_LayerTable;
//	mLayerTable->SetLayers(mObjectGroup);
//	mLayerTableGeometry = new WED_LayerTableGeometry;
//	mLayerTableGeometry->SetLayers(mObjectGroup);
	
	int		splitter_b[4];	
	GUI_Splitter * splitter = new GUI_Splitter(gui_Split_Horizontal);;
	splitter->SetParent(this);
	splitter->Show();
	GUI_Pane::GetBounds(splitter_b);
	splitter->SetBounds(splitter_b);
	splitter->SetSticky(1,1,1,1);
	
	GUI_ScrollerPane * map_scroller = new GUI_ScrollerPane(1,1);
	map_scroller->SetParent(splitter);
	map_scroller->Show();
	map_scroller->SetBounds(splitter_b);
	map_scroller->SetSticky(1,1,1,1);
	
	GUI_ScrollerPane * layer_scroller = new GUI_ScrollerPane(1,1);
	layer_scroller->SetParent(splitter);
	layer_scroller->Show();
	layer_scroller->SetBounds(splitter_b);
	layer_scroller->SetSticky(0,1,1,1);
	
	WED_MapPane * map = new WED_MapPane();
	map->SetParent(map_scroller);
	map->Show();
	map_scroller->PositionInContentArea(map);
	map_scroller->SetContent(map);
	
	GUI_TextTable * text_table = new GUI_TextTable;
	text_table->SetProvider(mTestTable);
	
	GUI_Table *	layer_table = new GUI_Table;
	layer_table->SetGeometry(mTestTable->GetGeometry());
	layer_table->SetContent(text_table);
	layer_table->SetParent(layer_scroller);
	layer_table->Show();
	layer_scroller->PositionInContentArea(layer_table);
	layer_scroller->SetContent(layer_table);
	
	double	lb[4];
	mDocument->GetBounds(lb);
	map->SetMapVisibleBounds(lb[0], lb[1], lb[2], lb[3]);
	map->SetMapLogicalBounds(lb[0], lb[1], lb[2], lb[3]);
	
	splitter->AlignContents();
	map->ZoomShowAll();
}

WED_DocumentWindow::~WED_DocumentWindow()
{
//	delete mObjects;
//	delete mObjectGroup;	
//	delete mLayerTable;
//	delete mLayerTableGeometry;
	delete mTestTable;
}

int	WED_DocumentWindow::KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	return 0;
}

int	WED_DocumentWindow::HandleCommand(int command)
{
	return 0;
}

int	WED_DocumentWindow::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	return 0;
}

void	WED_DocumentWindow::ReceiveMessage(
				GUI_Broadcaster *		inSrc,
				int						inMsg,
				int						inParam)
{
}

bool	WED_DocumentWindow::Closed(void)
{
	delete mDocument;
	return false;
}

