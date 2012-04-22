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
#include "WED_XPluginNode.h"
#include "WED_XPluginMgr.h"
#include "XPLMGraphics.h"

#if LIN || IBM
#include <GL/gl.h>
#else
#if __GNUC__
#include <OpenGL/Gl.h>
#else
#include <gl.h>
#endif
#endif


WED_XPluginNode::WED_XPluginNode(int inType,WED_XPluginMgr * inRef):
    WED_XPluginEntity(inType,inRef),
    mWantDraw(true),mX(0),mY(0),mZ(0),mOX(0),mOY(0),mOH(0),
    mLon(0),mLat(0),mAlt(0),mHdg(0),
    mSetToTerrain(true),mProbeRef(inRef->GetProbeRef())
{
    //ctor
}

WED_XPluginNode::~WED_XPluginNode()
{
    //dtor
}

void WED_XPluginNode::GetLoc(double * outLat,double * outLon,double * outAlt)
{
    if (outLat) *outLat = mLat;
    if (outLon) *outLon = mLon;
    if (outAlt) *outAlt = mAlt;
}

void WED_XPluginNode::SetLoc(const double inLat,const double inLon,const double inAlt)
{
    mLat = inLat;
    mLon = inLon;
    mAlt = inAlt;
}

void WED_XPluginNode::GetPos(double * outX,double * outY,double * outZ)
{
    if (outX) *outX = mX;
    if (outY) *outY = mY;
    if (outZ) *outZ = mZ;
}

void WED_XPluginNode::SetPos(const double inX,const double inY,const double inZ)
{
    mX = inX;
    mY = inY;
    mZ = inZ;
}

void WED_XPluginNode::Draw(bool isLit)
{
    if(mWantDraw)
    {
        XPLMSetGraphicsState(0, 0, 0, 0, 0, 1, 1);
        glColor3f(1.0, 0.0, 0.5);

        glBegin(GL_LINES);
        glVertex3f(mX+1, mY, mZ+1);
        glVertex3f(mX-1, mY, mZ-1);
        glVertex3f(mX-1, mY, mZ+1);
        glVertex3f(mX+1, mY, mZ-1);
        glEnd();
    }
}


void WED_XPluginNode::SetOffsets(const double inX,const double inY,const double inHdg)
{
    mOX = inX;
    mOY = inY;
    mOH = inHdg;
}

void WED_XPluginNode::WorldToLocal(WED_XPluginNode * inNode)
{
    double x,y,z;
    if(inNode->mSetToTerrain)
    {
        XPLMProbeRef aProbeRef = inNode->mProbeRef;
        if (aProbeRef != NULL)
        {
            double lat,lon,alt = 0;

            XPLMProbeInfo_t	 aProbeInfo;
            aProbeInfo.structSize = sizeof(XPLMProbeInfo_t);

            XPLMWorldToLocal(inNode->mLat,inNode->mLon,0,&x,&y,&z);

            if (XPLMProbeTerrainXYZ(aProbeRef,x,y,z,&aProbeInfo) == xplm_ProbeHitTerrain)
            {
                y = aProbeInfo.locationY;

                XPLMLocalToWorld(x,y,z,&lat,&lon,&alt);
                XPLMWorldToLocal(inNode->mLat,inNode->mLon,alt,&x,&y,&z);

                XPLMProbeTerrainXYZ(aProbeRef,x,y,z,&aProbeInfo);
                y = aProbeInfo.locationY;

                XPLMLocalToWorld(x,y,z,&lat,&lon,&inNode->mAlt);
            }
        }
    }
    else
        XPLMWorldToLocal(inNode->mLat,inNode->mLon,inNode->mAlt,&x,&y,&z);

    inNode->X(x);
    inNode->Y(y);
    inNode->Z(z);
}
