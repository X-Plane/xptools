#include "WED_ImageOverlayTool.h"
#include "WED_Persistent.h"
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



// Handles:
// 2-1
// | |
// 3-0

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
		if (LoadTextureFromFile(buf, mTexID, tex_Rescale + tex_Linear + tex_Mipmap, NULL, NULL, NULL, NULL))
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
				GetZoomer()->GetMapVisibleBounds(mCoords[3].x, mCoords[3].y, mCoords[1].x, mCoords[1].y);
				GetZoomer()->GetMapVisibleBounds(mCoords[2].x, mCoords[0].y, mCoords[0].x, mCoords[2].y);
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
		glColor4f(1.0, 1.0, 1.0, 0.5);
		g->BindTex(mTexID, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[3].x),
					GetZoomer()->LatToYPixel(mCoords[3].y));
		glTexCoord2f(0.0, 1.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[2].x),
					GetZoomer()->LatToYPixel(mCoords[2].y));
		glTexCoord2f(1.0, 1.0);
		glVertex2f( GetZoomer()->LonToXPixel(mCoords[1].x),
					GetZoomer()->LatToYPixel(mCoords[1].y));
		glTexCoord2f(1.0, 0.0);
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
	return 4;
}

void	WED_ImageOverlayTool::GetNthControlHandle(int id, int n, int * active, HandleType_t * con_type, Point2 * p, Vector2 * direction) const
{
	if (active) *active=1;
	if (con_type) *con_type = handle_Square;
	if (direction) *direction = Vector2();
	if (p) *p = mCoords[n];
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
	mCoords[c] += delta;
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

