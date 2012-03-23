/*
 * Copyright (c) 2007, Laminar Research.
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

#include "WED_TerraserverLayer.h"
#include "WED_MapZoomerNew.h"

#include "GUI_Fonts.h"
#include "PCSBSocket.h"
#include "TerraServer.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

inline long long hash_xy(int x, int y) { return ((long long) x << 32) + (long long) y; }


WED_TerraserverLayer::WED_TerraserverLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver) : WED_MapLayer(h, zoomer, resolver)
{
	static bool first_time = true;
	if (first_time)
		PCSBSocket::StartupNetworking(true);
	first_time = false;
	mPool = new AsyncConnectionPool(10,3);
	for(int z = 0; z < NUM_LEVELS; ++z)
	{
		mLocator[z] = new AsyncImageLocator(mPool);
		mHas[z] = 0;
	}
	mData = "1";	// "1" = B&W, "2" = topo, "4" = color orthos.
}

WED_TerraserverLayer::~WED_TerraserverLayer()
{
	delete mPool;
	for (int z = 0; z < NUM_LEVELS; ++z)
	{
		for (map<long long, AsyncImage *>::iterator i = mImages[z].begin(); i != mImages[z].end(); ++i)
		{
			delete i->second;
		}
		delete mLocator[z];
	}
}

void		WED_TerraserverLayer::ToggleVisible(void) { WED_MapLayer::ToggleVisible(); if(IsVisible()) Start(0.1); else Stop(); }


void		WED_TerraserverLayer::DrawVisualization		(bool inCurrent, GUI_GraphState * g)
{
//	if (!inCurrent && !mHas) return;
	double	s, n, e, w;
	GetZoomer()->GetMapVisibleBounds(w, s, e, n);

	int bnds[4];
	GetHost()->GetBounds(bnds);
	int x_lim = (bnds[2]-bnds[0]) / 200 * 2 + 2;	// Fudge-factor - double our max res...
	int y_lim = (bnds[3]-bnds[1]) / 200 * 2 + 2;

	int top_res			= -1;
	int	total_vis		=  0;
	int	total_now		=  0;
	int total_err		=  0;
	int total_loaded	=  0;

	static int our_gen = 0;
	++our_gen;

	for(int z = (NUM_LEVELS-1); z >= 0; --z)
	{
		total_loaded += mImages[z].size();
		if (mLocator[z]->GetLocation(ResString(z), mData.c_str(), w, s, e, n, mX1[z], mX2[z], mY1[z], mY2[z], mDomain[z], mStatus))
		{
			mHas[z] = 1;
			if(mX2[z]-mX1[z] < x_lim && mY2[z]-mY1[z] < y_lim)
			{
				top_res = z;
				for (int x = mX1[z]; x < mX2[z]; ++x)
				for (int y = mY1[z]; y < mY2[z]; ++y)
				{
					++total_vis;
					long long h = hash_xy(x,y);
					if (mImages[z].count(h) == 0)
					{
						mImages[z][h] = new AsyncImage(mPool,ResString(z), mData.c_str(), mDomain[z], x, y);
					}

					AsyncImage * i = mImages[z][h];

					i->SetGen(our_gen);

					if (i->HasErr())
						++total_err;
					else
					{
						ImageInfo * bits = i->GetImage();
						if (bits)
						{
							double	coords[4][2];
							if (i->GetCoords(coords))
							{
								for (int n = 0; n < 4; ++n)
								{
									coords[n][0] = GetZoomer()->LatToYPixel(coords[n][0]);
									coords[n][1] = GetZoomer()->LonToXPixel(coords[n][1]);
								}
                                i->Draw(coords, g);
								++total_now;
							}
						}
					}
				}
			}
		} else
			mHas[z] = 0;
	}


	char buf[1024];
	sprintf(buf,"%d of %d (%f%% done, %d errors).  %d ppm Mem use: %d %s",total_now,total_vis,total_vis ? (100.0 * (float) total_now / (float) total_vis) : 100.0, total_err, top_res == -1 ? 0 : (1 << top_res), total_loaded, mStatus.c_str());
	float clr[4] = { 1, 1, 1, 1 };
	GUI_FontDraw(g, font_UI_Basic, clr, bnds[0] + 10, bnds[1] + 10, buf);

	if (total_loaded > 2000)
	{
		multimap<int, pair<int, long long> > kill_list;

		for (int z = 0; z < NUM_LEVELS; ++z)
		{
			for (map<long long, AsyncImage *>::iterator i = mImages[z].begin(); i != mImages[z].end(); ++i)
			if (i->second->GetGen() != our_gen)
				kill_list.insert(map<int, pair<int, long long> >::value_type(i->second->GetGen(),pair<int, long long>(z,i->first)));
		}

		while(!kill_list.empty() && total_loaded > 2000)
		{
			delete mImages[kill_list.begin()->second.first][kill_list.begin()->second.second];
			mImages[kill_list.begin()->second.first].erase(kill_list.begin()->second.second);
			--total_loaded;
			kill_list.erase(kill_list.begin());
		}
	}
}

const char *	WED_TerraserverLayer::ResString(int res)
{
	res = 1L << res;
	static char	resbuf[256];
	sprintf(resbuf, "%dm", res);
	return resbuf;
}

void	WED_TerraserverLayer::TimerFired(void)
{
	GetHost()->Refresh();
}

void		WED_TerraserverLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}
