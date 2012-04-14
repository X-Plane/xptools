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
#ifndef WED_XPLUGINNODE_H
#define WED_XPLUGINNODE_H

#include "XPLMScenery.h"
#include "WED_XPluginEntity.h"

class WED_XPluginMgr;

class WED_XPluginNode : public WED_XPluginEntity
{
public:
    WED_XPluginNode(int inTyp,WED_XPluginMgr * inRef);
    virtual ~WED_XPluginNode();

    virtual void         Draw(bool isLit);
    virtual void         SceneryShift(){WorldToLocal(this);}
    virtual void         UpdatePos(){WorldToLocal(this);}

            bool         GetDrawVertex(){return mWantDraw;}
            void         SetDrawVertex(bool inWantDraw){mWantDraw=inWantDraw;}

            bool         GetToTerrain(){return mSetToTerrain;}
            void         SetToTerrain(bool inToTerrain){mSetToTerrain = inToTerrain;}

            void         SetLoc(const double inLat,const double intLon,const double inAlt);
            void         GetLoc(double * outLat,double * outLon,double * outAlt);
            void         SetPos(const double inX,const double inY,const double inZ);
            void         GetPos(double * outX,double * outY,double * outZ);

            void         SetHdg(float inHdg){mHdg = inHdg;}
            float        GetHdg(){return mHdg;}

            void         X(const double inVal){mX = inVal;}
            void         Y(const double inVal){mY = inVal;}
            void         Z(const double inVal){mZ = inVal;}

            double       X(){return mX;}
            double       Y(){return mY;}
            double       Z(){return mZ;}

protected:
private:

    friend class WED_XPluginObject;
    friend class WED_XPluginDrawObj;
//    friend class WED_XPluginBezierNode;
//    friend class WED_XPluginFacNode;
//    friend class WED_XPluginLinNode;
//    friend class WED_XPluginFacade;

    bool         mWantDraw;

    XPLMProbeRef mProbeRef;

    double       mX;
    double       mY;
    double       mZ;

    double       mLon;
    double       mLat;
    double       mAlt;
    float        mHdg;

    bool         mSetToTerrain;

    static void  WorldToLocal(WED_XPluginNode * inNode);

};

#endif // WED_XPLUGINNODE_H
