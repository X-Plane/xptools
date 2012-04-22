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

#include "WED_XPluginFacRing.h"
#include "WED_XPluginFacade.h"
#include "WED_XPluginMgr.h"


#include "stdio.h"

WED_XPluginFacRing::WED_XPluginFacRing(WED_XPluginMgr * inRef,const vector<string>& inArgs):
    WED_XPluginEntity(nw_obj_FacadeRing,inRef)
{
    this->Update(inArgs);
}

WED_XPluginFacRing::WED_XPluginFacRing(WED_XPluginMgr * inRef):
    WED_XPluginEntity(nw_obj_FacadeRing,inRef)
{

}

WED_XPluginFacRing::~WED_XPluginFacRing()
{
    WED_XPluginFacade * fac = dynamic_cast<WED_XPluginFacade *>(mParent);
    if( fac != NULL )   fac->DelRing(this);
}

void WED_XPluginFacRing::UpdatePos()
{
   if(mParent) mParent->UpdatePos();
}

void WED_XPluginFacRing::Update(const vector<string>& inArgs)
{
    int pid;

    if (inArgs.size() == 6)
    {
        sscanf(inArgs[3].c_str(),"%d",&pid);
        sscanf(inArgs[4].c_str(),"%d",&mIdx);
        SetName(inArgs[5]);

        if(!mParent || mParent->GetType() != nw_obj_Facade)
        {
            mParent = mMgrRef->GetbyId(pid);
            if(!mParent || mParent->GetType() != nw_obj_Facade)
            {
                vector<string> args;
                mMgrRef->Add(pid,nw_obj_Facade,args);
                mMgrRef->SendData(WED_NWP_GET,nw_obj_Facade,pid,"");
                mParent = mMgrRef->GetbyId(pid);
            }

            WED_XPluginFacade * fac = dynamic_cast<WED_XPluginFacade *>(mParent);
            if(fac != NULL)     fac->AddRing(this);
        }

        if(mParent) mParent->UpdatePos();
    }
}
