#include "XPWidgetDialogs.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMGraphics.h"
#include "XPUIGraphics.h"
#include "XPWidgetUtils.h"
#if !NO_POPUPS
#include "XSBPopups.h"
#endif
#if !NO_TABS
#include "XPWidgetTab.h"
#endif
#include <stdarg.h>
#include <list>

#define	DEBUG_SHOW_ROWCOLUMNS	0

#if APL
#if defined(__MWERKS__)
#include <Scrap.h>
#else
#include <Carbon/Carbon.h>
#endif
#endif



/*
todo on widgets:

	1. Need a word-wrapping caption!!
	2. grid layout? - think through justification carefully first!

	Eventually: justification
	Eventually: numeric field key filters!  Other key filters?
	Eventually: tabbing!

	Feature req: file format based generation

*/

/**********************************************************************
 * LAYOUT MANAGEMENT APIS
 **********************************************************************/

typedef	pair<int,int>			IntPair;
typedef	pair<IntPair,IntPair>	IntQuad;
typedef	hash_map<XPWidgetClass, hash_map<XPWidgetClass, int > >		LayoutMarginTable;	// Sibbling, contain
typedef	hash_map<XPWidgetClass, hash_map<XPWidgetClass, IntQuad > >	LayoutContainTable;	// ((left,right),(bottom,top))

#define kDefaultMarginH	10
#define kDefaultMarginV 10

static	LayoutMarginTable	kMarginTableH;
static	LayoutMarginTable	kMarginTableV;
static	LayoutContainTable	kMarginTableC;

int		XPLayout_GetMinimumWidth(XPWidgetClass inClass, XPWidgetID inWidget)
{
	if (XPSendMessageToWidget(inWidget, xpMsg_RecalcMinSizeH, xpMode_Direct, 0, 0) > 0) return XPGetWidgetProperty(inWidget, xpProperty_MinWidth, NULL);

	int l, t, r, b;
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	return r - l;
}

int		XPLayout_GetMinimumHeight(XPWidgetClass inClass, XPWidgetID inWidget)
{
	if (XPSendMessageToWidget(inWidget, xpMsg_RecalcMinSizeV, xpMode_Direct, 0, 0) > 0) return XPGetWidgetProperty(inWidget, xpProperty_MinHeight, NULL);

	int l, t, r, b;
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	return t - b;
}

void	XPLayout_Reshape(XPWidgetClass inClass, XPWidgetID inWidget, int inIdealWidth, int inIdealHeight)
{
	if (inIdealWidth != -1) XPSetWidgetProperty(inWidget, xpProperty_MinWidth, inIdealWidth);
	if (inIdealHeight != -1) XPSetWidgetProperty(inWidget, xpProperty_MinHeight, inIdealHeight);
	if (XPSendMessageToWidget(inWidget, xpMsg_DoReshape, xpMode_Direct, 0, 0) > 0) return;
}

int		XPLayout_GetMinimumMarginH(XPWidgetClass inLeft, XPWidgetClass inRight)
{
	if (kMarginTableH.count(inLeft) == 0)			inLeft = 0;
	if (kMarginTableH.count(inLeft) == 0)			return kDefaultMarginH;

	if (kMarginTableH[inLeft].count(inRight) == 0)	inRight = 0;
	if (kMarginTableH[inLeft].count(inRight) == 0)	return kDefaultMarginH;

	return kMarginTableH[inLeft][inRight];
}

int		XPLayout_GetMinimumMarginV(XPWidgetClass inBottom, XPWidgetClass inTop)
{
	if (kMarginTableV.count(inBottom) == 0) 		inBottom = 0;
	if (kMarginTableV.count(inBottom) == 0) 		return kDefaultMarginV;

	if (kMarginTableV[inBottom].count(inTop)==0)	inTop = 0;
	if (kMarginTableV[inBottom].count(inTop)==0) 	return kDefaultMarginV;

	return kMarginTableV[inBottom][inTop];
}

void	XPLayout_GetContainMargins(XPWidgetClass inParent, XPWidgetClass inChild, int& outLeft, int& outTop, int& outRight, int& outBottom)
{
	outLeft = outRight = kDefaultMarginH;
	outBottom = outTop = kDefaultMarginV;
	if (kMarginTableC.count(inParent) == 0)		inParent = 0;
	if (kMarginTableC.count(inParent) == 0)		return;
	if (kMarginTableC[inParent].count(inChild) == 0)		inChild = 0;
	if (kMarginTableC[inParent].count(inChild) == 0)		return;
	outLeft = kMarginTableC[inParent][inChild].first.first;
	outRight = kMarginTableC[inParent][inChild].first.second;
	outBottom = kMarginTableC[inParent][inChild].second.first;
	outTop = kMarginTableC[inParent][inChild].second.second;
}

void	XPLayout_RegisterSpecialMarginsH(XPWidgetClass inLeft, XPWidgetClass inRight, int margin)
{
	kMarginTableH[inLeft][inRight] = margin;
}

void	XPLayout_RegisterSpecialMarginsV(XPWidgetClass inBottom, XPWidgetClass inTop, int margin)
{
	kMarginTableV[inBottom][inTop] = margin;
}

void	XPLayout_RegisterSpecialMarginsContains(XPWidgetClass inParent, XPWidgetClass inChild, int inLeft, int inTop, int inRight, int inBottom)
{
	kMarginTableC[inParent][inChild] = IntQuad(IntPair(inLeft,inRight),IntPair(inBottom, inTop));
}


#pragma mark -
/**********************************************************************
 * WRAPPER WIDGETS
 **********************************************************************/

static bool	XPGetTextFromClipboard(std::string& outText)
{
#if IBM
		HGLOBAL   	hglb;
		LPTSTR    	lptstr;
		bool		retVal = false;

	if (!IsClipboardFormatAvailable(CF_TEXT))
		return false;

	if (!OpenClipboard(NULL))
		return false;

	hglb = GetClipboardData(CF_TEXT);
	if (hglb != NULL)
	{
		lptstr = (LPSTR)GlobalLock(hglb);
		if (lptstr != NULL)
		{
			outText = lptstr;
			GlobalUnlock(hglb);
  			retVal = true;
		}
	}

	CloseClipboard();

	return retVal;
#endif
#if APL
		ScrapRef	scrap;
	if (::GetCurrentScrap(&scrap) != noErr)
		return false;

	SInt32		byteCount = 0;
	OSStatus	status = ::GetScrapFlavorSize(scrap, kScrapFlavorTypeText, &byteCount);
	if (status != noErr)
		return false;

	outText.resize(byteCount);

	return (::GetScrapFlavorData(scrap, kScrapFlavorTypeText, &byteCount, &*outText.begin() ) == noErr);
#endif
}

static bool	XPSetTextToClipboard(const std::string& inText)
{
#if IBM
		LPTSTR  lptstrCopy;
		HGLOBAL hglbCopy;

	if (!OpenClipboard(NULL))
		return false;
	EmptyClipboard();

	hglbCopy = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR) * (inText.length() + 1));
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		return false;
	}

	lptstrCopy = (LPSTR)GlobalLock(hglbCopy);
	strcpy(lptstrCopy, inText.c_str());
	GlobalUnlock(hglbCopy);

	SetClipboardData(CF_TEXT, hglbCopy);
	CloseClipboard();
	return true;
#endif
#if APL
	ScrapRef	scrap;
	if (::ClearCurrentScrap() != noErr) return false;
	if (::GetCurrentScrap(&scrap) != noErr) return false;

	return ::PutScrapFlavor( scrap, kScrapFlavorTypeText, kScrapFlavorMaskNone, inText.size(), &*inText.begin()) == noErr;

#endif
}

int XPWF_Caption			(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	static XPWidgetFunc_t parent = XPGetWidgetClassFunc(xpWidgetClass_Caption);
	if (msg == xpMsg_Create)
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_Caption);

	int	fh, fv;
	int	l, t, r, b;
	char	buf[1024];

	if (msg == xpMsg_RecalcMinSizeH)
	{
		XPLMGetFontDimensions(xplmFont_Basic, &fh, NULL, NULL);
		XPGetWidgetDescriptor(id, buf, sizeof(buf));
		XPSetWidgetProperty(id, xpProperty_MinWidth, 10 + strlen(buf) * fh);
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		XPLMGetFontDimensions(xplmFont_Basic, NULL, &fv, NULL);
		XPSetWidgetProperty(id, xpProperty_MinHeight, fv+4);
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		fh = XPGetWidgetProperty(id, xpProperty_MinWidth, NULL);
		fv = XPGetWidgetProperty(id, xpProperty_MinHeight, NULL);
		XPGetWidgetGeometry(id, &l, &t,&r,&b);
		XPSetWidgetGeometry(id, l,t,l+fh,t-fv);
		return 1;
	}
	return parent(msg, id, p1, p2);
}

static int	XPWF_EditText_CommonLayout(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	static XPWidgetFunc_t parent = XPGetWidgetClassFunc(xpWidgetClass_TextField);
	if (msg == xpMsg_Create)
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_TextField);
	if (msg == xpMsg_RecalcMinSizeH)
	{
		int cw, fw, ew;
		cw = XPGetWidgetProperty(id, xpProperty_FieldVisChars, NULL);
		XPLMGetFontDimensions(xplmFont_Basic, &fw, NULL, NULL);
		XPGetElementDefaultDimensions(xpElement_TextField, &ew, NULL, NULL);
		XPSetWidgetProperty(id, xpProperty_MinWidth, max(ew, 10 + cw * fw));
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		int fh;
		XPGetElementDefaultDimensions(xpElement_TextField, NULL, &fh, NULL);
		XPSetWidgetProperty(id, xpProperty_MinHeight, fh);
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		int iw, ih, l,t,r,b;
		iw = XPGetWidgetProperty(id, xpProperty_MinWidth, NULL);
		ih = XPGetWidgetProperty(id, xpProperty_MinHeight, NULL);
		XPGetWidgetGeometry(id, &l, &t,&r,&b);
		XPSetWidgetGeometry(id, l,t,l+iw,t-ih);
		return 1;
	}

	if (msg == xpMsg_KeyPress)
	{
		char			theChar = KEY_VKEY(p1);
		XPLMKeyFlags	flags = KEY_FLAGS(p1);

		if ((flags & (xplm_DownFlag + xplm_ControlFlag)) == (xplm_DownFlag + xplm_ControlFlag))
		{
			long	selStart = XPGetWidgetProperty(id, xpProperty_EditFieldSelStart, NULL);
			long	selEnd = XPGetWidgetProperty(id, xpProperty_EditFieldSelEnd, NULL);
			long	strLen = XPGetWidgetDescriptor(id, NULL, 0);
			std::string	txt;
			txt.resize(strLen);
			XPGetWidgetDescriptor(id, &*txt.begin(), txt.size()+1);
			if (theChar == XPLM_VK_V)
			{
				std::string	scrap;
				if (XPGetTextFromClipboard(scrap) && !scrap.empty())
				{
					if ((selEnd > selStart) && (selStart >= 0) && (selEnd <= strLen))
					{
						txt.replace(selStart, selEnd - selStart, scrap);
						XPSetWidgetDescriptor(id, txt.c_str());
						XPSetWidgetProperty(id, xpProperty_EditFieldSelStart, selStart + scrap.size());
						XPSetWidgetProperty(id, xpProperty_EditFieldSelEnd, selStart + scrap.size());
					} else if ((selStart >= 0) && (selStart <= strLen)) {
						txt.insert(selStart, scrap);
						XPSetWidgetDescriptor(id, txt.c_str());
						XPSetWidgetProperty(id, xpProperty_EditFieldSelStart, selStart + scrap.size());
						XPSetWidgetProperty(id, xpProperty_EditFieldSelEnd, selStart + scrap.size());
					}
				}
				return 1;
			}
			if ((theChar == XPLM_VK_C) || (theChar == XPLM_VK_X))
			{
				if ((selStart >= 0) && (selStart < selEnd) && (selEnd <= strLen))
				{
					std::string	scrap = txt.substr(selStart, selEnd - selStart);
					if (XPSetTextToClipboard(scrap) && (theChar == XPLM_VK_X))
					{
						txt.erase(selStart, selEnd - selStart);
						XPSetWidgetDescriptor(id, txt.c_str());
						XPSetWidgetProperty(id, xpProperty_EditFieldSelEnd, selStart);
					}
				}
				return 1;
			}
		}
	}

	return parent(msg, id, p1,p2);
}

int	XPWF_EditText_String	(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	char	buf[2048];
	int max_chars = XPGetWidgetProperty(id, xpProperty_MaxCharacters, NULL);
	if (max_chars > sizeof(buf)) max_chars = sizeof(buf);

	if (msg == xpMsg_DataToDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			char * src = reinterpret_cast<char *>(ptri);
			XPSetWidgetDescriptor(id, src);
		}
		return 1;
	}
	if (msg == xpMsg_DataFromDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			char * src = reinterpret_cast<char *>(ptri);
			XPGetWidgetDescriptor(id, src, max_chars);
		}
		return 1;
	}
	return XPWF_EditText_CommonLayout(msg, id, p1, p2);
}

int	XPWF_EditText_Int		(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	char	buf[2048];
	int max_chars = XPGetWidgetProperty(id, xpProperty_MaxCharacters, NULL);
	if (max_chars > sizeof(buf)) max_chars = sizeof(buf);

	if (msg == xpMsg_DataToDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			sprintf(buf, "%d", *src);
			XPSetWidgetDescriptor(id, buf);
		}
		return 1;
	}
	if (msg == xpMsg_DataFromDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			XPGetWidgetDescriptor(id, buf, max_chars);
			*src = atoi(buf);
		}
		return 1;
	}
	return XPWF_EditText_CommonLayout(msg, id, p1, p2);
}

int	XPWF_EditText_Float		(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	char	buf[2048];
	int max_chars = XPGetWidgetProperty(id, xpProperty_MaxCharacters, NULL);
	if (max_chars > sizeof(buf)) max_chars = sizeof(buf);
	if (msg == xpMsg_DataToDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		int precis = XPGetWidgetProperty(id, xpProperty_FieldPrecision, NULL);
		if (ptri != 0)
		{
			float * src = reinterpret_cast<float *>(ptri);
			char	format[20];
			sprintf(format, "%%.%df", precis);
			sprintf(buf, format, *src);
			XPSetWidgetDescriptor(id, buf);
		}
		return 1;
	}
	if (msg == xpMsg_DataFromDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			float * src = reinterpret_cast<float *>(ptri);
			XPGetWidgetDescriptor(id, buf, max_chars);
			*src = atof(buf);
		}
		return 1;
	}
	return XPWF_EditText_CommonLayout(msg, id, p1, p2);
}

int	XPWF_PushButton_Action	(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	static XPWidgetFunc_t parent = XPGetWidgetClassFunc(xpWidgetClass_Button);
	char	buf[1024];
	int		width, height, cwidth, l,t,r,b;

	if (msg == xpMsg_Create)
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_Button);

	if (msg == xpMsg_PushButtonPressed)
	{
		int ret = 0;
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			void (*func)(XPWidgetID) = reinterpret_cast<void (*)(XPWidgetID)>(ptri);
			func(id);
			ret= 1;
		}
		ptri = XPGetWidgetProperty(id, xpProperty_NotifyPtr, NULL);
		if (ptri != 0)
		{
			void (*func)(XPWidgetID) = reinterpret_cast<void (*)(XPWidgetID)>(ptri);
			func(id);
			ret = 1;
		}
		return ret;
	}
	if (msg == xpMsg_RecalcMinSizeH)
	{
		XPGetElementDefaultDimensions(xpElement_PushButton, &width, &height, NULL);
		XPGetWidgetDescriptor(id, buf, sizeof(buf));
		XPLMGetFontDimensions(xplmFont_Basic, &cwidth, NULL, NULL);
		width = max((size_t) width, 20 + cwidth * strlen(buf));
		XPSetWidgetProperty(id, xpProperty_MinWidth, width);
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		XPGetElementDefaultDimensions(xpElement_PushButton, &width, &height, NULL);
		XPSetWidgetProperty(id, xpProperty_MinHeight, height);
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		width = XPGetWidgetProperty(id ,xpProperty_MinWidth, NULL);
		height = XPGetWidgetProperty(id ,xpProperty_MinHeight, NULL);
		XPGetWidgetGeometry(id, &l,&t,&r,&b);
		XPSetWidgetGeometry(id, l,t,l+width,t-height);
		return 1;
	}
	return parent(msg, id, p1, p2);
}

int XPWF_CheckBox_Int		(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	static XPWidgetFunc_t parent = XPGetWidgetClassFunc(xpWidgetClass_Button);
	char	buf[1024];
	int		width, height, cwidth, l,t,r,b;

	if (msg == xpMsg_Create)
	{
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_Button);
		XPSetWidgetProperty(id, xpProperty_ButtonType, xpRadioButton);
		XPSetWidgetProperty(id, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	}
	if (msg == xpMsg_DataToDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			XPSetWidgetProperty(id, xpProperty_ButtonState, *src);
		}
		return 1;
	}
	if (msg == xpMsg_DataFromDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			*src = XPGetWidgetProperty(id, xpProperty_ButtonState, NULL);
		}
		return 1;
	}

	if (msg == xpMsg_RecalcMinSizeH)
	{
		XPGetElementDefaultDimensions(xpElement_CheckBox, &width, &height, NULL);
		XPGetWidgetDescriptor(id, buf, sizeof(buf));
		XPLMGetFontDimensions(xplmFont_Basic, &cwidth, NULL, NULL);
		width = max((size_t) width, 20 + cwidth * strlen(buf));
		XPSetWidgetProperty(id, xpProperty_MinWidth, width);
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		XPGetElementDefaultDimensions(xpElement_CheckBox, &width, &height, NULL);
		XPSetWidgetProperty(id, xpProperty_MinHeight, height);
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		width = XPGetWidgetProperty(id ,xpProperty_MinWidth, NULL);
		height = XPGetWidgetProperty(id ,xpProperty_MinHeight, NULL);
		XPGetWidgetGeometry(id, &l,&t,&r,&b);
		XPSetWidgetGeometry(id, l,t,l+width,t-height);
		return 1;
	}
	if (msg == xpMsg_ButtonStateChanged)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_NotifyPtr, NULL);
		if (ptri != 0)
		{
			void (*func)(XPWidgetID) = reinterpret_cast<void (*)(XPWidgetID)>(ptri);
			func(id);
		}
		return 1;
	}

	return parent(msg, id, p1, p2);
}

int XPWF_RadioButton_Int		(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	static XPWidgetFunc_t parent = XPGetWidgetClassFunc(xpWidgetClass_Button);
	char	buf[1024];
	int		width, height, cwidth, l,t,r,b;

	if (msg == xpMsg_Create)
	{
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_Button);
		XPSetWidgetProperty(id, xpProperty_ButtonType, xpRadioButton);
		XPSetWidgetProperty(id, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);
	}
	if (msg == xpMsg_DataToDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		int	my_enum = XPGetWidgetProperty(id, xpProperty_RadioButtonEnum, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			XPSetWidgetProperty(id, xpProperty_ButtonState, (*src == my_enum) ? 1 : 0);
		}
		return 1;
	}
	if (msg == xpMsg_DataFromDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		int	my_enum = XPGetWidgetProperty(id, xpProperty_RadioButtonEnum, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			if (XPGetWidgetProperty(id, xpProperty_ButtonState, NULL))
				*src = my_enum;
		}
		return 1;
	}
	if (msg == xpMsg_ButtonStateChanged)
	{
		XPWidgetID dad = XPGetParentWidget(id);
		if (dad)
		{
			int sib_count = XPCountChildWidgets(dad);
			for (int n = 0; n < sib_count; ++n)
			{
				XPWidgetID sib = XPGetNthChildWidget(dad, n);
				if (sib != id)
					XPSetWidgetProperty(sib, xpProperty_ButtonState, 0);
			}
		}
		int ptri = XPGetWidgetProperty(id, xpProperty_NotifyPtr, NULL);
		if (ptri != 0)
		{
			void (*func)(XPWidgetID) = reinterpret_cast<void (*)(XPWidgetID)>(ptri);
			func(id);
		}
		return 1;
	}

	if (msg == xpMsg_RecalcMinSizeH)
	{
		XPGetElementDefaultDimensions(xpElement_CheckBox, &width, &height, NULL);
		XPGetWidgetDescriptor(id, buf, sizeof(buf));
		XPLMGetFontDimensions(xplmFont_Basic, &cwidth, NULL, NULL);
		width = max((size_t) width, 20 + cwidth * strlen(buf));
		XPSetWidgetProperty(id, xpProperty_MinWidth, width);
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		XPGetElementDefaultDimensions(xpElement_CheckBox, &width, &height, NULL);
		XPSetWidgetProperty(id, xpProperty_MinHeight, height);
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		width = XPGetWidgetProperty(id ,xpProperty_MinWidth, NULL);
		height = XPGetWidgetProperty(id ,xpProperty_MinHeight, NULL);
		XPGetWidgetGeometry(id, &l,&t,&r,&b);
		XPSetWidgetGeometry(id, l,t,l+width,t-height);
		return 1;
	}
	return parent(msg, id, p1, p2);
}




#if !NO_POPUPS
int	XPWF_Popup_Int			(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	if (msg == xpMsg_Create)
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_Popup);
	if (msg == xpMsg_DataToDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			XPSetWidgetProperty(id, xpProperty_PopupCurrentItem, *src);
		}
		return 1;
	}
	if (msg == xpMsg_DataFromDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			*src = XPGetWidgetProperty(id, xpProperty_PopupCurrentItem, NULL);
		}
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeH)
	{
		char	buf[2048];
		XPGetWidgetDescriptor(id, buf, sizeof(buf));
		int		max_num_chars = 0;
		int		local_num_chars = 0;
		char * p = buf;
		while (*p)
		{
			if (*p == ';')
			{
				max_num_chars = max(max_num_chars, local_num_chars);
				local_num_chars =  0;
			} else
				++local_num_chars;
			++p;
		}
		max_num_chars = max(max_num_chars, local_num_chars);
		int		cw;
		XPLMGetFontDimensions(xplmFont_Basic, &cw, NULL, NULL);
		XPSetWidgetProperty(id, xpProperty_MinWidth, 35 + cw * max_num_chars);
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		XPSetWidgetProperty(id, xpProperty_MinHeight, 20);
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		int width = XPGetWidgetProperty(id ,xpProperty_MinWidth, NULL);
		int height = XPGetWidgetProperty(id ,xpProperty_MinHeight, NULL);
		int 	l,t,r,b;
		XPGetWidgetGeometry(id, &l,&t,&r,&b);
		XPSetWidgetGeometry(id, l,t,l+width,t-height);
		return 1;
	}
	if (msg == xpMessage_NewItemPicked)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_NotifyPtr, NULL);
		if (ptri != 0)
		{
			void (*func)(XPWidgetID) = reinterpret_cast<void (*)(XPWidgetID)>(ptri);
			func(id);
		}
		return 1;
	}

	return XSBPopupButtonProc(msg, id, p1, p2);
}
#endif

#if !NO_TABS
int	XPWF_Tabs_IntShowHide	(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	int child_count, dim, localdim, n, l, r, t, b, dx, dy, ideal_x, ideal_y, cl, cr, ct, cb, child_type, ml, mr, mt, mb;
	XPWidgetID child;
	if (msg == xpMsg_Create)
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_Tab);
	if (msg == xpMsg_DataToDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			XPSetWidgetProperty(id, xpProperty_CurrentTab, *src);
		}
		return 1;
	}
	if (msg == xpMsg_DataFromDialog)
	{
		int ptri = XPGetWidgetProperty(id, xpProperty_DataPtr, NULL);
		if (ptri != 0)
		{
			int * src = reinterpret_cast<int *>(ptri);
			*src = XPGetWidgetProperty(id, xpProperty_CurrentTab, NULL);
		}
		return 1;
	}
	if (msg == xpMsg_TabChanged	|| msg == xpMsg_PropertyChanged || msg == xpMsg_Shown)
	{
		int index = XPGetWidgetProperty(id, xpProperty_CurrentTab, NULL);
		int children = XPCountChildWidgets(id);
		for (int n = 0; n < children; ++n)
		{
			XPWidgetID child = XPGetNthChildWidget(id, n);
			if (n == index)
				XPShowWidget(child);
			else
				XPHideWidget(child);
		}
		if (msg == xpMsg_TabChanged)
		{
			int ptri = XPGetWidgetProperty(id, xpProperty_NotifyPtr, NULL);
			if (ptri != 0)
			{
				void (*func)(XPWidgetID) = reinterpret_cast<void (*)(XPWidgetID)>(ptri);
				func(id);
			}
		}
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeH)
	{
		child_count = XPCountChildWidgets(id);
		char	buf[1024];
		XPGetWidgetDescriptor(id, buf, sizeof(buf));
		dim = 114;
		for (char * p = buf; *p; ++p)
		if (*p == ';')
			dim += 100;
		for (n = 0; n < child_count; ++n)
		{
			child = XPGetNthChildWidget(id, n);
			child_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);
			XPLayout_GetContainMargins(xpWidgetClass_Tab, child_type, ml, mt, mr, mb);
			localdim = XPLayout_GetMinimumWidth(0, child) + ml + mr;
			dim = max(dim, localdim);
		}
		XPSetWidgetProperty(id, xpProperty_MinWidth, dim);
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		child_count = XPCountChildWidgets(id);
		dim = 0;
		for (n = 0; n < child_count; ++n)
		{
			child = XPGetNthChildWidget(id, n);
			child_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);
			XPLayout_GetContainMargins(xpWidgetClass_Tab, child_type, ml, mt, mr, mb);
			localdim = XPLayout_GetMinimumHeight(0, child) + mt + mb;
			dim = max(dim, localdim);
		}
		XPSetWidgetProperty(id, xpProperty_MinHeight, dim);
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		child_count = XPCountChildWidgets(id);
		XPGetWidgetGeometry(id, &l, &t, &r, &b);
		ideal_x = XPGetWidgetProperty(id, xpProperty_MinWidth, NULL);
		ideal_y = XPGetWidgetProperty(id, xpProperty_MinHeight, NULL);
		if (ideal_x != -1) r = l + ideal_x;
		if (ideal_y != -1) b = t - ideal_y;
		XPSetWidgetGeometry(id, l, t, r, b);
		dx = l;
		dy = t;
		for (n = 0; n < child_count; ++n)
		{
			child = XPGetNthChildWidget(id, n);
			child_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);
			XPLayout_GetContainMargins(xpWidgetClass_Tab, child_type, ml, mt, mr, mb);
			XPGetWidgetGeometry(child, &cl, &ct, &cr, &cb);
			XPSetWidgetGeometry(child, dx+mr, dy-mt, dx+mr + (cr - cl), dy-mt - (ct - cb));
			XPLayout_Reshape(0, child, ideal_x-ml-mr, ideal_y-mt-mb);
		}
		return 1;
	}
	return XPTabProc(msg, id, p1, p2);
}
#endif

#pragma mark -
/**********************************************************************
 * LAYOUT WIDGETS
 **********************************************************************/

int	XPWF_RowColumn	(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	if (XPUFixedLayout(msg, id, p1, p2)) return 1;

	int last_type = -1;
	int	our_type = -1;

	int	vertical = XPGetWidgetProperty(id, xpProperty_RowColumnIsVertical, NULL);
	int justH = XPGetWidgetProperty(id, xpProperty_JustifyH, NULL);
	int justV = XPGetWidgetProperty(id, xpProperty_JustifyV, NULL);
	int	dim, localdim;
	int child_count;
	XPWidgetID	child;
	int n;
	int l, t, b, r, dx, dy, cl, ct, cb, cr;
	int	ideal_x, ideal_y;

	if (msg == xpMsg_Create)
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_RowColumn);

#if DEBUG_SHOW_ROWCOLUMNS
	if (msg == xpMsg_Draw)
	{
		XPLMSetGraphicsState(0,0,0,   0, 1,   0, 0);
		glColor4f(0.0, 1.0, 0.0, 0.5);
		XPGetWidgetGeometry(id, &l, &t, &r, &b);
		glBegin(GL_LINE_LOOP);
		glVertex2i(l,b);
		glVertex2i(l,t);
		glVertex2i(r,t);
		glVertex2i(r,b);
		glEnd();
		return 1;
	}
#endif

	if (msg == xpMsg_RecalcMinSizeH)
	{
		child_count = XPCountChildWidgets(id);
		dim = 0;
		for (n = 0; n < child_count; ++n)
		{
			child = XPGetNthChildWidget(id, n);
			last_type = our_type;
			our_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);
			localdim = XPLayout_GetMinimumWidth(0, child);
			if (vertical)
				dim = max(dim, localdim);
			else {
				dim += localdim;
				if (last_type != -1)
					dim += XPLayout_GetMinimumMarginH(last_type, our_type);
			}
		}
		XPSetWidgetProperty(id, xpProperty_MinWidth, dim);
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		child_count = XPCountChildWidgets(id);
		dim = 0;
		for (n = 0; n < child_count; ++n)
		{
			child = XPGetNthChildWidget(id, n);
			last_type = our_type;
			our_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);
			localdim = XPLayout_GetMinimumHeight(0, child);
			if (vertical) {
				dim += localdim;
				if (last_type != -1)
					dim += XPLayout_GetMinimumMarginV(last_type, our_type);
			} else
				dim = max(dim, localdim);
		}
		XPSetWidgetProperty(id, xpProperty_MinHeight, dim);
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		child_count = XPCountChildWidgets(id);
		XPGetWidgetGeometry(id, &l, &t, &r, &b);
		ideal_x = XPGetWidgetProperty(id, xpProperty_MinWidth, NULL);
		ideal_y = XPGetWidgetProperty(id, xpProperty_MinHeight, NULL);
		if (ideal_x != -1) r = l + ideal_x;
		if (ideal_y != -1) b = t - ideal_y;
		XPSetWidgetGeometry(id, l, t, r, b);
		dx = l;
		dy = t;
		for (n = 0; n < child_count; ++n)
		{
			child = XPGetNthChildWidget(id, n);
			last_type = our_type;
			our_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);

			if (vertical) {
				if (last_type != -1)
					dy -= XPLayout_GetMinimumMarginV(last_type, our_type);
			} else {
				if (last_type != -1)
					dx += XPLayout_GetMinimumMarginH(last_type, our_type);
			}


			XPGetWidgetGeometry(child, &cl, &ct, &cr, &cb);
			XPSetWidgetGeometry(child, dx, dy, dx + (cr - cl), dy - (ct - cb));
			XPLayout_Reshape(0, child, vertical ? ideal_x : -1, vertical ? -1 : ideal_y);
			XPGetWidgetGeometry(child, &cl, &ct, &cr, &cb);

			if (vertical) {
				dy -= (ct - cb);
			} else {
				dx += (cr - cl);
			}
		}
		return 1;
	}
	return 0;
}

#pragma mark -
/**********************************************************************
 * DIALOG BOX STUFF
 **********************************************************************/

void	PBAction_DoneDialogOK(XPWidgetID id)
{
	XPWidgetID root = XPFindRootWidget(id);
	if (root)
		XPSendMessageToWidget(root, xpMsg_DialogDone, xpMode_Direct, xpDialog_ResultOK, 0);
}

void	PBAction_DoneDialogCancel(XPWidgetID id)
{
	XPWidgetID root = XPFindRootWidget(id);
	if (root)
		XPSendMessageToWidget(root, xpMsg_DialogDone, xpMode_Direct, xpDialog_ResultCancel, 0);
}

int	XPWF_DialogBox	(XPWidgetMessage msg, XPWidgetID id,long p1,long p2)
{
	static XPWidgetFunc_t parent = XPGetWidgetClassFunc(xpWidgetClass_MainWindow);
	int	ml, mr, mt, mb;

#if DEBUG_SHOW_ROWCOLUMNS
	if (msg == xpMsg_Draw)
	{
		int	l,t,r,b;
		parent(msg,id,p1,p2);
		XPGetWidgetGeometry(id, &l,&t,&r,&b);
		XPLMSetGraphicsState(0,0,0,   0, 1,   0, 0);
		glColor4f(1.0, 0.0, 1.0, 0.5);
		XPGetWidgetGeometry(id, &l, &t, &r, &b);
		glBegin(GL_LINE_LOOP);
		glVertex2i(l,b);
		glVertex2i(l,t);
		glVertex2i(r,t);
		glVertex2i(r,b);
		glEnd();
		return 1;
	}
#endif

	if (msg == xpMsg_Create)
		XPSetWidgetProperty(id, xpProperty_WidgetClass, xpWidgetClass_Dialog);

	if (msg == xpMessage_CloseButtonPushed)
	{
		XPSendMessageToWidget(id, xpMsg_DialogDone, xpMode_Direct, xpDialog_ResultCancel, 0);
		return 1;
	}
	if (msg == xpMsg_DialogDone)
	{
		if (p1 == xpDialog_ResultOK)
		{
			XPSendMessageToWidget(id, xpMsg_DataFromDialog, xpMode_Recursive, 0, 0);
		}
		int cb = XPGetWidgetProperty(id, xpProperty_DialogDismissCB, NULL);
		if (cb != 0)
		{
			void (*func)(XPWidgetID, int) = reinterpret_cast<void(*)(XPWidgetID, int)>(cb);
			func(id, p1);
		}
		if (XPGetWidgetProperty(id, xpProperty_DialogSelfHide, NULL))
			XPHideWidget(id);
		if (XPGetWidgetProperty(id, xpProperty_DialogSelfDestroy, NULL))
			XPDestroyWidget(id, 1);
		return 1;
	}
	if (msg == xpMsg_Shown)
	{
		int meX = XPLayout_GetMinimumWidth(xpWidgetClass_Dialog, id);
		int meY = XPLayout_GetMinimumHeight(xpWidgetClass_Dialog, id);
		XPLayout_Reshape(xpWidgetClass_Dialog, id, meX, meY);

		XPSendMessageToWidget(id, xpMsg_DataToDialog, xpMode_Recursive, 0, 0);
		return 1;
	}
	if (msg == xpMsg_Hidden)
	{
		// Ben says: WTF was I thinking?!?  This is not what we want to do!!
//		XPSendMessageToWidget(id, xpMsg_DataFromDialog, xpMode_Recursive, 0, 0);
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeH)
	{
		XPWidgetID child = XPGetNthChildWidget(id, 0);
		if (child)
		{
			int child_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);
			XPLayout_GetContainMargins(xpWidgetClass_Dialog, child_type, ml, mt, mr, mb);
			XPSetWidgetProperty(id, xpProperty_MinWidth, XPLayout_GetMinimumWidth(0, child) + mr + ml);
		}
		return 1;
	}
	if (msg == xpMsg_RecalcMinSizeV)
	{
		XPWidgetID child = XPGetNthChildWidget(id, 0);
		if (child)
		{
			int child_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);
			XPLayout_GetContainMargins(xpWidgetClass_Dialog, child_type, ml, mt, mr, mb);
			XPSetWidgetProperty(id, xpProperty_MinHeight, XPLayout_GetMinimumHeight(0, child) + mt + mb);
		}
		return 1;
	}
	if (msg == xpMsg_DoReshape)
	{
		XPWidgetID child = XPGetNthChildWidget(id, 0);
		if (child)
		{
			int child_type = XPGetWidgetProperty(child, xpProperty_WidgetClass, 0);
			XPLayout_GetContainMargins(xpWidgetClass_Dialog, child_type, ml, mt, mr, mb);

			int width = XPGetWidgetProperty(id, xpProperty_MinWidth, 0) - ml - mr;
			int height = XPGetWidgetProperty(id, xpProperty_MinHeight, 0) - mt - mb;


			XPLayout_Reshape(0, child, width, height);
			int l, t, r, b;
			int cl, ct, cr, cb;
			XPGetWidgetGeometry(id, &l, &t, &r, &b);
			XPGetWidgetGeometry(child, &cl, &ct, &cr, &cb);

			XPSetWidgetGeometry(id, l, t, l+ml+mr+(cr-cl), t-mt-mb-(ct-cb));

			XPSetWidgetGeometry(child, l+ml,t-mt, l+ml+(cr-cl), t-mt-(ct-cb));
		}
		return 1;
	}
	return parent(msg, id, p1, p2);
}

#pragma mark -
/**********************************************************************
 * CONSTRUCTION HELPER
 **********************************************************************/

typedef hash_map<int, XPWidgetID(*)(XPWidgetID, va_list&)>	CreateHandlerTable;
static	CreateHandlerTable									sCreateHandlerTable;


XPWidgetID		XPCreateWidgetLayout(int dummy, ...)
{
	va_list		args;
	va_start	(args, dummy);

	char *				title;
	void *				func_ptr;
	XPWidgetID			new_widget = NULL;
	list<XPWidgetID>	widget_stack;
	int					prop;
	bool				done = false;
	while (!done)
	{
		int token = va_arg(args, int);
		switch(token) {
		case XP_TAG:
			prop = va_arg(args, int);
			if (new_widget)
				XPSetWidgetProperty(new_widget, xpProperty_DialogTag, prop);
			break;
		case XP_NOTIFY:
			func_ptr = va_arg(args, void *);
			if (new_widget)
				XPSetWidgetProperty(new_widget, xpProperty_NotifyPtr, (int) func_ptr);
			break;
		case XP_DIALOG_BOX:
			title = va_arg(args, char *);
			new_widget = XPCreateCustomWidget(50, 550, 100, 500, 0, title, 1, NULL, XPWF_DialogBox);
			prop = va_arg(args, int);
			if (prop == XP_DIALOG_CLOSEBOX)
			{
				XPSetWidgetProperty(new_widget, xpProperty_MainWindowHasCloseBoxes, 1);
				prop = va_arg(args, int);
			}
			XPSetWidgetProperty(new_widget, xpProperty_DialogSelfHide, prop);
			prop = va_arg(args, int);
			XPSetWidgetProperty(new_widget, xpProperty_DialogSelfDestroy, prop);
			func_ptr = va_arg(args, void *);
			XPSetWidgetProperty(new_widget, xpProperty_DialogDismissCB, (int) func_ptr);
			widget_stack.push_back(new_widget);
			break;
		case XP_ROW:
		case XP_COLUMN:
			new_widget = XPCreateCustomWidget(50, 550, 100, 500, 1, "", 0, widget_stack.back(), XPWF_RowColumn);
			if (token == XP_COLUMN)	XPSetWidgetProperty(new_widget, xpProperty_RowColumnIsVertical, 1);
			widget_stack.push_back(new_widget);
			break;
		case XP_END:
			if (widget_stack.size() <= 1)					done = true;
			else											widget_stack.pop_back();
			break;
		case XP_BUTTON_OK:
		case XP_BUTTON_CANCEL:
		case XP_BUTTON_ACTION:
			title = va_arg(args, char *);
			func_ptr = NULL;
				 if (token == XP_BUTTON_ACTION)			func_ptr = va_arg(args, void *);
			else if (token == XP_BUTTON_OK)				func_ptr = (void*)PBAction_DoneDialogOK;
			else if (token == XP_BUTTON_CANCEL)			func_ptr = (void*)PBAction_DoneDialogCancel;
			new_widget = XPCreateCustomWidget(50, 550, 150, 530, 1, title, 0, widget_stack.back(), XPWF_PushButton_Action);
			XPSetWidgetProperty(new_widget, xpProperty_DataPtr, (int) func_ptr);
			break;
		case XP_CAPTION:
			title = va_arg(args, char *);
			new_widget = XPCreateCustomWidget(50, 550,150, 530, 1, title, 0, widget_stack.back(), XPWF_Caption);
			break;
		case XP_EDIT_STRING:
		case XP_EDIT_INT:
		case XP_EDIT_FLOAT:
			if (token == XP_EDIT_STRING)	new_widget = XPCreateCustomWidget(50, 550, 150, 530, 1, "", 0, widget_stack.back(), XPWF_EditText_String);
			if (token == XP_EDIT_INT)		new_widget = XPCreateCustomWidget(50, 550, 150, 530, 1, "", 0, widget_stack.back(), XPWF_EditText_Int);
			if (token == XP_EDIT_FLOAT)		new_widget = XPCreateCustomWidget(50, 550, 150, 530, 1, "", 0, widget_stack.back(), XPWF_EditText_Float);
			prop = va_arg(args, int);				// max len
			if (prop == XP_EDIT_PASSWORD)
			{
				XPSetWidgetProperty(new_widget, xpProperty_PasswordMode, 1);
				prop = va_arg(args, int);				// max len
			}
			XPSetWidgetProperty(new_widget, xpProperty_MaxCharacters, prop);
			prop = va_arg(args, int);				// max len
			XPSetWidgetProperty(new_widget, xpProperty_FieldVisChars, prop);
			if (token == XP_EDIT_FLOAT)
			{
				prop = va_arg(args, int);				// precision
				XPSetWidgetProperty(new_widget, xpProperty_FieldPrecision, prop);
			}
			func_ptr = va_arg(args, void *);		// data ptr
			XPSetWidgetProperty(new_widget, xpProperty_DataPtr, (int) func_ptr);
			break;
		case XP_POPUP_MENU:
			title = va_arg(args, char *);
			new_widget = XPCreateCustomWidget(50, 550, 150, 530, 1, title, 0, widget_stack.back(), XPWF_Popup_Int);
			func_ptr = va_arg(args, void *);
			XPSetWidgetProperty(new_widget, xpProperty_DataPtr, (int) func_ptr);
			break;
		case XP_TABS:
			title = va_arg(args, char *);
			new_widget = XPCreateCustomWidget(50, 550, 150, 530, 1, title, 0, widget_stack.back(), XPWF_Tabs_IntShowHide);
			func_ptr = va_arg(args, void *);
			XPSetWidgetProperty(new_widget, xpProperty_DataPtr, (int) func_ptr);
			widget_stack.push_back(new_widget);
			break;
		case XP_CHECKBOX:
			title = va_arg(args, char *);
			new_widget = XPCreateCustomWidget(50, 550, 150, 530, 1, title, 0, widget_stack.back(), XPWF_CheckBox_Int);
			func_ptr = va_arg(args, void *);
			XPSetWidgetProperty(new_widget, xpProperty_DataPtr, (int) func_ptr);
			break;
		case XP_RADIOBUTTON:
			title = va_arg(args, char *);
			new_widget = XPCreateCustomWidget(50, 550, 150, 530, 1, title, 0, widget_stack.back(), XPWF_RadioButton_Int);
			func_ptr = va_arg(args, void *);
			XPSetWidgetProperty(new_widget, xpProperty_DataPtr, (int) func_ptr);
			prop = va_arg(args, int);
			XPSetWidgetProperty(new_widget, xpProperty_RadioButtonEnum, prop);
			break;
		default:
			if (sCreateHandlerTable.count(token) > 0)
			{
				new_widget = sCreateHandlerTable[token](widget_stack.back(), args);
				if (new_widget != NULL) widget_stack.push_back(new_widget);
			}
			break;
		}
	}
	return widget_stack.back();
}

XPWidgetID		XPFindWidgetByTag(XPWidgetID dialog, int tag)
{
	if (XPGetWidgetProperty(dialog, xpProperty_DialogTag, NULL) == tag)	return dialog;
	int n = XPCountChildWidgets(dialog);
	for (int i = 0; i < n; ++i)
	{
		XPWidgetID child = XPGetNthChildWidget(dialog, i);
		XPWidgetID found = XPFindWidgetByTag(child, tag);
		if (found != NULL) return found;
	}
	return NULL;
}

void			XPDataFromItem(XPWidgetID dialog, int tag)
{
	XPWidgetID who = XPFindWidgetByTag(dialog, tag);
	if (who)
		XPSendMessageToWidget(who, xpMsg_DataFromDialog, xpMode_Recursive, 0, 0);
}

void			XPDataToItem(XPWidgetID dialog, int tag)
{
	XPWidgetID who = XPFindWidgetByTag(dialog, tag);
	if (who)
		XPSendMessageToWidget(who, xpMsg_DataToDialog, xpMode_Recursive, 0, 0);
}

void			XPEnableByTag(XPWidgetID dialog, int tag, int enable)
{
	XPWidgetID who = XPFindWidgetByTag(dialog, tag);
	if (who)
		XPSetWidgetProperty(who, xpProperty_Enabled, enable);
}

void			XPInitDefaultMargins(void)
{
	// Basic rule: 20 pixel title bar on thetop of windows - make extra room for it!
	XPLayout_RegisterSpecialMarginsContains(xpWidgetClass_Dialog, 0,					10, 30, 10, 10);

	// Basic rule: 28 pixels of extra room for the etab bar on top of tabs.
	XPLayout_RegisterSpecialMarginsContains(xpWidgetClass_Tab, 0,						10, 38, 10, 10);

	// Special case: when a tab is inside a dialog box, remove the side margins and cheat on the top.
	XPLayout_RegisterSpecialMarginsContains(xpWidgetClass_Dialog, xpWidgetClass_Tab, 	0, 25, 0, 0);

	// Special case: when tabs are nested, remove the side margins and cheat on the top.
	XPLayout_RegisterSpecialMarginsContains(xpWidgetClass_Tab, xpWidgetClass_Tab, 		0, 35, 0, 0);
}
