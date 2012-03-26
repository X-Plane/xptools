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


#include "WED_XPluginMgr.h"
#include "WED_XPluginClient.h"
#include "WED_XPluginFacade.h"
#include "WED_XPluginFacRing.h"
#include "WED_XPluginFacNode.h"
#include "XPLMUtilities.h"

#include <stdio.h>
#include <string.h>

#define SCNFOLDERNAME "Custom Scenery"

WED_XPluginMgr::WED_XPluginMgr() : WED_XPluginClient(this),
	mPackage("unknown"),mIsLit(false)

{
    mProbeRef     = XPLMCreateProbe(xplm_ProbeY);
    mLiteLevelRef = XPLMFindDataRef("sim/graphics/scenery/percent_lights_on");

	XPLMRegisterDrawCallback(WEDXPluginDrawObjCB, xplm_Phase_Objects, 0,this);
}

WED_XPluginMgr::~WED_XPluginMgr()
{
	XPLMUnregisterDrawCallback(WEDXPluginDrawObjCB,xplm_Phase_Objects,0,this);

	map<int ,WED_XPluginEntity *>::iterator it ;
	for (it = mEntities.begin(); it != mEntities.end(); ++it) delete it->second ;

	mEntities.clear();
	XPLMDestroyProbe(mProbeRef);
}

// Callback for Draw Objects
int	 WED_XPluginMgr::WEDXPluginDrawObjCB(XPLMDrawingPhase inPhase,int inIsBefore,void * inRefcon)
{
	WED_XPluginMgr * mgr = static_cast<WED_XPluginMgr *>(inRefcon);
    XPLMSetGraphicsState(0, 0, 1, 1, 1, 1, 1);
	map<int,WED_XPluginEntity *>::iterator it;
	for (it = mgr->mEntities.begin();it != mgr->mEntities.end();++it)
		(it->second)->Draw(mgr->mIsLit);
	return 1;
}

void  WED_XPluginMgr::EnableCam(bool inEnable)
{
    char buf[256];
    if(inEnable)
    {
        XPLMCameraPosition_t cam_pos;

        if(mCamera.IsEnabled())
            mCamera.GetPos(&cam_pos);
        else
            mCamera.Enable(&cam_pos);

        double x,y,z,lat,lon,alt,agl;
        XPLMLocalToWorld(cam_pos.x,cam_pos.y,cam_pos.z,&lat,&lon,&alt);
        XPLMWorldToLocal(lat,lon,0,&x,&y,&z);

        XPLMProbeInfo_t	 aProbeInfo;
        aProbeInfo.structSize = sizeof(XPLMProbeInfo_t);
        if (XPLMProbeTerrainXYZ(mProbeRef,cam_pos.x,cam_pos.y,cam_pos.z,&aProbeInfo) == xplm_ProbeHitTerrain)
        {
            y = aProbeInfo.locationY ;
        }
        XPLMLocalToWorld(x,y,z,&lat,&lon,&agl);
        agl = alt-agl;
        sprintf(buf,"%d:",1);
        SendData(WED_NWP_CAM,nw_cam_state,0,buf);
        sprintf(buf,"%.8lf:%.8lf:%.2lf:%.2f:%.2f:%.2f:",lat,lon,agl,cam_pos.pitch,cam_pos.roll,cam_pos.heading);
        SendData(WED_NWP_CAM,nw_cam_data,0,buf);
    }
    else
    {
        if(!mCamera.IsEnabled()) return;
        mCamera.Disable();
        sprintf(buf,"%d:",0);
        SendData(WED_NWP_CAM,nw_cam_state,0,buf);
    }
}

void WED_XPluginMgr::UpdateCam(int inType,const vector<string>& inArgs)
{
    if(inType == nw_cam_state && inArgs.size() == 4)
    {
        int is_enabled = 0;
        sscanf(inArgs[3].c_str(),"%d",&is_enabled);
        EnableCam(is_enabled);
    }
    else
    if (inType == nw_cam_data && inArgs.size() == 9 && mCamera.IsEnabled())
    {
        double x,y,z,lat,lon,alt,agl;

        XPLMCameraPosition_t cam_pos;

        sscanf(inArgs[3].c_str(),"%lf",&lat);
        sscanf(inArgs[4].c_str(),"%lf",&lon);
        sscanf(inArgs[5].c_str(),"%lf",&agl);
        sscanf(inArgs[6].c_str(),"%f",&cam_pos.pitch);
        sscanf(inArgs[7].c_str(),"%f",&cam_pos.roll);
        sscanf(inArgs[8].c_str(),"%f",&cam_pos.heading);

        XPLMWorldToLocal(lat,lon,0,&x,&y,&z);

        XPLMProbeInfo_t	 aProbeInfo;
        aProbeInfo.structSize = sizeof(XPLMProbeInfo_t);
        if (XPLMProbeTerrainXYZ(mProbeRef,x,y,z,&aProbeInfo) == xplm_ProbeHitTerrain)
        {
            y = aProbeInfo.locationY ;
        }
        XPLMLocalToWorld(x,y,z,&lat,&lon,&alt);
        alt += agl;
        XPLMWorldToLocal(lat,lon,alt,&x,&y,&z);

        cam_pos.x = x;
        cam_pos.y = y;
        cam_pos.z = z;
        cam_pos.zoom = 0;

        mCamera.Update(&cam_pos);
    }
}

WED_XPluginEntity * WED_XPluginMgr::GetbyId(int inId)
{
    map<int,WED_XPluginEntity *>::iterator it = mEntities.find(inId);
    if(it == mEntities.end()) return NULL;
    else return it->second;
}

string  WED_XPluginMgr::GetPackagePath()
{
	if (mPackage=="unknown") return mPackage;

    char DirPath[512];
    char PS[1];

    strcpy(PS,XPLMGetDirectorySeparator());

    XPLMGetSystemPath(DirPath);

    sprintf(DirPath,"%s%s%s%s%s",DirPath,SCNFOLDERNAME,PS,mPackage.c_str(),PS);

    return DirPath ;
}

void WED_XPluginMgr::Sync()
{
	//TODO:mroe this forces WED_Server to processing data
	//even there is no object in our list and we would'nt send any data
	//i think we should request document related data here
	SendData(WED_NWP_GET,0,0,"");

	if(mCamera.IsEnabled()) EnableCam(true);

	map<int,WED_XPluginEntity *>::iterator it;
	for (it = mEntities.begin();it != mEntities.end();++it)
	{
		SendData(WED_NWP_GET,(it->second)->GetType(),it->first,"");
	}
}

void WED_XPluginMgr::Add(int inId,int inType,const vector<string>& inArgs)
{
	map<int ,WED_XPluginEntity *>::iterator it = mEntities.find(inId);
	//Id is allready there ,if  same type do change ,else delete and add new
	if(it != mEntities.end())
	{
		if (it->second->GetType() == inType)
		{
			Chg(inId,inType,inArgs);
			return;
		}
		else
		{
			delete it->second;
			mEntities.erase(it);
		}
	}

	switch (inType)
	{
		case nw_obj_none		:return;

		case nw_obj_Object		:
		{
			WED_XPluginObject * aObj = new WED_XPluginObject(this);
			aObj->Update(inArgs);
			mEntities[inId] = aObj;
		}
		break;
		case nw_obj_Facade 		:
		{
			//WED_XPluginFacade * aFac = new WED_XPluginFacade(this);
			//aFac->Update(inArgs);
			//mEntities[inId] = aFac;
		}
		break;
		case nw_obj_FacadeRing	:
		{
		    //WED_XPluginFacRing * aFrg = new WED_XPluginFacRing(this);
			//aFrg->Update(inArgs);
			//mEntities[inId] = aFrg;

		}
		break;
		case nw_obj_FacadeNode	:
		{
			//WED_XPluginFacNode * aFnd = new WED_XPluginFacNode(this);
			//aFnd->Update(inArgs);
			//mEntities[inId] = aFnd;
		}
		break;

		default : return;
	}
}

void WED_XPluginMgr::Chg(int inId,int inType,const vector<string>& inArgs)
{
	map<int ,WED_XPluginEntity *>::iterator it =  mEntities.find(inId);;

	if(it != mEntities.end())
	{
		WED_XPluginEntity * aEntity = it->second ;
		//TODO:mroe wrong type for id in List --> all is going wrong -> delete this one
		//add with the correct type ask for ressource;
		if (aEntity->GetType() != inType )
		{
			delete it->second;
			mEntities.erase(it);
			Add(inId,inType,inArgs);
			it = mEntities.find(inId);
			if(it == mEntities.end()) return;
			if(it->second->GetName() == "unknown")	SendData(WED_NWP_GET,inType,inId,"");
			return;
		}

		switch (inType)
		{
			case nw_obj_none		: break;

			case nw_obj_Object 		:
			{
				(static_cast<WED_XPluginObject *>(aEntity))->Update(inArgs);
			}
			break;
			case nw_obj_Facade 		:
			{
                //(static_cast<WED_XPluginFacade*>(aEntity))->Update(inArgs);
			}
			break;
			case nw_obj_FacadeRing	:
			{
                //(static_cast<WED_XPluginFacRing*>(aEntity))->Update(inArgs);
			}
			break;
			case nw_obj_FacadeNode	:
			{
                //(static_cast<WED_XPluginFacNode*>(aEntity))->Update(inArgs);
			}
			break;

			default : break;
		} //switch type
	}
	else // Id not in my list , add and request ressource
	{
        Add(inId,inType,inArgs);
        SendData(WED_NWP_GET,inType,inId,"");
	}
}

void WED_XPluginMgr::Del(int inId)
{
	//delete everything ,if we have it
	map<int ,WED_XPluginEntity *>::iterator it = mEntities.find(inId);
	if(it == mEntities.end()) return;
	 delete it->second ;
	 mEntities.erase(it);
}


