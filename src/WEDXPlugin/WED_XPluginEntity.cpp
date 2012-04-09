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

#include "WED_XPluginEntity.h"
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


WED_XPluginEntity::WED_XPluginEntity(int inType,XPLMProbeRef inProbeRef,WED_XPluginMgr * inRef):
 	mType(inType),mMgrRef(inRef),mIdx(0),mParent(NULL),
 	mProbeRef(inProbeRef),mX(0),mY(0),mZ(0),mLon(0),mLat(0),mAlt(0),mHdg(0),
 	mSetToTerrain(false)
{
	mResPath="";
	mName="unknown";
}


WED_XPluginEntity::~WED_XPluginEntity()
{

}

void WED_XPluginEntity::GetLoc(double * outLat,double * outLon,double * outAlt)
{
    if (outLat) *outLat = mLat;
    if (outLon) *outLon = mLon;
    if (outAlt) *outAlt = mAlt;
}

void WED_XPluginEntity::SetLoc(const double inLat,const double inLon,const double inAlt)
{
    mLat = inLat;
    mLon = inLon;
    mAlt = inAlt;
}

void WED_XPluginEntity::GetPos(double * outX,double * outY,double * outZ)
{
    if (outX) *outX = mX;
    if (outY) *outY = mY;
    if (outZ) *outZ = mZ;
}

void WED_XPluginEntity::SetPos(const double inX,const double inY,const double inZ)
{
    mX = inX;
    mY = inY;
    mZ = inZ;
}

void WED_XPluginEntity::Draw(bool isLit)
{
	XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);
	glColor3f(1.0, 0.0, 0.5);

	glBegin(GL_LINES);
	glVertex3f(mX+1, mY, mZ+1);
	glVertex3f(mX-1, mY, mZ-1);
	glVertex3f(mX-1, mY, mZ+1);
	glVertex3f(mX+1, mY, mZ-1);
	glEnd();
}

void WED_XPluginEntity::WorldToLocal(WED_XPluginEntity * inEnt)
{
    double x,y,z;
	if(inEnt->mSetToTerrain)
	{
		if (inEnt->mProbeRef)
		{
			double lat,lon,alt = 0;

			XPLMProbeInfo_t	 aProbeInfo;
			aProbeInfo.structSize = sizeof(XPLMProbeInfo_t);

			XPLMWorldToLocal(inEnt->mLat,inEnt->mLon,0,&x,&y,&z);

			if (XPLMProbeTerrainXYZ(inEnt->mProbeRef,x,y,z,&aProbeInfo) == xplm_ProbeHitTerrain)
			{
				y = aProbeInfo.locationY;

				XPLMLocalToWorld(x,y,z,&lat,&lon,&alt);
				XPLMWorldToLocal(inEnt->mLat,inEnt->mLon,alt,&x,&y,&z);

				XPLMProbeTerrainXYZ(inEnt->mProbeRef,x,y,z,&aProbeInfo);
				y = aProbeInfo.locationY;

				XPLMLocalToWorld(x,y,z,&lat,&lon,&inEnt->mAlt);
			}
		}
    }
    else
    	XPLMWorldToLocal(inEnt->mLat,inEnt->mLon,inEnt->mAlt,&x,&y,&z);

    inEnt->mX = x;
    inEnt->mY = y;
    inEnt->mZ = z;
}
