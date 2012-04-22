/*
 * Copyright (c) 2012, mroe.
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
#ifndef WED_XPLUGINFACADE_H
#define WED_XPLUGINFACADE_H

#include "WED_XPluginNode.h"

class WED_XPluginMgr;
class WED_XPluginFacRing;

class WED_XPluginFacade : public WED_XPluginNode
{
public:

    WED_XPluginFacade(WED_XPluginMgr * inRef,const vector<string>& inArgs);
    WED_XPluginFacade(WED_XPluginMgr * inRef);
    virtual ~WED_XPluginFacade();

    enum fac_topo
    {
        topo_unknown = -1,
        topo_Area    =  0,
        topo_Ring    =  1,
        topo_Chain   =  2
    };

    void     Draw(bool isLit);
    void     UpdatePos();
    void     Update(const vector<string>& inArgs);
    void     SceneryShift(){;}

    void     SetTopo(int inTopo){mTopo = inTopo;}
    void     SetHeight(int inHeight){mHeight = inHeight;}
    void     SetHasWall(int inHasWall){mHasWall = inHasWall;}

    int      SetRessource(const string& inPath);
    void     AddRing(WED_XPluginFacRing * inRing);
    void     DelRing(WED_XPluginFacRing * inRing);
    int      GetRingCnt(){return mRings.size();}
    WED_XPluginFacRing * GetRingAt(int n){return mRings.at(n);}

protected:
private:

    float     mHeight;
    int       mHasWall;
    int       mTopo;

    vector<WED_XPluginFacRing *> mRings;
};

#endif // WED_XPLUGINFACADE_H
