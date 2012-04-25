/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

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
	msg_SystemFolderUpdated,

	msg_LibraryChanged

#if WITHNWLINK
	,msg_NetworkStatusInfo
#endif

	,WED_LAST_MSG,

	WED_PRIVATE_MSG_BASE = WED_LAST_MSG + 1000
};

#endif /* WED_MESSAGES_H */

