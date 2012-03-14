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

#include "XPLMUtilities.h"
#include "XPLMGraphics.h"
#include "WED_XPluginObject.h"

#include "stdio.h"

///////////////////////////////////////////////////////////////////////////////////
//WED_XPluginObj constructor
WED_XPluginObject::WED_XPluginObject(int inType,XPLMProbeRef inProbeRef,WED_XPluginMgr * inRef):
		WED_XPluginEntity(inType,inProbeRef,inRef),mObjRef(NULL),mWantDraw(true)

{

}

///////////////////////////////////////////////////////////////////////////////////
//WED_XPluginObj destructor
WED_XPluginObject::~WED_XPluginObject()
{
	if(mObjRef)XPLMUnloadObject(mObjRef);
}

void WED_XPluginObject::Update(const vector<string>& inArgs)
{
	if (inArgs.size() == 6 || inArgs.size() == 8)	//Object
	{
		SetToTerrain(true);
		sscanf(inArgs[3].c_str(),"%lf",&mLon) ;
		sscanf(inArgs[4].c_str(),"%lf",&mLat) ;
		sscanf(inArgs[5].c_str(),"%f" ,&mHdg) ;
		if(inArgs.size() == 8)
		{
			mName = inArgs[6];
			SetRessource(inArgs[7]);
		}
	}
	else
	if (inArgs.size() == 7 || inArgs.size() == 9)	//ObjectAbs
	{
		SetToTerrain(false);
		sscanf(inArgs[3].c_str(),"%lf",&mLon) ;
		sscanf(inArgs[4].c_str(),"%lf",&mLat) ;
		sscanf(inArgs[5].c_str(),"%lf",&mAlt) ;
		sscanf(inArgs[6].c_str(),"%f" ,&mHdg) ;
		if(inArgs.size() == 9)
		{
			mName = inArgs[7];
			SetRessource(inArgs[8]);
		}
	}
}

void WED_XPluginObject::XPDM_LoadObjectCB(const char* inPath,void * inRef)
{

	WED_XPluginObject * aobj= static_cast<WED_XPluginObject*>(inRef);
    if (aobj->mObjRef) return;
    aobj->mObjRef = XPLMLoadObject(inPath);
}

int  WED_XPluginObject::SetRessource(const string& inPath)
{
	if (inPath == mResPath) return 1;

	SetResPath(inPath);

	if(mObjRef)
	{
		XPLMUnloadObject(mObjRef);
		mObjRef = NULL;
	}

	if (!XPLMLookupObjects(inPath.c_str(),mLat,mLon,XPDM_LoadObjectCB,this))
	{
        string fullpath = mMgrRef->GetPackagePath() + inPath;
		mObjRef = XPLMLoadObject(fullpath.c_str());
		if( mObjRef) return 1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// drawing the Object
void WED_XPluginObject::Draw(bool isLit)
{

	WED_XPluginEntity::Draw(isLit);

    if (!(this->mObjRef)) return;
    if ( mWantDraw )
    {
		float x,y,z;
		WorldToLocal(&x,&y,&z,mSetToTerrain);
		XPLMDrawInfo_t aDrawInfo[1];
		aDrawInfo[0].structSize = sizeof(XPLMDrawInfo_t);
        aDrawInfo[0].x 		 = x;
        aDrawInfo[0].y 		 = y;
        aDrawInfo[0].z 		 = z;
        aDrawInfo[0].pitch 	 = 0;
        aDrawInfo[0].heading = mHdg;
        aDrawInfo[0].roll	 = 0;
        XPLMDrawObjects(this->mObjRef,1,aDrawInfo,isLit,1);
    }
}

