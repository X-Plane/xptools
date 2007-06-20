#include "WED_ImageOverlayTool.h"
#include "WED_Persistent.h"
#include "XESConstants.h"
#include "GUI_GraphState.h"
#include "WED_MapZoomerNew.h"
#include "WED_UIDefs.H"
#include "PlatformUtils.h"
#include "TexUtils.h"
#include "GISUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

static double kCornerBlend0[10] = { 1.0,	0.0, 0.0, 0.0, 0.5,	0.0, 0.0, 0.5,  0.55, -0.25 };
static double kCornerBlend1[10] = { 0.0,	1.0, 0.0, 0.0, 0.5,	0.5, 0.0, 0.0, -0.05,  0.75 };
static double kCornerBlend2[10] = { 0.0,	0.0, 1.0, 0.0, 0.0,	0.5, 0.5, 0.0, -0.05,  0.75 };
static double kCornerBlend3[10] = { 0.0,	0.0, 0.0, 1.0, 0.0,	0.0, 0.5, 0.5,  0.55, -0.25 };

// Handles:
//  9
// 251
// 6 4
// 370
//  8 <- uniform resize

WED_ImageOverlayTool::WED_ImageOverlayTool(const char *	tool_name, GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_HandleToolBase(tool_name, host,zoomer,resolver),
	mVisible(false),
	mBits(false),
	mTexID(0)
{
	mCoords.resize(4);
	SetControlProvider(this);	
	SetCanSelect(0);
}

WED_ImageOverlayTool::~WED_ImageOverlayTool()
{
	if (mBits)
		glDeleteTextures(1, (GLuint *) &mTexID);
}

void		WED_ImageOverlayTool::PickFile(void)
{
	char buf[1024];
	if (GetFilePathFromUser(getFile_Open, "Please pick an image file", "Open", FILE_DIALOG_PICK_IMAGE_OVERLAY, buf, sizeof(buf)))
	{
		if (mTexID == 0)
			glGenTextures(1, (GLuint *) &mTexID);
	
//		XPLMBindTexture2d(mTexID, 0);

		int w, h;

		if (LoadTextureFromFile(buf, mTexID, tex_Linear + tex_Mipmap, &w, &h, &mS, &mT))
		{
			mVisible = true;
			mBits = true;
			mFile = buf;
			double c[8];
			if (FetchTIFFCorners(buf, c))
			{
				// SW, SE, NW, NE from tiff, but SE NE NW SW internally
				mCoords[3].x = c[0];
				mCoords[3].y = c[1];
				mCoords[0].x = c[2];
				mCoords[0].y = c[3];
				mCoords[2].x = c[4];
				mCoords[2].y = c[5];
				mCoords[1].x = c[6];
				mCoords[1].y = c[7];
			}
			else
			{
				double	nn,ss,ee,ww;
				GetZoomer()->GetPixelBounds(ww,ss,ee,nn);
				
				Point2 center((ee+ww)*0.5,(nn+ss)*0.5);
								
				double grow_x = 0.5*(ee-ww)/((double) w*mS);
				double grow_y = 0.5*(nn-ss)/((double) h*mT);
				
				double pix_w, pix_h;
				
				if (grow_x < grow_y) { pix_w = grow_x * (double) w*mS;	pix_h = grow_x * (double) h*mT; }
				else				 { pix_w = grow_y * (double) w*mS;	pix_h = grow_y * (double) h*mT; }
				
				mCoords[0] = GetZoomer()->PixelToLL(center + Vector2( pix_w,-pix_h));
				mCoords[1] = GetZoomer()->PixelToLL(center + Vector2( pix_w,+pix_h));
				mCoords[2] = GetZoomer()->PixelToLL(center + Vector2(-pix_w,+pix_h));
				mCoords[3] = GetZoomer()->PixelToLL(center + Vector2(-pix_w,-pix_h));
			}
		}
		GetHost()->Refresh();
	}
}

void		WED_ImageOverlayTool::ToggleVisible(void)
{ 
	mVisible = !mVisible;
	GetHost()->Refresh();
}


void		WED_ImageOverlayTool::DrawVisualization		(int inCurrent, GUI_GraphState * g)
{
	if (mVisible && mBits)
	{
		g->SetState(0, 1, 0,    0, 1,  0, 0);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		g->BindTex(mTexID, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[3].x),
					GetZoomer()->LatToYPixel(mCoords[3].y));
		glTexCoord2f(0.0, mT );
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[2].x),
					GetZoomer()->LatToYPixel(mCoords[2].y));
		glTexCoord2f(mS , mT );
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[1].x),
					GetZoomer()->LatToYPixel(mCoords[1].y));
		glTexCoord2f(mS , 0.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[0].x),
					GetZoomer()->LatToYPixel(mCoords[0].y));
		glEnd();
	}

}

void	WED_ImageOverlayTool::BeginEdit(void)
{
}

void	WED_ImageOverlayTool::EndEdit(void)
{
}

int		WED_ImageOverlayTool::CountEntities(void) const
{
	return mBits ? 1 : 0;
}

int		WED_ImageOverlayTool::GetNthEntityID(int n) const
{
	return 0;
}

int		WED_ImageOverlayTool::CountControlHandles(int id						  ) const
{
	return 10;
}

void	WED_ImageOverlayTool::GetNthControlHandle(int id, int n, int * active, HandleType_t * con_type, Point2 * p, Vector2 * direction, float * radius) const
{
	if (p) *p = Point2(kCornerBlend0[n] * mCoords[0].x + kCornerBlend1[n] * mCoords[1].x + kCornerBlend2[n] * mCoords[2].x + kCornerBlend3[n] * mCoords[3].x,
					   kCornerBlend0[n] * mCoords[0].y + kCornerBlend1[n] * mCoords[1].y + kCornerBlend2[n] * mCoords[2].y + kCornerBlend3[n] * mCoords[3].y);
	if (con_type) *con_type = (n==9) ? handle_Arrow : handle_Square;
	if (direction) *direction = (n==9) ? Vector2(
											Point2(	kCornerBlend0[5] * mCoords[0].x + kCornerBlend1[5] * mCoords[1].x + kCornerBlend2[5] * mCoords[2].x + kCornerBlend3[5] * mCoords[3].x,
													kCornerBlend0[5] * mCoords[0].y + kCornerBlend1[5] * mCoords[1].y + kCornerBlend2[5] * mCoords[2].y + kCornerBlend3[5] * mCoords[3].y),
											Point2(	kCornerBlend0[9] * mCoords[0].x + kCornerBlend1[9] * mCoords[1].x + kCornerBlend2[9] * mCoords[2].x + kCornerBlend3[9] * mCoords[3].x,
													kCornerBlend0[9] * mCoords[0].y + kCornerBlend1[9] * mCoords[1].y + kCornerBlend2[9] * mCoords[2].y + kCornerBlend3[9] * mCoords[3].y)) : Vector2();		
	if (active) *active=1;
}

int		WED_ImageOverlayTool::GetLinks		    (int id) const
{
	return 4;
}

void	WED_ImageOverlayTool::GetNthLinkInfo		(int id, int n, int * active, LinkType_t * ltype) const
{
	if (active) *active=1;
	if (ltype) *ltype = link_Solid;
}

int		WED_ImageOverlayTool::GetNthLinkSource   (int id, int n) const
{
	return n;
}

int		WED_ImageOverlayTool::GetNthLinkSourceCtl(int id, int n) const
{
	return -1;
}

int		WED_ImageOverlayTool::GetNthLinkTarget   (int id, int n) const
{
	return (n+1)% 4;
}

int		WED_ImageOverlayTool::GetNthLinkTargetCtl(int id, int n) const
{
	return -1;
}

bool	WED_ImageOverlayTool::PointOnStructure(int id, const Point2& p) const
{
	return mCoords.inside(p);
}

void	WED_ImageOverlayTool::ControlsMoveBy(int id, const Vector2& delta)
{
	mCoords[0] += delta;
	mCoords[1] += delta;
	mCoords[2] += delta;
	mCoords[3] += delta;
}

void	WED_ImageOverlayTool::ControlsHandlesBy(int id, int c, const Vector2& delta)
{
	Point2 me(  mCoords[0].x * 0.25 + mCoords[1].x * 0.25 + mCoords[2].x * 0.25 + mCoords[3].x * 0.25,
				mCoords[0].y * 0.25 + mCoords[1].y * 0.25 + mCoords[2].y * 0.25 + mCoords[3].y * 0.25);
	if (c < 4)
		mCoords[c] += delta;
	if (c == 9)
	{
		Point2 p;
		p.x =	mCoords[0].x * kCornerBlend0[9] + mCoords[1].x * kCornerBlend1[9] + mCoords[2].x * kCornerBlend2[9] + mCoords[3].x * kCornerBlend3[9];
		p.y =	mCoords[0].y * kCornerBlend0[9] + mCoords[1].y * kCornerBlend1[9] + mCoords[2].y * kCornerBlend2[9] + mCoords[3].y * kCornerBlend3[9];
		Vector2 old_dir(me,p);
		p += delta;
		Vector2 new_dir(me,p);

		old_dir.normalize();
		new_dir.normalize();
		
		double cos_v = old_dir.dot(new_dir);
		double sin_v = sqrt(max(1.0 - cos_v * cos_v, 0.0));

		me = GetZoomer()->LLToPixel(me);

		for (int n = 0; n < 4; ++n)
		{
			mCoords[n] = GetZoomer()->LLToPixel(mCoords[n]);
			mCoords[n] -= Vector2(me);

			if (old_dir.right_turn(new_dir))
				mCoords[n] = Point2(mCoords[n].y * sin_v + mCoords[n].x * cos_v,
									mCoords[n].y * cos_v - mCoords[n].x * sin_v);
			else
				mCoords[n] = Point2(-mCoords[n].y * sin_v + mCoords[n].x * cos_v,
									 mCoords[n].y * cos_v + mCoords[n].x * sin_v);

			mCoords[n] += Vector2(me);
			mCoords[n] = GetZoomer()->PixelToLL(mCoords[n]);			
		}			
	}
	
	if (c >= 4 && c < 9)
	{
		Vector2	dir(me,Point2(mCoords[0].x * kCornerBlend0[c] + mCoords[1].x * kCornerBlend1[c] + mCoords[2].x * kCornerBlend2[c] + mCoords[3].x * kCornerBlend3[c],
							  mCoords[0].y * kCornerBlend0[c] + mCoords[1].y * kCornerBlend1[c] + mCoords[2].y * kCornerBlend2[c] + mCoords[3].y * kCornerBlend3[c]));
		Vector2	radius(dir);
		dir.normalize();

		Vector2	move(dir.projection(delta));
		
		switch(c) {
		case 4:	mCoords[0] += move; mCoords[1] += move; mCoords[2] -= move; mCoords[3] -= move; break;
		case 5:	mCoords[0] -= move; mCoords[1] += move; mCoords[2] += move; mCoords[3] -= move; break;
		case 6:	mCoords[0] -= move; mCoords[1] -= move; mCoords[2] += move; mCoords[3] += move; break;
		case 7:	mCoords[0] += move; mCoords[1] -= move; mCoords[2] -= move; mCoords[3] += move; break;
		case 8: 
		
			{
				double old_rad = sqrt(radius.squared_length());
				radius += move;
				double new_rad = sqrt(radius.squared_length());
				double rescale = new_rad / old_rad;
			
				me = GetZoomer()->LLToPixel(me);
			
				for (int n = 0; n < 4; ++n)
				{
					mCoords[n] = GetZoomer()->LLToPixel(mCoords[n]);
					mCoords[n] -= Vector2(me);

					mCoords[n].x *= rescale;
					mCoords[n].y *= rescale;
	
					mCoords[n] += Vector2(me);
					mCoords[n] = GetZoomer()->PixelToLL(mCoords[n]);
				}
			}
		}		
	}
	
}

void	WED_ImageOverlayTool::ControlsLinksBy	 (int id, int c, const Vector2& delta)
{
	mCoords[c] += delta;
	mCoords[(c+1)%4] += delta;
}

#pragma mark -

static const char * strs[8] = { 
	"SE Lon",
	"SE Lat",
	"NE Lon",
	"NE Lat",
	"NW Lon",
	"NW Lat",
	"SW Lon",
	"SW Lat" };

int			WED_ImageOverlayTool::FindProperty(const char * in_prop)
{
	for (int n = 0; n < 8; ++n)
		if (strcmp(strs[n],in_prop)==0) return n;
	return -1;
}

int			WED_ImageOverlayTool::CountProperties(void)
{
	return 8;
}

void		WED_ImageOverlayTool::GetNthPropertyInfo(int n, PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_Double;
	info.prop_name = strs[n];
	info.digits = 10;
	info.decimals = 6;
}

void		WED_ImageOverlayTool::GetNthPropertyDict(int n, PropertyDict_t& dict)
{
}

void		WED_ImageOverlayTool::GetNthPropertyDictItem(int n, int e, string& item)
{
}
	
void		WED_ImageOverlayTool::GetNthProperty(int n, PropertyVal_t& val)
{
	val.prop_kind = prop_Double;
	val.double_val = ((n % 2) ? mCoords[n / 2].y : mCoords[n / 2].x);	
}

void		WED_ImageOverlayTool::SetNthProperty(int n, const PropertyVal_t& val)
{
	((n % 2) ? mCoords[n / 2].y : mCoords[n / 2].x) = val.double_val;
}

