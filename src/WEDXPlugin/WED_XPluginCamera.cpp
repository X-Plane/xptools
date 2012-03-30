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
#include "WED_XPluginCamera.h"
#include "WED_XPluginMgr.h"
#include "WED_NWDefs.h"
#include "XPLMUtilities.h"
#include "XPLMGraphics.h"

#include "stdio.h"


WED_XPluginCamera::WED_XPluginCamera(WED_XPluginMgr * inRef):
   mEnabled(false),mProbeRef(NULL),mMgrRef(inRef)
{
    mPos.x = mPos.y = mPos.z = 0;
    mPos.heading = mPos.pitch = mPos.roll = 0;
    mPos.zoom = 0;
}

WED_XPluginCamera::~WED_XPluginCamera()
{
    if(XPLMIsCameraBeingControlled(NULL)) XPLMDontControlCamera();
}

int WED_XPluginCamera::IsEnabled()
{
    return XPLMIsCameraBeingControlled(NULL) && mEnabled ;
}

void WED_XPluginCamera::Update(int inType,const vector<string>& inArgs)
{
    if(inType == nw_cam_state && inArgs.size() == 4)
    {
        int is_enabled = 0;
        sscanf(inArgs[3].c_str(),"%d",&is_enabled);
        if(is_enabled) Enable();
        else Disable();
    }
    else
    if (inType == nw_cam_data && inArgs.size() == 9 && IsEnabled())
    {
        double x,y,z,lat,lon,alt,agl;

        sscanf(inArgs[3].c_str(),"%lf",&lat);
        sscanf(inArgs[4].c_str(),"%lf",&lon);
        sscanf(inArgs[5].c_str(),"%lf",&agl);
        sscanf(inArgs[6].c_str(),"%f",&mPos.pitch);
        sscanf(inArgs[7].c_str(),"%f",&mPos.roll);
        sscanf(inArgs[8].c_str(),"%f",&mPos.heading);

        XPLMWorldToLocal(lat,lon,0,&x,&y,&z);

        XPLMProbeInfo_t	 aProbeInfo;
        aProbeInfo.structSize = sizeof(XPLMProbeInfo_t);
        if (mProbeRef && XPLMProbeTerrainXYZ(mProbeRef,x,y,z,&aProbeInfo) == xplm_ProbeHitTerrain)
        {
            y = aProbeInfo.locationY ;
        }
        XPLMLocalToWorld(x,y,z,&lat,&lon,&alt);
        alt += agl;
        XPLMWorldToLocal(lat,lon,alt,&x,&y,&z);

        mPos.x = x;
        mPos.y = y;
        mPos.z = z;
    }
}

void WED_XPluginCamera::GetPos(XPLMCameraPosition_t * outCameraPosition)
{
    XPLMReadCameraPosition(outCameraPosition);
}

void WED_XPluginCamera::Enable()
{
    char buf[256];
    double x,y,z,lat,lon,alt,agl;

    XPLMReadCameraPosition(&mPos);

    XPLMLocalToWorld(mPos.x,mPos.y,mPos.z,&lat,&lon,&alt);
    XPLMWorldToLocal(lat,lon,0,&x,&y,&z);

    XPLMProbeInfo_t	 aProbeInfo;
    aProbeInfo.structSize = sizeof(XPLMProbeInfo_t);
    if (mProbeRef && XPLMProbeTerrainXYZ(mProbeRef,mPos.x,mPos.y,mPos.z,&aProbeInfo) == xplm_ProbeHitTerrain)
    {
        y = aProbeInfo.locationY ;
    }
    XPLMLocalToWorld(x,y,z,&lat,&lon,&agl);
    agl = alt-agl;

    mMgrRef->SendData(WED_NWP_CAM,nw_cam_state,0,"1:");
    sprintf(buf,"%.8lf:%.8lf:%.2lf:%.2f:%.2f:%.2f:",lat,lon,agl,mPos.pitch,mPos.roll,mPos.heading);
    mMgrRef->SendData(WED_NWP_CAM,nw_cam_data,0,buf);

    if(!IsEnabled())
    {
        XPLMControlCamera(xplm_ControlCameraForever, CamUpdFunc, this);
        mEnabled=true;
    }
}

void WED_XPluginCamera::Disable()
{
    if(!IsEnabled()) return;
    mMgrRef->SendData(WED_NWP_CAM,nw_cam_state,0,"0:");
    XPLMDontControlCamera();
    mEnabled=false;
}

int WED_XPluginCamera::CamUpdFunc(XPLMCameraPosition_t * outCameraPosition,
                                   int                  inIsLosingControl,
                                   void *               inRefcon)
{

    WED_XPluginCamera * cam = static_cast< WED_XPluginCamera *>(inRefcon);

    if(inIsLosingControl || outCameraPosition == NULL)
    {
        cam->mEnabled = false;
        return 0;
    }

    outCameraPosition->x        = cam->mPos.x;
    outCameraPosition->y        = cam->mPos.y;
    outCameraPosition->z        = cam->mPos.z;
    outCameraPosition->pitch    = cam->mPos.pitch ;
    outCameraPosition->roll     = cam->mPos.roll;
    outCameraPosition->heading  = cam->mPos.heading;
    outCameraPosition->zoom     = cam->mPos.zoom ;

    return 1;
}

