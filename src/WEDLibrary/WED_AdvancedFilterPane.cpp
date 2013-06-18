#include "WED_AdvancedFilterPane.h"
#include "GUI_Menus.h"

WED_AdvancedFilterPane::WED_AdvancedFilterPane(GUI_Commander * commander) : GUI_Commander(commander), GUI_Broadcaster()
{
	int temp[4] = {0,0,0,0};
	mResetAllBtn = new GUI_Button("reset_options.png",btn_Push,temp,temp,temp,temp);
	mTFWith = new GUI_TextField(1,this);
	mTFWithout = new GUI_TextField(1,this);
}


WED_AdvancedFilterPane::~WED_AdvancedFilterPane(void)
{

}


void WED_AdvancedFilterPane::ToggleAllOptions()
{
	/*For all elements
	* Reset to their default states
	*	Checkboxes->uncheck
	*	Textfields->clear strings
	* Immediantly send the search
	*/

	mTFWith->HandleCommand(gui_Clear);
	mTFWithout->HandleCommand(gui_Clear);

	SendSearch();
}


void WED_AdvancedFilterPane::SendSearch()
{
	BroadcastMessage(GUI_FILTER_MENU_CHANGED,0);
}