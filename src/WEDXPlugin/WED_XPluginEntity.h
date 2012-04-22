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

#include <string>
#include <vector>

using  std::vector ;
using  std::string ;

class WED_XPluginMgr;

class WED_XPluginEntity
{
public:

    WED_XPluginEntity(int inType,WED_XPluginMgr * inRef);
    virtual ~WED_XPluginEntity();

    virtual void        Draw(bool isLit){;}
    virtual void        UpdatePos(){;}
    virtual void        SceneryShift(){;}
    virtual void        SetToTerrain(bool inToTerrain){;}

    int                 GetType(){return mType;}
    void                SetResPath(const string& inPath){mResPath=inPath;}
    string              GetResPath(){return mResPath;}
    void                SetName(const string& inName){mName=inName;}
    string              GetName(){return mName;}
    void                SetParent(WED_XPluginEntity * inEntity){mParent=inEntity;}
    WED_XPluginEntity * GetParent(){return mParent;}
    void                SetIdx(unsigned int inIdx){mIdx = inIdx;}
    unsigned int        GetIdx(){return mIdx;}

private:

    friend class        WED_XPluginObject;
    friend class        WED_XPluginFacade;
    friend class        WED_XPluginFacNode;
    friend class        WED_XPluginFacRing;
    friend class        WED_XPluginLine;
    friend class        WED_XPluginLinNode;

    unsigned int        mIdx;

    int					mType;
    string 			 	mName;
    string			 	mResPath;
    WED_XPluginMgr * 	mMgrRef;
    WED_XPluginEntity * mParent;

};

#endif // WED_XPLUGINENTITY_H
