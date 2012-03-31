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

#if WITHNWLINK

#include "WED_NWLinkAdapter.h"
#include "WED_Messages.h"
#include "WED_Server.h"
#include "WED_Thing.h"
#include "WED_Archive.h"
#include "WED_NWDefs.h"
#include "WED_ObjPlacement.h"
#include "WED_FacadePlacement.h"
#include "WED_FacadeNode.h"
#include "WED_FacadeRing.h"

WED_NWLinkAdapter::WED_NWLinkAdapter(WED_Server * inServer,WED_Archive * inArchive) :
    mServer(inServer),mArchive(inArchive),mTimerIsStarted(false)
{

}

WED_NWLinkAdapter::~WED_NWLinkAdapter()
{
    mObjCache.clear();
}

int     WED_NWLinkAdapter::IsReady(void)
{
    return  (mServer != NULL) && mServer->IsReady();
}

//acting as archive-change event
void  	WED_NWLinkAdapter::TimerFired()
{
    GUI_Timer::Stop();
    mTimerIsStarted = false;
    if(mServer && mServer->IsReady())
    {
        this->DoReadData();
        this->DoSendData();
        mServer->DoProcessing();
    }
}

void 	WED_NWLinkAdapter::ObjectCreated(WED_Persistent * inObject)
{
    mObjCache[inObject] = wed_Change_CreateDestroy ;
    if (mTimerIsStarted) return;
    GUI_Timer::Start(0);
    mTimerIsStarted = true;
}

void	WED_NWLinkAdapter::ObjectChanged(WED_Persistent * inObject, int chgkind)
{
    if (chgkind == wed_Change_Selection) return;
    if (chgkind == wed_Change_Topology) return;
    if (chgkind == wed_Change_Any)
        mObjCache[inObject] |= wed_Change_Any_Ex;
    else
        mObjCache[inObject] |= chgkind;

    if (mTimerIsStarted) return;
    GUI_Timer::Start(0);
    mTimerIsStarted = true;
}

void 	WED_NWLinkAdapter::ObjectDestroyed(WED_Persistent * inObject)
{
    mObjCache.erase(inObject);
    if(	inObject->GetClass() == WED_ObjPlacement::sClass 	||
            inObject->GetClass() == WED_FacadePlacement::sClass	||
            inObject->GetClass() == WED_FacadeRing::sClass		||
            inObject->GetClass() == WED_FacadeNode::sClass 		 )
    {
        mDelList.insert(inObject->GetID());
    }

    if (mTimerIsStarted) return;
    GUI_Timer::Start(0);
    mTimerIsStarted = true;
}

void	WED_NWLinkAdapter::SendCamData()
{
    mCamera.changed = true;
    if (mTimerIsStarted) return;
    GUI_Timer::Start(0);
    mTimerIsStarted = true;
}

void 	WED_NWLinkAdapter::DoSendData()
{
    char buf[256];
    Point2 p;
    BezierPoint2 bp;

    WED_ObjPlacement *		obj;
    WED_FacadePlacement *	fac;
    WED_FacadeNode *		facnode;
    WED_FacadeRing *		facring;

    map<WED_Persistent *,int >::iterator it;
    for(it = mObjCache.begin(); it != mObjCache.end(); ++it)
    {
        string n,r  = "unkown";
        string hdr  = "???";
        string astr = "";

        int id = -1;
        int chgkind,objtype = 0;

        if(it->second & wed_Change_CreateDestroy)
            hdr = WED_NWP_ADD;
        else
            hdr = WED_NWP_CHG;

        if((obj = dynamic_cast<WED_ObjPlacement *>(it->first)) != NULL)
        {
            objtype = nw_obj_Object;
            id = obj->GetID();
            obj->GetLocation(gis_Geo,p);

            float hdg = obj->GetHeading();
            while(hdg < 0) hdg += 360.0;
            while(hdg >= 360.0) hdg -= 360.0;
#if AIRPORT_ROUTING
            if(obj->HasCustomMSL())
            {
                double alt = obj->GetCustomMSL();
                sprintf(buf,"%.8lf:%.8lf:%.8lf:%.2f:",p.x(),p.y(),alt,hdg);
            }
            else
#endif
                sprintf(buf,"%.8lf:%.8lf:%.2f:",p.x(),p.y(),hdg);

            astr += buf;

            if( it->second & wed_Change_CreateDestroy ||
                    (it->second == wed_Change_Properties ))
            {
                obj->GetName(n);
                obj->GetResource(r);
                astr +=  n + ":" + r +":";
            }
        }
        else if((fac = dynamic_cast<WED_FacadePlacement *>(it->first)) != NULL)
        {
            objtype = nw_obj_Facade;
            id = fac->GetID();
            sprintf(buf,"%.2f:",fac->GetHeight());
            astr += buf;
            if( it->second & wed_Change_CreateDestroy ||
                    (it->second == wed_Change_Properties ))
            {
                sprintf(buf,"%d:%d:",fac->GetTopoMode(),fac->HasCustomWalls());
                fac->GetName(n);
                fac->GetResource(r);
                astr += buf + n + ":" + r +":";
            }
        }
        else if((facring = dynamic_cast<WED_FacadeRing *>(it->first)) != NULL)
        {
            objtype = nw_obj_FacadeRing;
            id = facring->GetID();
            sprintf(buf,"%d:%d:",facring->GetParent()->GetID(),facring->GetMyPosition());
            facring->GetName(n);
            astr += buf + n + ":";
        }
        else if((facnode = dynamic_cast<WED_FacadeNode*>(it->first)) != NULL)
        {
            objtype = nw_obj_FacadeNode;
            id = facnode->GetID();
            facnode->GetBezierLocation(gis_Geo,bp);

            sprintf(buf,"%.8lf:%.8lf:",bp.pt.x(),bp.pt.y());
            astr += buf;
            if (bp.has_lo()||bp.has_hi())
            {
                sprintf(buf,"%.8lf:%.8lf:%.8lf:%.8lf:",bp.hi.x(),bp.hi.y(),bp.lo.x(),bp.lo.y());
                astr += buf;
            }
            if( it->second & wed_Change_CreateDestroy ||
                    (it->second == wed_Change_Properties ))
            {
                sprintf(buf,"%d:%d:%d:0:",facnode->GetParent()->GetID(),facnode->GetMyPosition(),facnode->GetWallType());
                facnode->GetName(n);
                astr += buf + n + ":";
            }
        }
        else
        {
            continue;
        }

        mServer->SendData(hdr.c_str(),objtype,id,astr);

    } // for mObjCache
    mObjCache.clear();

    set<int >::iterator sit ;
    for (sit = mDelList.begin(); sit != mDelList.end(); ++sit)
    {
        mServer->SendData(WED_NWP_DEL,nw_obj_none,*sit,"");
    }
    mDelList.clear();

    if(mCamera.enabled && mCamera.changed)
    {
        sprintf(buf,"%.8lf:%.8lf:%.2lf:%.2f:%.2f:%.2f:",
                mCamera.lat,mCamera.lon,mCamera.alt,
                mCamera.pitch,mCamera.roll,mCamera.heading);
        mServer->SendData(WED_NWP_CAM,nw_cam_data,0,buf);
        mCamera.changed=false;
    }
}

void	WED_NWLinkAdapter::DoReadData()
{
    if(!mServer||!mArchive) return;

    vector<string> args;

    while(mServer->GetData(args))
    {
        if(args.size() < 3) continue;

        int type,id = 0;
        sscanf(args[1].c_str(),"%d",&type);
        sscanf(args[2].c_str(),"%d",&id);

        if (args[0] == WED_NWP_GET)
        {
            switch(type)
            {

            case nw_obj_none :
                continue;

            default :
            {
                WED_Persistent * entity = mArchive->Fetch(id);
                if(entity && (
                            entity->GetClass() == WED_ObjPlacement::sClass 		||
                            entity->GetClass() == WED_FacadePlacement::sClass	||
                            entity->GetClass() == WED_FacadeRing::sClass		||
                            entity->GetClass() == WED_FacadeNode::sClass 		))
                {
                    mObjCache[entity] = wed_Change_CreateDestroy;
                }
                else
                    mDelList.insert(id);
            }
            break;
            }
        }
        else
        if (args[0] == WED_NWP_CAM)
        {
            switch(type)
            {
                case nw_cam_state:
                {
                    if(args.size() < 4) continue;
                    sscanf(args[3].c_str(),"%d",&mCamera.enabled);
                }break;
                case nw_cam_data:
                {
                    if(args.size() < 9) continue;
                    sscanf(args[3].c_str(),"%lf",&mCamera.lat);
                    sscanf(args[4].c_str(),"%lf",&mCamera.lon);
                    sscanf(args[5].c_str(),"%lf",&mCamera.alt);
                    sscanf(args[6].c_str(),"%f",&mCamera.pitch);
                    sscanf(args[7].c_str(),"%f",&mCamera.roll);
                    sscanf(args[8].c_str(),"%f",&mCamera.heading);
                }break;

            }
            BroadcastMessage(msg_NetworkStatusInfo,WED_Server::s_newdata);

        }
        else
        if (args[0] == WED_NWP_CMD)
        {

        }
    }
}

void	WED_NWLinkAdapter::ReceiveMessage(GUI_Broadcaster * inSrc,intptr_t inMsg,intptr_t inParam)
{
    if(inMsg == msg_NetworkStatusInfo)
    {
        WED_Server * ser = dynamic_cast<WED_Server *>(inSrc);
        if(!ser) return;

        switch(inParam)
        {
        case WED_Server::s_newdata :
            if(!mTimerIsStarted) TimerFired();   break;
        case WED_Server::s_started :
                BroadcastMessage(inMsg,inParam); break;
        case WED_Server::s_stopped :
        case WED_Server::s_changed :
            {
                if(!IsReady()) mCamera.enabled = false;
				BroadcastMessage(inMsg,inParam);
            }
            break;
        return;
        }
    }
}
#endif
