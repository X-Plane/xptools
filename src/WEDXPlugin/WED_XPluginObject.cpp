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
#include "WED_XPluginObject.h"
#include "WED_XPluginDrawUtils.h"
#include "WED_XPluginMgr.h"

#include <sstream>
#include "stdio.h"
#include "string.h"

#define AGPVERS 1000

///////////////////////////////////////////////////////////////////////////////////
//WED_XPluginObj constructor
WED_XPluginObject::WED_XPluginObject(WED_XPluginMgr * inRef,const vector<string>& inArgs):
    WED_XPluginNode(nw_obj_Object,inRef),
    mIsAGP(false),mWantDraw(true)
{
    this->Update(inArgs);
}

WED_XPluginObject::WED_XPluginObject(WED_XPluginMgr * inRef):
    WED_XPluginNode(nw_obj_Object,inRef),
    mIsAGP(false),mWantDraw(true)
{

}
///////////////////////////////////////////////////////////////////////////////////
//WED_XPluginObj destructor
WED_XPluginObject::~WED_XPluginObject()
{
    ClearObjs();
}

void WED_XPluginObject::ClearObjs()
{
    vector<WED_XPluginDrawObj *>::iterator it;
    for(it = mObjs.begin(); it != mObjs.end(); ++it)
        if (*it) delete (*it);

    mObjs.clear();
}



void WED_XPluginObject::UpdatePos()
{
    WED_XPluginNode::UpdatePos();

    double x,z,lat,lon,alt;

    vector<WED_XPluginDrawObj *>::iterator it;
    for(it = mObjs.begin(); it != mObjs.end(); ++it)
    {
        WED_XPluginDrawObj * obj = *it;
        if(obj == NULL) continue;

        rotate2d(mHdg,obj->oX(),-obj->oY(),mX,mZ,&x,&z);

        XPLMLocalToWorld(x,mY,z,&lat,&lon,&alt);
        obj->SetLoc(lat,lon,alt);
        obj->UpdatePos();
        obj->SetHdg(mHdg+obj->oH());
    }
}

void WED_XPluginObject::Update(const vector<string>& inArgs)
{
    if (inArgs.size() == 6 || inArgs.size() == 8)	//Object
    {
        sscanf(inArgs[3].c_str(),"%lf",&mLon);
        sscanf(inArgs[4].c_str(),"%lf",&mLat);
        sscanf(inArgs[5].c_str(),"%f" ,&mHdg);

        if(inArgs.size() == 8)
        {
            SetName(inArgs[6]);
            SetRessource(inArgs[7]);
        }
        SetToTerrain(true);
        UpdatePos();
    }
    else if (inArgs.size() == 7 || inArgs.size() == 9)	//ObjectAbs
    {
        sscanf(inArgs[3].c_str(),"%lf",&mLon);
        sscanf(inArgs[4].c_str(),"%lf",&mLat);
        sscanf(inArgs[5].c_str(),"%lf",&mAlt);
        sscanf(inArgs[6].c_str(),"%f" ,&mHdg);
        if(inArgs.size() == 9)
        {
            SetName(inArgs[7]);
            SetRessource(inArgs[8]);
        }
        SetToTerrain(false);
        UpdatePos();
    }
}
int WED_XPluginObject::DoParseAGP(const string& inPath,WED_XPluginMgr * inMgr,WED_XPluginObject * inObj)
{

    char PS[1];
    char msg[512];
    string  dir("");
    strcpy(PS,XPLMGetDirectorySeparator());
    size_t pos = inPath.find_last_of(PS);
    if(pos != string::npos) dir = inPath.substr(0,pos+1);

    FILE * fi = fopen(inPath.c_str(),"r") ;
    if (!fi)
    {
        sprintf(msg,"wedxpl: ERROR could not open %s\n",inPath.c_str());
        XPLMDebugString(const_cast<char *>(msg));
        return false;
    }

    char	buf[512];
    char	buf2[512];
    int 	vers = -1;

    fgets(buf, sizeof(buf), fi);
    fgets(buf, sizeof(buf), fi);
    sscanf(buf," %d", &vers);
    fgets(buf, sizeof(buf), fi);
    sscanf(buf," %s", buf2);
    if (strncmp(buf,"AG_POINT", strlen("AG_POINT")) || vers < AGPVERS)
    {
        sprintf(msg,"wedxpl: ERROR fileheader wrong or bad version in file: %s\n",inPath.c_str());
        XPLMDebugString(const_cast<char *>(msg));
        fclose(fi);
        return false;
    }

    vector<string>objpaths;

    int p1,p2;
    int rotation = 0;
    unsigned int idx;
    double s1,t1,s2,t2,x,y,h;
    double tex_s = 1.0, tex_t = 1.0;		// these scale from pixels to UV coords
    double tex_x = 1.0, tex_y = 1.0;		// meters for tex, x & y
    double anchor_x = 0 , anchor_y = 0;

    while (fgets(buf, sizeof(buf), fi))
    {
        if (strlen(buf) == 1) continue;

        if(sscanf(buf,"TEXTURE_SCALE %lf %lf",&x,&y) == 2)
        {
            tex_s = 1.0 / x;
            tex_t = 1.0 / y;
        }
        else if(sscanf(buf,"TEXTURE_WIDTH %lf",&x) == 1)
        {
            tex_x = x;
            tex_y = tex_x * tex_s / tex_t;
        }
        else if (sscanf(buf,"OBJECT %[^\r\n]",buf2) == 1)
        {
            vector<string> paths;
            XPLMLookupObjects(buf2,0,0,LoadObjectCB,&paths);
            if(paths.empty()) paths.push_back(dir + buf2);

            objpaths.push_back(paths[0]);
            continue;
        }
        else if (sscanf(buf,"TILE %lf %lf %lf %lf ",&s1,&t1,&s2,&t2) == 4)
        {

            double x1 = s1 * tex_s * tex_x;
            double x2 = s2 * tex_s * tex_x;
            double y1 = t1 * tex_t * tex_y;
            double y2 = t2 * tex_t * tex_y;

            s1 *= tex_s;
            s2 *= tex_s;
            t1 *= tex_t;
            t2 *= tex_t;

            anchor_x = (x1 + x2) * 0.5;
            anchor_y = (y1 + y2) * 0.5;

            //FIXME:mroe fill in


        }
        else if( (sscanf(buf,"OBJ_DRAPED %lf %lf %lf %u ",&x,&y,&h,&idx) == 4             )||
                 (sscanf(buf,"OBJ_DRAPED %lf %lf %lf %u %d %d",&x,&y,&h,&idx,&p1,&p2) == 6) )
        {
            if(idx > objpaths.size()-1) continue;
            XPLMObjectRef oref = XPLMLoadObject(objpaths[idx].c_str());
            WED_XPluginDrawObj * obj = new WED_XPluginDrawObj(oref ,inMgr);
            obj->SetToTerrain(true);
            obj->SetDrawVertex(false);
            obj->SetOffsets(x*tex_s*tex_x , y*tex_t*tex_y ,h);
            obj->SetHdg(h);
            inObj->AddObj(obj);
            continue;
        }
        else if( (sscanf(buf,"OBJ_GRADED %lf %lf %lf %u ",&x,&y,&h,&idx) == 4             )||
                 (sscanf(buf,"OBJ_GRADED %lf %lf %lf %u %d %d",&x,&y,&h,&idx,&p1,&p2) == 6) )
        {
            if(idx > objpaths.size()-1) continue;
            XPLMObjectRef oref = XPLMLoadObject(objpaths[idx].c_str());
            WED_XPluginDrawObj * obj = new WED_XPluginDrawObj(oref ,inMgr);
            obj->SetToTerrain(false);
            obj->SetDrawVertex(false);
            obj->SetOffsets(x*tex_s*tex_x , y*tex_t*tex_y ,h);
            obj->SetHdg(h);
            inObj->AddObj(obj);
            continue;
        }
        else if (sscanf(buf,"ANCHOR_PT %lf %lf",&x,&y) == 2)
        {
            anchor_x = x*tex_s*tex_x;
            anchor_y = y*tex_t*tex_y;
            continue;
        }
        else if (sscanf(buf,"ROTATION %d ",&rotation) == 1)
        {
            //FIXME:TODO:mroe still missing
            continue;
        }
    }

    if(anchor_x + anchor_x + rotation != 0 )
    {
        int ocnt = inObj->GetObjCnt();
        for(int i = 0 ; i < ocnt ; ++i)
        {
            //TODO:mroe implement rotation
            WED_XPluginDrawObj * obj = inObj->GetObjAt(i);

            obj->oX(obj->oX()-anchor_x);
            obj->oY(obj->oY()-anchor_y);
        }
    }

    if (fi) fclose(fi);

    return true;
}

void WED_XPluginObject::LoadObjectCB(const char* inPath,void * inRef)
{
    vector<string> * paths = static_cast<vector<string>  *>(inRef);
    paths->push_back(inPath);
}

int  WED_XPluginObject::SetRessource(const string& inPath)
{
    if (inPath == mResPath) return 1;

    SetResPath(inPath);
    ClearObjs();

    vector<string> paths;
    XPLMLookupObjects(inPath.c_str(),mLat,mLon,LoadObjectCB,&paths);
    if(paths.empty()) paths.push_back(mMgrRef->GetPackagePath() + inPath);

    string path = paths[0];

    size_t pos = inPath.find_last_of('.');
    if(pos == string::npos) return 0;
    string  suffix = inPath.substr(pos);

    if(suffix == ".obj")
    {
        mIsAGP = false;
        WED_XPluginDrawObj * obj = new WED_XPluginDrawObj(XPLMLoadObject(path.c_str()),mMgrRef);
        obj->SetToTerrain(false);
        obj->SetDrawVertex(false);
        mObjs.push_back(obj);
    }
    else if(suffix == ".agp")
    {
        mIsAGP = true;
        //DoParseAGP(path,mMgrRef,this);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// drawing the structure
void WED_XPluginObject::Draw(bool isLit)
{
    WED_XPluginNode::Draw(isLit);

    if ( !mWantDraw ) return;

    vector<WED_XPluginDrawObj *>::iterator it;
    for(it = mObjs.begin(); it != mObjs.end(); ++it)
        (*it)->Draw(isLit);
}

