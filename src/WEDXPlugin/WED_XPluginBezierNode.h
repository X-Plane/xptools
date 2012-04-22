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
#ifndef WED_XPLUGINBEZIERNODE_H
#define WED_XPLUGINBEZIERNODE_H

#include "WED_XPluginNode.h"
#include "WED_XPluginMgr.h"
#include "XPLMScenery.h"

class WED_XPluginMgr;

class WED_XPluginBezierNode : public WED_XPluginNode
{
    public:
        WED_XPluginBezierNode(int inTyp,WED_XPluginMgr * inRef);
        virtual ~WED_XPluginBezierNode();

        virtual void    Draw(bool isLit);
        virtual void    UpdatePos();
        virtual void    SceneryShift(){UpdatePos();}
        bool            HasBez(){return mHasBez;}
        void            HasBez(bool inHasBez){mHasBez = inHasBez;}
        double          GetLatLo(){return mLat_lo;}
        double          GetLonLo(){return mLon_lo;}
        void            GetPosLo(double * outX,double * outY,double * outZ);
        double          GetLatHi(){return mLat_hi;}
        double          GetLonHi(){return mLon_hi;}
        void            GetLocLo(double * outLat,double * outLon,double * outAlt);
        void            GetPosHi(double * outX,double * outY,double * outZ);
        void            GetLocHi(double * outLat,double * outLon,double * outAlt);
        void            SetParam(int inVal){mParam1 = inVal;}
        int             GetParam(){return mParam1;}

    protected:
    private:

        friend class WED_XPluginFacNode;
        friend class WED_XPluginLinNode;

        bool            mHasBez;
        int             mParam1;
        double          mX_lo;
        double          mY_lo;
        double          mZ_lo;
        double          mX_hi;
        double          mY_hi;
        double          mZ_hi;
        double          mLat_lo;
        double          mLon_lo;
        double          mAlt_lo;
        double          mLat_hi;
        double          mLon_hi;
        double          mAlt_hi;

        static void     WorldTolocal(WED_XPluginBezierNode * inBez);

};

#endif // WED_XPLUGINBEZIERNODE_H
