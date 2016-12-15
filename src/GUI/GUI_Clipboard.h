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

#ifndef GUI_CLIPBOARD_H
#define GUI_CLIPBOARD_H

#include <vector>
#include "GUI_Defs.h"
using std::vector;

//---------------------------------------------------------------------------------------------------------
// TYPE MANAGEMENT
//---------------------------------------------------------------------------------------------------------
//
// Your application defines private clip types by registering them by string.  Text and any other already
// known non-private clip types don't need to be registered - functions return their enum type.


void			GUI_InitClipboard(void);
GUI_ClipType	GUI_RegisterPrivateClipType(const char * clip_type);
GUI_ClipType	GUI_GetTextClipType(void);

//---------------------------------------------------------------------------------------------------------
// DATA MANAGEMENT
//---------------------------------------------------------------------------------------------------------
//
// These routines do low-level clipboard I/O.  Warning: getting the size and data of the clipboard can be
// expensive because the providing app may be doing lazy conversions.  Getting the set of types or checking
// for types should be quick.

bool			GUI_Clipboard_HasClipType(GUI_ClipType inType);
void			GUI_Clipboard_GetTypes(vector<GUI_ClipType>& outTypes);
int				GUI_Clipboard_GetSize(GUI_ClipType inType);
bool			GUI_Clipboard_GetData(GUI_ClipType inType, int size, void * ptr);

bool			GUI_Clipboard_SetData(int type_count, GUI_ClipType inTypes[], int sizes[], const void * ptrs[]);

//---------------------------------------------------------------------------------------------------------
// CONVENIENCE ROUTINES
//---------------------------------------------------------------------------------------------------------
//
// These routines provide an easy way to move plain-old-text around.

bool			GUI_GetTextFromClipboard(string& outText);
bool			GUI_SetTextToClipboard(const string& inText);

//---------------------------------------------------------------------------------------------------------
// DRAG & DROP - CROSS-PLATFORM
//---------------------------------------------------------------------------------------------------------

// GUI_DragData is a generic interface to info being dropped on a pane.  The pane receives one of these
// and can then ask about what is being dropped and decide if it will accept it.

class	GUI_DragData {
public:

	virtual	int		CountItems(void)=0;
	virtual	bool	NthItemHasClipType(int n, GUI_ClipType ct)=0;
	virtual	int		GetNthItemSize(int n, GUI_ClipType ct)=0;
	virtual	bool	GetNthItemData(int n, GUI_ClipType ct, int size, void * ptr)=0;

};

//---------------------------------------------------------------------------------------------------------
// DRAG & DROP - WINDOWS
//---------------------------------------------------------------------------------------------------------

#if IBM

// Convenience routines to convert our drop-actions to native and back.

inline DWORD	OP_GUI2Win(GUI_DragOperation fx)
{
	return (fx & gui_Drag_Move ? DROPEFFECT_MOVE : DROPEFFECT_NONE) +
		   (fx & gui_Drag_Copy ? DROPEFFECT_COPY : DROPEFFECT_NONE);
}

inline GUI_DragOperation	OP_Win2GUI(DWORD fx)
{
	return (fx & DROPEFFECT_MOVE ?  gui_Drag_Move: gui_Drag_None) +
		   (fx & DROPEFFECT_COPY ?  gui_Drag_Copy: gui_Drag_None);
}

// GUI_OLE_Adapter implements GUI's generic drop-data interface as an
// adapter from a COM IDataObject.  The idea is that we can wrap this
// around the COM object and feed it to a pane - it essentially teaches
// the pane how to speak COM.  (It is not a COM object in itself, and
// is light-weight, so it can be use don the stack.)

class	GUI_OLE_Adapter : public GUI_DragData {
public:
			 GUI_OLE_Adapter(IDataObject * data_obj);
			~GUI_OLE_Adapter();

	virtual	int		CountItems(void);
	virtual	bool	NthItemHasClipType(int n, GUI_ClipType ct);
	virtual	int		GetNthItemSize(int n, GUI_ClipType ct);
	virtual	bool	GetNthItemData(int n, GUI_ClipType ct, int size, void * ptr);
private:
	IDataObject *	mObject;
};

// GUI_SimpleDataObject is a COM object that implements IDataObject, the interface
// for stuff you can drag around on Windows.  It does this by taking an explicit pile
// of data, or a fetch-func that can be used to get the data later on demand.  (To use
// the fetch func, pass NULL for the specific ptr).  Typically we would create one
// of these with "new", use it to DoDragDrop, then Release() it.
//
// Note that we cannot attach multiple drag objects to this object, even though the
// generic query mechanism reveals multiple items.  There is some funky stuff going on
// here:
//
// 1. Windows does not support multiple drag items on the OLE level - rather the specific
//	  data must have an internal grouping that is teased apart by the application.
// 2. The standard file-shell drag & drop (CF_HDROP) does have composite structure internally
//	  with APIs to parse it and find the files.  Since we'd like to support "a single file"
//    as a native type, the fetch API allows us to see each file by index.  This matches the
//    Mac's approach to multiple-file dropping.
// 3. Mac can have multiple drag items.
//
// So we make a receive API with multiple items (allowing the chance that there MIGHT be
// multiple items, especially if we understand OS-file encoding) but we make a SEND API that
// supports only one item, forcing our native types to fake their own groups.

class GUI_SimpleDataObject : public IDataObject {
public:
	GUI_SimpleDataObject(
							int						type_count,
							GUI_ClipType			inTypes[],
							int						sizes[],
							const void *			ptrs[],
							GUI_GetData_f			fetch_func,
							void *					ref);

	STDMETHOD(QueryInterface)	(REFIID riid, void **ppv);
	STDMETHOD_(ULONG,AddRef)	(void);
	STDMETHOD_(ULONG,Release)	(void);

	STDMETHOD(GetData)					(FORMATETC * pformatetcIn, STGMEDIUM * pmedium);
	STDMETHOD(GetDataHere)				(FORMATETC * pformatetcIn, STGMEDIUM * pmedium);
	STDMETHOD(QueryGetData)				(FORMATETC * pformatetcIn);
	STDMETHOD(GetCanonicalFormatEtc)	(FORMATETC *pformatectIn, FORMATETC  *pformatetcOut);
	STDMETHOD(SetData)					(FORMATETC *pformatetc, STGMEDIUM  *pmedium,BOOL fRelease);
	STDMETHOD(EnumFormatEtc)			(DWORD dwDirection, IEnumFORMATETC  * *ppenumFormatEtc);
	STDMETHOD(DAdvise)					(FORMATETC  *pformatetc, DWORD advf, IAdviseSink  *pAdvSink, DWORD  *pdwConnection);
	STDMETHOD(DUnadvise)				(DWORD dwConnection);
	STDMETHOD(EnumDAdvise)				(IEnumSTATDATA ** ppenumAdvise);

private:

	friend	class	GUI_SimpleEnumFORMATETC;

	GUI_GetData_f				mFetchFunc;
	void *						mFetchRef;

	map<GUI_ClipType, vector<char> >		mData;		// A map of buffers for our types.
	ULONG									mRefCount;
};





#endif /* IBM */

//---------------------------------------------------------------------------------------------------------
// DRAG & DROP - MAC
//---------------------------------------------------------------------------------------------------------

#if APL

// Convenience routines to convert our drop-actions to native and back. - These match NSDragging.h secretly

inline int	OP_GUI2Mac(GUI_DragOperation fx)
{
	return (fx & gui_Drag_Move ? 16 : 0) +
		   (fx & gui_Drag_Copy ? 1 : 0);
}

inline GUI_DragOperation	OP_Mac2GUI(int fx)
{
	return (fx & 16 ?  gui_Drag_Move: gui_Drag_None) +
		   (fx & 1 ?  gui_Drag_Copy: gui_Drag_None);
}

// GUI_DragMgr_Adapter is a light-weight adapter that makes a Mac Drag Mgr reference
// look like GUI's generic drag & drop data.  It allows us to pass DragRefs to a pane.
// IT is light weight - we can use it on the stack.

class	GUI_DragMgr_Adapter : public GUI_DragData {
public:
			 GUI_DragMgr_Adapter(void * ns_dragging_info);
			~GUI_DragMgr_Adapter();

	virtual	int		CountItems(void);
	virtual	bool	NthItemHasClipType(int n, GUI_ClipType ct);
	virtual	int		GetNthItemSize(int n, GUI_ClipType ct);
	virtual	bool	GetNthItemData(int n, GUI_ClipType ct, int size, void * ptr);
private:
	void *		mObject;
};

// This routine loads  data into a single drag item.  The returned void * is really
// an NSDraggingItem with one retain count.

void * GUI_LoadOneSimpleDrag(
							int						typeCount,
							GUI_ClipType			inTypes[],
							int						sizes[],
							const void *			ptrs[],
							const int				bounds[4]);

// A utility bridge for drag and drop - this returns a vector of the UTI strings of all
// registered drag types within the app.  GUI_Window uses this just-in-time to make sure
// we can receive the right kinds of drags.
void GUI_GetMacNativeDragTypeList(vector<string>& out_types);

#endif /* APL */

//---------------------------------------------------------------------------------------------------------
// DRAG & DROP - LIN
//---------------------------------------------------------------------------------------------------------

#if LIN
#include <QtGui/QClipboard>

inline  Qt::DropActions OP_GUI2LIN(GUI_DragOperation fx)
{
	return (fx & gui_Drag_Move ? Qt::MoveAction : Qt::IgnoreAction) |
		   (fx & gui_Drag_Copy ? Qt::CopyAction : Qt::IgnoreAction);
}

inline GUI_DragOperation	OP_LIN2GUI(Qt::DropActions fx)
{
	return (fx & Qt::MoveAction ? gui_Drag_Move : gui_Drag_None) +
		   (fx & Qt::CopyAction ? gui_Drag_Copy : gui_Drag_None);
}


// mroe:We need a adapter to present GUI's generic drag & drop data in Linux too.
class	GUI_DragData_Adapter : public GUI_DragData {
public:
			 GUI_DragData_Adapter(void * data_obj);
			~GUI_DragData_Adapter();

	virtual	int		CountItems(void);
	virtual	bool	NthItemHasClipType(int n, GUI_ClipType ct);
	virtual	int		GetNthItemSize(int n, GUI_ClipType ct);
	virtual	bool	GetNthItemData(int n, GUI_ClipType ct, int size, void * ptr);
private:
	void *		mObject;
};

#endif /* LIN */

#endif /* GUI_CLIPBOAD_H */
