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
#include "Terraserver.h"
#include "HTTPClient.h"
#if WED
	#include "GUI_GraphState.h"
#else
	#include "XPLMGraphics.h"
#endif
#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif
#include "BitmapUtils.h"
#include "TexUtils.h"
#include "XMLObject.h"
#include "PCSBSocket.h"

// Debug code to dump the actual JPEG files we fetch as a debugging measure.
#define DUMP_RAW_JPEG 0

extern	void decode( const char * startP, const char * endP, char * destP, char ** out);

#define SOAP_GETTHEMEINFO		\
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"				\
"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"	\
"  <soap:Body>"												\
"    <GetTheme xmlns=\"http://terraservice-usa.com/\">"		\
"      <theme>%s</theme>"									\
"    </GetTheme>"											\
"  </soap:Body>"											\
"</soap:Envelope>\r\n"


#define	SOAP_GETAREAFROMRECT		\
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"	\
"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"	\
"  <soap:Body>"	\
"    <GetAreaFromRect xmlns=\"http://terraservice-usa.com/\">"	\
"      <upperLeft>"						\
"        <Lon>%f</Lon>"					\
"        <Lat>%f</Lat>"					\
"      </upperLeft>"					\
"      <lowerRight>"					\
"        <Lon>%f</Lon>"					\
"        <Lat>%f</Lat>"					\
"      </lowerRight>"					\
"      <theme>%s</theme>"			\
"      <scale>Scale%s</scale>"			\
"    </GetAreaFromRect>"				\
"  </soap:Body>"						\
"</soap:Envelope>\r\n"

#define SOAP_GETTILE	\
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"	\
"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"	\
"  <soap:Body>"	\
"    <GetTile xmlns=\"http://terraservice-usa.com/\">"	\
"		<id>"							\
"			<Theme>%s</Theme>"		\
"			<Scale>Scale%s</Scale>"	\
"			<Scene>%d</Scene>"			\
"			<X>%d</X>"					\
"			<Y>%d</Y>"					\
"		</id>"							\
"    </GetTile>"						\
"  </soap:Body>"						\
"</soap:Envelope>\r\n"

#define SOAP_GETTILEMETAFROMTILEID	\
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"	\
"<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"	\
"  <soap:Body>"	\
"    <GetTileMetaFromTileId xmlns=\"http://terraservice-usa.com/\">"	\
"		<id>"							\
"			<Theme>%s</Theme>"		\
"			<Scale>Scale%s</Scale>"	\
"			<Scene>%d</Scene>"			\
"			<X>%d</X>"					\
"			<Y>%d</Y>"					\
"		</id>"							\
"    </GetTileMetaFromTileId>"			\
"  </soap:Body>"						\
"</soap:Envelope>\r\n"


char	req_string[65536];

int	GetThemeInfo(const char *inTheme, string	info[9])
{
	for (int n = 0; n < 9; ++n)
		info[n].clear();
	FieldMap	fields;
	fields.insert(FieldMap::value_type("Host", "www.terraserver-usa.com"));
	fields.insert(FieldMap::value_type("Content-Type", "text/xml; charset=utf-8"));
	fields.insert(FieldMap::value_type("SOAPAction", "http://terraservice-usa.com/GetTheme"));

	sprintf(req_string, SOAP_GETTHEMEINFO, inTheme);
	
	HTTPConnection	con(
						"www.terraserver-usa.com",
						80);
	
	HTTPRequest		client(
						&con,
						"/TerraService2.asmx",
						true,	// Post!
						fields,
						req_string,
						strlen(req_string),
						NULL);

	while (!client.IsDone())
	{
		con.DoProcessing();
	}
	
	int responseNum = client.GetResponseNum();
	if ((responseNum < 200) || (responseNum > 299))
		return responseNum;
	
	vector<char>	foo;
	client.GetData(foo);

	XMLObject * root = ParseXML(&*foo.begin(), foo.size());
	if (!root) return -1; 	// Must be bad XML

	XMLObject * result = root->GetNestedSubObject("soap:Envelope/soap:Body/GetThemeResponse/GetThemeResult");
	if (result)
	{
		XMLObject * themeid = result->GetNestedSubObject("Theme");
		XMLObject * name = result->GetNestedSubObject("Name");
		XMLObject * descrip = result->GetNestedSubObject("Description");
		XMLObject * supplier = result->GetNestedSubObject("Supplier");
		XMLObject * loscale = result->GetNestedSubObject("LoScale");
		XMLObject * hiscale = result->GetNestedSubObject("HiScale");
		XMLObject * projid = result->GetNestedSubObject("ProjectionID");
		XMLObject * projname = result->GetNestedSubObject("ProjectionName");
		XMLObject * copyright = result->GetNestedSubObject("CopyrightNotice");
		
		if (themeid)	themeid->GetContents(info[0]);
		if (name)		name->GetContents(info[1]);
		if (descrip)	descrip->GetContents(info[2]);
		if (supplier)	supplier->GetContents(info[3]);
		if (loscale)	loscale->GetContents(info[4]);
		if (hiscale)	hiscale->GetContents(info[5]);
		if (projid)		projid->GetContents(info[6]);
		if (projname)	projname->GetContents(info[7]);
		if (copyright)	copyright->GetContents(info[8]);
	}

	delete root;

	return 0;
}



// Returns 0 or an error code.
int		FetchTile(const char * scale, const char * theme, int domain, int x, int y, ImageInfo * destBitmap, int pixLeft, int pixTop)
{
	FieldMap	fields;
	fields.insert(FieldMap::value_type("Host", "www.terraserver-usa.com"));
	fields.insert(FieldMap::value_type("Content-Type", "text/xml; charset=utf-8"));
	fields.insert(FieldMap::value_type("SOAPAction", "http://terraservice-usa.com/GetTile"));

	sprintf(req_string, SOAP_GETTILE, theme, scale, domain, x, y);
	
	HTTPConnection	con(
						"www.terraserver-usa.com",
						80);
	HTTPRequest		client(
						&con,
						"/TerraService2.asmx",
						true,	// Post!
						fields,
						req_string,
						strlen(req_string),
						NULL);

	while (!client.IsDone())
	{
		con.DoProcessing();
	}
	
	int responseNum = client.GetResponseNum();
	if ((responseNum < 200) || (responseNum > 299))
		return responseNum;
	
	vector<char>	foo;
	client.GetData(foo);

	XMLObject * root = ParseXML(&*foo.begin(), foo.size());
	if (!root) return -1; 	// Must be bad XML

	XMLObject * result = root->GetNestedSubObject("soap:Envelope/soap:Body/GetTileResponse/GetTileResult");
	if (result)
	{
		string	b64;
		result->GetContents(b64);
		vector<char>	abuf;
		abuf.resize(b64.length());		// Post-B64 decoding is always at least smaller than in b64.
		char * inp = &*abuf.begin();
		char *	outP;
		decode(&*b64.begin(), &*b64.end(), inp, &outP);
		int len = outP - inp;

		ImageInfo	mini;
#if DUMP_RAW_JPEG
		FILE * fi = fopen("raw.jpg", "wb");
		fwrite(inp, 1, len, fi);		
		fclose(fi);
#endif				
		if (CreateBitmapFromJPEGData(inp, len, &mini) == 0)
		{
			CopyBitmapSection(&mini, destBitmap,
			0, 0, mini.width, mini.height,
			pixLeft,
			pixTop, 
			pixLeft + mini.width,
			pixTop + mini.height);
						
			DestroyBitmap(&mini);
			delete root;
			return 0;	// Success!!
			
		} // Else our tile result's contents didn't look much like a JPEG image!

	} // Else the XML response doesn't have the nodes we exepct

	delete root;	
	return -1;
}

// Order is NW, NE, SE, SW, lat then lon
int		FetchTilePositioning(const char * scale, const char * theme, int domain, int x, int y,
						double	coords[4][2])
{
	FieldMap	fields;
	fields.insert(FieldMap::value_type("Host", "www.terraserver-usa.com"));
	fields.insert(FieldMap::value_type("Content-Type", "text/xml; charset=utf-8"));
	fields.insert(FieldMap::value_type("SOAPAction", "http://terraservice-usa.com/GetTileMetaFromTileId"));

	sprintf(req_string, SOAP_GETTILEMETAFROMTILEID, theme, scale, domain, x, y);
	
	HTTPConnection	con(
						"www.terraserver-usa.com",
						80);
	HTTPRequest		client(
						&con,
						"/TerraService2.asmx",
						true,	// Post!
						fields,
						req_string,
						strlen(req_string),
						NULL);

	while (!client.IsDone())
	{
		con.DoProcessing();
	}
	
	int responseNum = client.GetResponseNum();
	if ((responseNum < 200) || (responseNum > 299))
		return responseNum;	
	
	vector<char>	foo;
	client.GetData(foo);
	XMLObject * root = ParseXML(&*foo.begin(), foo.size());
	if (!root) return -1;	// Must be bad XML

	XMLObject * result = root->GetNestedSubObject("soap:Envelope/soap:Body/GetTileMetaFromTileIdResponse/GetTileMetaFromTileIdResult");
	if (!result)
	{
		// Strange response!
		delete root;
		return -1;
	}

	XMLObject *	nw_lat = result->GetNestedSubObject("NorthWest/Lat");
	XMLObject *	ne_lat = result->GetNestedSubObject("NorthEast/Lat");
	XMLObject *	se_lat = result->GetNestedSubObject("SouthEast/Lat");
	XMLObject *	sw_lat = result->GetNestedSubObject("SouthWest/Lat");
	XMLObject *	nw_lon = result->GetNestedSubObject("NorthWest/Lon");
	XMLObject *	ne_lon = result->GetNestedSubObject("NorthEast/Lon");
	XMLObject *	se_lon = result->GetNestedSubObject("SouthEast/Lon");
	XMLObject *	sw_lon = result->GetNestedSubObject("SouthWest/Lon");
	
	if (!nw_lat || !ne_lat || !se_lat || !sw_lat ||
		!nw_lon || !ne_lon || !se_lon || !sw_lon)
	{
		delete root;
		return -1;
	}

	coords[0][0] = nw_lat->GetContentsDouble();
	coords[1][0] = ne_lat->GetContentsDouble();
	coords[2][0] = se_lat->GetContentsDouble();
	coords[3][0] = sw_lat->GetContentsDouble();
	coords[0][1] = nw_lon->GetContentsDouble();
	coords[1][1] = ne_lon->GetContentsDouble();
	coords[2][1] = se_lon->GetContentsDouble();
	coords[3][1] = sw_lon->GetContentsDouble();

	bool	allZero = true;
	for (int i = 0; i < 4; ++i)
	for (int j = 0; j < 2; ++j)
		if (coords[i][j] != 0.0) allZero = false;

	delete root;
	return allZero ? 1 : 0;
}

// Tiles are NW, NE, SE, SW for X/Y/Scene
int	GetTilesForArea(const char * scale,
			const char * theme,
			double	inLatSouth, double inLonWest, double inLatNorth, double inLonEast,
			int		tiles[4][3])
{
	FieldMap	fields;
	fields.insert(FieldMap::value_type("Host", "www.terraserver-usa.com"));
	fields.insert(FieldMap::value_type("Content-Type", "text/xml; charset=utf-8"));
	fields.insert(FieldMap::value_type("SOAPAction", "http://terraservice-usa.com/GetAreaFromRect"));

	sprintf(req_string, SOAP_GETAREAFROMRECT,	inLonWest, inLatNorth, inLonEast, inLatSouth, theme, scale);		
	
	HTTPConnection	con(
						"www.terraserver-usa.com",
						80);
	HTTPRequest		client(
						&con,
						"/TerraService2.asmx",
						true,	// Post!
						fields,
						req_string,
						strlen(req_string),
						NULL);

	while (!client.IsDone())
	{
		con.DoProcessing();
	}

	int responseNum = client.GetResponseNum();
	if ((responseNum < 200) || (responseNum > 299))
		return responseNum;	
	
	vector<char>	foo;
	client.GetData(foo);
	XMLObject * root = ParseXML(&*foo.begin(), foo.size());
	if (root == NULL) return -1;	// Bad XML
	
	XMLObject * result = root->GetNestedSubObject(
		"soap:Envelope/soap:Body/GetAreaFromRectResponse/GetAreaFromRectResult");
	if (!result)
	{
		delete root;
		return -1;	// No result found
	}
    
    XMLObject * nw_x = result->GetNestedSubObject("NorthWest/TileMeta/Id/X");
    XMLObject * nw_y = result->GetNestedSubObject("NorthWest/TileMeta/Id/Y");
    XMLObject * nw_scene = result->GetNestedSubObject("NorthWest/TileMeta/Id/Scene");        

    XMLObject * sw_x = result->GetNestedSubObject("SouthWest/TileMeta/Id/X");
    XMLObject * sw_y = result->GetNestedSubObject("SouthWest/TileMeta/Id/Y");
    XMLObject * sw_scene = result->GetNestedSubObject("SouthWest/TileMeta/Id/Scene");       

    XMLObject * ne_x = result->GetNestedSubObject("NorthEast/TileMeta/Id/X");
    XMLObject * ne_y = result->GetNestedSubObject("NorthEast/TileMeta/Id/Y");
    XMLObject * ne_scene = result->GetNestedSubObject("NorthEast/TileMeta/Id/Scene");       

    XMLObject * se_x = result->GetNestedSubObject("SouthEast/TileMeta/Id/X");
    XMLObject * se_y = result->GetNestedSubObject("SouthEast/TileMeta/Id/Y");
    XMLObject * se_scene = result->GetNestedSubObject("SouthEast/TileMeta/Id/Scene");       

	if (!nw_x || !nw_y || !nw_scene ||
		!sw_x || !sw_y || !sw_scene ||
		!ne_x || !ne_y || !ne_scene ||
		!se_x || !se_y || !se_scene)
	{
		delete root;
		return -1;
	}

	tiles[0][0] = nw_x->GetContentsInt();
	tiles[1][0] = ne_x->GetContentsInt();
	tiles[2][0] = se_x->GetContentsInt();
	tiles[3][0] = sw_x->GetContentsInt();
	tiles[0][1] = nw_y->GetContentsInt();
	tiles[1][1] = ne_y->GetContentsInt();
	tiles[2][1] = se_y->GetContentsInt();
	tiles[3][1] = sw_y->GetContentsInt();
	tiles[0][2] = nw_scene->GetContentsInt();
	tiles[1][2] = ne_scene->GetContentsInt();
	tiles[2][2] = se_scene->GetContentsInt();
	tiles[3][2] = sw_scene->GetContentsInt();

	delete root;
	return 0;
}

int		AsyncImage::sPending = 0;

HTTPConnection *AsyncImage::mConnection = NULL;
HTTPConnection *AsyncImageLocator::mConnection = NULL;


#define	FETCH_LIMIT	60


AsyncImage::AsyncImage(const char * scale, const char * theme, int domain, int x, int y)
{
	mX = x;
	mY = y;
	mDomain = domain;
	mTheme = theme;
	mScale = scale;
	mHasCoords = false;
	mHasErr = false;
	mImage = NULL;
	mTexNum = 0;
	mFetchCoords = NULL;
	mFetchImage = NULL;	
}

void AsyncImage::TryCoords(void)
{
	FieldMap	fields;
	fields.insert(FieldMap::value_type("Host", "www.terraserver-usa.com"));
	fields.insert(FieldMap::value_type("Content-Type", "text/xml; charset=utf-8"));
	fields.insert(FieldMap::value_type("SOAPAction", "http://terraservice-usa.com/GetTileMetaFromTileId"));

	sprintf(req_string, SOAP_GETTILEMETAFROMTILEID, mTheme.c_str(), mScale.c_str(), mDomain, mX, mY);
	
	++sPending;
	
	if (!mConnection)
		mConnection = new HTTPConnection(
						"www.terraserver-usa.com",
						80);
	mFetchCoords = new HTTPRequest(
						mConnection,
						"/TerraService2.asmx",
						true,	// Post!
						fields,
						req_string,
						strlen(req_string),
						NULL);
}

void AsyncImage::TryImage()
{
	FieldMap	fields;
	fields.insert(FieldMap::value_type("Host", "www.terraserver-usa.com"));
	fields.insert(FieldMap::value_type("Content-Type", "text/xml; charset=utf-8"));
	fields.insert(FieldMap::value_type("SOAPAction", "http://terraservice-usa.com/GetTile"));

	sprintf(req_string, SOAP_GETTILE, mTheme.c_str(), mScale.c_str(), mDomain, mX, mY);

	++sPending;
	if (!mConnection)
		mConnection = new HTTPConnection(
						"www.terraserver-usa.com",
						80);
	mFetchImage = new HTTPRequest(
						mConnection,
						"/TerraService2.asmx",
						true,	// Post!
						fields,
						req_string,
						strlen(req_string),
						NULL);
}

	
AsyncImage::~AsyncImage()
{
	if (mImage)
	{
		DestroyBitmap(mImage);
		delete mImage;
	}
	if (mFetchImage)	sPending--;
	if (mFetchCoords)	sPending--;
	delete mFetchImage;
	delete mFetchCoords;
	
	if (mTexNum != 0)
		glDeleteTextures(1, (GLuint*) &mTexNum);
}
	
ImageInfo *		AsyncImage::GetImage(void)
{
	if (mImage) return mImage;
	if (mHasErr) return NULL;
	
	if (!mFetchImage)
	{
		if (sPending < FETCH_LIMIT)
			TryImage();
		else
			return NULL;
	}	
	
	if (!mFetchImage->IsDone())
	{
		mConnection->DoProcessing();
		return NULL;
	}
	
	int responseNum = mFetchImage->GetResponseNum();
	if ((responseNum < 200) || (responseNum > 299))
	{
		mHasErr = true;
		--sPending;
		delete mFetchImage;
		mFetchImage = NULL;
		return NULL;
	}	
	vector<char>	foo;
	mFetchImage->GetData(foo);

	XMLObject * root = ParseXML(&*foo.begin(), foo.size());
	if (!root) {
		--sPending;
		delete mFetchImage;
		mFetchImage = NULL;
		mHasErr = true;
		return NULL;
	}

	XMLObject * result = root->GetNestedSubObject("soap:Envelope/soap:Body/GetTileResponse/GetTileResult");
	if (result)
	{
		string	b64;
		result->GetContents(b64);
		vector<char>	abuf;
		abuf.resize(b64.length());		// Post-B64 decoding is always at least smaller than in b64.
		char * inp = &*abuf.begin();
		char *	outP;
		decode(&*b64.begin(), &*b64.end(), inp, &outP);
		int len = outP - inp;

		mImage = new ImageInfo;
		
		if (CreateBitmapFromJPEGData(inp, len, mImage) == 0)
		{
			--sPending;
			delete root;
			delete mFetchImage;
			mFetchImage = NULL;
			#if WED
				glGenTextures(1,&mTexNum);
			#else
				XPLMGenerateTextureNumbers(&mTexNum, 1);
			#endif
			
			LoadTextureFromImage(*mImage, mTexNum, tex_Linear, NULL, NULL, &mS, &mT);			
			return mImage;
		} // Else our tile result's contents didn't look much like a JPEG image!

	} // Else the XML response doesn't have the nodes we exepct

	delete root;	
	--sPending;
	delete mFetchImage;
	mFetchImage = NULL;
	
	mHasErr = true;
	return NULL;
}

bool			AsyncImage::GetCoords(double	coords[4][2])
{
	if (mHasErr) return false;
	
	if (mHasCoords)
	{
		memcpy(coords, mCoords, sizeof(mCoords));
		return true;
	}

	if (!mFetchCoords)
	{
		if (sPending < FETCH_LIMIT)
			TryCoords();
		else
			return NULL;
	}	
	


	if (!mFetchCoords->IsDone())
	{
		mConnection->DoProcessing();
		return false;
	}
	
	int responseNum = mFetchCoords->GetResponseNum();
	if ((responseNum < 200) || (responseNum > 299))
	{
		mHasErr = true;
		--sPending;
		delete mFetchCoords;
		mFetchCoords = NULL;
		return false;
	}
	
	vector<char>	foo;
	mFetchCoords->GetData(foo);
	XMLObject * root = ParseXML(&*foo.begin(), foo.size());
	if (!root) { 
		mHasErr = true; 
		--sPending;
		delete mFetchCoords;
		mFetchCoords = NULL;
		return false; 
	}

	XMLObject * result = root->GetNestedSubObject("soap:Envelope/soap:Body/GetTileMetaFromTileIdResponse/GetTileMetaFromTileIdResult");
	if (!result)
	{
		// Strange response!
		--sPending;
		delete root;
		delete mFetchCoords;
		mFetchCoords = NULL;
		mHasErr = true; 
		return false;
	}

	XMLObject *	nw_lat = result->GetNestedSubObject("NorthWest/Lat");
	XMLObject *	ne_lat = result->GetNestedSubObject("NorthEast/Lat");
	XMLObject *	se_lat = result->GetNestedSubObject("SouthEast/Lat");
	XMLObject *	sw_lat = result->GetNestedSubObject("SouthWest/Lat");
	XMLObject *	nw_lon = result->GetNestedSubObject("NorthWest/Lon");
	XMLObject *	ne_lon = result->GetNestedSubObject("NorthEast/Lon");
	XMLObject *	se_lon = result->GetNestedSubObject("SouthEast/Lon");
	XMLObject *	sw_lon = result->GetNestedSubObject("SouthWest/Lon");
	
	if (!nw_lat || !ne_lat || !se_lat || !sw_lat ||
		!nw_lon || !ne_lon || !se_lon || !sw_lon)
	{
		--sPending;
		delete root;
		delete mFetchCoords;
		mFetchCoords = NULL;
		mHasErr = true; 
		return false;
	}

	mCoords[0][0] = nw_lat->GetContentsDouble();
	mCoords[1][0] = ne_lat->GetContentsDouble();
	mCoords[2][0] = se_lat->GetContentsDouble();
	mCoords[3][0] = sw_lat->GetContentsDouble();
	mCoords[0][1] = nw_lon->GetContentsDouble();
	mCoords[1][1] = ne_lon->GetContentsDouble();
	mCoords[2][1] = se_lon->GetContentsDouble();
	mCoords[3][1] = sw_lon->GetContentsDouble();

	bool	allZero = true;
	for (int i = 0; i < 4; ++i)
	for (int j = 0; j < 2; ++j)
		if (mCoords[i][j] != 0.0) allZero = false;

		--sPending;
	delete root;
	delete mFetchCoords;
	mFetchCoords = NULL;

	memcpy(coords, mCoords, sizeof(mCoords));

	if (allZero)	mHasErr = true;
	else			mHasCoords = true;
	return mHasCoords;
}

bool			AsyncImage::HasErr(void)
{
	return mHasErr;
}

bool			AsyncImage::IsDone(void)
{
	if (mHasCoords && mImage) return true;
	if (mFetchCoords == NULL && mFetchImage == NULL) return false;
	bool im = GetImage() != NULL;
	double coords[4][2];
	bool cr = GetCoords(coords);
	return cr && im;
}

#if WED
void	AsyncImage::Draw(double coords[4][2],GUI_GraphState * g)
{
	g->SetState(0,1,0,  0, 0,   0, 0);
	glColor3f(1.0, 1.0, 1.0);
	g->BindTex(mTexNum, 0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, mT);		glVertex2d(coords[0][1],coords[0][0]);
	glTexCoord2f(mS, mT);		glVertex2d(coords[1][1],coords[1][0]);
	glTexCoord2f(mS, 0.0);		glVertex2d(coords[2][1],coords[2][0]);
	glTexCoord2f(0.0, 0.0);		glVertex2d(coords[3][1],coords[3][0]);
	glEnd();
}

#else
void	AsyncImage::Draw(double coords[4][2])
{
	XPLMSetGraphicsState(0,1,0,  0, 0,   0, 0);
	glColor3f(1.0, 1.0, 1.0);
	XPLMBindTexture2d(mTexNum, 0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, mT);		glVertex2d(coords[0][1],coords[0][0]);
	glTexCoord2f(mS, mT);		glVertex2d(coords[1][1],coords[1][0]);
	glTexCoord2f(mS, 0.0);		glVertex2d(coords[2][1],coords[2][0]);
	glTexCoord2f(0.0, 0.0);		glVertex2d(coords[3][1],coords[3][0]);
	glEnd();
}
#endif


AsyncImageLocator::AsyncImageLocator()
{
	mFetch = NULL;
	mNorth = mSouth = mEast = mWest = -9.9e9;
}
AsyncImageLocator::~AsyncImageLocator()
{
	if (mFetch) delete mFetch;
}

bool	AsyncImageLocator::GetLocation(const char* scale, const char * theme, double w, double s, double e, double n,
					int& x1, int& x2, int& y1, int& y2,
					int& layer)
{
	if (mFetch)
	{
		if (!mFetch->IsDone())
			mConnection->DoProcessing();
		else {

			int responseNum = mFetch->GetResponseNum();
			if ((responseNum >= 200) && (responseNum <= 299))
			{				
				vector<char>	foo;
				mFetch->GetData(foo);
				XMLObject * root = ParseXML(&*foo.begin(), foo.size());
				if (root != NULL)
				{
					XMLObject * result = root->GetNestedSubObject(
						"soap:Envelope/soap:Body/GetAreaFromRectResponse/GetAreaFromRectResult");
					if (result)
					{
					    XMLObject * nw_x = result->GetNestedSubObject("NorthWest/TileMeta/Id/X");
					    XMLObject * nw_y = result->GetNestedSubObject("NorthWest/TileMeta/Id/Y");
					    XMLObject * nw_scene = result->GetNestedSubObject("NorthWest/TileMeta/Id/Scene");        

					    XMLObject * sw_x = result->GetNestedSubObject("SouthWest/TileMeta/Id/X");
					    XMLObject * sw_y = result->GetNestedSubObject("SouthWest/TileMeta/Id/Y");
					    XMLObject * sw_scene = result->GetNestedSubObject("SouthWest/TileMeta/Id/Scene");       

					    XMLObject * ne_x = result->GetNestedSubObject("NorthEast/TileMeta/Id/X");
					    XMLObject * ne_y = result->GetNestedSubObject("NorthEast/TileMeta/Id/Y");
					    XMLObject * ne_scene = result->GetNestedSubObject("NorthEast/TileMeta/Id/Scene");       

					    XMLObject * se_x = result->GetNestedSubObject("SouthEast/TileMeta/Id/X");
					    XMLObject * se_y = result->GetNestedSubObject("SouthEast/TileMeta/Id/Y");
					    XMLObject * se_scene = result->GetNestedSubObject("SouthEast/TileMeta/Id/Scene");       

						if (!nw_x || !nw_y || !nw_scene ||
							!sw_x || !sw_y || !sw_scene ||
							!ne_x || !ne_y || !ne_scene ||
							!se_x || !se_y || !se_scene)
						{
						} else {

							int tiles[4][3];
							tiles[0][0] = nw_x->GetContentsInt();
							tiles[1][0] = ne_x->GetContentsInt();
							tiles[2][0] = se_x->GetContentsInt();
							tiles[3][0] = sw_x->GetContentsInt();
							tiles[0][1] = nw_y->GetContentsInt();
							tiles[1][1] = ne_y->GetContentsInt();
							tiles[2][1] = se_y->GetContentsInt();
							tiles[3][1] = sw_y->GetContentsInt();
							tiles[0][2] = nw_scene->GetContentsInt();
							tiles[1][2] = ne_scene->GetContentsInt();
							tiles[2][2] = se_scene->GetContentsInt();
							tiles[3][2] = sw_scene->GetContentsInt();
							
							mX1 = mX2 = tiles[0][0];
							mY1 = mY2 = tiles[0][1];
							mX2++;
							mY2++;
							mLayer = tiles[0][2];
							for (int n = 1; n < 4; ++n)
							{
								mX1 = min (mX1, tiles[n][0]);
								mX2 = max (mX2, tiles[n][0]+1);
								mY1 = min (mY1, tiles[n][1]);
								mY2 = max (mY2, tiles[n][1]+1);
							}		
							
							mHas = true;	
							
						}
					}
					delete root;			
				}
			}
			delete mFetch;
			mFetch = NULL;
		}
	}
	
	if (w != mWest || e != mEast || s != mSouth || n != mNorth)
	if (mFetch == NULL)
	{
		FieldMap	fields;
		fields.insert(FieldMap::value_type("Host", "www.terraserver-usa.com"));
		fields.insert(FieldMap::value_type("Content-Type", "text/xml; charset=utf-8"));
		fields.insert(FieldMap::value_type("SOAPAction", "http://terraservice-usa.com/GetAreaFromRect"));

		sprintf(req_string, SOAP_GETAREAFROMRECT,	w, n, e, s, theme, scale);		
		
	if (!mConnection)
		mConnection = new HTTPConnection(
							"www.terraserver-usa.com",
							80);
		mFetch = new HTTPRequest(
							mConnection,
							"/TerraService2.asmx",
							true,	// Post!
							fields,
							req_string,
							strlen(req_string),
							NULL);	
		mWest = w;
		mNorth = n;
		mEast = e;
		mSouth = s;
	}
	
	if (mHas)
	{
		x1 = mX1;
		x2 = mX2;
		y1 = mY1;
		y2 = mY2;
		layer = mLayer;
	}
	return mHas;
}					

void	AsyncImageLocator::Purge(void)
{
	if (mFetch) delete mFetch;
	mFetch = NULL;
	
	mHas = false;
	mWest = -9.9e9;
}
