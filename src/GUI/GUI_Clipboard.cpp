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

#include "GUI_Clipboard.h"

#if APL
	#if defined(__MWERKS__)
		#include <Carbon.h>
	#else
		#include <Carbon/Carbon.h>
	#endif
#elif IBM
#include <Windows.h>
#endif

#if APL
	typedef	OSType	GUI_CIT;		// Clipboard Internal Type
#elif IBM
	typedef	CLIPFORMAT	GUI_CIT;		// Clipboard Internal Type
#else
    #warning linux fixme here
	typedef	int	GUI_CIT;
#endif

enum {
	gui_Clip_Text = 0,
	gui_First_Private
};

static vector<string>		sClipStrings;
static vector<GUI_CIT>		sCITs;

//---------------------------------------------------------------------------------------------------------
// TYPE MANAGEMENT
//---------------------------------------------------------------------------------------------------------

static int	GUI2CIT(GUI_ClipType in_t, GUI_CIT& out_t)
{
	if (in_t >= 0 && in_t < sCITs.size())
	{
		out_t = sCITs[in_t];
		return 1;
	}
	return 0;
}

static int	CIT2GUI(GUI_CIT in_t, GUI_ClipType& out_t)
{
	for (out_t = 0; out_t < sCITs.size(); ++out_t)
		if (sCITs[out_t] == in_t)	return 1;
	return 0;
}

void GUI_InitClipboard(void)
{
	#if APL
		sCITs.push_back(kScrapFlavorTypeText);
	#elif IBM
		sCITs.push_back(CF_TEXT);
	#else
		#warning implement clipboard init() for linux
	#endif
	sClipStrings.push_back("text");
}

GUI_ClipType	GUI_RegisterPrivateClipType(const char * clip_type)
{
	for (int n = 0; n < sClipStrings.size(); ++n)
	{
		if (strcmp(sClipStrings[n].c_str(), clip_type) == 0)
			return n;
	}
	
	sClipStrings.push_back(clip_type);
	#if APL
		static GUI_CIT private_counter = 'PRV0';
		sCITs.push_back(private_counter);
		++private_counter;
	#elif IBM
		sCITs.push_back(CF_PRIVATEFIRST + sCITs.size() - gui_First_Private);
	#else
		#warning implement clipboard type for linux
	#endif
	return sCITs.size()-1;
}

GUI_ClipType	GUI_GetTextClipType(void)
{
	return gui_Clip_Text;
}

//---------------------------------------------------------------------------------------------------------
// DATA MANAGEMENT
//---------------------------------------------------------------------------------------------------------

#pragma mark -

bool			GUI_Clipboard_HasClipType(GUI_ClipType inType)
{
	#if APL
		ScrapRef			scrap;
		ScrapFlavorFlags	flags;
		if (GetCurrentScrap(&scrap) != noErr) return false;
		if (GetScrapFlavorFlags(scrap, sCITs[inType], &flags) != noErr) return false;

		return (inType < gui_First_Private || (flags & kScrapFlavorMaskSenderOnly));
	
	#elif IBM
		return (IsClipboardFormatAvailable(sCITs[inType]));
	#else
		#warning implement clipboard typecheck for linux
	#endif
}

void			GUI_Clipboard_GetTypes(vector<GUI_ClipType>& outTypes)
{
	#if APL
			ScrapRef			scrap;
			UInt32				count;
		outTypes.clear();
		if (GetCurrentScrap(&scrap) != noErr) return;
		if (GetScrapFlavorCount(scrap, &count) != noErr) return;
		if (count == 0) return;
		vector<ScrapFlavorInfo>	info(count);
		if (GetScrapFlavorInfoList(scrap, &count, &*info.begin()) != noErr) return;
		if (info.size() != count) return;

		for (int n = 0; n < info.size(); ++n)
		{
			GUI_CIT raw_type = info[n].flavorType;
			GUI_ClipType ct;
			if (CIT2GUI(raw_type, ct))
			if (ct < gui_First_Private || (info[n].flavorFlags & kScrapFlavorMaskSenderOnly))
				outTypes.push_back(ct);
		}
		
	#elif IBM
		int total = CountClipboardFormats();		
		outTypes.clear();
		for (int n = 0; n < total; ++n)
		{
			GUI_CIT raw_type = EnumClipboardFormats(n);
			GUI_ClipType ct;
			if (CIT2GUI(raw_type, ct))
				outTypes.push_back(ct);
		}
	#else
		#warning implement clipboard getTypes for linux
	#endif
}

#if IBM
struct	StOpenClipboard {
	 StOpenClipboard() { is_open = OpenClipboard(NULL); }
	~StOpenClipboard() { if (is_open) CloseClipboard(); }
	bool operator()(void) const { return is_open; }
	bool is_open;
};

struct	StGlobalBlock {
	 StGlobalBlock(int mem) { handle = GlobalAlloc(GMEM_MOVEABLE, mem); }
	~StGlobalBlock()		{ if (handle) GlobalFree(handle); }	
	HGLOBAL	operator()(void) const { return handle; }
	void release(void) { handle = NULL; }
	HGLOBAL handle;
};

struct	StGlobalLock {
	 StGlobalLock(HGLOBAL h) { handle = h; ptr = GlobalLock(handle); }
	~StGlobalLock()			 { if (ptr) GlobalUnlock(handle); }
	void * operator()(void) const { return ptr; }	
	HGLOBAL handle;
	void *	ptr;
};
	


#endif

int				GUI_Clipboard_GetSize(GUI_ClipType inType)
{
	#if APL
			ScrapRef			scrap;
			Size				sz;
		if (GetCurrentScrap(&scrap) != noErr) return 0;
		if (GetScrapFlavorSize(scrap, sCITs[inType], &sz) != noErr) return 0;
		return sz;
				
	#elif IBM

			HGLOBAL   	hglb;
		if (!IsClipboardFormatAvailable(sCITs[inType]))		return 0;
		
		StOpenClipboard	open_it;	
		if (!open_it())										return 0;
		
		hglb = GetClipboardData(sCITs[inType]);
		if (hglb == NULL)									return 0;

		int sz = GlobalSize(hglb);
		return sz;
	
	#else
		#warning implement clipboard GetSize for linux
	#endif
}

bool			GUI_Clipboard_GetData(GUI_ClipType inType, int size, void * ptr)
{
	#if APL
			ScrapRef			scrap;
			Size				sz;
		if (GetCurrentScrap(&scrap) != noErr) return false;
		if (GetScrapFlavorSize(scrap, sCITs[inType], &sz) != noErr) return false;
		if (sz != size) return false;

		if (GetScrapFlavorData(scrap, sCITs[inType], &sz, ptr) != noErr) return false;
		if (sz != size) return false;
		return true;
	#elif IBM

			HGLOBAL   	hglb;
		if (!IsClipboardFormatAvailable(sCITs[inType]))		return false;
		
		StOpenClipboard	open_it;	
		if (!open_it())										return false;
		
		hglb = GetClipboardData(sCITs[inType]);
		if (hglb == NULL)									return false;

		if (GlobalSize(hglb) != size)						return false;

		StGlobalLock	lock_it(hglb);
		
		if (!lock_it())										return false;

		memcpy(ptr, lock_it(), size);
		return true;
	
	#else
		#warning implement clipboard getData() for linux
	#endif
}
bool			GUI_Clipboard_SetData(int type_count, GUI_ClipType inTypes[], int sizes[], const void * ptrs[])
{
	#if APL
			ScrapRef	scrap;
		if (ClearCurrentScrap() != noErr) return false;
		if (GetCurrentScrap(&scrap) != noErr) return false;

		for (int n = 0; n < type_count; ++n)
		if (PutScrapFlavor(
				scrap, 
				sCITs[inTypes[n]], 
				(n >= gui_First_Private) ? kScrapFlavorMaskSenderOnly : 
				kScrapFlavorMaskNone, sizes[n], 
				ptrs[n]) != noErr) 
			return false;
		return true;
	#elif IBM
		StOpenClipboard	open_it;
		if (!open_it())										return false;
		if (!EmptyClipboard())								return false;
		
		for (int n = 0; n < type_count; ++n)
		{
			StGlobalBlock		block(sizes[n]);
			if (!block())									return false;
			
			{
				StGlobalLock lock_it(block());
				if (!lock_it())								return false;
				memcpy(lock_it(), ptrs[n], sizes[n]);
			}
			
			if (SetClipboardData(sCITs[inTypes[n]], block()) == NULL)	return false;
			
			// Ben says: ownership rules of the block are as follows:
			// - function fails...we still own the handle.
			// - function succeeds...we own the handle IF it is a private scrap, but NOT if it is public.
			// So for successful public scrap set, release the block so we don't double-deallocate
			if (inTypes[n] < gui_First_Private)	
				block.release();
		}
		return true;
	
	#else
		#warning implement clipboard setData() for linux
	#endif
}

//---------------------------------------------------------------------------------------------------------
// CONVENIENCE ROUTINES
//---------------------------------------------------------------------------------------------------------

bool			GUI_GetTextFromClipboard(string& outText)
{
	GUI_ClipType text = GUI_GetTextClipType();
	if (!GUI_Clipboard_HasClipType(text)) return false;
	int sz = GUI_Clipboard_GetSize(text);
	if (sz <= 0) return false;
	vector<char> buf(sz);
	if (!GUI_Clipboard_GetData(text, sz, &*buf.begin())) return false;
	#if APL
		outText = string(buf.begin(),buf.begin()+sz);
	#elif IBM
		outText = string(buf.begin(),buf.begin()+sz-1);
	#else
		#warning implement clipboard getText() for linux
	#endif
	return true;
}

bool			GUI_SetTextToClipboard(const string& inText)
{
	GUI_ClipType text = GUI_GetTextClipType();
	#if APL
		int sz = inText.size();
	#elif IBM
		int sz = inText.size()+1;
	#else
		#warning implement clipboard setText() for linux
        int sz = inText.size();
	#endif
	const void * ptr = inText.c_str();
	return GUI_Clipboard_SetData(1, &text, &sz, &ptr);
}


//---------------------------------------------------------------------------------------------------------
// DRAG & DROP -- WINDOWS
//---------------------------------------------------------------------------------------------------------

#if IBM

// Enumerating the viable drag & drop formats on Windows is done via a separate COM object.  Because it is
// an iterator and has "state" and can be  cloned, we must actually use a real object.  Annyoing!

class	GUI_SimpleEnumFORMATETC : public IEnumFORMATETC {
public:
	GUI_SimpleEnumFORMATETC(GUI_SimpleDataObject * parent);
	GUI_SimpleEnumFORMATETC(const GUI_SimpleEnumFORMATETC& me);

	STDMETHOD(QueryInterface)	(REFIID riid, void **ppv);
	STDMETHOD_(ULONG,AddRef)	();
	STDMETHOD_(ULONG,Release)	();
	
	STDMETHOD(Next)				(ULONG count, FORMATETC * formats, ULONG * out_count);
	STDMETHOD(Skip)				(ULONG count);
	STDMETHOD(Reset)			(void);
	STDMETHOD(Clone)			(IEnumFORMATETC ** pp_obj);

private:

	ULONG					mRefCount;
	ULONG					mIndex;		// Position in our iteration
	vector<GUI_ClipType>	mTypes;		// Note - we pre-copy our types to a vector.  Easier and puts them in a random-accessible container
};


GUI_OLE_Adapter::GUI_OLE_Adapter(IDataObject * data_obj)
{
	mObject = data_obj;
	mObject->AddRef();
}

GUI_OLE_Adapter::~GUI_OLE_Adapter()
{
	mObject->Release();
}

int		GUI_OLE_Adapter::CountItems(void)
{
	return 1;
}

bool	GUI_OLE_Adapter::NthItemHasClipType(int n, GUI_ClipType ct)
{
	FORMATETC format;
	if (n != 0) return false;	
	if (!GUI2CIT(ct,format.cfFormat)) return false;
	format.ptd      = NULL;
	format.dwAspect = DVASPECT_CONTENT;  
	format.lindex   = -1;
	format.tymed    = TYMED_HGLOBAL;     
	return mObject->QueryGetData(&format) == S_OK;
}

int		GUI_OLE_Adapter::GetNthItemSize(int n, GUI_ClipType ct)
{	
	FORMATETC	format;
	STGMEDIUM	medium;
	if (n != 0) return false;	
	if (!GUI2CIT(ct,format.cfFormat)) return false;
	format.ptd      = NULL;
	format.dwAspect = DVASPECT_CONTENT;  
	format.lindex   = -1;
	format.tymed    = TYMED_HGLOBAL;     
	if (mObject->GetData(&format, &medium) != S_OK) return 0;
	
	int block_size = GlobalSize(medium.hGlobal);
	
	ReleaseStgMedium(&medium);
	return block_size;	
}

bool	GUI_OLE_Adapter::GetNthItemData(int n, GUI_ClipType ct, int size, void * ptr)
{
	FORMATETC	format;
	STGMEDIUM	medium;
	if (n != 0) return false;	
	if (!GUI2CIT(ct,format.cfFormat)) return false;
	format.ptd      = NULL;
	format.dwAspect = DVASPECT_CONTENT;  
	format.lindex   = -1;
	format.tymed    = TYMED_HGLOBAL;     
	if (mObject->GetData(&format, &medium) != S_OK) return 0;
	
	if (GlobalSize(medium.hGlobal) != size)
	{
		ReleaseStgMedium(&medium);
		return false;
	}
	
	{
		StGlobalLock lock_it(medium.hGlobal);
		if (!lock_it())
		{
			ReleaseStgMedium(&medium);
			return false;
		}
		
		memcpy(ptr, lock_it(), size);
	}
	ReleaseStgMedium(&medium);
	return true;	
}

GUI_SimpleDataObject::GUI_SimpleDataObject(
					int						type_count, 
					GUI_ClipType			inTypes[], 
					int						sizes[], 
					const void *			ptrs[],
					GUI_GetData_f			get_data_func,
					void *					ref) :
	mRefCount(1),
	mFetchFunc(get_data_func),
	mFetchRef(ref)
{
	for (int n = 0; n < type_count; ++n)
	{
		if (ptrs[n] == NULL)
			mData[inTypes[n]] = vector<char>();			
		else
			mData[inTypes[n]] = vector<char>((const char*)ptrs[n],(const char *)ptrs[n]+sizes[n]);			
	}
}



STDMETHODIMP		 GUI_SimpleDataObject::QueryInterface(REFIID riid, void **ppvOut)
{
	*ppvOut = NULL;

		 if(IsEqualIID(riid, IID_IUnknown))		*ppvOut = this;
	else if(IsEqualIID(riid, IID_IDataObject))	*ppvOut = (IDataObject*)this;

	if(*ppvOut)
	{
		(*(LPUNKNOWN*)ppvOut)->AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) GUI_SimpleDataObject::AddRef()
{
	return ++mRefCount;
}

STDMETHODIMP_(ULONG) GUI_SimpleDataObject::Release()
{
	if (--mRefCount == 0)
	{
		delete this;
		return 0;
	}
	return mRefCount;
}

// GetData copies the data from the format to the medium.  We allocate storage and our caller
// deallocates it.  So the only thing that can go wrong is they want something we don't do, 
// e.g. a weird data format or weird storage format.
STDMETHODIMP		GUI_SimpleDataObject::GetData				(FORMATETC * format, STGMEDIUM * medium)
{
	GUI_ClipType	desired_type;
	if (!CIT2GUI(format->cfFormat, desired_type))	return DV_E_FORMATETC;	
	if (mData.count(desired_type) == 0)				return DV_E_FORMATETC;	
	if ((format->tymed & TYMED_HGLOBAL) == 0)		return DV_E_TYMED;

	if (mData[desired_type].empty())
	{
		if (mFetchFunc == NULL)						return E_UNEXPECTED;
	
		const void * start_p;
		const void * end_p;
		
		GUI_FreeFunc_f free_it = mFetchFunc(desired_type, &start_p, &end_p, mFetchRef);
		if (start_p == NULL)						return E_OUTOFMEMORY;
		
		vector<char>	buf((const char*)start_p,(const char*)end_p);
		mData[desired_type].swap(buf);
		if (free_it) free_it(start_p, mFetchRef);		
	}

	medium->tymed = TYMED_HGLOBAL;
	
	StGlobalBlock	new_block(mData[desired_type].size());
	if (!new_block())								return E_OUTOFMEMORY;
	
	{
		StGlobalLock lock_it(new_block());
		if (!lock_it())								return E_OUTOFMEMORY;
		memcpy(lock_it(), &*mData[desired_type].begin(), mData[desired_type].size());
	}

	medium->hGlobal = new_block();	
	medium->pUnkForRelease = NULL;		// This tells the caller to use the standard release (GlobalFree) on our handle.
	new_block.release();

	return S_OK;
}

// This copies our data into a medium that is totally pre-allocated.  I can't imagine this would work even remotely
// well...the caller must pre-allocate the handle to the exact right size.  So...pre-flight and then just use the 
// handle if we have it. 
STDMETHODIMP		GUI_SimpleDataObject::GetDataHere			(FORMATETC * format, STGMEDIUM * medium)
{
	GUI_ClipType	desired_type;
	if (!CIT2GUI(format->cfFormat, desired_type))	return DV_E_FORMATETC;	
	if (mData.count(desired_type) == 0)				return DV_E_FORMATETC;	
	if ((format->tymed & TYMED_HGLOBAL) == 0)		return DV_E_TYMED;
	if (medium->tymed != TYMED_HGLOBAL)			return DV_E_TYMED;

	if (mData[desired_type].empty())
	{
		if (mFetchFunc == NULL)						return E_UNEXPECTED;
	
		const void * start_p;
		const void * end_p;
		
		GUI_FreeFunc_f free_it = mFetchFunc(desired_type, &start_p, &end_p, mFetchRef);
		if (start_p == NULL)						return E_OUTOFMEMORY;
		
		vector<char>	buf((const char *)start_p,(const char *)end_p);
		mData[desired_type].swap(buf);
		if (free_it) free_it(start_p, mFetchRef);		
	}

	if (medium->hGlobal == NULL)									return E_INVALIDARG;
	if (GlobalSize(medium->hGlobal) != mData[desired_type].size())	return STG_E_MEDIUMFULL;

	{
		StGlobalLock lock_it(medium->hGlobal);
		if (!lock_it())												return E_OUTOFMEMORY;
		memcpy(lock_it(), &*mData[desired_type].begin(), mData[desired_type].size());
	}

	// Ben says: docs on GetDataHere says we must fill this out...strange, but necessary.
	medium->pUnkForRelease = NULL;
	return S_OK;

}

// This is the equivalent of "do you have this format" - we just do preflighting.
STDMETHODIMP		GUI_SimpleDataObject::QueryGetData			(FORMATETC * format)
{
	GUI_ClipType	desired_type;
	if (!CIT2GUI(format->cfFormat, desired_type))	return DV_E_FORMATETC;	
	if (mData.count(desired_type) == 0)				return DV_E_FORMATETC;	
	if ((format->tymed & TYMED_HGLOBAL) == 0)		return DV_E_TYMED;
													return S_OK;
}

// I still can't say I fully understand what this is for ... somehow it is used by calling code to analyze the
// various conversion options.  Bottom line is returning what we got with no ptd is fair game per the MS docs
// for trivial clients.  (Of course we provide exactly one rendering per clipboard-type.)
STDMETHODIMP		GUI_SimpleDataObject::GetCanonicalFormatEtc	(FORMATETC *format_in, FORMATETC  *format_out)
{	
	GUI_ClipType	desired_type;
	if (!CIT2GUI(format_in->cfFormat, desired_type))	return DV_E_FORMATETC;	
	if (mData.count(desired_type) == 0)					return DV_E_FORMATETC;	

	memcpy(format_out, format_in, sizeof(FORMATETC));
	format_out->ptd = NULL;
	return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP		GUI_SimpleDataObject::SetData				(FORMATETC *pformatetc, STGMEDIUM  *pmedium,BOOL fRelease)
{
	return E_NOTIMPL;
}

STDMETHODIMP		GUI_SimpleDataObject::EnumFormatEtc			(DWORD direction, IEnumFORMATETC  ** ppEnumObj)
{
	if (direction != DATADIR_GET) return E_NOTIMPL;
	*ppEnumObj = new GUI_SimpleEnumFORMATETC(this);
	return S_OK;
}

STDMETHODIMP		GUI_SimpleDataObject::DAdvise				(FORMATETC  *pformatetc, DWORD advf, IAdviseSink  *pAdvSink, DWORD  *pdwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP		GUI_SimpleDataObject::DUnadvise				(DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}
STDMETHODIMP		GUI_SimpleDataObject::EnumDAdvise			(IEnumSTATDATA ** ppenumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

GUI_SimpleEnumFORMATETC::GUI_SimpleEnumFORMATETC(GUI_SimpleDataObject * parent) :
	mRefCount(1),
	mIndex(0)
{
	for (map<GUI_ClipType, vector<char> >::iterator i = parent->mData.begin(); i != parent->mData.end(); ++i)
		mTypes.push_back(i->first);
}

GUI_SimpleEnumFORMATETC::GUI_SimpleEnumFORMATETC(const GUI_SimpleEnumFORMATETC& rhs) :
	mRefCount(1),
	mIndex(rhs.mIndex),
	mTypes(rhs.mTypes)
{
}

STDMETHODIMP		 GUI_SimpleEnumFORMATETC::QueryInterface(REFIID riid, void **ppvOut)
{
	*ppvOut = NULL;

		 if(IsEqualIID(riid, IID_IUnknown))			*ppvOut = this;
	else if(IsEqualIID(riid, IID_IEnumFORMATETC))	*ppvOut = (IEnumFORMATETC*)this;

	if(*ppvOut)
	{
		(*(LPUNKNOWN*)ppvOut)->AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) GUI_SimpleEnumFORMATETC::AddRef()
{
	return ++mRefCount;
}

STDMETHODIMP_(ULONG) GUI_SimpleEnumFORMATETC::Release()
{
	if (--mRefCount == 0)
	{
		delete this;
		return 0;
	}
	return mRefCount;
}

STDMETHODIMP		GUI_SimpleEnumFORMATETC::Next			(ULONG count, FORMATETC * formats, ULONG * out_count)
{
	if (mIndex >= mTypes.size()) { if (out_count) *out_count = NULL; return S_FALSE; }
	ULONG remaining = mTypes.size() - mIndex;	
	int to_fetch = min(count, remaining);
	
	for (int n = 0; n < to_fetch; ++n)
	{
		GUI2CIT(mTypes[n+mIndex],formats[n].cfFormat);
		formats[n].ptd			= NULL;
		formats[n].dwAspect		= DVASPECT_CONTENT;
		formats[n].lindex		= -1;
		formats[n].tymed		= TYMED_HGLOBAL;
	}
	
	mIndex += to_fetch;
	if (out_count) *out_count = to_fetch;
	return (count == to_fetch) ? S_OK : S_FALSE;
}

STDMETHODIMP		GUI_SimpleEnumFORMATETC::Skip			(ULONG count)
{
	if (mIndex >= mTypes.size()) { return S_FALSE; }
	ULONG remaining = mTypes.size() - mIndex;	
	int to_fetch = min(count, remaining);	
	mIndex += to_fetch;
	return (count == to_fetch) ? S_OK : S_FALSE;
}

STDMETHODIMP		GUI_SimpleEnumFORMATETC::Reset			(void)
{
	mIndex = 0;
	return S_OK;
}

STDMETHODIMP		GUI_SimpleEnumFORMATETC::Clone			(IEnumFORMATETC ** pp_obj)
{
	*pp_obj = new GUI_SimpleEnumFORMATETC(*this);
	return S_OK;
}

#endif /* IBM */

//---------------------------------------------------------------------------------------------------------
// DRAG & DROP -- MAC
//---------------------------------------------------------------------------------------------------------

#if APL

// The GUI_DragMgr_Adapter is a simple adapter class from the drag mgr API to the 
// GUI abstract class.  IT's almost a 1:1 coding of the API.
GUI_DragMgr_Adapter::GUI_DragMgr_Adapter(DragRef data_obj) : mObject(data_obj)
{
}

GUI_DragMgr_Adapter::~GUI_DragMgr_Adapter()
{
}

int		GUI_DragMgr_Adapter::CountItems(void)
{
	UInt16 count;
	if (CountDragItems(mObject, &count) != noErr) return 0;
	return count;
}

bool	GUI_DragMgr_Adapter::NthItemHasClipType(int n, GUI_ClipType ct)
{
	GUI_CIT		native;
	DragItemRef ref;
	FlavorFlags	flags;
	if (!GUI2CIT(ct,native))										return false;
	if (GetDragItemReferenceNumber(mObject, n+1, &ref) != noErr)	return false;
	if (GetFlavorFlags(mObject,ref,native,&flags) != noErr)			return false;
	if (ct >= gui_First_Private && (flags & flavorSenderOnly)==0)	return false;
																	return true;
}

int		GUI_DragMgr_Adapter::GetNthItemSize(int n, GUI_ClipType ct)
{
	GUI_CIT		native;
	DragItemRef ref;
	FlavorFlags	flags;
	Size		sz;
	if (!GUI2CIT(ct,native))										return 0;
	if (GetDragItemReferenceNumber(mObject, n+1, &ref) != noErr)	return 0;
	if (GetFlavorFlags(mObject,ref,native,&flags) != noErr)			return 0;
	if (ct >= gui_First_Private && (flags & flavorSenderOnly)==0)	return 0;
	if (GetFlavorDataSize(mObject,ref,native,&sz) != noErr)			return 0;
																	return sz;
	
}

bool	GUI_DragMgr_Adapter::GetNthItemData(int n, GUI_ClipType ct, int size, void * ptr)
{
	GUI_CIT		native;
	DragItemRef ref;
	FlavorFlags	flags;
	Size		sz;
	if (!GUI2CIT(ct,native))										return false;
	if (GetDragItemReferenceNumber(mObject, n+1, &ref) != noErr)	return false;
	if (GetFlavorFlags(mObject,ref,native,&flags) != noErr)			return false;
	if (ct >= gui_First_Private && (flags & flavorSenderOnly)==0)	return false;
	if (GetFlavorDataSize(mObject,ref,native,&sz) != noErr)			return false;
	if (size != sz)													return false;
	if (GetFlavorData(mObject,ref,native,ptr,&sz,0) != noErr)		return false;
	if (size != sz)													return false;
																	return true;
}

// This function is used to pull missing drag flavors from the "get data" function.
static pascal OSErr GUI_DragDataFetcher(FlavorType native, void * ref, DragItemRef item, DragRef drag)
{
	// Ben says: weird encoding - we need TWO refcons and I am too lazy to allocate a stack-based struct - it's awkward due
	// to the use of utility funcs.  So: item ref is picked to be refcon for pull func.  Real mac refcon to the func is
	// our func ptr.
	GUI_GetData_f		func = (GUI_GetData_f) ref;
	void *				func_ref = (void *) item;

	GUI_ClipType	cit;
	if (!CIT2GUI(native, cit)) return badDragFlavorErr;
	
	const void * start_p;
	const void * end_p;
	GUI_FreeFunc_f	free_it = func(cit, &start_p, &end_p, func_ref);
	if (start_p == NULL) return badDragFlavorErr;
	
	OSErr err = SetDragItemFlavorData(drag, item, native, start_p, (const char *) end_p - (const char *) start_p, 0);
	if (free_it) free_it(start_p, func_ref);
	return err;
}

static DragSendDataUPP	gui_DragDataFetcher = NewDragSendDataUPP(GUI_DragDataFetcher);

void GUI_LoadSimpleDrag(
							DragRef					drag,
							int						type_count, 
							GUI_ClipType			inTypes[], 
							int						sizes[], 
							const void *			ptrs[],
							GUI_GetData_f			fetch_func,
							void *					ref)
{
	// First, load up an item with all of the data we've been given.

	GUI_CIT	native;
	for (int n = 0; n < type_count; ++n)
	if (GUI2CIT(inTypes[n],native))
		AddDragItemFlavor(drag, (DragItemRef) ref, native, ptrs[n], sizes[n], (inTypes[n] >= gui_First_Private) ? flavorSenderOnly : 0);

	// If a fetch-func is provided, also register it for later use.
	if (fetch_func)
		SetDragSendProc(drag, gui_DragDataFetcher, (void *) fetch_func);
}

#endif /* APL */
