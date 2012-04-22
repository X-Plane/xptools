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

#include "WED_XPluginMgr.h"
#include "WED_XPluginFacNode.h"
#include "WED_XPluginFacRing.h"

#include "stdio.h"


WED_XPluginFacNode::WED_XPluginFacNode(WED_XPluginMgr * inRef,const vector<string>& inArgs):
    WED_XPluginBezierNode(nw_obj_FacadeNode,inRef)
{
    this->Update(inArgs);
}
WED_XPluginFacNode::WED_XPluginFacNode(WED_XPluginMgr * inRef):
    WED_XPluginBezierNode(nw_obj_FacadeNode,inRef)
{

}

WED_XPluginFacNode::~WED_XPluginFacNode()
{
    WED_XPluginFacRing * ring = dynamic_cast<WED_XPluginFacRing *>(mParent) ;
    if(ring != NULL)     ring->DelNode(this);
}

void WED_XPluginFacNode::SceneryShift()
{
    WED_XPluginBezierNode::SceneryShift();
    if(mIdx < 2 && mParent != NULL) mParent->UpdatePos();
}

void   WED_XPluginFacNode::Update(const vector<string>& inArgs)
{
    int pid,dummy = 0;

    bool propchange = false;
    bool poschange  = false;

    if (inArgs.size() == 5 || inArgs.size() ==10)
    {
        sscanf(inArgs[3].c_str(),"%lf",&mLon)      ;
        sscanf(inArgs[4].c_str(),"%lf",&mLat)      ;
        if(inArgs.size() == 10)
        {
            sscanf(inArgs[5].c_str(),"%d",&pid)    ;
            sscanf(inArgs[6].c_str(),"%d",&mIdx)   ;
            sscanf(inArgs[7].c_str(),"%d",&mParam1);
            sscanf(inArgs[8].c_str(),"%d",&dummy)  ;
            SetName(inArgs[9]);
            propchange=true;
        }
        mHasBez = false;
        poschange = true;
    }
    else if(inArgs.size() == 9 || inArgs.size() == 14)
    {
        sscanf(inArgs[3].c_str(),"%lf",&mLon)     ;
        sscanf(inArgs[4].c_str(),"%lf",&mLat)     ;
        sscanf(inArgs[5].c_str(),"%lf",&mLon_hi)  ;
        sscanf(inArgs[6].c_str(),"%lf",&mLat_hi)  ;
        sscanf(inArgs[7].c_str(),"%lf",&mLon_lo)  ;
        sscanf(inArgs[8].c_str(),"%lf",&mLat_lo)  ;
        mHasBez= true;
        poschange = true;
        if(inArgs.size() == 14)
        {
            sscanf(inArgs[9].c_str(),"%d" ,&pid)  ;
            sscanf(inArgs[10].c_str(),"%d",&mIdx) ;
            sscanf(inArgs[11].c_str(),"%d",&mParam1);
            sscanf(inArgs[12].c_str(),"%d",&dummy);
            SetName(inArgs[13]);
            propchange = true;
        }
    }

    if (propchange)
    {

        if(!mParent || mParent->GetType() != nw_obj_FacadeRing)
        {
            mParent = mMgrRef->GetbyId(pid);
            if(!mParent || mParent->GetType() != nw_obj_FacadeRing)
            {
                vector<string> args;
                mMgrRef->Add(pid,nw_obj_FacadeRing,args);
                mMgrRef->SendData(WED_NWP_GET,nw_obj_FacadeRing,pid,"");
                mParent = (mMgrRef->GetbyId(pid));
            }

            WED_XPluginFacRing * ring = dynamic_cast<WED_XPluginFacRing *>(mParent);
            if(ring != NULL)     ring->AddNode(this);
        }
    }

    if (poschange)
    {
        SetToTerrain(true);
        UpdatePos();
        if(mParent != NULL) mParent->UpdatePos();
    }
}

///////////////////////////////////////////////////////////////////////////////////
// drawing
void WED_XPluginFacNode::Draw(bool isLit)
{
    WED_XPluginBezierNode::Draw(isLit);
}

