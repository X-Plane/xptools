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
#include <time.h>
#include "hl_types.h"
#include "XObjDefs.h"
#include "ObjUtils.h"
#include "XUtils.h"
#include "GeoUtils.h"
#include "TexUtils.h"
#include "MatrixUtils.h"
#include "trackball.h"
#include "XObjReadWrite.h"
#include "XWinGL.h"
#include "XGUIApp.h"
#include <set>
#include "PlatformUtils.h"
#include "OE_Zoomer3d.h"
#include "FacadeObj.h"
//#include <glut.h>
#include <glu.h>

static	bool	gHasMultitexture = false;
static	bool	gHasEnvAdd = false;
static	bool	gHasCombine = false;

void	CHECK_ERR(void)
{
	GLint	err = glGetError();
	if (err != GL_NO_ERROR)
	{
		printf("OGL Error: %d\n", err);
	}
}

void	MyVertexConverter(double lat, double lon, double h, float * x, float * y, float * z)
{
	*x = lat;
	*y = lon;
	*z = h;
}

class	XObjWin;

XObjWin *	gReceiver = NULL;
float	gCamDist = 200;

map<string, pair<string, GLenum> >		gDayTextures;
map<string, pair<string, GLenum> >		gNightTextures;

static	void		PlotOneObj(const XObj& inObj, int inShowCulled, bool inLit, bool inSolid);
static	GLenum		FindTexture(const string& inName, bool inNight);
static	void		AccumTexture(const string& inFileName);
static	void		ReloadTexture(const string& inName);

class	XObjWin;
static	set<XObjWin *>	sWindows;

class	XObjWin : public XWinGL {
public:

					XObjWin(const char * inFileName);
	virtual			~XObjWin();
	
			void			ReceiveObject(double x, double y, double z, double r, const string& obj);
	
	virtual	void			Timer(void) { } 
	virtual	void			GLReshaped(int inWidth, int inHeight);
	virtual	void			GLDraw(void);
	
	virtual	bool			Closed(void) { return true; }
	virtual	void			ClickDown(int inX, int inY, int inButton);
	virtual	void			ClickUp(int inX, int inY, int inButton);
	virtual	void			ClickDrag(int inX, int inY, int inButton);
	virtual	void			MouseWheel(int inX, int inY, int inDelta, int inAxis);
	virtual	void			ReceiveFiles(const vector<string>& inFiles, int, int);
	virtual	int				KeyPressed(char inKey, long, long, long);

	virtual	void			DragEnter(int inX, int inY) { }
	virtual	void			DragOver(int inX, int inY) { }
	virtual	void			DragLeave(void) { }

private:

			void			ScaleToObj(void);

	struct ObjPlacement_t {
		double	x;
		double	y;
		double	z;
		double	r;
		string	obj;
	};

	map<string, XObj>	mObjDB;	
	vector<ObjPlacement_t>	mObjInst;

	XObj			mObj;
	Prototype_t		mPrototype;
	FacadeObj_t		mFacade;
	
	Polygon2 mPts;
	Sphere3			mBounds;

	bool	mIsPrototype;
	bool	mIsFacade;
	int		mFloors;
	
//	float	mScale;
//	float	mSpin[4];
//	float	mXTrans;
//	float	mYTrans;
	bool	mSolid;
	bool	mLit;
	bool	mMeasureOnOpen;
	int		mShowCulled;
	int		mShowBounds;
	
	int		mEditNum;
	int		mLastX;
	int		mLastY;
	
//	double	mYmax, mYmin, mXmin, mXmax, mNear, mFar;
	OE_Zoomer3d	mZoomer;

};

void			XObjWin::ReceiveObject(double x, double y, double z, double r, const string& obj)
{
	ObjPlacement_t	o;
	o.x = x;
	o.y = y;
	o.z = z;
	o.r = r;
	o.obj = obj;
	mObjInst.push_back(o);
}	


void	MyObjReceiver(double x, double y, double z, double r, const char * obj, void * inRef)
{
	if (gReceiver)
		gReceiver->ReceiveObject(x,y,z,r,obj);
}



XObjWin::XObjWin(const char * inFileName) : XWinGL(inFileName ? inFileName : "Drag Obj Here", 50, 50, 600, 600, sWindows.empty() ? NULL : *sWindows.begin()),
	/*(mScale(1.0),*/ mSolid(true), mShowCulled(false), mShowBounds(false), mLit(false), mMeasureOnOpen(false), /*mXTrans(0), mYTrans(0), */mFloors(1)
{
	mPts.push_back(Point2(-10.0,  10.0));
	mPts.push_back(Point2( 10.0,  10.0));
	mPts.push_back(Point2( 15.0,   0.0));
	mPts.push_back(Point2( 10.0, -10.0));
	mPts.push_back(Point2(-10.0, -10.0));

	float	pt1[] = { -10, 0, 0 };
	float	pt2[] = {  10, 0, 0 };

	if (inFileName)
	{
		vector<string>	v;
		v.push_back(inFileName);
		XObjWin::ReceiveFiles(v, 0, 0);
	}
	sWindows.insert(this);
	
//	float a[3] = { 0.0, 1.0, 0.0 };
//	axis_to_quat(a, 0.0, mSpin); 	
}	
	
XObjWin::~XObjWin()
{
	sWindows.erase(this);
	if (sWindows.empty())
		XGrinder_Quit();
}
	
void			XObjWin::GLReshaped(int inWidth, int inHeight)
{
	ForceRefresh();
}

void			XObjWin::GLDraw(void)
{
	SetGLContext();
	if (mLit)
		glClearColor(0.1, 0.2, 0.3, 0);
	else
		glClearColor(0.3, 0.5, 0.6, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glAlphaFunc		(GL_GREATER,0						);	// when ALPHA-TESTING  is enabled, we display the GREATEST alpha
	glBlendFunc		(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	// when ALPHA-BLENDING is enabled, we blend normally
	glDepthFunc		(GL_LEQUAL							);	// less than OR EQUAL plots on top
	glEnable(GL_BLEND);
	
	int	i[4] = { 0, 0, 0, 0 };
	GetBounds(i+2,i+3);

	mZoomer.SetupMatrices(i);
/*	
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
	
//	CHECK_ERR();

	mNear = 50.0;
	mFar = 2000.0;
  	mYmax = mNear * tan(60.0 * M_PI / 360.0);
  	mYmin = -mYmax;
  	mXmin = mYmin * p[2] / p[3];
  	mXmax = mYmax * p[2] / p[3];
	glFrustum(mXmin, mXmax, mYmin, mYmax, mNear, mFar);

	CHECK_ERR();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, -gCamDist, 
		0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0);

	float	m[4][4];

	glTranslatef(mXTrans, mYTrans, 0);
	
	build_rotmatrix(m, mSpin);
	glMultMatrixf(&m[0][0]);

		
	glScalef(mScale, mScale, mScale);
*/	
	PlotOneObj(mObj, mShowCulled, mLit, mSolid);
	
		
	for (vector<ObjPlacement_t>::iterator p = mObjInst.begin(); p != mObjInst.end(); ++p)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(p->x, p->y, p->z);
		glRotatef(-p->r, 0.0, 1.0, 0.0);
		if (mObjDB.find(p->obj) != mObjDB.end())
			PlotOneObj(mObjDB[p->obj], mShowCulled, mLit, mSolid);		
		glPopMatrix();
	}
	
	if (mShowBounds)
	{
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.2, 0.2, 0.2);
		GLUquadric * quad = gluNewQuadric();
		gluQuadricDrawStyle(quad, GLU_LINE);
		glPushMatrix();
		glTranslatef(mBounds.c.x, mBounds.c.y, mBounds.c.z);
		gluSphere (quad, sqrt(mBounds.radius_squared), 10, 10);
		gluDeleteQuadric(quad);
		glPopMatrix();
	}
		

	if (mIsPrototype || mIsFacade)
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_DEPTH_TEST);
		glPointSize(5.0);
		glColor3f(0.0, 1.0, 1.0);
		glBegin(GL_POINTS);
		for (int n = 0; n < mPts.size(); ++n)
		{
			glVertex3f(mPts[n].x, 0.0, mPts[n].y);
		}
		glEnd();
		glPointSize(1.0);
	}
	mZoomer.ResetMatrices();

}

void			XObjWin::ClickDown(int inX, int inY, int inButton)
{
	SetGLContext();
	mLastX = inX;
	mLastY = inY;

	int	h;
	GetBounds(NULL, &h);
	inY = h - inY;

	int	i[4] = { 0, 0, 0, 0 };
	GetBounds(i+2,i+3);

	mZoomer.SetupMatrices(i);
	
	mEditNum = -1;
	if (mIsPrototype || mIsFacade)
	{
		for (int n = 0; n < mPts.size(); ++n)
		{
			double	mpt[3] = { mPts[n].x, 0.0, mPts[n].y };
			double	spt[2];
			ModelToScreenPt(mpt, spt);
			
			double	xdif = abs(spt[0] - inX);
			double	ydif = abs(spt[1] - inY);
			if (xdif < 4 && ydif < 4)
				mEditNum = n;
		}
	}
	if (mEditNum == -1)
	{
		if (inButton == 0)
			mZoomer.HandleTranslationClick(i, xplm_MouseDown, inX, inY);
		else
			mZoomer.HandleRotationClick(i, xplm_MouseDown, inX, inY);
	}
	mZoomer.ResetMatrices();	
}


void			XObjWin::ClickUp(int inX, int inY, int inButton)
{
	SetGLContext();
	int	i[4] = { 0, 0, 0, 0 };
	GetBounds(i+2,i+3);

	if (mEditNum >= 0)
		this->ClickDrag(inX, inY, inButton);
	else {
	
		int	h;
		GetBounds(NULL, &h);
		inY = h - inY;
	
		if (inButton == 0)
			mZoomer.HandleTranslationClick(i, xplm_MouseUp, inX, inY);
		else
			mZoomer.HandleRotationClick(i, xplm_MouseUp, inX, inY);
		ForceRefresh();
	}
}

void			XObjWin::ClickDrag(int inX, int inY, int inButton)
{
	SetGLContext();
	int	h;
	GetBounds(NULL, &h);
	inY = h - inY;

	int	i[4] = { 0, 0, 0, 0 };
	GetBounds(i+2,i+3);

	if (mEditNum >= 0)
	{
	
		double	plane[4] = { 0.0, 1.0, 0.0, 0.0 };
		double	clickPt[3];
		
		if (mZoomer.FindPointOnPlane(i, plane, inX, inY, clickPt))
		{
			mPts[mEditNum].x = clickPt[0];
			mPts[mEditNum].y = clickPt[2];
			if (mIsPrototype)
				ApplyPrototype(mPrototype, mPts, mFloors, mObj);
			if (mIsFacade)
			{
				Polygon3	pts;
				for (vector<Point2>::iterator p = mPts.begin(); p != mPts.end(); ++p)
				{
					pts.push_back(Point3(p->x, 0.0, p->y));
				}
			
				mObj.cmds.clear();
				mObj.texture = mFacade.texture;
				BuildFacadeObj(mFacade, pts, mFloors, Vector3(0.0, 1.0, 0.0), ExtrudeFuncToObj, &mObj);
			}
			GetObjBoundingSphere(mObj, mBounds);
			ForceRefresh();
		}
	} else {
		if (inButton == 0)
			mZoomer.HandleTranslationClick(i, xplm_MouseDrag, inX, inY);
		else
			mZoomer.HandleRotationClick(i, xplm_MouseDrag, inX, inY);
	
		ForceRefresh();
	}
}

void			XObjWin::MouseWheel(int inX, int inY, int inDelta, int inAxis)
{
	SetGLContext();
	int	i[4] = { 0, 0, 0, 0 };
	GetBounds(i+2,i+3);

	mZoomer.HandleZoomWheel(i, inDelta, inX, inY);
	ForceRefresh();
}

void			XObjWin::ReceiveFiles(const vector<string>& files, int, int)
{
	SetGLContext();

	for (vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		if (HasExtNoCase(*i, ".obj")) {
			if (XObjRead(i->c_str(), mObj))
			{
				mIsPrototype = false;
				mIsFacade = false;
				string foo(*i);
				StripPath(foo);
				ScaleToObj();
				SetTitle(foo.c_str());
				ForceRefresh();
				
				if (mMeasureOnOpen)
					KeyPressed('m', 0,0,0);			
			}
		} else if (HasExtNoCase(*i, ".bmp") || HasExtNoCase(*i, ".png") || HasExtNoCase(*i, ".tif"))
		{
			SetGLContext();
			AccumTexture(*i);
		} else if (HasExtNoCase(*i, ".fac"))
		{
			if (ReadFacadeObjFile(i->c_str(), mFacade))
			{
				mIsPrototype = false;
				mIsFacade = true;
				mFloors = mFacade.lods[0].walls[0].bottom + mFacade.lods[0].walls[0].middle + mFacade.lods[0].walls[0].top;
				
				Polygon3	pts;
				for (vector<Point2>::iterator p = mPts.begin(); p != mPts.end(); ++p)
				{
					pts.push_back(Point3(p->x, 0.0, p->y));
				}
			
				mObj.cmds.clear();
				mObj.texture = mFacade.texture;
				BuildFacadeObj(mFacade, pts, mFloors, Vector3(0.0, 1.0, 0.0), ExtrudeFuncToObj, &mObj);
				GetObjBoundingSphere(mObj, mBounds);
				string foo(*i);
				StripPath(foo);
				ScaleToObj();
				SetTitle(foo.c_str());
				ForceRefresh();

			}
		} else if (HasExtNoCase(*i, ".pto"))		
		{			
			if (LoadPrototype(i->c_str(), mPrototype))
			{
				mIsPrototype = true;
				mIsFacade = false;
				mFloors = mPrototype.layers.size();
				
				ApplyPrototype(mPrototype, mPts, mFloors, mObj);
				GetObjBoundingSphere(mObj, mBounds);
				string foo(*i);
				StripPath(foo);
				ScaleToObj();
				SetTitle(foo.c_str());
				ForceRefresh();
				
			}
		} 
	}
}

int			XObjWin::KeyPressed(char inKey, long, long, long)
{
	SetGLContext();

	switch(inKey) {
	case 'M':
		mMeasureOnOpen = !mMeasureOnOpen;
		break;
	case 'm':
		{
			float	mins[3];
			float	maxs[3];
			GetObjDimensions(mObj, mins, maxs);
			char	buf[1024];
			sprintf(buf, "%f x %f x %f", maxs[0] - mins[0], maxs[1] - mins[1], maxs[2] - mins[2]);		
			DoUserAlert(buf);
		}	
		break;
	case '[':
		gCamDist -= 500;
		break;
	case ']':
		gCamDist += 500;
		break;
	case 28:
//		mXTrans += (mScale / 4.0);
		break;
	case 29:
//		mXTrans -= (mScale / 4.0);
		break;
	case 30:
//		mYTrans += (mScale / 4.0);
		break;
	case 31:
//		mYTrans -= (mScale / 4.0);
		break;
	case '=':
//		mScale *= 1.1;
		break;
	case '-':
//		mScale *= 0.9;
		break;
	case 'f':
	case 'F':
		mSolid = !mSolid;
		break;
	case 'l':
	case 'L':
		mLit = !mLit;
		break;
	case 'c':
	case 'C':
		mShowCulled = 1 - mShowCulled;
		break;
	case 'b':
	case 'B':
		mShowBounds = 1 - mShowBounds;
		break;
	case 'U':
	case 'u':
		mFloors++;
		if (mIsPrototype)
		{
			ApplyPrototype(mPrototype, mPts, mFloors, mObj);
			GetObjBoundingSphere(mObj, mBounds);
		}
		if (mIsFacade)
		{
			Polygon3	pts;
			for (vector<Point2>::iterator p = mPts.begin(); p != mPts.end(); ++p)
			{
				pts.push_back(Point3(p->x, 0.0, p->y));
			}
		
			mObj.cmds.clear();
			mObj.texture = mFacade.texture;
			BuildFacadeObj(mFacade, pts, mFloors, Vector3(0.0, 1.0, 0.0), ExtrudeFuncToObj, &mObj);
			GetObjBoundingSphere(mObj, mBounds);
		}			
		break;
	case 'D':
	case 'd':
		mFloors--;
		if (mFloors < 0) mFloors = 0;
		if (mIsPrototype)
		{
			ApplyPrototype(mPrototype, mPts, mFloors, mObj);
			GetObjBoundingSphere(mObj, mBounds);
		}
		if (mIsFacade)
		{
			Polygon3	pts;
			for (vector<Point2>::iterator p = mPts.begin(); p != mPts.end(); ++p)
			{
				pts.push_back(Point3(p->x, 0.0, p->y));
			}
		
			mObj.cmds.clear();
			mObj.texture = mFacade.texture;
			BuildFacadeObj(mFacade, pts, mFloors, Vector3(0.0, 1.0, 0.0), ExtrudeFuncToObj, &mObj);
			GetObjBoundingSphere(mObj, mBounds);
		}			
		break;
	}
	if (gCamDist < 0) gCamDist = 0;
	ForceRefresh();
	return 1;
}

void		XObjWin::ScaleToObj(void)
{
	mZoomer.ResetToIdentity();
	GetObjBoundingSphere(mObj, mBounds);
	double	rad = GetObjRadius(mObj);
	if (rad > 0.0)
	{
		mZoomer.SetScale(50.0 / rad);
	}
//	mXTrans = 0;
//	mYTrans = 0;
//	float a[3] = { 0.0, 1.0, 0.0 };
//	axis_to_quat(a, 0.0, mSpin); 	
}


#pragma mark -

void	XGrindFiles(const vector<string>& files)
{
	for (vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		if (HasExtNoCase(*i, ".obj"))
			(new XObjWin(i->c_str()))->ForceRefresh();
		else if (HasExtNoCase(*i, ".bmp") || HasExtNoCase(*i,".png") || HasExtNoCase(*i, ".tif"))
		{
			if (!sWindows.empty())
				(*sWindows.begin())->SetGLContext();
			AccumTexture(*i);
		}
	}
}

void	XGrindInit(void)
{
	(new XObjWin(NULL))->ForceRefresh();
	const char * ext = (const char *) glGetString(GL_EXTENSIONS);
	int	major=0, minor=0, revision=0;
	sscanf((const char *) glGetString(GL_VERSION), "%d.%d.%d", &major, &minor, &revision);
	int full = major * 100 + minor * 10 + revision;
	if (strstr(ext,"GL_ARB_multitexture") || full >= 121)
		gHasMultitexture = true;
	if (strstr(ext,"GL_EXT_texture_env_add") || strstr(ext,"GL_ARB_texture_env_add"))
		gHasEnvAdd = true;
	if (strstr(ext,"GL_EXT_texture_env_combine") || strstr(ext,"GL_ARB_texture_env_combine") || full >= 130)
		gHasCombine = true;
}

#pragma mark -

GLenum		FindTexture(const string& inName, bool inNight)
{
	if (inName.empty()) return 0;

	string 	name(inName);
	StringToUpper(name);
	if (inNight)
	{
		map<string, pair<string, GLenum> >::iterator i = gNightTextures.find(name);
		if (i == gNightTextures.end()) return 0;
		return i->second.second;
	} else {
		map<string, pair<string, GLenum> >::iterator i = gDayTextures.find(name);
		if (i == gDayTextures.end()) return 0;
		return i->second.second;
	}
}

void		AccumTexture(const string& inFileName)
{
	static	GLenum	gCounter = 1;	
	bool	lit = HasExtNoCase(inFileName, "LIT.bmp") || HasExtNoCase(inFileName, "LIT.png") || HasExtNoCase(inFileName, "LIT.tif");
	bool	lit_new = HasExtNoCase(inFileName, "_LIT.bmp") || HasExtNoCase(inFileName, "_LIT.png") || HasExtNoCase(inFileName, "_LIT.tif");
	map<string, pair<string, GLenum> >&	texDB = lit ? gNightTextures : gDayTextures;
	string	shortName = inFileName;
	if (!HasExtNoCase(inFileName, ".bmp") && !HasExtNoCase(inFileName, ".png") && !HasExtNoCase(inFileName, ".tif"))
		return;

	if (lit_new)
		shortName = shortName.substr(0, shortName.length() - 8);	
	else if (lit)	
		shortName = shortName.substr(0, shortName.length() - 7);
	else 
		shortName = shortName.substr(0, shortName.length() - 4);
		
	StripPathCP(shortName);	

	StringToUpper(shortName);
		
	for (map<string, pair<string, GLenum> >::iterator i = texDB.begin(); i != texDB.end(); ++i)
	{
		if (i->second.first == inFileName)
		{
			LoadTextureFromFile(inFileName.c_str(), i->second.second, tex_Wrap + (lit ? 0 : tex_MagentaAlpha), NULL, NULL, NULL, NULL);
			return;
		}
	}
	
	GLenum	texID = gCounter++;
	if (LoadTextureFromFile(inFileName.c_str(), texID, tex_Wrap + (lit ? 0 : tex_MagentaAlpha), NULL, NULL, NULL, NULL))
	{
		texDB.insert(map<string, pair<string, GLenum> >::value_type(
				shortName, pair<string, GLenum>(inFileName, texID)));
	}	
}

void		ReloadTexture(const string& inName)
{
	string	name(inName);
	StringToUpper(name);
	map<string, pair<string, GLenum> >::iterator i;
	i = gDayTextures.find(name);
	if (i != gDayTextures.end())
		LoadTextureFromFile(i->second.first.c_str(), i->second.second, tex_Wrap + tex_MagentaAlpha, NULL, NULL, NULL, NULL);

	i = gNightTextures.find(name);
	if (i != gNightTextures.end())
		LoadTextureFromFile(i->second.first.c_str(), i->second.second, tex_Wrap, NULL, NULL, NULL, NULL);

}

#pragma mark -

void	PlotOneObj(const XObj& inObj, int inShowCulled, bool inLit, bool inSolid)
{
	string	tex = inObj.texture;
	StripPathCP(tex);
	GLenum t = FindTexture(tex, false);
	
	if (t)
		glBindTexture(GL_TEXTURE_2D, t);
	CHECK_ERR();
		


	bool	hasLit = false;		
	if (inLit)
	{
		t = FindTexture(tex, true);
		if (t && gHasMultitexture && gHasEnvAdd)
		{
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBindTexture(GL_TEXTURE_2D, t);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			hasLit = true;
		}
	}

	for (int side = 0; side < (inShowCulled+1); ++side)
	{
		glShadeModel(GL_SMOOTH);
		GLfloat col1[4]={0.2,0.2,0.2,1.0};	glMaterialfv(GL_FRONT,GL_AMBIENT  ,col1);
		GLfloat col2[4]={0.8,0.8,0.8,1.0};	glMaterialfv(GL_FRONT,GL_DIFFUSE  ,col2);
		GLfloat col3[4]={0.0,0.0,0.0,1.0};	glMaterialfv(GL_FRONT,GL_SPECULAR ,col3);
		GLfloat col4[4]={0.0,0.0,0.0,1.0};	glMaterialfv(GL_FRONT,GL_EMISSION ,col4);
											glMaterialf (GL_FRONT,GL_SHININESS,   0);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		
		if (side) 
		{
			glCullFace(GL_FRONT);
		} else {
			glCullFace(GL_BACK);
		}
	
		bool	draw = true;

		for (vector<XObjCmd>::const_iterator i = inObj.cmds.begin(); i != inObj.cmds.end(); ++i)
		{
			switch(i->cmdType) {
			case type_Attr:
				switch(i->cmdID) {
				case attr_LOD:
					draw = (gCamDist >= i->attributes[0]) &&
							(gCamDist < i->attributes[1]);
					break;
				case attr_Shade_Flat  :glShadeModel(GL_FLAT  );																				break;
				case attr_Shade_Smooth:glShadeModel(GL_SMOOTH);																				break;
				case attr_Ambient_RGB :{float c[4]={i->attributes[0],i->attributes[1],i->attributes[2],1.0}; glMaterialfv(GL_FRONT, GL_AMBIENT ,c);}	break;
				case attr_Diffuse_RGB :{float c[4]={i->attributes[0],i->attributes[1],i->attributes[2],1.0}; glMaterialfv(GL_FRONT, GL_DIFFUSE ,c);}	break;
				case attr_Emission_RGB:{float c[4]={i->attributes[0],i->attributes[1],i->attributes[2],1.0}; glMaterialfv(GL_FRONT, GL_EMISSION,c);}	break;
				case attr_Specular_RGB:{float c[4]={i->attributes[0],i->attributes[1],i->attributes[2],1.0}; glMaterialfv(GL_FRONT, GL_SPECULAR,c);}	break;
				case attr_Shiny_Rat   :glMaterialf(GL_FRONT,GL_SHININESS, i->attributes[0]);												break;
				case attr_No_Depth    :glDisable(GL_DEPTH_TEST);																			break;
				case attr_Depth       :glEnable(GL_DEPTH_TEST);																				break;
				case attr_Offset	  :if(i->attributes[0]==0.0)
									   		{glDisable(GL_POLYGON_OFFSET_FILL);	glPolygonOffset( 0.0,0.0);}
									   else
									   		{glEnable(GL_POLYGON_OFFSET_FILL);	glPolygonOffset(-5.0 * i->attributes[0],0.0);}				break;
				case attr_Reset       :
			               GLfloat col1[4]={0.2,0.2,0.2,1.0};	glMaterialfv(GL_FRONT,GL_AMBIENT  ,col1);
				           GLfloat col2[4]={0.8,0.8,0.8,1.0};	glMaterialfv(GL_FRONT,GL_DIFFUSE  ,col2);
				           GLfloat col3[4]={0.0,0.0,0.0,1.0};	glMaterialfv(GL_FRONT,GL_SPECULAR ,col3);
				           GLfloat col4[4]={0.0,0.0,0.0,1.0};	glMaterialfv(GL_FRONT,GL_EMISSION ,col4);
																glMaterialf (GL_FRONT,GL_SHININESS,   0);									break;
				case attr_Cull        :glEnable (GL_CULL_FACE );																			break;
				case attr_NoCull      :glDisable(GL_CULL_FACE );																			break;
				}
				break;
				
			case type_PtLine:
				if (draw) 
				{
					glActiveTextureARB(GL_TEXTURE1_ARB);
					glDisable(GL_TEXTURE_2D);
					glActiveTextureARB(GL_TEXTURE0_ARB);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_ALPHA_TEST);
					glPointSize(5);
					switch(i->cmdID) {
					case obj_Line:
						glBegin(GL_LINES);
						break;
					case obj_Light:
						glBegin(GL_POINTS);
						break;
					}
					
					for (vector<vec_rgb>::const_iterator j = i->rgb.begin(); j != i->rgb.end(); ++j)
					{
						glColor3f(j->rgb[0] / 10.0, j->rgb[1] / 10.0, j->rgb[2] / 10.0);
						glVertex3fv(j->v);
					}
					
					glEnd();
				}
				break;			
				
				
				
			case type_Poly:
				if (draw) 
				{
					if (inSolid)
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					else 
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					if (side)
					{
						if (inLit)
							glColor3f(0.5, 0.1, 0.1);
						else
							glColor3f(1.0, 0.3, 0.3);
					} else {
						if (inLit)
							glColor3f(0.1, 0.1, 0.1);
						else
							glColor3f(1.0, 1.0, 1.0);
					}
					glActiveTextureARB(GL_TEXTURE1_ARB);
					if (hasLit)
						glEnable(GL_TEXTURE_2D);
					else
						glDisable(GL_TEXTURE_2D);
					
					if (hasLit)
					{
							GLint	hasAlpha;
							glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &hasAlpha);
						if (!hasAlpha || !gHasCombine)					
							glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
						else {
							glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
				
							glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, 	GL_ADD);
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, 	GL_PREVIOUS_EXT);
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, 	GL_SRC_COLOR);
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, 	GL_TEXTURE);
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, 	GL_SRC_COLOR);

							glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, 	GL_ADD);
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, 	GL_PREVIOUS_EXT);
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, 	GL_SRC_ALPHA);
							glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, 	GL_TEXTURE);
							glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, 	GL_SRC_ALPHA);
						}
					}
					glActiveTextureARB(GL_TEXTURE0_ARB);					
					glEnable(GL_TEXTURE_2D);						
					glEnable(GL_ALPHA_TEST);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
					switch(i->cmdID) {
					case obj_Tri:
						glBegin(GL_TRIANGLES);
						break;
					case obj_Quad:
					case obj_Quad_Hard:
					case obj_Smoke_Black:
					case obj_Smoke_White:
					case obj_Movie:
						glBegin(GL_QUADS);
						break;
					case obj_Polygon:
						glBegin(GL_POLYGON);
						break;
					case obj_Quad_Strip:
						glBegin(GL_QUAD_STRIP);
						break;
					case obj_Tri_Strip:
						glBegin(GL_TRIANGLE_STRIP);
						break;
					case obj_Tri_Fan:
						glBegin(GL_TRIANGLE_FAN);
						break;
					}
					
					for (vector<vec_tex>::const_iterator j = i->st.begin(); j != i->st.end(); ++j)
					{						
						if (hasLit)
							glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, j->st);
						glTexCoord2fv(j->st);
						glVertex3fv(j->v);
					}
					
					glEnd();
				}
				break;
			}
		}
	}	
}