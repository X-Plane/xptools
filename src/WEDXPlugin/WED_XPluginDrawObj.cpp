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

#include "WED_XPluginDrawObj.h"
#include "XPLMGraphics.h"
#include "WED_XPluginMgr.h"

///////////////////////////////////////////////////////////////////////////////////
//WED_XPluginDrawObj constructor

WED_XPluginDrawObj::WED_XPluginDrawObj(XPLMObjectRef inObjRef,WED_XPluginMgr * inRef):
		WED_XPluginNode(nw_obj_Object,inRef),
		mWantDraw(true),mDrawVertex(true),mObjRef(inObjRef)
{

}
///////////////////////////////////////////////////////////////////////////////////
//WED_XPluginDrawObj destructor
WED_XPluginDrawObj::~WED_XPluginDrawObj()
{
	if(mObjRef)XPLMUnloadObject(mObjRef);
}

///////////////////////////////////////////////////////////////////////////////////
// drawing the Object
void WED_XPluginDrawObj::Draw(bool isLit)
{
	if(mDrawVertex) WED_XPluginNode::Draw(isLit);

    if (!(this->mObjRef)) return;
    if ( mWantDraw )
    {
        XPLMDrawInfo_t aDrawInfo[1];
        aDrawInfo[0].structSize = sizeof(XPLMDrawInfo_t);
        aDrawInfo[0].x 		 = mX;
        aDrawInfo[0].y 		 = mY;
        aDrawInfo[0].z 		 = mZ;
        aDrawInfo[0].pitch 	 = 0;
        aDrawInfo[0].heading = mHdg;
        aDrawInfo[0].roll	 = 0;
        XPLMDrawObjects(this->mObjRef,1,aDrawInfo,isLit,1);
    }
}

