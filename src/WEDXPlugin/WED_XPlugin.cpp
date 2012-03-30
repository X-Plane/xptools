/*
 * Copyright (c) 2011, mroe.
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

#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "WED_XPluginMgr.h"
#include "WED_XPluginClient.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char WEDXPluginVersNumber[] = "v0.1d2 (development version)";

WED_XPluginMgr *	gWEDXPluginMgr	= NULL;

// Used by menus
int	 		WEDXPluginItem;
XPLMMenuID	WEDXPluginMenuId;

int WEDXPluginXPLMVers;
int WEDXPluginXPlaneVers;

XPLMHostApplicationID WEDXPluginXHostID;

//  text window
XPLMWindowID WEDXPluginWindow;

// flightloop callback
float WEDXPluginInitCB(float elapsedMe, float elapsedSim, int counter, void * refcon);

// Menu Prototype
void WEDXPluginMenuHandler(void *, void *);

// Draw Text window callbacks
void WEDXPluginWindowDraw(XPLMWindowID inWindowID, void * inRefcon);
void WEDXPluginWindowKey(XPLMWindowID inWindowID, char inKey,XPLMKeyFlags inFlags,
                         char vkey, void * inRefcon, int losingFocus);
int	 WEDXPluginWindowMouse(XPLMWindowID inWindowID, int x, int y,
                           XPLMMouseStatus isDown, void * inRefcon);
int	 WEDXPluginWindowMouseWheel(XPLMWindowID	inWindowID,int x,int y,
                                int wheel,int clicks, void * inRefcon);
XPLMCursorStatus WEDXPluginWindowCursor(XPLMWindowID inWindowID,
                                        int x,int y,void * inRefcon);

#if DEBUG
// Callback for Error Tests
void WEDXPluginErrorCB(const char * msg)
{
    XPLMDebugString("xpdo: ERROR_CB called \n: ");
    XPLMDebugString(msg);
    XPLMDebugString("\n");
}
#endif

PLUGIN_API int XPluginStart(char *	outName,char *	outSig,char *outDesc)
{
    strcpy(outName, "WEDXPlugin");
    strcpy(outSig,  "wed.link.xplane");
    sprintf(outDesc,"WED View, %s ",WEDXPluginVersNumber );

    XPLMGetVersions(&WEDXPluginXPlaneVers,&WEDXPluginXPLMVers,&WEDXPluginXHostID);
    //check version
    if ((WEDXPluginXPlaneVers < 900)||(WEDXPluginXPLMVers < 200))
    {
        printf("xpdo ERROR : only X-Plane 9.0 / XPSDK 2.0 or greater supported\n");
        return 0;
    }

    // Create the menus
    WEDXPluginItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(),"WEDPlugin", NULL, 1);
    WEDXPluginMenuId = XPLMCreateMenu("WEDPlugin",
                                      XPLMFindPluginsMenu(),
                                      WEDXPluginItem,
                                      WEDXPluginMenuHandler,
                                      NULL);

    XPLMAppendMenuItem(WEDXPluginMenuId, "Reload Scenery", (void *)"ReloadScenery", 1);
    XPLMAppendMenuItem(WEDXPluginMenuId, "Reload Plugins", (void *)"Reload plugins",1);
    XPLMAppendMenuSeparator(WEDXPluginMenuId);
    XPLMAppendMenuItem(WEDXPluginMenuId, "Toggle Connect",(void *)"ToggleConnect", 1);
    XPLMAppendMenuItem(WEDXPluginMenuId, "View from WED ",(void *)"ViewFromWED", 1);
    //XPLMEnableMenuItem (WEDXPluginMenuId,4,false);

    // This is used to create a text window using the new XPLMCreateWindowEx function.
    XPLMCreateWindow_t WEDXPluginWinData;
    WEDXPluginWinData.structSize = sizeof(WEDXPluginWinData);
    WEDXPluginWinData.left = 2;	// <- 275 ->
    WEDXPluginWinData.top = 50;   //          |
    WEDXPluginWinData.right = 277; //         100
    WEDXPluginWinData.bottom = 10; //		    |
    WEDXPluginWinData.visible= 1;
    WEDXPluginWinData.drawWindowFunc		= WEDXPluginWindowDraw;
    WEDXPluginWinData.handleKeyFunc			= WEDXPluginWindowKey;
    WEDXPluginWinData.handleMouseClickFunc	= WEDXPluginWindowMouse;
    WEDXPluginWinData.handleMouseWheelFunc	= WEDXPluginWindowMouseWheel;
    WEDXPluginWinData.handleCursorFunc		= WEDXPluginWindowCursor;
    WEDXPluginWinData.refcon = NULL;
    WEDXPluginWindow = XPLMCreateWindowEx(&WEDXPluginWinData);

    XPLMSetWindowIsVisible( WEDXPluginWindow,true);

#if DEBUG
    // Register the callback for errors
    XPLMSetErrorCallback(WEDXPluginErrorCB);
#endif
    XPLMRegisterFlightLoopCallback(WEDXPluginInitCB, 1.0, NULL);
    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    XPLMDestroyMenu(WEDXPluginMenuId);
    XPLMUnregisterFlightLoopCallback(WEDXPluginInitCB, NULL);
    XPLMDestroyWindow(WEDXPluginWindow);
}

PLUGIN_API int XPluginEnable(void)
{
    XPLMCheckMenuItem(WEDXPluginMenuId,3,xplm_Menu_Unchecked);
    XPLMCheckMenuItem(WEDXPluginMenuId,4,xplm_Menu_Unchecked);
    gWEDXPluginMgr = new WED_XPluginMgr();
    return 1;
}

PLUGIN_API void XPluginDisable(void)
{
    XPLMCheckMenuItem(WEDXPluginMenuId,3,xplm_Menu_Unchecked);
    XPLMCheckMenuItem(WEDXPluginMenuId,4,xplm_Menu_Unchecked);
    delete gWEDXPluginMgr;
    gWEDXPluginMgr = NULL;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void * inParam)
{
    if (inFrom == XPLM_PLUGIN_XPLANE)
        if ((inMsg == XPLM_MSG_SCENERY_LOADED))//||(inMsg == XPLM_MSG_PLANE_LOADED))
        {
            ;
        }
}

float WEDXPluginInitCB(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
    //FIXME : this CB is processed once after scenery is loaded.
    // leave it in for some future use
    return 0;
}

// Process the menu selections
void WEDXPluginMenuHandler(void * mRef, void * iRef)
{

    XPLMMenuCheck acheck;

    if (!strcmp((char *) iRef, "Reload plugins"))
    {
        XPLMReloadPlugins();
    }
    else
    if (!strcmp((char *) iRef, "ReloadScenery"))
    {
        XPLMReloadScenery();

    }
    else
    if (!strcmp((char *) iRef, "ToggleConnect"))
    {
        XPLMCheckMenuItemState(WEDXPluginMenuId,3,&acheck);
        switch (acheck)
        {
        case xplm_Menu_Checked    :

            if(gWEDXPluginMgr)gWEDXPluginMgr->Disconnect();
            XPLMCheckMenuItem( WEDXPluginMenuId,3,xplm_Menu_Unchecked);
            break;

        case xplm_Menu_Unchecked	:

            if(gWEDXPluginMgr)gWEDXPluginMgr->Connect();
            XPLMCheckMenuItem( WEDXPluginMenuId,3,xplm_Menu_Checked);
            break;
        }
    }
    else
    if (!strcmp((char *) iRef, "ViewFromWED"))
    {
        if(!gWEDXPluginMgr) return;

        if(gWEDXPluginMgr->IsEnabledCam())
        {
            if(gWEDXPluginMgr)gWEDXPluginMgr->EnableCam(false);
            XPLMCheckMenuItem( WEDXPluginMenuId,4,xplm_Menu_Unchecked);
        }
        else
        {
            if(gWEDXPluginMgr)gWEDXPluginMgr->EnableCam(true);
            XPLMCheckMenuItem( WEDXPluginMenuId,4,xplm_Menu_Checked);
        }
    }

}

// Draw statistic window
void	WEDXPluginWindowDraw(XPLMWindowID inWindowID, void * inRefcon)
{
    float		rgb1 [] = { 0.8, 0.8, 0.8 };
    float		rgb2 [] = { 0.0, 0.5, 0.8 };

    int			Left, Top, Right, Bottom;
    char 		Buffer[256];

    string status = "";
    if(gWEDXPluginMgr) status = gWEDXPluginMgr->GetStatus();

    XPLMGetWindowGeometry(inWindowID, &Left, &Top, &Right, &Bottom);
    XPLMDrawTranslucentDarkBox(Left, Top, Right, Bottom);

    sprintf(Buffer,"  WEDXPlugin %s ",WEDXPluginVersNumber);
    XPLMDrawString(rgb2, Left+10, Top-10, Buffer, NULL, xplmFont_Basic);
    sprintf(Buffer,"   %s",status.c_str());
    XPLMDrawString(rgb1, Left+10, Top-30, Buffer, NULL, xplmFont_Basic);

}

// Not used
void WEDXPluginWindowKey(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char vkey, void * inRefcon, int losingFocus)
{
}

// Not used
int	WEDXPluginWindowMouse(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus isDown, void * inRefcon)
{
    return 0;
}

// Not used
int	WEDXPluginWindowMouseWheel(XPLMWindowID inWindowID,
                               int            x,
                               int            y,
                               int            wheel,
                               int            clicks,
                               void *         inRefcon)
{
    return 0;
}

// This will change the type of cursor
XPLMCursorStatus WEDXPluginWindowCursor(XPLMWindowID inWindowID,
                                        int           x,
                                        int           y,
                                        void *        inRefcon)
{
    return xplm_CursorDefault;
}

//---------------------------------------------------------------------------
