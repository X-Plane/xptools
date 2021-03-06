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

#ifndef WED_XPLUGINMGR_H
#define WED_XPLUGINMGR_H


#include "WED_XPluginClient.h"
#include "WED_XPluginObject.h"
#include "WED_XPluginCamera.h"
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"
#include "XPLMDisplay.h"
#include "WED_NWDefs.h"


struct WED_XPluginStats_t
{
    void clear()
    {

    }
};


class  WED_XPluginMgr : public WED_XPluginClient
{
public:
             WED_XPluginMgr();
    virtual ~WED_XPluginMgr();

        void    SetPackage(const string& inPackage){mPackage=inPackage;}
        string  GetPackage(){return mPackage;}
        string  GetPackagePath();
        XPLMProbeRef GetProbeRef(){return mProbeRef;}

    	void 	Add(int inId,int inType,const vector<string>& inArgs);
    	void	Chg(int inId,int inType,const vector<string>& inArgs);
		void	Del(int inId);
		void 	Sync();

        bool    IsEnabledCam(){return mCamera.IsEnabled();}
        void    EnableCam(bool inEnable);
        void    UpdateCam(int inType,const vector<string>& inArgs);

    WED_XPluginEntity * GetbyId(int inId);
	static int	WEDXPluginDrawObjCB(XPLMDrawingPhase inPhase,int inIsBefore,void * inRefcon);

    //WED_XPluginStats_t * GetStats(){return &mEntityStats;}

protected:
private:

   string                           mPackage;

   XPLMProbeRef		    			mProbeRef;
   XPLMDataRef						mLiteLevelRef;
   bool								mIsLit;
   WED_XPluginStats_t	    		mStats;
   map<int,WED_XPluginEntity *>  	mEntities;

   WED_XPluginCamera                mCamera;

};

#endif // WED_XPLUGINMGR_H
