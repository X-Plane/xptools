/* 
 * Copyright (c) 2004, Ben Supnik and Sandy Barbour.
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
#include "XPLMMenus.h"
#include "XPWidgetWin.h"

struct	MenuInfo_t {

	xmenu				menu;

	XPLMMenuHandler_f	handler;
	void *				handlerRef;
	vector<void *>		itemRefs;
	
};

typedef	set<MenuInfo_t *>	MenuSet;
static	MenuSet	sMenus;

#if APL
static MenuID	gIDs = 1000;
#endif
#if IBM
static int		gIDs = 1000;
#endif

#if APL
static	void		StripAndPascal(unsigned char * foo)
{
	for (int n = 1; n <= foo[0]; ++n)
	{
		if (foo[n] == '&')
		{		
			memmove(foo+n,foo+n+1,foo[0] - n);
			foo[0]--;
			return;
		}
	}
}
#endif

#if IBM
void	RegisterAccel(const ACCEL& inAccel);
#endif

XPLMMenuID           XPLMFindPluginsMenu(void)
{
	return NULL;
}

XPLMMenuID           XPLMCreateMenu(
                                   const char *         inName,    
                                   XPLMMenuID           inParentMenu,    
                                   int                  inParentItem,    
                                   XPLMMenuHandler_f    inHandler,    
                                   void *               inMenuRef)
{
	MenuInfo_t * pMenu = new MenuInfo_t;
	pMenu->handler = inHandler;
	pMenu->handlerRef = inMenuRef;
	
#if APL
	::CreateNewMenu(gIDs++, kMenuAttrAutoDisable, &pMenu->menu);
	::MacInsertMenu(pMenu->menu, (inParentMenu == NULL) ? 0 : kInsertHierarchicalMenu);
	
	Str255	pStr;
	pStr[0] = strlen(inName);
	memcpy(pStr+1, inName, pStr[0]);
	StripAndPascal(pStr);
	::SetMenuTitle(pMenu->menu, pStr);
	
	if (inParentMenu)
	{
		xmenu	pmr = ((MenuInfo_t *) inParentMenu)->menu;
		::SetMenuItemHierarchicalID(pmr, inParentItem + 1, ::GetMenuID(pMenu->menu));
	}
#endif
#if IBM
	xmenu parent = (inParentMenu) ? 
		(((MenuInfo_t *) inParentMenu)->menu) :
		(gWidgetWin->GetMenuBar());

	pMenu->menu = CreateMenu();

	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.hSubMenu = pMenu->menu;
	mif.fType = MFT_STRING;
	mif.dwTypeData = const_cast<char *>(inName);
	mif.fMask = (inParentMenu) ? MIIM_SUBMENU : (MIIM_TYPE | MIIM_SUBMENU);

	if (inParentMenu == NULL)
	{
		InsertMenuItem(parent, -1, true, &mif);
	} else {
		SetMenuItemInfo(parent, inParentItem, true, &mif);
		
	}		
	
#endif

	sMenus.insert(pMenu);
	return pMenu;	
}                                    

void                 XPLMDestroyMenu(
                                   XPLMMenuID           inMenuID)
{
	MenuInfo_t * pMenu = (MenuInfo_t *) inMenuID;
	
#if APL
	MacDeleteMenu(::GetMenuID(pMenu->menu));
	::ReleaseMenu(pMenu->menu);
#endif
#if IBM
	DestroyMenu(pMenu->menu);
#endif	

	sMenus.erase(pMenu);
	delete pMenu;
}                                   

void                 XPLMClearAllMenuItems(
                                   XPLMMenuID           inMenuID)
{
	MenuInfo_t * pMenu = (MenuInfo_t *) inMenuID;
	pMenu->itemRefs.clear();
#if APL
	while (::CountMenuItems(pMenu->menu) > 0)
		::DeleteMenuItem(pMenu->menu, 1);
#endif
#if IBM
	while (::GetMenuItemCount(pMenu->menu) > 0)
		::RemoveMenu(pMenu->menu, 0, MF_BYPOSITION);
#endif	
}                                   


int                  XPLMAppendMenuItem(
                                   XPLMMenuID           inMenu,    
                                   const char *         inItemName,    
                                   void *               inItemRef,    
                                   int                  inForceEnglish)
{
	MenuInfo_t * pMenu = (MenuInfo_t *) inMenu;
#if APL
	Str255	pStr;
	pStr[0] = strlen(inItemName);
	memcpy(pStr+1,inItemName, pStr[0]);
	StripAndPascal(pStr);
	::AppendMenuItemText(
		pMenu->menu, pStr);	
		
	SetMenuItemCommandID(pMenu->menu, pMenu->itemRefs.size() + 1, 1000);
		
#endif
#if IBM
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	if (!strcmp(inItemName, "-"))
	{
		mif.fType = MFT_SEPARATOR;
		mif.fMask = MIIM_TYPE;	
	} else {
		mif.fType = MFT_STRING;
		mif.dwTypeData = const_cast<char *>(inItemName);
		mif.fMask = MIIM_TYPE | MIIM_ID;
		mif.wID = gIDs++;
	}
	InsertMenuItem(pMenu->menu, -1, true, &mif);
#endif

	pMenu->itemRefs.push_back(inItemRef);
	return pMenu->itemRefs.size() - 1;
}                                

XPLM_API void				  XPLMSetMenuItemKey(
								   XPLMMenuID			inMenu,
								   int					inItem,
								   char					inKey,
								   XPLMKeyFlags			inFlags)
{
#if APL
	MenuInfo_t *	pMenu = (MenuInfo_t *)	inMenu;
	switch(inKey) {
	case XPLM_KEY_UP:
		SetMenuItemKeyGlyph(pMenu->menu,inItem+1, kMenuUpArrowGlyph);
		break;
	case XPLM_KEY_DOWN:
		SetMenuItemKeyGlyph(pMenu->menu,inItem+1, kMenuDownArrowGlyph);
		break;
	case XPLM_KEY_RIGHT:
		SetMenuItemKeyGlyph(pMenu->menu,inItem+1, kMenuRightArrowGlyph);
		break;
	case XPLM_KEY_LEFT:
		SetMenuItemKeyGlyph(pMenu->menu,inItem+1, kMenuLeftArrowGlyph);
		break;
	case XPLM_KEY_DELETE:
		SetMenuItemKeyGlyph(pMenu->menu,inItem+1, kMenuDeleteLeftGlyph);
		break;
	case XPLM_KEY_RETURN:
		SetMenuItemKeyGlyph(pMenu->menu,inItem+1, kMenuReturnGlyph);
		break;
	default:
		::SetItemCmd(pMenu->menu, inItem+1, inKey);
		break;
	}

	::SetMenuItemModifiers(pMenu->menu, inItem+1,
			((inFlags & xplm_ShiftFlag) ? kMenuShiftModifier : 0) +
			((inFlags & xplm_OptionAltFlag) ? kMenuOptionModifier : 0) +
			((inFlags & xplm_ControlFlag) ? 0 : kMenuNoCommandModifier));		
	
#endif

#if IBM
	MenuInfo_t * pMenu = (MenuInfo_t *) inMenu;

	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.dwTypeData = NULL;
	mif.fMask = MIIM_TYPE | MIIM_ID;
	mif.fType = MFT_STRING;
	GetMenuItemInfo(pMenu->menu, inItem, true, &mif);
	string	foo;
	foo.resize(mif.cch);
	mif.fMask = MIIM_TYPE;
	mif.fType = MFT_STRING;
	mif.cch++;
	mif.dwTypeData = const_cast<char *>(foo.c_str());
	GetMenuItemInfo(pMenu->menu, inItem, true, &mif);

	foo += "\t";
	
	ACCEL	acc;
	acc.cmd = mif.wID;
	acc.fVirt = FVIRTKEY;

	if (inFlags & xplm_ShiftFlag)
	{
		foo += "Shift+";
		acc.fVirt |= FSHIFT;
	}
	if (inFlags & xplm_OptionAltFlag)
	{
		foo += "Alt+";
		acc.fVirt |= FALT;
	}
	if (inFlags & xplm_ControlFlag)
	{
		foo += "Ctrl+";
		acc.fVirt |= FCONTROL;
	}

	switch(inKey) {
	case '[':
		acc.key = VK_OEM_4;
		foo += "[";
		break;
	case ']':
		foo += "]";
		acc.key = VK_OEM_6;
		break;
	case XPLM_KEY_UP:
		foo += "Up";
		acc.key = VK_UP;
		break;
	case XPLM_KEY_DOWN:
		foo += "Down";
		acc.key = VK_DOWN;
		break;
	case XPLM_KEY_RIGHT:
		foo += "Right";
		acc.key = VK_RIGHT;
		break;
	case XPLM_KEY_LEFT:
		foo += "Left";
		acc.key = VK_LEFT;
		break;
	case XPLM_KEY_DELETE:
		foo += "Del";
		acc.key = VK_BACK;
		break;
	case XPLM_KEY_RETURN:
		acc.key = VK_RETURN;
		foo += "Enter";
		break;
	default:
		acc.key = inKey;	
		foo += inKey;
		break;
	}
	
	mif.fMask = MIIM_TYPE;
	mif.fType = MFT_STRING;
	mif.dwTypeData = const_cast<char *>(foo.c_str());
	SetMenuItemInfo(pMenu->menu, inItem, true, &mif);

	RegisterAccel(acc);
#endif
}

void                 XPLMAppendMenuSeparator(
                                   XPLMMenuID           inMenu)
{
#if APL
	XPLMAppendMenuItem(inMenu, "-", 0, 1);
#endif
#if IBM
	MenuInfo_t * pMenu = (MenuInfo_t *) inMenu;
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fType = MFT_SEPARATOR;
	mif.fMask = MIIM_TYPE;
	InsertMenuItem(pMenu->menu, -1, true, &mif);
	pMenu->itemRefs.push_back(NULL);
#endif	
}                                   

void                 XPLMSetMenuItemName(
                                   XPLMMenuID           inMenu,    
                                   int                  inIndex,    
                                   const char *         inItemName,    
                                   int                  inForceEnglish)
{
	MenuInfo_t * pMenu = (MenuInfo_t *) inMenu;
#if APL
	Str255 pStr;
	pStr[0] = strlen(inItemName);
	memcpy(pStr+1, inItemName, pStr[0]);
	StripAndPascal(pStr);
	::SetMenuItemText(pMenu->menu, inIndex+1, pStr);	
#endif
#if IBM
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fType = MFT_STRING;
	mif.fMask = MIIM_TYPE;
	mif.dwTypeData = const_cast<char *>(inItemName);
	SetMenuItemInfo(pMenu->menu, inIndex, true, &mif);
#endif	
}                                   

void                 XPLMCheckMenuItem(
                                   XPLMMenuID           inMenu,    
                                   int                  index,    
                                   XPLMMenuCheck        inCheck)
{
	MenuInfo_t * pMenu = (MenuInfo_t *) inMenu;
#if APL
	::MacCheckMenuItem(pMenu->menu, index+1, (inCheck == xplm_Menu_Checked) ? 1 : 0);
#endif
#if IBM
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fMask = MIIM_STATE;
	GetMenuItemInfo(pMenu->menu, index, true, &mif);
	if (inCheck == xplm_Menu_Checked)
		mif.fState |= MFS_CHECKED;
	else
		mif.fState &= ~MFS_CHECKED;	
	SetMenuItemInfo(pMenu->menu, index, true, &mif);
#endif	
}                                   

void                 XPLMEnableMenuItem(
                                   XPLMMenuID           inMenu,    
                                   int                  index,    
                                   int                  enabled)
{
	MenuInfo_t * pMenu = (MenuInfo_t *) inMenu;
#if APL
	if (enabled)
		::MacEnableMenuItem(pMenu->menu, index+1);
	else
		::DisableMenuItem(pMenu->menu, index+1);	
#endif
#if IBM
	MENUITEMINFO	mif = { 0 };
	mif.cbSize = sizeof(mif);
	mif.fMask = MIIM_STATE;
	GetMenuItemInfo(pMenu->menu, index, true, &mif);
	if (!enabled)
		mif.fState |= MFS_GRAYED;
	else
		mif.fState &= ~MFS_GRAYED;	
	SetMenuItemInfo(pMenu->menu, index, true, &mif);

#endif	
}

int	DispatchMenuCmd(xmenu menuH, int zIndex)
{
#if APL
	for (MenuSet::iterator iter = sMenus.begin(); iter != sMenus.end(); ++iter)
	{
		MenuInfo_t *	pMenu = *iter;
		if (pMenu->menu == menuH)
		{
			if (pMenu->handler)
				pMenu->handler(pMenu->handlerRef, pMenu->itemRefs[zIndex]);
			return 1;
		}
	}
	return 0;
#endif	

#if IBM
	for (MenuSet::iterator iter = sMenus.begin(); iter != sMenus.end(); ++iter)
	{
		MenuInfo_t *	pMenu = *iter;	
		for (int i = 0; i < pMenu->itemRefs.size(); ++i)
		{
			MENUITEMINFO	mif = { 0 };
			mif.cbSize = sizeof(mif);
			mif.fMask = MIIM_ID;
			GetMenuItemInfo(pMenu->menu, i, true, &mif);
			if (mif.wID == zIndex)
			{
				if (pMenu->handler)
					pMenu->handler(pMenu->handlerRef, pMenu->itemRefs[i]);
				return 1;
			}
		}
	}
	return 0;
#endif
}