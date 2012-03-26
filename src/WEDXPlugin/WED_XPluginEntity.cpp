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
 	mType(inType),mMgrRef(inRef),mProbeRef(inProbeRef),mLon(0),mLat(0),mAlt(0),mHdg(0),
 	mSetToTerrain(false)
{
	mResPath="";
	mName="unknown";
}


WED_XPluginEntity::~WED_XPluginEntity()
{

}

void   WED_XPluginEntity::GetLoc(double * outLat,double * outLon,double * outAlt)
{
    if (outLat) *outLat = mLat;
	if (outLon) *outLon = mLon;
	if (outAlt) *outAlt = mAlt;
}

void   WED_XPluginEntity::GetPos(float * outX,float * outY,float * outZ)
{
	WorldToLocal(outX,outY,outZ,this);
}

void WED_XPluginEntity::Draw(bool isLit)
{
	float x,y,z;
	WorldToLocal(&x,&y,&z,this);

	XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);
	glColor3f(1.0, 0.0, 0.5);
//  glPointSize(3.0);
//	glBegin(GL_POINTS);
//	glVertex3f(x,y,z);
//	glEnd();
	glBegin(GL_QUADS);
	glVertex3f(x+1, y, z+1);
	glVertex3f(x-1, y, z+1);
	glVertex3f(x-1, y, z-1);
	glVertex3f(x+1, y, z-1);
	glEnd();
}

void WED_XPluginEntity::WorldToLocal(float * outX,float * outY,float * outZ,WED_XPluginEntity* inEntity)
{
	double x,y,z = 0;

	if(inEntity->mSetToTerrain)
	{
		if (inEntity->mProbeRef)
		{
			double lat,lon,alt = 0;

			XPLMProbeInfo_t	 aProbeInfo;
			aProbeInfo.structSize = sizeof(XPLMProbeInfo_t);

			XPLMWorldToLocal(inEntity->mLat,inEntity->mLon,0,&x,&y,&z);

			if (XPLMProbeTerrainXYZ(inEntity->mProbeRef,x,y,z,&aProbeInfo) == xplm_ProbeHitTerrain)
			{
				y = aProbeInfo.locationY ;

				XPLMLocalToWorld(x,y,z,&lat,&lon,&alt);
				XPLMWorldToLocal(inEntity->mLat,inEntity->mLon,alt,&x,&y,&z);

				XPLMProbeTerrainXYZ(inEntity->mProbeRef,x,y,z,&aProbeInfo);
				y = aProbeInfo.locationY ;

				XPLMLocalToWorld(x,y,z,&lat,&lon,&inEntity->mAlt);
				inEntity->mSetToTerrain = false;
			}
		}
    }
    else
    	XPLMWorldToLocal(inEntity->mLat,inEntity->mLon,inEntity->mAlt,&x,&y,&z);

	if (outX) *outX = (float) x;
	if (outY) *outY = (float) y;
	if (outZ) *outZ = (float) z;
}
