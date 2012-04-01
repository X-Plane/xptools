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

#ifndef WED_XPLUGINENTITY_H
#define WED_XPLUGINENTITY_H

#include "XPLMScenery.h"
#include <map>
#include <string>
#include <vector>

using namespace std ;

class WED_XPluginMgr;

class WED_XPluginEntity
{
public:

    WED_XPluginEntity(int inType,XPLMProbeRef inProbeRef,WED_XPluginMgr * inRef);
    virtual ~WED_XPluginEntity();

    virtual void    Draw(bool isLit);
    virtual int     GetType(){return mType;}
    virtual void    SetResPath(const string& inPath){mResPath=inPath;}
    virtual string  GetResPath(){return mResPath;}
    virtual void    SetName(const string& inName){mName=inName;}
    virtual string  GetName(){return mName;}
    virtual void    SetLat(double inLat){mLat = inLat;}
    virtual void    SetLon(double inLon){mLon = inLon;}
    virtual void    SetAlt(double inAlt){mAlt = inAlt;}
    virtual void    GetLoc(double * outLat,double * outLon,double * outAlt);
    virtual void    GetPos(double * outX,double * outY,double * outZ);
    virtual void    SetHdg(double inHdg){mHdg = inHdg;}
    virtual void    SetToTerrain(bool inToTerrain){mSetToTerrain = inToTerrain;}

private:

    friend class WED_XPluginObject;
    friend class WED_XPluginFacade;
    friend class WED_XPluginFacNode;
    friend class WED_XPluginFacRing;

    int					mType;
    string 			 	mName;
    string			 	mResPath;
    WED_XPluginMgr * 	mMgrRef;
    XPLMProbeRef	 	mProbeRef;

    double				mLon;
    double				mLat;
    double			 	mAlt;
    float				mHdg;

    bool				mSetToTerrain;

    static void	WorldToLocal(double * outX,double * outY,double * outZ,WED_XPluginEntity* inEntity);
};

#endif // WED_XPLUGINENTITY_H
