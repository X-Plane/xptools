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
#include "WED_TerraTool.h"
#include "WED_MapZoomer.h"

#include "PCSBSocket.h"
#include "PlatformUtils.h"
#include "TerraServer.h"
#include "BitmapUtils.h"
#include "XPLMGraphics.h"
#include <gl.h>
class AsyncImage;

inline long long hash_xy(int x, int y) { return ((long long) x << 32) + (long long) y; }


WED_TerraTool::WED_TerraTool(WED_MapZoomer * inZoomer) :
	WED_MapTool(inZoomer)
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

WED_TerraTool::~WED_TerraTool()
{
	for (map<long long, AsyncImage *>::iterator i = mImages.begin(); i != mImages.end(); ++i)
		delete i->second;
	delete mLocator;
}

void	WED_TerraTool::DrawFeedbackUnderlay(
				bool				inCurrent)
{
	if (!inCurrent && !mHas) return;
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
						i->Draw(coords);
					}
				}
			}
		}
	} else 
		mHas = 0;
}

void	WED_TerraTool::DrawFeedbackOverlay(
				bool				inCurrent)
{
}
	

bool	WED_TerraTool::HandleClick(
				XPLMMouseStatus		inStatus,
				int 				inX, 
				int 				inY, 
				int 				inButton)
{
	return false;
}

int		WED_TerraTool::GetNumProperties(void)
{
	return 1;
}

void	WED_TerraTool::GetNthPropertyName(int n, string& s)
{
	s = "Res (m):";
}

double	WED_TerraTool::GetNthPropertyValue(int)
{
	return mRes;
}

void	WED_TerraTool::SetNthPropertyValue(int, double v)
{
	mRes = v;
	if (mRes < 1) mRes = 1;
	NthButtonPressed(1);
}

int		WED_TerraTool::GetNumButtons(void)
{
	return 6;
}
void	WED_TerraTool::GetNthButtonName(int n, string& s)
{
	switch(n) {
	case 0:		s = "Retry";		break;
	case 1:		s = "Clear";		break;
	case 2:		s = "Photo";		break;
	case 3:		s = "Ortho";		break;
	case 4:		s = "Urban";		break;
	case 5:		s = "Save ";		break;
	}
}

void	WED_TerraTool::NthButtonPressed(int n)
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
	case 5:
		{	
			double	s, n, e, w;
			char	buf[1024];
			buf[0] = 0;
			GetZoomer()->GetMapVisibleBounds(w, s, e, n);		
			if (mLocator && mLocator->GetLocation(ResString(), mData.c_str(), w, s, e, n, mX1, mX2, mY1, mY2, mDomain))
			if (GetFilePathFromUser(getFile_Save, "Please Name Your PNG File", "Save", 11, buf))
			{
				ImageInfo	img;

				if (CreateNewBitmap((mX2-mX1)*200,(mY2-mY1)*200,3,&img)==0)
				{
					for (int x = mX1; x < mX2; ++x)
					for (int y = mY1; y < mY2; ++y)
					{
						long long h = hash_xy(x,y);
						if (mImages.count(h) != 0)
						{
							AsyncImage * i = mImages[h];
						
							if (!i->HasErr())
							{
								ImageInfo * bits = i->GetImage();
								if (bits)
								{
									CopyBitmapSectionDirect(*bits, img, 0, 0, (x-mX1)*200,(y-mY1)*200,200,200);
								}
							}
						}
					}
					WriteBitmapToPNG(&img, buf, NULL, NULL);
					DestroyBitmap(&img);
				}
			}	
		}
		break;
	}
}

char *	WED_TerraTool::GetStatusText(void)
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
		sprintf(buf, "Domain=%d, X=%d-%d,Y=%d-%d %dx%d Done=%d Pending=%d Bad=%d Total=%d Sockets=%d", mDomain, mX1,mX2,mY1,mY2, (mX2-mX1)*200,(mY2-mY1)*200,done, pending, bad, total, AsyncImage::TotalPending());
	else 
		sprintf(buf, "No area established.");
	return buf;
}

const char *	WED_TerraTool::ResString(void)
{
	static char	resbuf[256];
	sprintf(resbuf, "%dm", mRes);
	return resbuf;
}