/*
 * Copyright (c) 2011, mroe .
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
#if WITHNWLINK

#include "WED_NWInfoLayer.h"
#include "WED_MapZoomerNew.h"
#include "WED_Server.h"
#include "WED_Messages.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "XESConstants.h"
#include "WED_Colors.h"

#include "GUI_Fonts.h"
#if APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif


WED_NWInfoLayer::WED_NWInfoLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver,WED_NWLinkAdapter * nwlink) :
    WED_MapLayer(h, zoomer, resolver),mNWLink(nwlink)
{
    mColor[0]= .5;
    mColor[1]= .5;
    mColor[2]= .5;
    mColor[3]= 1;
    if(IsVisible()) ToggleVisible();
    mTmpX = mTmpY = 0;
    mDragMode = dm_none;
}

WED_NWInfoLayer::~WED_NWInfoLayer()
{

}

void	WED_NWInfoLayer::DrawStructure(bool inCurrent, GUI_GraphState * g)
{
    if(mNWLink && mNWLink->IsCamEnabled())
    {
        g->SetState(false,0, false,false,false,false,false);
        glColor3f(.5,.1,.1);

        float hX,hY;
        float x = GetZoomer()->LonToXPixel(mNWLink->GetCamLon());
        float y = GetZoomer()->LatToYPixel(mNWLink->GetCamLat());
        float h = mNWLink->GetCamHdg();

        glPointSize(10);
        glBegin(GL_POINTS);
        glVertex2f(x,y);
        glEnd();

        glColor4fv(WED_Color_RGBA(wed_ControlHandle));

        if(mDragMode == dm_heading)
        {
            hX=mTmpX;
            hY=mTmpY;
        }
        else
        {
            if(mDragMode == dm_altitude || mDragMode == dm_pitch)
            {
                glBegin(GL_POINTS);
                glVertex2f(x,mTmpY);
                glEnd();
            }
            hX = x+sin(h * DEG_TO_RAD)*20;
            hY = y+cos(h * DEG_TO_RAD)*20;
        }

        glBegin(GL_LINE);
        glVertex2f(x,y);
        glVertex2f(hX,hY);
        glEnd();
        GUI_PlotIcon(g,"handle_rotatehead.png",hX,hY,h,1.0);

        glPointSize(1);
    }
}
void	WED_NWInfoLayer::DrawVisualization(bool inCurrent, GUI_GraphState * g)
{
    int bnds[4];
    GetHost()->GetBounds(bnds);
    GUI_FontDraw(g, font_UI_Basic, mColor, bnds[2] - 90, bnds[3] - 18,"live mode");
}

void	WED_NWInfoLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
    draw_ent_v = draw_ent_s = cares_about_sel = 0;
    wants_clicks = 1;
}

void	WED_NWInfoLayer::ReceiveMessage(GUI_Broadcaster * inSrc,intptr_t inMsg,intptr_t inParam)
{
    WED_NWLinkAdapter * nwlink = dynamic_cast<WED_NWLinkAdapter *>(inSrc);
    if(!nwlink) return;

    if(inMsg == msg_NetworkStatusInfo )
    {
        switch(inParam)
        {
        case WED_Server::s_newdata :
            break;
        case WED_Server::s_started :
            if (!IsVisible())ToggleVisible();
            break;
        case WED_Server::s_stopped :
            if (IsVisible())ToggleVisible();
            break;
        case WED_Server::s_changed :
            break;

        default :
            return;
        }

        if (nwlink->IsReady())
        {
            mColor[0]= .2 ;
            mColor[1]=  1;
            mColor[2]= .2;
        }
        else
        {
            mColor[0]=  1 ;
            mColor[1]= .2;
            mColor[2]= .2;
        }

        GetHost()->Refresh();
    }
}

int			WED_NWInfoLayer::HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
    if(!mNWLink || mNWLink->IsCamEnabled() == false) return 0;

    float x = GetZoomer()->LonToXPixel(mNWLink->GetCamLon());
    float y = GetZoomer()->LatToYPixel(mNWLink->GetCamLat());

    if(fabsf(inX - x) < 10.0f&&
        fabsf(inY - y) < 10.0f)
    {
        if(modifiers == gui_ControlFlag)
        {
            mTmpY = y;
            mDragMode = dm_altitude;
        }
        else if(modifiers == gui_ShiftFlag)
        {
            mTmpY = y;
            mDragMode = dm_pitch;
            mNWLink->SetCamPit(0);
        }
        else
        {
            mNWLink->SetCamLon(GetZoomer()->XPixelToLon(inX));
            mNWLink->SetCamLat(GetZoomer()->YPixelToLat(inY));
            mDragMode = dm_none;
        }
        mNWLink->SendCamData();
        return 1;
    }

    float psi = mNWLink->GetCamHdg() * DEG_TO_RAD;
    float tx = x+sin(psi)*20;
    float ty = y+cos(psi)*20;

    if (fabsf(inX - tx) < 5.0f&&
        fabsf(inY - ty) < 5.0f)
    {
        mTmpX = inX;
        mTmpY = inY;
        mNWLink->SetCamHdg(atan2(inX-x,inY-y)*RAD_TO_DEG);
        mDragMode = dm_heading;
        mNWLink->SendCamData();
        return 1;
    }

    return 0;
}

void		WED_NWInfoLayer::HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
    if(!mNWLink || mNWLink->IsCamEnabled() == false) return;

    float dx,dy,dt,v;
    switch(mDragMode)
    {
    case  dm_none :

        mNWLink->SetCamLon(GetZoomer()->XPixelToLon(inX));
        mNWLink->SetCamLat(GetZoomer()->YPixelToLat(inY));
        break;

    case dm_heading  :

        mTmpX = inX;
        mTmpY = inY;
        dx = inX-GetZoomer()->LonToXPixel(mNWLink->GetCamLon());
        dy = inY-GetZoomer()->LatToYPixel(mNWLink->GetCamLat());
        mNWLink->SetCamHdg(atan2(dx,dy)*RAD_TO_DEG);
        if(modifiers == gui_ShiftFlag)
        {
            dt = sqrt(dx*dx + dy*dy);
            v = mNWLink->GetCamAlt() * GetZoomer()->GetPPM();
            mNWLink->SetCamPit((atan2(dt,v)*RAD_TO_DEG)-90);
        }
        break;

    case  dm_altitude :

        v = mNWLink->GetCamAlt();
        v += inY-mTmpY;
        if ( v < 2 )v = 2;
        else  mTmpY = inY;
        mNWLink->SetCamAlt(v);
        break;

    case dm_pitch :

        v = mNWLink->GetCamPit();
        v += inY-mTmpY;
        if      ( v < -90 )v = -90;
        else if ( v >  90 )v =  90;
        else  mTmpY = inY;
        mNWLink->SetCamPit(v);
        break;
    }

    mNWLink->SendCamData();
}

void		WED_NWInfoLayer::HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers)
{
    if(!mNWLink || mNWLink->IsCamEnabled() == false) return;

    float dx,dy,dt,v;
    switch(mDragMode)
    {
    case  dm_none :

        mNWLink->SetCamLon(GetZoomer()->XPixelToLon(inX));
        mNWLink->SetCamLat(GetZoomer()->YPixelToLat(inY));
        break;

    case dm_heading :

        mTmpX = inX;
        mTmpY = inY;
        dx = inX-GetZoomer()->LonToXPixel(mNWLink->GetCamLon());
        dy = inY-GetZoomer()->LatToYPixel(mNWLink->GetCamLat());
        mNWLink->SetCamHdg(atan2(dx,dy)*RAD_TO_DEG);
        if(modifiers == gui_ShiftFlag)
        {
            dt = sqrt(dx*dx + dy*dy);
            v = mNWLink->GetCamAlt() * GetZoomer()->GetPPM();
            mNWLink->SetCamPit((atan2(dt,v)*RAD_TO_DEG)-90);
        }
        break;

    case dm_altitude :

        v = mNWLink->GetCamAlt();
        v += inY-mTmpY;
        if ( v < 2 )v = 2;
        else  mTmpY = inY;
        mNWLink->SetCamAlt(v);
        break;

    case dm_pitch :

        v = mNWLink->GetCamPit();
        v += inY-mTmpY;
        if      ( v < -90 )v = -90;
        else if ( v >  90 )v =  90;
        else  mTmpY = inY;
        mNWLink->SetCamPit(v);
        break;

    }

    mDragMode = dm_none;
    mNWLink->SendCamData();
}


#endif
