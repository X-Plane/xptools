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
#include "WED_XPluginBezierNode.h"
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

WED_XPluginBezierNode::WED_XPluginBezierNode(int inTyp,WED_XPluginMgr * inRef):
    WED_XPluginNode(inTyp,inRef),
    mHasBez(false),mParam1(0),
    mX_lo(0),mY_lo(0),mZ_lo(0),mX_hi(0),mY_hi(0),mZ_hi(0),
    mLat_lo(0),mLon_lo(0),mLat_hi(0),mLon_hi(0)
{
    //ctor
}

WED_XPluginBezierNode::~WED_XPluginBezierNode()
{
    //dtor
}

void   WED_XPluginBezierNode::UpdatePos()
{
    WED_XPluginNode::UpdatePos();// must come first , probes for mAlt
    if(mHasBez)
        this->WorldTolocal(this);
    else
    {
        mX_lo = mX_hi = mX;
        mY_lo = mY_hi = mY;
        mZ_lo = mZ_hi = mZ;
        mLat_lo = mLat_hi = mLat;
        mLon_lo = mLon_hi = mLon;
        mAlt_lo = mAlt_hi = mAlt;
    }
}

void   WED_XPluginBezierNode::GetPosHi(double * outX,double * outY,double * outZ)
{
	if (outX) *outX =  mX_hi;
	if (outY) *outY =  mY_hi;
	if (outZ) *outZ =  mZ_hi;
}

void   WED_XPluginBezierNode::GetPosLo(double * outX,double * outY,double * outZ)
{
	if (outX) *outX =  mX_lo;
	if (outY) *outY =  mY_lo;
	if (outZ) *outZ =  mZ_lo;
}

void   WED_XPluginBezierNode::GetLocLo(double * outLat,double * outLon,double * outAlt)
{
    if (outLat) *outLat = mLat_lo;
	if (outLon) *outLon = mLon_lo;
	if (outAlt) *outAlt = mAlt_lo;
}

void   WED_XPluginBezierNode::GetLocHi(double * outLat,double * outLon,double * outAlt)
{
    if (outLat) *outLat = mLat_hi;
	if (outLon) *outLon = mLon_hi;
	if (outAlt) *outAlt = mAlt_hi;
}
///////////////////////////////////////////////////////////////////////////////////
// drawing
void WED_XPluginBezierNode::Draw(bool isLit)
{
    WED_XPluginNode::Draw(isLit);

    if(!mHasBez || !mWantDraw) return;

	glBegin(GL_QUADS);
	glVertex3f(mX_hi+.5, mY_hi, mZ_hi+.5);
	glVertex3f(mX_hi-.5, mY_hi, mZ_hi+.5);
	glVertex3f(mX_hi-.5, mY_hi, mZ_hi-.5);
	glVertex3f(mX_hi+.5, mY_hi, mZ_hi-.5);

	glVertex3f(mX_lo+.5, mY_lo, mZ_lo+.5);
	glVertex3f(mX_lo-.5, mY_lo, mZ_lo+.5);
	glVertex3f(mX_lo-.5, mY_lo, mZ_lo-.5);
	glVertex3f(mX_lo+.5, mY_lo, mZ_lo-.5);
	glEnd();
}

void   WED_XPluginBezierNode::WorldTolocal(WED_XPluginBezierNode * inBez)
{
    double lat = 0,lon = 0;

    if(inBez->mSetToTerrain)
    {
            XPLMProbeInfo_t	 aProbeInfo;
            aProbeInfo.structSize = sizeof(XPLMProbeInfo_t);

            XPLMWorldToLocal(inBez->mLat_lo,inBez->mLon_lo,inBez->mAlt,&inBez->mX_lo,&inBez->mY_lo,&inBez->mZ_lo);
            if(inBez->mProbeRef)
            {
                if (XPLMProbeTerrainXYZ(inBez->mProbeRef,inBez->mX_lo,inBez->mY_lo,inBez->mZ_lo,&aProbeInfo) == xplm_ProbeHitTerrain)
                {
                    inBez->mY_lo = aProbeInfo.locationY ;
                }
            }
            XPLMLocalToWorld(inBez->mX_lo,inBez->mY_lo,inBez->mZ_lo,&lat,&lon,&inBez->mAlt_lo);

            XPLMWorldToLocal(inBez->mLat_hi,inBez->mLon_hi,inBez->mAlt,&inBez->mX_hi,&inBez->mY_hi,&inBez->mZ_hi);
            if(inBez->mProbeRef)
            {
                if (XPLMProbeTerrainXYZ(inBez->mProbeRef,inBez->mX_hi,inBez->mY_hi,inBez->mZ_hi,&aProbeInfo) == xplm_ProbeHitTerrain)
                {
                    inBez->mY_hi = aProbeInfo.locationY ;
                }
            }
            XPLMLocalToWorld(inBez->mX_hi,inBez->mY_hi,inBez->mZ_hi,&lat,&lon,&inBez->mAlt_hi);

    }
    else
    {
        XPLMWorldToLocal(inBez->mLat_lo,inBez->mLon_lo,inBez->mAlt_lo,&inBez->mX_lo,&inBez->mY_lo,&inBez->mZ_lo);
        XPLMWorldToLocal(inBez->mLat_hi,inBez->mLon_hi,inBez->mAlt_hi,&inBez->mX_hi,&inBez->mY_hi,&inBez->mZ_hi);
    }
}

