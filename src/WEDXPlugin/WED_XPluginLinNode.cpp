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

#include "WED_XPluginLinNode.h"
#include "WED_XPluginLine.h"
#include "WED_XPluginMgr.h"
#include "stdio.h"


WED_XPluginLinNode::WED_XPluginLinNode(WED_XPluginMgr * inRef,const vector<string>& inArgs):
    WED_XPluginBezierNode(nw_obj_LineNode,inRef)
{
    this->Update(inArgs);
}
WED_XPluginLinNode::WED_XPluginLinNode(WED_XPluginMgr * inRef):
    WED_XPluginBezierNode(nw_obj_LineNode,inRef)
{

}

WED_XPluginLinNode::~WED_XPluginLinNode()
{
    WED_XPluginLine * line = dynamic_cast<WED_XPluginLine *>(mParent);
    if(line != NULL)  line->DelNode(this);
}

void   WED_XPluginLinNode::Update(const vector<string>& inArgs)
{
    int pid = 0;
    bool propchange = false;

    if (inArgs.size() == 5 || inArgs.size() == 8)
    {
        sscanf(inArgs[3].c_str(),"%lf",&mLon);
        sscanf(inArgs[4].c_str(),"%lf",&mLat);
        if(inArgs.size() == 8)
        {
            sscanf(inArgs[5].c_str(),"%d",&pid);
            sscanf(inArgs[6].c_str(),"%d",&mIdx);
            SetName(inArgs[7]);
            propchange=true;
        }
        mHasBez = false;
        SetToTerrain(true);
        UpdatePos();
    }
    else if(inArgs.size() == 9 || inArgs.size() == 12)
    {
        sscanf(inArgs[3].c_str(),"%lf",&mLon)   ;
        sscanf(inArgs[4].c_str(),"%lf",&mLat)   ;
        sscanf(inArgs[5].c_str(),"%lf",&mLon_hi);
        sscanf(inArgs[6].c_str(),"%lf",&mLat_hi);
        sscanf(inArgs[7].c_str(),"%lf",&mLon_lo);
        sscanf(inArgs[8].c_str(),"%lf",&mLat_lo);
        mHasBez = true;
        SetToTerrain(true);
        UpdatePos();
        if(inArgs.size() == 12)
        {
            sscanf(inArgs[9].c_str(),"%d" ,&pid);
            sscanf(inArgs[10].c_str(),"%d",&mIdx);
            SetName(inArgs[11]);
            propchange = true;
        }
    }

    if (propchange)
    {
        if(!mParent || mParent->GetType() != nw_obj_Line)
        {
            mParent = mMgrRef->GetbyId(pid);
            if(!mParent || mParent->GetType() != nw_obj_Line)
            {
                vector<string> args;
                mMgrRef->Add(pid,nw_obj_Line,args);
                mMgrRef->SendData(WED_NWP_GET,nw_obj_Line,pid,"");
                mParent = mMgrRef->GetbyId(pid);
            }

            WED_XPluginLine * line = dynamic_cast<WED_XPluginLine *>(mParent);
            if(line != NULL)  line->AddNode(this);
        }
    }
}
