#include "WED_LayerTable.h"
#include "WED_AbstractLayers.h"
#include "WED_Messages.h"

#include "GUI_GraphState.h"
#include "GUI_Fonts.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

enum {
	col_Visible = 0,
	col_Export,
	col_Name,
	col_Total
};

const int	kDefWidths[col_Total] = { 10, 10, 100 };
const int	kRowHeight = 20;


WED_LayerTable::WED_LayerTable() : mLayers(NULL)
{
}


WED_LayerTable::~WED_LayerTable()
{
}

void		WED_LayerTable::SetLayers(WED_AbstractLayers * inLayers)
{
	mLayers = inLayers;
	mLayers->AddListener(this);
}

void		WED_LayerTable::CellDraw	 (int cell_bounds[4], int cell_x, int cell_y, GUI_GraphState * inState			  )
{
	inState->SetState(false,0,false,  false,false,  false,false);
	glColor3f(0,0,0);
	glBegin(GL_LINE_LOOP);
	glVertex2i(cell_bounds[0], cell_bounds[1]);
	glVertex2i(cell_bounds[0], cell_bounds[3]);
	glVertex2i(cell_bounds[2], cell_bounds[3]);
	glVertex2i(cell_bounds[2], cell_bounds[1]);
	glEnd();

	if (mLayers == NULL) return;
	
	int	caps = mLayers->GetLayerCapabilities(cell_y);
	int	flag = mLayers->GetFlags(cell_y);
	int	ind = 0;
	
	string	txt;
	
	switch(cell_x) {
	case col_Visible:
		if (caps & wed_Layer_Hide)
			txt = (flag & wed_Flag_Visible) ? 'V' : 'v';
		break;			
	case col_Export:
		if (caps & wed_Layer_Export)
			txt = (flag & wed_Flag_Export) ? 'E' : 'e';
		break;			
	case col_Name:
		ind = mLayers->GetIndent(cell_y);
		if (caps & wed_Layer_Children)
			txt = (flag & wed_Flag_Children) ? '-' : '+';
		txt += mLayers->GetName(cell_y);
		break;
	}
	
	GLfloat black[4] = { 0,0,0,1};
	GUI_FontDraw(inState, font_UI_Basic, black, cell_bounds[0] + 3 + ind * 10, cell_bounds[1] + 4, txt.c_str());

	
}	
int			WED_LayerTable::CellMouseDown(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
	return 0;
}

void		WED_LayerTable::CellMouseDrag(int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
}

void		WED_LayerTable::CellMouseUp  (int cell_bounds[4], int cell_x, int cell_y, int mouse_x, int mouse_y, int button)
{
}

void	WED_LayerTable::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam)
{
	switch(inMsg) {
	case msg_LayerStatusChanged:		BroadcastMessage(GUI_TABLE_CONTENT_CHANGED,0);	break;
	case msg_LayerCountChanged:			BroadcastMessage(GUI_TABLE_CONTENT_RESIZED,0);	break;
	}
}


WED_LayerTableGeometry::WED_LayerTableGeometry() : mLayers(NULL)
{
	int t = 0;
	for (int n = 0; n < col_Total; ++n)
	{
		mWidths.push_back(kDefWidths[n]+t);
		t += kDefWidths[n];
	}
}

WED_LayerTableGeometry::~WED_LayerTableGeometry()
{
}

void		WED_LayerTableGeometry::SetLayers(WED_AbstractLayers * inLayers)
{
	mLayers = inLayers;
}

int			WED_LayerTableGeometry::GetColCount(void)
{
	return col_Total;
}

int			WED_LayerTableGeometry::GetRowCount(void)
{
	return mLayers ? mLayers-> CountLayers() : 0;
}

int			WED_LayerTableGeometry::GetCellLeft (int n)
{
	return n == 0 ? 0 : mWidths[n-1];
}

int			WED_LayerTableGeometry::GetCellRight(int n)
{
	return mWidths[n];
}

int			WED_LayerTableGeometry::GetCellWidth(int n)
{
	return n == 0 ? mWidths[n] :  mWidths[n] - mWidths[n-1];
}

int			WED_LayerTableGeometry::GetCellBottom(int n)
{
	return n * kRowHeight;
}

int			WED_LayerTableGeometry::GetCellTop	 (int n)
{
	return (n+1) * kRowHeight;
}

int			WED_LayerTableGeometry::GetCellHeight(int n)
{
	return kRowHeight;
}

int			WED_LayerTableGeometry::ColForX(int n)
{
	if (n < 0) return -1;
	if (n >= mWidths.back()) return mWidths.size();
	int p = lower_bound(mWidths.begin(),mWidths.end(),n) - mWidths.begin();
	if (mWidths[p] == n) 	return p+1;
	return p;
}

int			WED_LayerTableGeometry::RowForY(int n)
{
	return n / kRowHeight;
}

void		WED_LayerTableGeometry::SetCellWidth (int n, int w)
{
}

void		WED_LayerTableGeometry::SetCellHeight(int n, int h)
{
}

