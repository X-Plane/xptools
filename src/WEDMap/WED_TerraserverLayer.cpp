#include "WED_TerraserverLayer.h"
#include "WED_MapZoomerNew.h"

#include "GUI_Fonts.h"
#include "PCSBSocket.h"
#include "TerraServer.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

inline long long hash_xy(int x, int y) { return ((long long) x << 32) + (long long) y; }


WED_TerraserverLayer::WED_TerraserverLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver) : WED_MapLayer(h, zoomer, resolver)
{
	static bool first_time = true;
	if (first_time)
		PCSBSocket::StartupNetworking(true);
	first_time = false;
	mLocator = new AsyncImageLocator;
	mHas = 0;
	mRes = 4;
	mData = "1";
}

WED_TerraserverLayer::~WED_TerraserverLayer()
{
	for (map<long long, AsyncImage *>::iterator i = mImages.begin(); i != mImages.end(); ++i)
		delete i->second;
	delete mLocator;
}

void		WED_TerraserverLayer::DrawVisualization		(int inCurrent, GUI_GraphState * g)
{
//	if (!inCurrent && !mHas) return;
	double	s, n, e, w;
	GetZoomer()->GetMapVisibleBounds(w, s, e, n);

	if (mLocator->GetLocation(ResString(), mData.c_str(), w, s, e, n, mX1, mX2, mY1, mY2, mDomain))
	{
		mHas = 1;
		for (int x = mX1; x < mX2; ++x)
		for (int y = mY1; y < mY2; ++y)
		{
			long long h = hash_xy(x,y);
			if (mImages.count(h) == 0)
			{
				mImages[h] = new AsyncImage(ResString(), mData.c_str(), mDomain, x, y);
			}
			
			AsyncImage * i = mImages[h];
			
			if (!i->HasErr())
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
					}
				}
			}
		}
	} else 
		mHas = 0;
		
	{
		int total = 0, done = 0, pending = 0, bad = 0;
		for (map<long long, AsyncImage *>::iterator i = mImages.begin(); i != mImages.end(); ++i)
		{
			if (i->second->HasErr()) ++bad;
			else if (i->second->IsDone()) ++done;
			else ++pending;
			++total;
		}
		
		static char buf[1024];
		if (mHas)
			sprintf(buf, "Domain=%d, X=%d-%d,Y=%d-%d Done=%d Pending=%d Bad=%d Total=%d Sockets=%d", mDomain, mX1,mX2,mY1,mY2, done, pending, bad, total, AsyncImage::TotalPending());
		else 
			sprintf(buf, "No area established.");
		
		int bnds[4];
		GetHost()->GetBounds(bnds);
		float clr[4] = { 1, 1, 1, 1 };
		GUI_FontDraw(g, font_UI_Basic, clr, bnds[0] + 10, bnds[1] + 10, buf);
	}	
	GetHost()->Refresh();
}

const char *	WED_TerraserverLayer::ResString(void)
{
	static char	resbuf[256];
	sprintf(resbuf, "%dm", mRes);
	return resbuf;
}