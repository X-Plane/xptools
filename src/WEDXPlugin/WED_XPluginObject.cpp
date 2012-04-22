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
#include "WED_XPluginDrawObj.h"
#include "WED_XPluginFacade.h"
#include "WED_XPluginFacRing.h"
#include "WED_XPluginBezierNode.h"
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
    ClearFacs();
    ClearTile();
}

void WED_XPluginObject::ClearTile()
{
    vector<WED_XPluginNode*>::iterator it;
    for(it = mTile.begin(); it != mTile.end(); ++it)
        if (*it) delete (*it);

    mTile.clear();
}

void WED_XPluginObject::ClearObjs()
{
    vector<WED_XPluginDrawObj *>::iterator it;
    for(it = mObjs.begin(); it != mObjs.end(); ++it)
        if (*it) delete (*it);

    mObjs.clear();
}

void WED_XPluginObject::ClearFacs()
{
    vector<WED_XPluginFacade *>::iterator it;
    for(it = mFacs.begin(); it != mFacs.end(); ++it)
    {
        WED_XPluginFacade * fac = *it;
        for(int i = 0 ; i < fac->GetRingCnt(); ++i)
        {
            WED_XPluginFacRing * ring = fac->GetRingAt(0);
            for(int j = 0 ; j < ring->GetNodeCnt(); ++j)
            {
                delete ring->GetNodeAt(j);
            }
            delete ring;
        }
        if (*it) delete (*it);
    }
    mFacs.clear();
}

void WED_XPluginObject::UpdatePos()
{
    WED_XPluginNode::UpdatePos();

    if(!mIsAGP && mObjs.size() == 1)
    {
        WED_XPluginDrawObj * obj = mObjs.at(0);
        obj->SetPos(mX,mY,mZ);
        obj->SetHdg(mHdg);
        return;
    }

    double x,y = mY,z,lat,lon,alt;

    vector<WED_XPluginNode*>::iterator nit;
    for(nit = mTile.begin(); nit != mTile.end(); ++nit)
    {
        WED_XPluginNode * node = *nit;
        if(node == NULL) continue;
        rotate2d(mHdg,node->oX(),-node->oY(),mX,mZ,&x,&z);
        XPLMLocalToWorld(x,y,z,&lat,&lon,&alt);
        node->SetLoc(lat,lon,alt);
        node->UpdatePos();
        if(nit == mTile.begin()) y = node->Y();
    }

    vector<WED_XPluginDrawObj *>::iterator it;
    for(it = mObjs.begin(); it != mObjs.end(); ++it)
    {
        WED_XPluginDrawObj * obj = *it;
        if(obj == NULL) continue;
        rotate2d(mHdg,obj->oX(),-obj->oY(),mX,mZ,&x,&z);
        XPLMLocalToWorld(x,y,z,&lat,&lon,&alt);
        obj->SetLoc(lat,lon,alt);
        obj->SetHdg(mHdg+obj->oH());
        obj->UpdatePos();
    }

    vector<WED_XPluginFacade *>::iterator fit;
    for(fit = mFacs.begin(); fit != mFacs.end(); ++fit)
    {
        WED_XPluginFacade * fac = *fit;
        if(fac == NULL) continue;
        for(int j = 0 ; j < fac->GetRingCnt() ; ++j)
        {
            WED_XPluginFacRing * ring = fac->GetRingAt(j);
            if(ring == NULL) continue;
            for(int k = 0 ; k < ring->GetNodeCnt() ; ++k)
            {
                WED_XPluginBezierNode * node = ring->GetNodeAt(k);
                if(node == NULL) continue;
                rotate2d(mHdg,node->oX(),-node->oY(),mX,mZ,&x,&z);
                XPLMLocalToWorld(x,y,z,&lat,&lon,&alt);
                node->SetLoc(lat,lon,alt);
                node->UpdatePos();
            }
        }
        fac->UpdatePos();
    }
}

void WED_XPluginObject::Update(const vector<string>& inArgs)
{
    if (inArgs.size() == 6 || inArgs.size() == 8)	//Object
    {
        sscanf(inArgs[3].c_str(),"%lf",&mLon);
        sscanf(inArgs[4].c_str(),"%lf",&mLat);
        sscanf(inArgs[5].c_str(),"%lf",&mHdg);

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
        sscanf(inArgs[6].c_str(),"%lf",&mHdg);
        if(inArgs.size() == 9)
        {
            SetName(inArgs[7]);
            SetRessource(inArgs[8]);
        }
        SetToTerrain(mIsAGP);
        UpdatePos();
    }
}
int WED_XPluginObject::DoParseAGP(const string& inPath,WED_XPluginMgr * inMgr,WED_XPluginObject * inObj)
{

    char PS[1];
    string  dir("");
    strcpy(PS,XPLMGetDirectorySeparator());
    size_t pos = inPath.find_last_of(PS);
    if(pos != string::npos) dir = inPath.substr(0,pos+1);

    FILE * fi = fopen(inPath.c_str(),"r") ;
    if (!fi) return false;

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
        fclose(fi);
        return false;
    }

    vector<string>objpaths;
    vector<double>tiles;

    int p1,p2;
    int rotation = 0;
    unsigned int idx;
    double s1,t1,s2,t2,x,y,h;
    double tex_s = 1.0, tex_t = 1.0;		// these scale from pixels to UV coords
    double tex_x = 1.0, tex_y = 1.0;		// meters for tex, x & y
    double anchor_x = 0 , anchor_y = 0;
    double ground_x = 0 , ground_y = 0;

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

            anchor_x = ground_x = (x1 + x2) * 0.5;
            anchor_y = ground_y = (y1 + y2) * 0.5;

            tiles.resize(16);
			tiles[ 0] = x1;
			tiles[ 1] = y1;
			tiles[ 2] = s1;
			tiles[ 3] = t1;
			tiles[ 4] = x2;
			tiles[ 5] = y1;
			tiles[ 6] = s2;
			tiles[ 7] = t1;
			tiles[ 8] = x2;
			tiles[ 9] = y2;
			tiles[10] = s2;
			tiles[11] = t2;
			tiles[12] = x1;
			tiles[13] = y2;
			tiles[14] = s1;
			tiles[15] = t2;
        }
		else if(sscanf(buf,"CROP_POLY %[^\r\n]",buf2) == 1)
        {
            tiles.clear();
            bool go_on = true;
            while(go_on)
            {
                int result = sscanf(buf2,"%lf %lf %[^\r\n]",&s1,&t1,buf2);
                if(result < 2) break;
                tiles.push_back(s1 * tex_s * tex_x);
                tiles.push_back(t1 * tex_t * tex_y);
                tiles.push_back(s1 * tex_s);
                tiles.push_back(t1 * tex_t);
                go_on = (result == 3 );
            }
            continue;
		}
        else if( (sscanf(buf,"OBJ_DRAPED %lf %lf %lf %u ",&x,&y,&h,&idx) == 4             )||
                 (sscanf(buf,"OBJ_DRAPED %lf %lf %lf %u %d %d",&x,&y,&h,&idx,&p1,&p2) == 6) )
        {
            if(idx > objpaths.size()-1) continue;
            XPLMObjectRef oref = XPLMLoadObject(objpaths[idx].c_str());
            WED_XPluginDrawObj * obj = new WED_XPluginDrawObj(oref ,inMgr);
            obj->SetToTerrain(true);
            obj->SetDrawVertex(true);
            obj->SetOffsets(x*tex_s*tex_x , y*tex_t*tex_y ,h);
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
            obj->SetDrawVertex(true);
            obj->SetOffsets(x*tex_s*tex_x , y*tex_t*tex_y ,h);
            inObj->AddObj(obj);
            continue;
        }
        else if( (sscanf(buf,"FAC %d %lf %[^\r\n]",&idx,&h,buf2) == 3     )||
                 (sscanf(buf,"FAC_WALLS %d %lf %[^\r\n]",&idx,&h,buf2) == 3) )
        {
            bool has_wall = !strncmp("FAC_WALLS",buf,strlen("FAC_WALLS"));
            WED_XPluginFacade * fac   = new WED_XPluginFacade(inMgr);
            WED_XPluginFacRing * ring = new WED_XPluginFacRing(inMgr);
            fac->AddRing(ring);
            fac->SetHasWall(has_wall);
            fac->SetHeight(2.0);
            fac->SetTopo(2);
            int nidx = 0 , result = 0;
            bool go_on = true;
            while(go_on)
            {
                if(has_wall)
                {
                    result = sscanf(buf2,"%lf %lf %d %[^\r\n]",&x,&y,&p1,buf2);
                    if(result < 3) break;
                }
                else
                {
                    result = sscanf(buf2,"%lf %lf %[^\r\n]",&x,&y,buf2);
                    p1 = 0;
                    if(result < 2) break;
                }

                WED_XPluginBezierNode * node = new WED_XPluginBezierNode(nw_obj_FacadeNode,inMgr);

                node->oX(x*tex_s*tex_x);
                node->oY(y*tex_t*tex_y);
                node->SetParam(p1);
                node->SetIdx(nidx++);
                ring->AddNode(node);
                go_on = ((has_wall && result == 4) || (!has_wall && result == 3) );
            }
            inObj->AddFac(fac);
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

    WED_XPluginNode * node = new WED_XPluginNode(0,inMgr);
    node->oX(ground_x - anchor_x);
    node->oY(ground_y - anchor_y);
    inObj->AddTileNode(node);

    for(unsigned int n = 0; n < tiles.size(); n += 4)
	{
	    node = new WED_XPluginNode(0,inMgr);

		node->oX(tiles[n  ] - anchor_x);
		node->oY(tiles[n+1] - anchor_y);
		inObj->AddTileNode(node);
		 //TODO:mroe implement rotation
		 //do_rotate(rotation,out_info.tile[n  ],out_info.tile[n+1]);
	}

    for(unsigned int i = 0 ; i < inObj->GetObjCnt() ; ++i)
    {
        //TODO:mroe implement rotation
        WED_XPluginDrawObj * obj = inObj->GetObjAt(i);
        obj->oX(obj->oX()-anchor_x);
        obj->oY(obj->oY()-anchor_y);
    }

    for(unsigned int i = 0 ; i < inObj->GetFacCnt() ; ++i)
    {
        //TODO:mroe implement rotation
        WED_XPluginFacade * fac = inObj->GetFacAt(i);
        for(int j = 0 ;j < fac->GetRingCnt() ; ++j)
        {
            WED_XPluginFacRing * ring = fac->GetRingAt(j);
            for(int k = 0 ;k < ring->GetNodeCnt() ; ++k)
            {
                WED_XPluginBezierNode * node = ring->GetNodeAt(k);
                node->oX(node->oX()-anchor_x);
                node->oY(node->oY()-anchor_y);
            }
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
    ClearFacs();
    ClearTile();

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
        return 1;
    }
    else if(suffix == ".agp")
    {
        mIsAGP = true;
        //DoParseAGP(path,mMgrRef,this);
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// drawing the structure
void WED_XPluginObject::Draw(bool isLit)
{
    WED_XPluginNode::Draw(isLit);

    if ( !mWantDraw ) return;

    if(mTile.size() > 1)
    {
        mTile[0]->Draw(isLit);

        XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0);
        glColor4f(0.5, 0.5, 0.5 ,0.4);
        glDisable(GL_CULL_FACE);
        glBegin(GL_TRIANGLE_FAN);
        vector<WED_XPluginNode*>::iterator nit;
        for(nit = mTile.begin(); nit != mTile.end(); ++nit)
        {
            if(nit == mTile.begin()) continue;
            glVertex3f((*nit)->X(),(*nit)->Y(),(*nit)->Z());
        }
        glEnd();
        glEnable(GL_CULL_FACE);
    }

    vector<WED_XPluginDrawObj *>::iterator it;
    for(it = mObjs.begin(); it != mObjs.end(); ++it)
        (*it)->Draw(isLit);

    vector<WED_XPluginFacade *>::iterator fit;
    for(fit = mFacs.begin(); fit != mFacs.end(); ++fit)
        (*fit)->Draw(isLit);
}

