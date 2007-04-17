#include "WED_Document.h"
#include "WED_Progress.h"
#include "XESIO.h"
#include "AptIO.h"
#include "MapAlgs.h"
#include "WED_Messages.h"
#include "GUI_Menus.h"
#include "WED_UndoMgr.h"
#include "WED_DocumentWindow.h"
#include "WED_MapPane.h"
#include "WED_PropertyPane.h"
#include "WED_Thing.h"

#include "GUI_Splitter.h"


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
					
	static const char * titles[] = { "name", "locked", "hidden", "longitude", "latitude", "Type", 0 };
	static int widths[] = { 100, 50, 50, 150, 150, 50 };
	WED_Thing * root = SAFE_CAST(WED_Thing,mDocument->GetArchive()->Fetch(1));
	WED_Select * s = SAFE_CAST(WED_Select,root->GetNamedChild("selection"));
	DebugAssert(root);
	DebugAssert(s);

	int		splitter_b[4];	
	GUI_Splitter * main_splitter = new GUI_Splitter(gui_Split_Horizontal);
	main_splitter->SetParent(this);
	main_splitter->Show();
	GUI_Pane::GetBounds(splitter_b);
	main_splitter->SetBounds(splitter_b);
	main_splitter->SetSticky(1,1,1,1);
		
	double	lb[4];
	mDocument->GetBounds(lb);
	WED_MapPane * map_pane = new WED_MapPane(lb, inDocument,inDocument->GetArchive());
	map_pane->SetParent(main_splitter);
	map_pane->Show();
	map_pane->SetSticky(1,1,1,1);
	
	WED_PropertyPane * prop_pane = new WED_PropertyPane(this, root, s, titles, widths,inDocument->GetArchive());	
	prop_pane->SetParent(main_splitter);
	prop_pane->Show();
	prop_pane->SetSticky(0,1,1,1);

	int map_b[4];

	main_splitter->AlignContentsAt((inBounds[2]-inBounds[0]>inBounds[3]-inBounds[1]) ? (inBounds[3]-inBounds[1]) : ((inBounds[2]-inBounds[0])/2));
	
	map_pane->ZoomShowAll();
}

WED_DocumentWindow::~WED_DocumentWindow()
{
}

int	WED_DocumentWindow::KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	return 0;
}

int	WED_DocumentWindow::HandleCommand(int command)
{
	WED_UndoMgr * um = mDocument->GetUndoMgr();
	switch(command) {
	case gui_Undo:	if (um->HasUndo()) { um->Undo(); return 1; }	break;
	case gui_Redo:	if (um->HasRedo()) { um->Redo(); return 1; }	break;
	case gui_Close:	return 1;
	}
	return 0;
}

int	WED_DocumentWindow::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	WED_UndoMgr * um = mDocument->GetUndoMgr();
	switch(command) {
	case gui_Undo:		if (um->HasUndo())	{ ioName = um->GetUndoName();	return 1; }
						else				{								return 0; }
	case gui_Redo:		if (um->HasRedo())	{ ioName = um->GetRedoName();	return 1; }
						else				{								return 0; }
	case gui_Close:															return 1;
	default:																return 0;
	}
}

void	WED_DocumentWindow::ReceiveMessage(
				GUI_Broadcaster *		inSrc,
				int						inMsg,
				int						inParam)
{
	if(inMsg == msg_DocumentDestroyed)
		delete this;
}

bool	WED_DocumentWindow::Closed(void)
{
	mDocument->AsyncDestroy();
	return false;
}

