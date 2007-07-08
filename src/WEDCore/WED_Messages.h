#ifndef WED_MESSAGES_H
#define WED_MESSAGES_H

#include "GUI_Messages.h"

enum {


	msg_PackageDestroyed = GUI_APP_MESSAGES,
	msg_DocumentDestroyed,
	
//	msg_LayerStatusChanged,					// Sent when layer flags are toggled, renamed, whatever
//	msg_LayerCountChanged,					// Sent when number of layers changes

//	msg_SelectionChanged,
	
	msg_ArchiveChanged,
	
	msg_DocWillSave,
	msg_DocLoaded,
	
	msg_SystemFolderChanged,
	msg_SystemFolderUpdated	

};

#endif /* WED_MESSAGES_H */

