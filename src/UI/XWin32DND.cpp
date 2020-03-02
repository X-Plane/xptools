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
#include "XWin32DND.h"

#if IBM
#include "GUI_Unicode.h"
#endif

/**************************************************************************

   CDropTarget::CDropTarget()

**************************************************************************/

CDropTarget::CDropTarget(void)
{
m_cRefCount = 1;
m_fAcceptFmt = FALSE;
m_pDropTargetHelper = NULL;
m_ReceiverObj = NULL;
m_Window = NULL;

CoCreateInstance( CLSID_DragDropHelper,
                  NULL,
                  CLSCTX_INPROC_SERVER,
                  IID_IDropTargetHelper,
                  (LPVOID*)&m_pDropTargetHelper);
}

/**************************************************************************

   CDropTarget::~CDropTarget()

**************************************************************************/

CDropTarget::~CDropTarget(void)
{
if(m_pDropTargetHelper)
   {
   m_pDropTargetHelper->Release();
   m_pDropTargetHelper = NULL;
   }
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

/**************************************************************************

   CDropTarget::QueryInterface()

**************************************************************************/

STDMETHODIMP CDropTarget::QueryInterface(REFIID riid, LPVOID *ppvOut)
{
*ppvOut = NULL;

//IUnknown
if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppvOut = this;
   }

//IDropTarget
else if(IsEqualIID(riid, IID_IDropTarget))
   {
   *ppvOut = (IDropTarget*)this;
   }

if(*ppvOut)
   {
   (*(LPUNKNOWN*)ppvOut)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}

/**************************************************************************

   CDropTarget::AddRef()

**************************************************************************/

STDMETHODIMP_(ULONG) CDropTarget::AddRef(void)
{
return ++m_cRefCount;
}

/**************************************************************************

   CDropTarget::Release()

**************************************************************************/

STDMETHODIMP_(ULONG) CDropTarget::Release(void)
{
if(--m_cRefCount == 0)
   {
   delete this;
   return 0;
   }

return m_cRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IDropTarget Implementation
//

/**************************************************************************

   CDropTarget::DragEnter()

**************************************************************************/

STDMETHODIMP CDropTarget::DragEnter(   LPDATAOBJECT pDataObj,
                                       DWORD grfKeyState,
                                       POINTL pt,
                                       LPDWORD pdwEffect)
{
if(m_pDropTargetHelper)
   m_pDropTargetHelper->DragEnter(m_Window, pDataObj, (LPPOINT)&pt, *pdwEffect);

FORMATETC fe;

fe.cfFormat = CF_HDROP;
fe.ptd      = NULL;
fe.dwAspect = DVASPECT_CONTENT;
fe.lindex   = -1;
fe.tymed    = TYMED_HGLOBAL;

// Does the drag source provide the clipboard format we are looking for?
m_fAcceptFmt = (S_OK == pDataObj->QueryGetData(&fe)) ? TRUE : FALSE;

QueryDrop(grfKeyState, pdwEffect);

return S_OK;
}

/**************************************************************************

   CDropTarget::DragOver()

**************************************************************************/

STDMETHODIMP CDropTarget::DragOver( DWORD grfKeyState,
                                    POINTL pt,
                                    LPDWORD pdwEffect)
{
if(m_pDropTargetHelper)
   m_pDropTargetHelper->DragOver((LPPOINT)&pt, *pdwEffect);

QueryDrop(grfKeyState, pdwEffect);

return S_OK;
}

/**************************************************************************

   CDropTarget::DragLeave()

**************************************************************************/

STDMETHODIMP CDropTarget::DragLeave()
{
if(m_pDropTargetHelper)
   m_pDropTargetHelper->DragLeave();

m_fAcceptFmt = FALSE;
return S_OK;
}

/**************************************************************************

   CDropTarget::Drop()

**************************************************************************/

STDMETHODIMP CDropTarget::Drop(  LPDATAOBJECT pDataObj,
                                 DWORD grfKeyState,
                                 POINTL pt,
                                 LPDWORD pdwEffect)
{
if(m_pDropTargetHelper)
   m_pDropTargetHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);

FORMATETC   fe;
STGMEDIUM   sm;
HRESULT     hr = E_FAIL;

if(QueryDrop(grfKeyState, pdwEffect))
   {
   fe.cfFormat = CF_HDROP;
   fe.ptd = NULL;
   fe.dwAspect = DVASPECT_CONTENT;
   fe.lindex = -1;
   fe.tymed = TYMED_HGLOBAL;

   // User has dropped on us. Get the data from drag source
   hr = pDataObj->GetData(&fe, &sm);
   if(SUCCEEDED(hr))
      {
      // Display the data and release it.
      DisplayFileNames(m_Window, sm.hGlobal);

      ReleaseStgMedium(&sm);
      }
   }

*pdwEffect = DROPEFFECT_NONE;

return hr;
}

/* OleStdGetDropEffect
** -------------------
**
** Convert a keyboard state into a DROPEFFECT.
**
** returns the DROPEFFECT value derived from the key state.
**    the following is the standard interpretation:
**          no modifier -- Default Drop     (0 is returned)
**          CTRL        -- DROPEFFECT_COPY
**          SHIFT       -- DROPEFFECT_MOVE
**          CTRL-SHIFT  -- DROPEFFECT_LINK
**
**    Default Drop: this depends on the type of the target application.
**    this is re-interpretable by each target application. a typical
**    interpretation is if the drag is local to the same document
**    (which is source of the drag) then a MOVE operation is
**    performed. if the drag is not local, then a COPY operation is
**    performed.
*/
#define OleStdGetDropEffect(grfKeyState)    \
    ( (grfKeyState & MK_CONTROL) ?          \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ) :  \
        ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 ) )

/**************************************************************************

   CDropTarget::QueryDrop()

**************************************************************************/

BOOL CDropTarget::QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect)
{
DWORD dwOKEffects = *pdwEffect;

if(!m_fAcceptFmt)
   {
   *pdwEffect = DROPEFFECT_NONE;
   return FALSE;
   }

*pdwEffect = OleStdGetDropEffect(grfKeyState);
if(*pdwEffect == 0)
   {
   // No modifier keys used by user while dragging.
   if (DROPEFFECT_COPY & dwOKEffects)
      *pdwEffect = DROPEFFECT_COPY;
   else if (DROPEFFECT_MOVE & dwOKEffects)
      *pdwEffect = DROPEFFECT_MOVE;
   else if (DROPEFFECT_LINK & dwOKEffects)
      *pdwEffect = DROPEFFECT_LINK;
   else
      {
      *pdwEffect = DROPEFFECT_NONE;
      }
   }
else
   {
   // Check if the drag source application allows the drop effect desired by user.
   // The drag source specifies this in DoDragDrop
   if(!(*pdwEffect & dwOKEffects))
      *pdwEffect = DROPEFFECT_NONE;

   // We don't accept links
   if(*pdwEffect == DROPEFFECT_LINK)
      *pdwEffect = DROPEFFECT_NONE;
   }

return (DROPEFFECT_NONE == *pdwEffect) ? FALSE : TRUE;
}

/**************************************************************************

   CDropTarget::DisplayFileNames()

**************************************************************************/

bool RecursiveAddFiles(const WCHAR *sourcePath, vector<string> &outputList)
{
    const size_t maxListEntries = 50;
    bool bContinue = true;

    DWORD fileAttribs = ::GetFileAttributesW(sourcePath);
    if (fileAttribs & FILE_ATTRIBUTE_DIRECTORY)
    {
        size_t reqLen = wcslen(sourcePath) + 3;
        WCHAR *wildcard = (WCHAR *)malloc(sizeof(wchar_t) * reqLen);
        if (wildcard != nullptr)
        {
            wcscpy_s(wildcard, reqLen, sourcePath); wildcard[reqLen - 1] = 0;
            wcscat_s(wildcard, reqLen, L"\\*"); wildcard[reqLen - 1] = 0;

            WIN32_FIND_DATAW findData = { 0 };
            HANDLE allFiles = ::FindFirstFileW(wildcard, &findData);
            if (allFiles != INVALID_HANDLE_VALUE)
            {
                do
                {
                    // Build the full path without using any additional Windows libraries
                    size_t reqSubLen = wcslen(sourcePath) + wcslen(findData.cFileName) + 2;

                    WCHAR *nextPath = (WCHAR *)malloc(sizeof(wchar_t) * reqSubLen);
                    if (nextPath != nullptr)
                    {
                        wcscpy_s(nextPath, reqSubLen, sourcePath);
                        wcscat_s(nextPath, reqSubLen, L"\\");
                        wcscat_s(nextPath, reqSubLen, findData.cFileName);

                        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        {
                            // Ignore '.' and '..'
                            if (findData.cFileName[0] != '.')
                            {
                                RecursiveAddFiles(nextPath, outputList);
                            }
                        }
                        else
                        {
                            outputList.push_back(convert_utf16_to_str(nextPath));
                        }

                        free(nextPath);
                    }
                } while (FindNextFileW(allFiles, &findData) != 0 && outputList.size() <= maxListEntries);

                FindClose(allFiles);
            }

            free(wildcard);
        }
    }
    else
    {
        outputList.push_back(convert_utf16_to_str(sourcePath));
    }

    return outputList.size() < maxListEntries;
}

void CDropTarget::DisplayFileNames(HWND hwndOwner, HGLOBAL hgFiles)
{
	UINT	i, nFiles;

	nFiles = DragQueryFile((HDROP)hgFiles, 0xFFFFFFFF, NULL, 0);

	WCHAR	path[MAX_PATH+1];

	vector<string>	fileList;
	for(i = 0; i < nFiles; i++)
	{
        DragQueryFileW((HDROP)hgFiles, i, path, MAX_PATH);

        if (!RecursiveAddFiles(path, fileList))
        {
            break;
        }
	}

	if(m_ReceiverObj)
		m_ReceiverObj->ReceiveFilesFromDrag(fileList);
}

void	CDropTarget::SetReceiver(XWinFileReceiver * inReceiver, HWND inWindow)
{
	m_ReceiverObj = inReceiver;
	m_Window = inWindow;
}
