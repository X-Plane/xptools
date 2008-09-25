/*
 * Copyright (c) 2004, Laminar Research.
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
#ifndef _XWIN32DND_H_
#define _XWIN32DND_H_

#include <windows.h>
#include <ole2.h>
#include <shlobj.h>

class	XWinFileReceiver {
public:

	virtual	void	ReceiveFilesFromDrag(
						const vector<string>& inFiles)=0;

};


class CDropTarget : public IDropTarget {
private:
   ULONG m_cRefCount;
   BOOL m_fAcceptFmt;
   IDropTargetHelper *m_pDropTargetHelper;

   XWinFileReceiver * m_ReceiverObj;
   HWND 			  m_Window;

   //Utility function to read type of drag from key state
   BOOL QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect);
   void DisplayFileNames(HWND, HGLOBAL);

public:
   CDropTarget(void);
   ~CDropTarget(void);

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID, LPVOID*);
   STDMETHOD_(ULONG, AddRef)(void);
   STDMETHOD_(ULONG, Release)(void);

   // IDropTarget methods
   STDMETHOD(DragEnter)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
   STDMETHOD(DragOver)(DWORD, POINTL, LPDWORD);
   STDMETHOD(DragLeave)(void);
   STDMETHOD(Drop)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);

   void	SetReceiver(XWinFileReceiver * inReceiver, HWND inWindow);
};


#endif