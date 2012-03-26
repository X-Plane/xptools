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
#include "XPLMUtilities.h"
#include "stdio.h"


WED_XPluginCamera::WED_XPluginCamera():
   mEnabled(false)
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

void WED_XPluginCamera::Update(XPLMCameraPosition_t * inCameraPosition)
{
    mPos.x        = inCameraPosition->x;
    mPos.y        = inCameraPosition->y;
    mPos.z        = inCameraPosition->z;
    mPos.pitch    = inCameraPosition->pitch;
    mPos.roll     = inCameraPosition->roll;
    mPos.heading  = inCameraPosition->heading;
    //mPos.zoom     = inCameraPosition->zoom;
}
void WED_XPluginCamera::GetPos(XPLMCameraPosition_t * outCameraPosition)
{
    XPLMReadCameraPosition(outCameraPosition);
}

void WED_XPluginCamera::Enable(XPLMCameraPosition_t * outCameraPosition)
{
    XPLMReadCameraPosition(&mPos);
 	XPLMReadCameraPosition(outCameraPosition);
	XPLMControlCamera(xplm_ControlCameraForever, CamUpdFunc, this);
	mEnabled=true;
}
void WED_XPluginCamera::Disable()
{
    if(XPLMIsCameraBeingControlled(NULL)) XPLMDontControlCamera();
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

