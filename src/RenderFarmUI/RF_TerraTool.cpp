/*
 * Copyright (c) 2004, Laminar Research.
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
#include "RF_TerraTool.h"
#include "RF_MapZoomer.h"

#include "PCSBSocket.h"
#include "TerraServer.h"
#include "XPLMGraphics.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

class AsyncImage;

inline long long hash_xy(int x, int y) { return ((long long) x << 32) + (long long) y; }


RF_TerraTool::RF_TerraTool(RF_MapZoomer * inZoomer) :
	RF_MapTool(inZoomer)
{
	static bool first_time = true;
	if (first_time)
		PCSBSocket::StartupNetworking(true);
	first_time = false;
	mPool = new AsyncConnectionPool(4,5);
	mLocator = new AsyncImageLocator(mPool);
	mHas = 0;
	mRes = 4;
	mData = "1";
}

RF_TerraTool::~RF_TerraTool()
{
	for (map<long long, AsyncImage *>::iterator i = mImages.begin(); i != mImages.end(); ++i)
		delete i->second;
	delete mLocator;
}

void	RF_TerraTool::DrawFeedbackUnderlay(
				bool				inCurrent)
{
	if (!inCurrent && !mHas) return;
	double	s, n, e, w;
	GetZoomer()->GetMapVisibleBounds(w, s, e, n);
	string status;
	if (mLocator->GetLocation(ResString(), mData.c_str(), w, s, e, n, mX1, mX2, mY1, mY2, mDomain, status))
	{
		mHas = 1;
		for (int x = mX1; x < mX2; ++x)
		for (int y = mY1; y < mY2; ++y)
		{
			long long h = hash_xy(x,y);
			if (mImages.count(h) == 0)
			{
				mImages[h] = new AsyncImage(mPool, ResString(), mData.c_str(), mDomain, x, y);
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
						i->Draw(coords);
					}
				}
			}
		}
	} else
		mHas = 0;
}

void	RF_TerraTool::DrawFeedbackOverlay(
				bool				inCurrent)
{
}


bool	RF_TerraTool::HandleClick(
				XPLMMouseStatus		inStatus,
				int 				inX,
				int 				inY,
				int 				inButton)
{
	return false;
}

int		RF_TerraTool::GetNumProperties(void)
{
	return 1;
}

void	RF_TerraTool::GetNthPropertyName(int n, string& s)
{
	s = "Res (m):";
}

double	RF_TerraTool::GetNthPropertyValue(int)
{
	return mRes;
}

void	RF_TerraTool::SetNthPropertyValue(int, double v)
{
	mRes = v;
	if (mRes < 1) mRes = 1;
	NthButtonPressed(1);
}

int		RF_TerraTool::GetNumButtons(void)
{
	return 5;
}
void	RF_TerraTool::GetNthButtonName(int n, string& s)
{
	switch(n) {
	case 0:		s = "Retry";		break;
	case 1:		s = "Clear";		break;
	case 2:		s = "Photo";		break;
	case 3:		s = "Ortho";		break;
	case 4:		s = "Urban";		break;
	}
}

void	RF_TerraTool::NthButtonPressed(int n)
{
	set<long long> nuke;
	switch(n) {
	case 0:
		{
			for (map<long long, AsyncImage *>::iterator i = mImages.begin(); i != mImages.end(); ++i)
			if (i->second->HasErr())
			{
				delete i->second;
				nuke.insert(i->first);
			}
			for (set<long long>::iterator j = nuke.begin(); j != nuke.end(); ++j)
				mImages.erase(*j);
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		{
			mLocator->Purge();
			mHas = 0;
			for (map<long long, AsyncImage *>::iterator i = mImages.begin(); i != mImages.end(); ++i)
				delete i->second;
			mImages.clear();
			switch(n) {
			case 2: 	mData = "1";	break;
			case 3: 	mData = "2";	break;
			case 4: 	mData = "4";	break;
			}
		}
		break;
	}
}

char *	RF_TerraTool::GetStatusText(void)
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
		sprintf(buf, "Domain=%d, X=%d-%d,Y=%d-%d Done=%d Pending=%d Bad=%d Total=%d", mDomain, mX1,mX2,mY1,mY2, done, pending, bad, total);
	else
		sprintf(buf, "No area established.");
	return buf;
}
                  
const char *	RF_TerraTool::ResString(void)
{
	static char	resbuf[256];
	sprintf(resbuf, "%dm", mRes);
	return resbuf;
}