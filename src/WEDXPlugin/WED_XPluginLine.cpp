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

#include "WED_XPluginLine.h"
#include "WED_XPluginLinNode.h"
#include "WED_XPluginDrawUtils.h"

#include "stdio.h"

#if LIN || IBM
#include <GL/gl.h>
#else
#if __GNUC__
#include <OpenGL/Gl.h>
#else
#include <gl.h>
#endif
#endif

#define LINE_OFFSET 0.1

WED_XPluginLine::WED_XPluginLine(WED_XPluginMgr * inRef,const vector<string>& inArgs):
    WED_XPluginEntity(nw_obj_Line,inRef),
    mIsClosed(false)
{
    this->Update(inArgs);
}
WED_XPluginLine::WED_XPluginLine(WED_XPluginMgr * inRef):
    WED_XPluginEntity(nw_obj_Line,inRef),
    mIsClosed(false)
{

}

WED_XPluginLine::~WED_XPluginLine()
{
    vector<WED_XPluginBezierNode *>::iterator it ;
    for(it = mNodes.begin(); it != mNodes.end(); ++it)
    {
        if(*it) (*it)->SetParent(NULL);
    }
}

void WED_XPluginLine::Update(const vector<string>& inArgs)
{
    if (inArgs.size() == 6)
    {
        sscanf(inArgs[3].c_str(),"%d",&mIsClosed);
        mName       = inArgs[4];
        mResPath    = inArgs[5];
    }
}

///////////////////////////////////////////////////////////////////////////////////
// drawing
void WED_XPluginLine::Draw(bool isLit)
{
    int cnt = mNodes.size();
    if(cnt < 2) return;

    XPLMSetGraphicsState(0, 0, 0, 0, 0, 1, 1);
    glColor4f(.0, .5, .5 ,1);

    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    for(int i = 0 ; i < cnt; ++i)
    {

        Point3   p;
        Bezier3  b;
        WED_XPluginBezierNode * aNode_1;
        WED_XPluginBezierNode * aNode_2;

        aNode_1 = mNodes[i];
        if(i == cnt-1)
        {
            if(mIsClosed) aNode_2 = mNodes[0];
            else  break;
        }
        else aNode_2 = mNodes[i+1];

        aNode_1->GetPos(&b.p1.x,&b.p1.y,&b.p1.z);
        aNode_1->GetPosHi(&b.c1.x,&b.c1.y,&b.c1.z);
        aNode_2->GetPosLo(&b.c2.x,&b.c2.y,&b.c2.z);
        aNode_2->GetPos(&b.p2.x,&b.p2.y,&b.p2.z);

        int point_count = 1;

        if(aNode_1->HasBez()||aNode_2->HasBez())
        {
            int approx = sqrt(Vector3(b.p1,b.c1).squared_length()) +
                         sqrt(Vector3(b.c1,b.c2).squared_length()) +
                         sqrt(Vector3(b.c2,b.p2).squared_length());

            point_count =  approx / BEZ_MTR_PER_SEG;
            if (point_count < BEZ_MIN_SEGS) point_count = BEZ_MIN_SEGS;
            if (point_count > BEZ_MAX_SEGS) point_count = BEZ_MAX_SEGS;
        }

        for (int j = 0; j < point_count; ++j)
        {
            p = b.midpoint((float) j / (float) (point_count));
            glVertex3f(p.x,p.y+LINE_OFFSET,p.z);
        }

        if(i == cnt-1 || (i == cnt-2 && !mIsClosed))
        {
            glVertex3f(b.p2.x,b.p2.y+LINE_OFFSET,b.p2.z);
        }
    }
    glEnd();
    glLineWidth(1);
}

