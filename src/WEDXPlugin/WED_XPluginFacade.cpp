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
#include "WED_XPluginFacade.h"
#include "WED_XPluginFacNode.h"
#include "WED_XPluginFacRing.h"
#include "WED_XPluginDrawUtils.h"

#include "stdio.h"


WED_XPluginFacade::WED_XPluginFacade(WED_XPluginMgr * inRef,const vector<string>& inArgs):
    WED_XPluginNode(nw_obj_Facade,inRef),
    mHeight(1.0),mHasWall(false),mTopo(topo_unknown)
{
    this->Update(inArgs);
}
WED_XPluginFacade::WED_XPluginFacade(WED_XPluginMgr * inRef):
    WED_XPluginNode(nw_obj_Facade,inRef),
    mHeight(0),mHasWall(false),mTopo(topo_unknown)
{

}

WED_XPluginFacade::~WED_XPluginFacade()
{
    vector<WED_XPluginFacRing *>::iterator it ;
    for(it = mRings.begin(); it != mRings.end(); ++it)
    {
        if(*it != NULL) (*it)->SetParent(NULL);
    }
}

void WED_XPluginFacade::Update(const vector<string>& inArgs)
{
    if (inArgs.size() == 8)	//facade
    {
        sscanf(inArgs[3].c_str(),"%f",&mHeight);
        sscanf(inArgs[4].c_str(),"%d",&mTopo);
        sscanf(inArgs[5].c_str(),"%d",&mHasWall);
        SetName(inArgs[6]);
        SetResPath(inArgs[7]);
        SetToTerrain(true);
    }
}

void WED_XPluginFacade::UpdatePos()
{
    if(mRings.empty()) return;
    WED_XPluginFacRing * aRing = mRings[0];
    if(aRing->GetNodeCnt() < 2) return;

    WED_XPluginBezierNode * aNode_1 = aRing->GetNodeAt(0);
    WED_XPluginBezierNode * aNode_2 = aRing->GetNodeAt(1);

    Point3   p;
    Segment3 s;

    aNode_1->GetPos(&s.p1.x,&s.p1.y,&s.p1.z);
    aNode_2->GetPos(&s.p2.x,&s.p2.y,&s.p2.z);
    p = s.midpoint();
    XPLMLocalToWorld(p.x,p.y,p.z,&mLat,&mLon,&mAlt);
    WED_XPluginNode::UpdatePos();
}

void  WED_XPluginFacade::AddRing(WED_XPluginFacRing * inRing)
{
    unsigned int idx = inRing->GetIdx();
    bool IsExists = false;
    vector<WED_XPluginFacRing *>::iterator it;

    for(it = mRings.begin(); it != mRings.end(); ++it)
    {
        if(idx == (*it)->GetIdx())
        {
            it = mRings.insert(it,inRing);
            IsExists = true;
            break;
        }
        else if(idx < (*it)->GetIdx())
        {
            mRings.insert(it,inRing);
            break;
        }
    }

    if(IsExists)
    {
        for(++it; it != mRings.end(); ++it)
        {
            (*it)->SetIdx((*it)->GetIdx()+1);
        }
    }
    else
    {
        if(it == mRings.end()) mRings.push_back(inRing);
    }
}

void  WED_XPluginFacade::DelRing(WED_XPluginFacRing * inRing)
{
    vector<WED_XPluginFacRing *>::iterator it ;
    for(it = mRings.begin(); it != mRings.end(); ++it)
    {
        if(*it == inRing)
        {
            mRings.erase(it);
            for(; it != mRings.end(); ++it)
            {
                (*it)->SetIdx((*it)->GetIdx()-1);
            }
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
// drawing
void WED_XPluginFacade::Draw(bool isLit)
{
    //WED_XPluginEntity::Draw(isLit);
    if (mTopo == topo_unknown) return;

    Point3   p;
    Bezier3  b;
    vector<Point3>	pts;
    vector<int>		contours;
    WED_XPluginFacRing * aRing   ;
    WED_XPluginBezierNode * aNode_1 ;
    WED_XPluginBezierNode * aNode_2 ;

    XPLMSetGraphicsState(0, 0, 0, 0, 0, 1, 1);

    glColor4f(.5, .5, .5 ,1);
    glDisable(GL_CULL_FACE);

    for(unsigned int i = 0 ; i < mRings.size(); ++i)
    {
        aRing = mRings[i];
        int cnt = aRing->GetNodeCnt();
        if(cnt < 2) continue;

        glShadeModel(GL_FLAT) ;
        glBegin(GL_QUAD_STRIP);
        for(int j = 0; j < cnt ; ++j)
        {
            aNode_1 = aRing->GetNodeAt(j);
            if(j == cnt-1)
            {
                if(mTopo == topo_Area || mTopo == topo_Ring)
                    aNode_2 = aRing->GetNodeAt(0);
                else  break;
            }
            else aNode_2 = aRing->GetNodeAt(j+1);

            if(mTopo == topo_Area)
            {
                double lat,lon,alt;
                aNode_1->GetLoc(&lat,&lon,&alt);
                XPLMWorldToLocal(lat,lon,mAlt,&b.p1.x,&b.p1.y,&b.p1.z);
                aNode_1->GetLocHi(&lat,&lon,&alt);
                XPLMWorldToLocal(lat,lon,mAlt,&b.c1.x,&b.c1.y,&b.c1.z);
                aNode_2->GetLocLo(&lat,&lon,&alt);
                XPLMWorldToLocal(lat,lon,mAlt,&b.c2.x,&b.c2.y,&b.c2.z);
                aNode_2->GetLoc(&lat,&lon,&alt);
                XPLMWorldToLocal(lat,lon,mAlt,&b.p2.x,&b.p2.y,&b.p2.z);
            }
            else
            {
                aNode_1->GetPos(&b.p1.x,&b.p1.y,&b.p1.z);
                aNode_1->GetPosHi(&b.c1.x,&b.c1.y,&b.c1.z);
                aNode_2->GetPosLo(&b.c2.x,&b.c2.y,&b.c2.z);
                aNode_2->GetPos(&b.p2.x,&b.p2.y,&b.p2.z);
            }

            int point_count = 1;

            if(aNode_1->HasBez()||aNode_2->HasBez())
            {
                int approx = sqrt(Vector3(b.p1,b.c1).squared_length()) +
                             sqrt(Vector3(b.c1,b.c2).squared_length()) +
                             sqrt(Vector3(b.c2,b.p2).squared_length());

                point_count =  approx / BEZ_MTR_PER_SEG;
                if (point_count < BEZ_MIN_SEGS) point_count = BEZ_MIN_SEGS;
                if (point_count > BEZ_MAX_SEGS) point_count = BEZ_MAX_SEGS;

                pts.reserve(pts.capacity() + point_count);
                contours.reserve(contours.capacity() + point_count);
            }

            for (int k = 0; k < point_count; ++k)
            {
                p = b.midpoint((float) k / (float) (point_count));
                glVertex3f(p.x,p.y,p.z);
                p.y += mHeight;
                glVertex3f(p.x,p.y,p.z);
                pts.push_back(p);
                contours.push_back(k == 0 && i > 0 && j == 0);
                if(k==0 && mHasWall)
                    glColor4fv(gWallCols + (aNode_1->GetParam() % 6) * 4);
            }

            if(j == cnt-1 || (j == cnt-2 && mTopo == topo_Chain))
            {
                p = b.p2;
                glVertex3f(p.x,p.y,p.z);
                p.y += mHeight;
                glVertex3f(p.x,p.y,p.z);
                pts.push_back(p);
                contours.push_back(0);
            }
        }
        glEnd();
    }

    if(mTopo == topo_Area )
    {
        glColor4f(.4, .4, .4, 1);
        glFrontFace(GL_CCW);
        glPolygon3(&*pts.begin(),0, &*contours.begin(), pts.size());
        glFrontFace(GL_CW);
    }

    glEnable(GL_CULL_FACE);

}
