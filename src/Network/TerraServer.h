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
#ifndef TERRASERVER_H
#define TERRASERVER_H

#if WED
class	GUI_GraphState;
#endif

struct	ImageInfo;
class	HTTPConnection;
class	HTTPRequest;

/*************************************************************************************************************************************************
 * SIMPLE SERVER ACCESS
 *************************************************************************************************************************************************/			
// All routines return 0 for success, or an error code.


int		GetThemeInfo(
			const char *	inTheme, 
			string			info[9]);

int		FetchTile(
			const char * 	scale, 		// 4m, 1m, 500mm, etc.
			const char *	theme,
			int 			domain, 	// Image Scene #
			int 			x, 			// Tile X and Y
			int 			y, 
			ImageInfo * 	destBitmap, // Dest bitmap to copy to
			int 			pixLeft, 	// Offset into bitmap
			int 			pixTop);


int		FetchTilePositioning(
			const char * 	scale, 
			const char * theme,
			int 	domain, 
			int 	x, 
			int 	y,
			double	coords[4][2]);		// NW, NE, SE, SW x Lat, Lon

int		GetTilesForArea(
			const char * 	scale,
			const char * theme,
			double	inLatSouth, 
			double 	inLonWest, 
			double 	inLatNorth, 
			double 	inLonEast,
			int		tiles[4][3]);		// NW, NE, SW, SE x X, Y, domain
			
/*************************************************************************************************************************************************
 * ASYNC SERVER ACCESS
 *************************************************************************************************************************************************/			
 
class	AsyncImage;
class	AsyncImageLocator;
 
class	AsyncConnectionPool {
public:
	AsyncConnectionPool(int max_cons, int max_depth);
	~AsyncConnectionPool();

	void			ServiceLocator(HTTPRequest * req);
	void			ServiceImage(HTTPRequest * req);
	
private:

	HTTPConnection *				mLocatorCon;
	vector<HTTPConnection *>		mImageCon;
	int								mMaxCons;
	int								mMaxDepths;
};
 
class	AsyncImage {
public:

	AsyncImage(AsyncConnectionPool * pool, const char * scale, const char * theme, int domain, int x, int y);
	~AsyncImage();
	
	ImageInfo *		GetImage(void);
	bool			GetCoords(double	coords[4][2]);		// NW, NE, SE, SW x Lat, Lon
	bool			HasErr(void);
	bool			IsDone(void);
	#if WED
		void		Draw(double coords[4][2], GUI_GraphState * g);
	#else
		void		Draw(double coords[4][2]);
	#endif
	
	int				GetGen(void) { return mGen; }
	void			SetGen(int g) { mGen = g; }
	
private:

	friend	class	AsyncConnectionPool;

	void			TryImage(void);
	void			TryCoords(void);

	AsyncConnectionPool *	mPool;

	string			mTheme;
	string			mScale;
	int				mX;
	int				mY;
	int				mDomain;

	HTTPRequest *	mFetchImage;
	HTTPRequest *	mFetchCoords;
	
	bool			mHasCoords;
	ImageInfo *		mImage;
	double			mCoords[4][2];
	bool			mHasErr;
	
	float 			mS;
	float			mT;
	#if WED
	unsigned long	mTexNum;
	#else
	int				mTexNum;
	#endif

	int				mGen;

};

class	AsyncImageLocator {
public:

	AsyncImageLocator(AsyncConnectionPool * pool);
	~AsyncImageLocator();
	
	bool		GetLocation(const char* scale, const char * theme, double w, double s, double n, double e,
						int& x1, int& x2, int& y1, int& y2,
						int& layer);
	void		Purge(void);

private:

	friend	class	AsyncConnectionPool;

	AsyncConnectionPool *	mPool;
	double			mWest;
	double			mEast;
	double			mNorth;
	double			mSouth;
	
	HTTPRequest *	mFetch;
	
	int				mX1;
	int 			mX2;
	int				mY1;
	int				mY2;
	int				mLayer;
	bool			mHas;
};
	
#endif

