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

#define FACADES 0

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>
#else
	#include "glew.h"
//	#include <GL/gl.h>
//	#include <GL/glu.h>
#endif


#include "XObjDefs.h"
#include "XWinGL.h"
#include "ObjDraw.h"
#include "ObjUtils.h"
#include "XUtils.h"
#include "GeoUtils.h"
#include "TexUtils.h"
#include "MatrixUtils.h"
#include "trackball.h"
#include "XObjReadWrite.h"
#include "XGUIApp.h"
#include <set>
#include "PlatformUtils.h"
#include "OE_Zoomer3d.h"
#include <time.h>
#include "PerfUtils.h"


#if SOTHIS_TEST
// XPopup testpopup;
#endif

#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"

static float gStopTime = 0.0;

inline float float_clock(void)
{
	static unsigned long tbase = query_hpc();
	unsigned long tnow = query_hpc() - tbase;
	double tdob = hpc_to_microseconds(tnow);
	return tdob / 1000000.0;
}

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

float DistForLOD8(const XObj8& obj, int lod)
{
	if(obj.lods.empty()) return 0.0;
	--lod;
	if(lod < 0) lod = 0;
	if (lod >= obj.lods.size()) lod = obj.lods.size()-1;
	return 0.5 * (obj.lods[lod].lod_near + obj.lods[lod].lod_far);
}

class	XObjWin;

XObjWin *	gReceiver = NULL;
float	gCamDist = 200;

map<string, pair<string, GLenum> >		gDayTextures;
//map<string, pair<string, GLenum> >		gNightTextures;

static	void		PlotOneObj(const XObj& inObj, int inShowCulled, bool inLit, bool inLighting, bool inSolid, bool inAnimate, float dist);
static	void		PlotOneObj8(const XObj8& inObj, int inShowCulled, bool inLit, bool inLighting, bool inSolid, bool inAnimate, float dist);
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

	virtual	void			Timer(void) { ForceRefresh(); }
	virtual	void			GLReshaped(int inWidth, int inHeight);
	virtual	void			GLDraw(void);

	virtual	bool			Closed(void) { return true; }
	virtual	void			ClickDown(int inX, int inY, int inButton);
	virtual	void			ClickUp(int inX, int inY, int inButton);
	virtual	void			ClickDrag(int inX, int inY, int inButton);
	virtual void			ClickMove(int inX, int inY) { }
	virtual	void			MouseWheel(int inX, int inY, int inDelta, int inAxis);
	virtual	void			ReceiveFiles(const vector<string>& inFiles, int, int);
	virtual	int				KeyPressed(unsigned int inKey, long, long, long);

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
	XObj8			mObj8;
//	Prototype_t		mPrototype;
	Polygon2 		mPts;
//	Sphere3			mBounds;

//	bool	mIsPrototype;
	bool	mIsObj8;

//	float	mScale;
//	float	mSpin[4];
//	float	mXTrans;
//	float	mYTrans;
	bool	mSolid;
	bool	mLit;
	int		mLOD;
	bool	mAnimate;
	bool	mLighting;
	bool	mMeasureOnOpen;
	int		mShowCulled;
//	int		mShowBounds;

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



XObjWin::XObjWin(const char * inFileName) : XWinGL(1, inFileName ? inFileName : "Drag Obj Here",
	xwin_style_resizable | xwin_style_visible | xwin_style_centered,
	50, 50, 600, 600, sWindows.empty() ? NULL : *sWindows.begin()),
	/*(mScale(1.0),*/ mSolid(true), mShowCulled(false), /*mShowBounds(false), */mLit(false), mAnimate(false), mLighting(true), mMeasureOnOpen(false), /*mXTrans(0), mYTrans(0), */ mIsObj8(false),mLOD(1)
{
	#if SOTHIS_TEST
	// testpopup.register_target(_mDisplay, mWindow);
	#endif
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
		glClearColor(0.0, 0.0, 0.0, 0);
	else
		glClearColor(0.3, 0.5, 0.6, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
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
	if (mIsObj8)	PlotOneObj8(mObj8, mShowCulled, mLit, mLighting, mSolid, mAnimate, DistForLOD8(mObj8,mLOD));
	else			PlotOneObj(mObj, mShowCulled, mLit, mLighting, mSolid, mAnimate, 0.0);


	for (vector<ObjPlacement_t>::iterator p = mObjInst.begin(); p != mObjInst.end(); ++p)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(p->x, p->y, p->z);
		glRotatef(-p->r, 0.0, 1.0, 0.0);
		if (mObjDB.find(p->obj) != mObjDB.end())
			PlotOneObj(mObjDB[p->obj], mShowCulled, mLit, mLighting, mSolid, mAnimate, 0.0f);
		glPopMatrix();
	}

/*	if (mShowBounds)
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
*/

	mZoomer.ResetMatrices();

}

void			XObjWin::ClickDown(int inX, int inY, int inButton)
{
	SetGLContext();
	mLastX = inX;
	mLastY = inY;

	#if SOTHIS_TEST
	// if (inButton == 1) testpopup.show_at_pointer();
	if (inButton == 1) DoUserAlert(mWindow, "Hello here");
	#endif

	int	h;
	GetBounds(NULL, &h);
	inY = h - inY;

	int	i[4] = { 0, 0, 0, 0 };
	GetBounds(i+2,i+3);

	mZoomer.SetupMatrices(i);

	mEditNum = -1;
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
			mPts[mEditNum].x_ = clickPt[0];
			mPts[mEditNum].y_ = clickPt[2];
//			xflt	s[4];
//			GetObjBoundingSphere(mObj, s);
//			mBounds.c = Point3(s[0], s[1], s[2]);
//			mBounds.radius_squared = s[3];
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
			if (XObj8Read(i->c_str(), mObj8))
			{
				mIsObj8 = true;
				string foo(*i);
				StripPath(foo);
				ScaleToObj();
				SetTitle(foo.c_str());
				ForceRefresh();

				if (mMeasureOnOpen)
					KeyPressed('m', 0,0,0);
			}
			else if (XObjRead(i->c_str(), mObj))
			{
				mIsObj8 = false;
				string foo(*i);
				StripPath(foo);
				ScaleToObj();
				SetTitle(foo.c_str());
				ForceRefresh();

				if (mMeasureOnOpen)
					KeyPressed('m', 0,0,0);
			}
		} else if (HasExtNoCase(*i, ".bmp") || HasExtNoCase(*i, ".png") || HasExtNoCase(*i, ".tif") || HasExtNoCase(*i,".dds"))
		{
			SetGLContext();
			AccumTexture(*i);
		}
	}
}

int			XObjWin::KeyPressed(unsigned int inKey, long, long, long)
{
	SetGLContext();
	int x, y;

	switch(inKey) {
	case 'M':
		mMeasureOnOpen = !mMeasureOnOpen;
		break;
	case 'm':
		{
			float	mins[3];
			float	maxs[3];
			if (mIsObj8)
			GetObjDimensions8(mObj8, mins, maxs);
			else
			GetObjDimensions(mObj, mins, maxs);
			char	buf[1024];
			sprintf(buf, "%f (x) x %f (y) x %f (z) meters", maxs[0] - mins[0], maxs[1] - mins[1], maxs[2] - mins[2]);
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
	case '-':
		GetMouseLoc(&x, &y);
		MouseWheel(x, y, inKey == '=' ? 1 : -1, 0);
		break;
	case 'f':
	case 'F':
		mSolid = !mSolid;
		break;
	case 'A':
	case 'a':
		if (!mAnimate)
			gStopTime = float_clock() - gStopTime;
		else
			gStopTime = float_clock();
		mAnimate = !mAnimate;
		if(mAnimate)
			SetTimerInterval(0.01);
		else
			SetTimerInterval(0);
		break;
	case 'n':
	case 'N':
		mLit = !mLit;
		break;
	case 'l':
	case 'L':
		mLighting = !mLighting;
		break;
	case 'c':
	case 'C':
		mShowCulled = 1 - mShowCulled;
		break;
//	case 'b':
//	case 'B':
//		mShowBounds = 1 - mShowBounds;
//		break;
	case '1':mLOD=1;break;
	case '2':mLOD=2;break;
	case '3':mLOD=3;break;
	case '4':mLOD=4;break;
	case '5':mLOD=5;break;
	}
	if (gCamDist < 0) gCamDist = 0;
	ForceRefresh();
	return 1;
}

void		XObjWin::ScaleToObj(void)
{
	mZoomer.ResetToIdentity();
//	xflt	s[4];
//	if (mIsObj8)
//		GetObjBoundingSphere8(mObj8, s);
//	else
//		GetObjBoundingSphere(mObj, s);
//	mBounds.c = Point3(s[0], s[1], s[2]);
//	mBounds.radius_squared = s[3];
	double	rad = mIsObj8 ? GetObjRadius8(mObj8) : GetObjRadius(mObj);
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
		else if (HasExtNoCase(*i, ".bmp") || HasExtNoCase(*i,".png") || HasExtNoCase(*i, ".tif") || HasExtNoCase(*i,".dds"))
		{
			if (!sWindows.empty())
				(*sWindows.begin())->SetGLContext();
			AccumTexture(*i);
		}
	}
}

void	XGrindInit(int argc, char* argv[])
{
	XObjWin * win = new XObjWin(NULL);
#if LIN
	win->show(argc,argv);
	win->SetGLContext();
#endif
#if !APL
	GLenum err = glewInit();
	if (err != GLEW_OK)
		AssertPrintf("Can not init GLEW: '%s'\n", glewGetErrorString(err));
#endif
	win->GetMenuBar();

	win->ForceRefresh();

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

	if (!gHasMultitexture)
	{
		DoUserAlert("Your OpenGL drivers are not new enough to run ObjView.");
		exit(1);
	}
}

#pragma mark -

GLenum		FindTexture(const string& inName, bool inNight)
{
	if (inName.empty()) return 0;

	string 	name(inName);
	StringToUpper(name);

	map<string, pair<string, GLenum> >::iterator i = gDayTextures.find(name);
	if (!inNight)
	{
		if (i != gDayTextures.end()) return i->second.second;
	}

	if (inNight)
	{
		i = gDayTextures.find(name + "_LIT");
		if (i != gDayTextures.end()) return i->second.second;

		i = gDayTextures.find(name + "LIT");
		if (i != gDayTextures.end()) return i->second.second;
	}
	return 0;
}

void		AccumTexture(const string& inFileName)
{
	static	GLenum	gCounter = 1;
	bool	lit = HasExtNoCase(inFileName, "LIT.bmp") || HasExtNoCase(inFileName, "LIT.png") || HasExtNoCase(inFileName, "LIT.tif") || HasExtNoCase(inFileName, "LIT.dds");
	bool	lit_new = HasExtNoCase(inFileName, "_LIT.bmp") || HasExtNoCase(inFileName, "_LIT.png") || HasExtNoCase(inFileName, "_LIT.tif") || HasExtNoCase(inFileName, "_LIT.dds");
	map<string, pair<string, GLenum> >&	texDB = gDayTextures;
	string	shortName = inFileName;
	if (!HasExtNoCase(inFileName, ".bmp") && !HasExtNoCase(inFileName, ".png") && !HasExtNoCase(inFileName, ".tif") && !HasExtNoCase(inFileName, ".dds"))
		return;

//	if (lit_new)
//		shortName = shortName.substr(0, shortName.length() - 8);
//	else if (lit)
//		shortName = shortName.substr(0, shortName.length() - 7);
//	else
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

//	i = gNightTextures.find(name);
//	if (i != gNightTextures.end())
//		LoadTextureFromFile(i->second.first.c_str(), i->second.second, tex_Wrap, NULL, NULL, NULL, NULL);

}

#pragma mark -

struct	ObjViewInfo_t {
	bool	lit;
	bool	solid;
	bool	backside;
	bool	animate;

	GLenum		tex;
	GLenum		tex_lit;
	GLenum		pan;
	GLenum		pan_lit;
	GLenum		drp;
};

static void	ObjView_SetupPoly(void * ref)
{
	ObjViewInfo_t * i = (ObjViewInfo_t *) ref;
	glActiveTextureARB(GL_TEXTURE1_ARB);
	if (i->lit && i->tex_lit != 0)
	{
		glBindTexture(GL_TEXTURE_2D, i->tex_lit);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	} else
		glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	if (i->tex != 0)
	{
		glBindTexture(GL_TEXTURE_2D, i->tex);
		glEnable(GL_TEXTURE_2D);
	} else
		glDisable(GL_TEXTURE_2D);

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	if (i->lit)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	else		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	if (i->backside) glColor3f(1.0, 0.0, 0.0); else glColor3f(1.0, 1.0, 1.0);
}

static void	ObjView_SetupPanel(void * ref)
{
	ObjViewInfo_t * i = (ObjViewInfo_t *) ref;
	glActiveTextureARB(GL_TEXTURE1_ARB);
	if (i->lit && i->pan_lit != 0)
	{
		glBindTexture(GL_TEXTURE_2D, i->pan_lit);
		glEnable(GL_TEXTURE_2D);
	} else
		glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	if (i->pan != 0)
	{
		glBindTexture(GL_TEXTURE_2D, i->pan);
		glEnable(GL_TEXTURE_2D);
	} else
		glDisable(GL_TEXTURE_2D);

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	if (i->lit)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	else		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

//	if (i->backside) glColor3f(1.0, 0.0, 0.0); else glColor3f(1.0, 1.0, 1.0);
}


static void 	ObjView_SetupLine(void * ref)
{
	glDisable(GL_LIGHTING);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
}

static void	ObjView_TexCoord(const float * st, void * ref)
{
	glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, st);
	glTexCoord2fv(st);
}

static void	ObjView_TexCoordPointer(int size, unsigned long type, long stride, const void * pointer, void * ref)
{
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(size, type, stride, pointer);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glTexCoordPointer(size, type, stride, pointer);
}

static float	ObjView_GetAnimParam(const char * string, float v1, float v2, void * ref)
{
	ObjViewInfo_t * i = (ObjViewInfo_t *) ref;
//	if (!i->animate) return (v1 + v2) * 0.5;
	if (v1 == v2) return v1;
	double	now = (i->animate) ? (float_clock() - gStopTime) : gStopTime;

	now *= 0.1;
	now -= (float) ((int) now);


	if (now > 0.5)	now = 1.0 - now;
	now *= 2.0;

	printf("%f\n", now);
	return v1 + (v2 - v1) * now;
}

static void ObjView_SetDraped(void * ref)
{
	ObjViewInfo_t * i = (ObjViewInfo_t *) ref;
	if(i->drp)
		glBindTexture(GL_TEXTURE_2D, i->drp);		CHECK_ERR();
}

static void ObjView_SetNoDraped(void * ref)
{
	ObjViewInfo_t * i = (ObjViewInfo_t *) ref;
	if(i->tex)
		glBindTexture(GL_TEXTURE_2D, i->tex);		CHECK_ERR();
}

static	ObjDrawFuncs10_t sCallbacks = {
	ObjView_SetupPoly, ObjView_SetupLine, ObjView_SetupLine,
	ObjView_SetupPoly, ObjView_SetupPanel, ObjView_TexCoord, ObjView_TexCoordPointer, ObjView_GetAnimParam,
	ObjView_SetDraped, ObjView_SetNoDraped
};

static void setup_lights(bool inLighting, bool inLit, bool inShowCulled)
{
	GLfloat lgt_amb[4]={ 1.0, 1.0, 1.0, 1.0 };
	GLfloat lgt_dif[4]={ 0.0, 0.0, 0.0, 1.0 };
	GLfloat lgt_dir[4]={ 1.0, 0.0, 0.0, 0.0 };

	if (inLighting)
	{
		lgt_amb[0] = lgt_amb[1] = lgt_amb[2] = 0.5;
		lgt_dif[0] = lgt_dif[1] = lgt_dif[2] = 0.5;
	}
	if (inLit) {
		lgt_dif[0] *= 0.01;
		lgt_dif[1] *= 0.01;
		lgt_dif[2] *= 0.01;
		lgt_amb[0] *= 0.01;
		lgt_amb[1] *= 0.01;
		lgt_amb[2] *= 0.01;
	}

	if (inShowCulled) { lgt_amb[1] = lgt_amb[2] = lgt_dif[1] = lgt_dif[2] = 0.0; }

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	GLfloat	wicked_dahk[4] = { 0.0, 0.0, 0.0, 0.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, wicked_dahk);
	glLightfv(GL_LIGHT0,GL_AMBIENT ,lgt_amb);
	glLightfv(GL_LIGHT0,GL_DIFFUSE ,lgt_dif);
	glLightfv(GL_LIGHT0,GL_POSITION,lgt_dir);

	glPopMatrix();
}

static void setup_baseline_ogl(int hidden_geo)
{
	glShadeModel(GL_SMOOTH);
	GLfloat col1[4]={0.2,0.2,0.2,1.0};	glMaterialfv(GL_FRONT,GL_AMBIENT  ,col1);
	GLfloat col2[4]={0.8,0.8,0.8,1.0};	glMaterialfv(GL_FRONT,GL_DIFFUSE  ,col2);
	GLfloat col3[4]={0.0,0.0,0.0,1.0};	glMaterialfv(GL_FRONT,GL_SPECULAR ,col3);
	GLfloat col4[4]={0.0,0.0,0.0,1.0};	glMaterialfv(GL_FRONT,GL_EMISSION ,col4);
										glMaterialf (GL_FRONT,GL_SHININESS,   0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glPointSize(4);
	glCullFace(hidden_geo ? GL_FRONT : GL_BACK);
}

static void setup_textures(const string& in_tex, const string& in_lit, const string& tex_drp, bool inLit, bool inSolid, ObjViewInfo_t& info)
{
	string	tex = in_tex;
	string	drp = tex_drp;
//	if (tex.size() > 4)	tex.erase(tex.size()-4);
	StripPathCP(tex);
	StripPathCP(drp);

	info.tex = FindTexture(tex, false);
	if (info.tex)	glBindTexture(GL_TEXTURE_2D, info.tex);		CHECK_ERR();

	if (inLit)
	{
		string tex_night = in_lit;
//		if (tex_night.size() > 4)	tex_night.erase(tex_night.size() - 4);
		StripPathCP(tex_night);
									info.tex_lit = FindTexture(tex_night, false);
		if (info.tex_lit)
		{
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBindTexture(GL_TEXTURE_2D, info.tex_lit);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	}

				info.pan	 = FindTexture("panel", false);
	if (inLit)	info.pan_lit = FindTexture("panel", true);

				info.drp = FindTexture(drp, false);

	if (inSolid)	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_NORMALIZE);
}

void	PlotOneObj(const XObj& inObj, int inShowCulled, bool inLit, bool inLighting, bool inSolid, bool inAnimate, float dist)
{
	inLighting = false;	// NEVER light these - it don't work yet!
	ObjViewInfo_t info = { inLit, inSolid, (inShowCulled != 0), inAnimate, 0, 0, 0, 0, 0 };

	string lit_tex = inObj.texture;
	if (!lit_tex.empty()) lit_tex += "_LIT";

	setup_textures(inObj.texture, lit_tex, "", inLit, inSolid, info);

	if (inShowCulled)
	{
		setup_lights(inLighting, inLit, inShowCulled);
		setup_baseline_ogl(true);
		info.backside = true;
		CHECK_ERR();
		ObjDraw(inObj, dist, &sCallbacks, &info);
		CHECK_ERR();
	}


	setup_lights(inLighting, inLit, false);
	setup_baseline_ogl(false);
	info.backside = false;
	CHECK_ERR();
	ObjDraw(inObj, dist, &sCallbacks, &info);
	CHECK_ERR();

	glPointSize(1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void	PlotOneObj8(const XObj8& inObj, int inShowCulled, bool inLit, bool inLighting, bool inSolid, bool inAnimate, float dist)
{
	CHECK_ERR();

	ObjViewInfo_t info = { inLit, inSolid, (inShowCulled != 0), inAnimate, 0, 0, 0, 0 };

	setup_textures(inObj.texture.substr(0,(inObj.texture.length() > 4) ? (inObj.texture.length() - 4) : (inObj.texture.length())),
					inObj.texture_lit.substr(0,(inObj.texture_lit.length() > 4) ? (inObj.texture_lit.length() - 4) : (inObj.texture_lit.length())),
					inObj.texture_draped.substr(0,(inObj.texture_draped.length() > 4) ? (inObj.texture_draped.length() - 4) : (inObj.texture_draped.length())),
					inLit, inSolid, info);

	if (inShowCulled)
	{
		setup_lights(inLighting, inLit, inShowCulled);
		setup_baseline_ogl(true);
		info.backside = true;
		CHECK_ERR();
		ObjDraw8(inObj, dist, &sCallbacks, &info);
		CHECK_ERR();
	}

	setup_lights(inLighting, inLit, false);
	setup_baseline_ogl(false);
	info.backside = false;
	CHECK_ERR();
	ObjDraw8(inObj, dist, &sCallbacks, &info);
	CHECK_ERR();

	glPointSize(1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}