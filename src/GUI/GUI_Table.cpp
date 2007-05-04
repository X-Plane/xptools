#include "GUI_Table.h"
#include "GUI_Messages.h"

// TODO -- visibility culling?

#if APL
	#include <OpenGL/gl.h>
#else	
	#inclde <gl/gl.h>
#endif

static bool ClipTo(int pane[4], int cell[4])
{
	int clip[4] = { max(pane[0], cell[0]),
					max(pane[1], cell[1]),
					min(pane[2], cell[2]),
					min(pane[3], cell[3]) };

	int w = clip[2] - clip[0];
	int h = clip[3] - clip[1];
	if (w <= 0 || h <= 0) return false;
	
	glScissor(clip[0], clip[1], w, h);					
}

/************************************************************************************************************
 * MAIN TABLE
 ************************************************************************************************************/

GUI_Table::GUI_Table() :
	mGeometry(NULL),
	mContent(NULL),
	mScrollH(0),
	mScrollV(0)
{	
}

GUI_Table::~GUI_Table()
{
}

void		GUI_Table::SetGeometry(GUI_TableGeometry * inGeometry)
{
	mGeometry = inGeometry;
}
		
void		GUI_Table::SetContent(GUI_TableContent * inContent)
{
	mContent = inContent;
}


void		GUI_Table::Draw(GUI_GraphState * state)
{
	if (mGeometry == NULL) return;
	if (mContent == NULL) return;

	glPushAttrib(GL_SCISSOR_BIT);
	glEnable(GL_SCISSOR_TEST);
	
	int me[4];
	GetVisibleBounds(me);
	
	int cells[4], cellbounds[4];
	if (CalcVisibleCells(cells))
	for (int y = cells[1]; y < cells[3]; ++y)
	for (int x = cells[0]; x < cells[2]; ++x)
	{
		if (CalcCellBounds(x,y,cellbounds))
		if (ClipTo(me, cellbounds))
			mContent->CellDraw(cellbounds,x,y,state);
	}
	glPopAttrib();
}

int			GUI_Table::MouseDown(int x, int y, int button)
{
	if (mGeometry == NULL) return 0;
	if (mContent == NULL) return 0;
	mClickCellX = MouseToCellX(x);
	mClickCellY = MouseToCellY(y);
	if (mClickCellX >= 0 &&
		mClickCellX < mGeometry->GetColCount() &&
		mClickCellY >= 0 &&
		mClickCellY < mGeometry->GetRowCount())
	{
		int cellbounds[4];
		if (CalcCellBounds(mClickCellX, mClickCellY, cellbounds))
		if (mContent->CellMouseDown(cellbounds, mClickCellX, mClickCellY, x, y, button))
			return 1;
	}
	return 0;
}

void		GUI_Table::MouseDrag(int x, int y, int button)
{
	if (mGeometry == NULL) return;
	if (mContent == NULL) return;
	int cellbounds[4];
	if (CalcCellBounds(mClickCellX, mClickCellY, cellbounds))
		mContent->CellMouseDrag(cellbounds, mClickCellX, mClickCellY, x, y, button);	
}

void		GUI_Table::MouseUp  (int x, int y, int button)
{
	if (mGeometry == NULL) return;
	if (mContent == NULL) return;
	int cellbounds[4];
	if (CalcCellBounds(mClickCellX, mClickCellY, cellbounds))
		mContent->CellMouseUp(cellbounds, mClickCellX, mClickCellY, x, y, button);	
}

void		GUI_Table::SetBounds(int x1, int y1, int x2, int y2)
{
	int b[4];
	GetBounds(b);
	int new_height = y2 - y1;
	int old_height = b[3] - b[1];
	int delta_y = new_height - old_height;
	mScrollV -= delta_y;
	GUI_Pane::SetBounds(x1,y1,x2,y2);
	AlignContents();
}

void		GUI_Table::SetBounds(int inBounds[4])
{
	int b[4];
	GetBounds(b);
	int new_height = inBounds[3] - inBounds[1];
	int old_height = b[3] - b[1];
	int delta_y = new_height - old_height;
	mScrollV -= delta_y;
	GUI_Pane::SetBounds(inBounds);
	AlignContents();
}

void		GUI_Table::AlignContents(void)
{
	float	vis[4],total[4];

	GetScrollBounds(total,vis);	
	if (total[1] > vis[1])		mScrollV -= (vis[1] - total[1]);

	GetScrollBounds(total,vis);	
	if (total[3] < vis[3])		mScrollV -= (vis[3] - total[3]);

	GetScrollBounds(total,vis);	
	if (total[2] < vis[2])		mScrollH -= (vis[2] - total[2]);

	GetScrollBounds(total,vis);	
	if (total[0] > vis[0])		mScrollH -= (vis[0] - total[0]);
}


void		GUI_Table::ReceiveMessage(
				GUI_Broadcaster *		inSrc,
				int						inMsg,
				int						inParam)
{
	switch(inMsg) {
	case GUI_TABLE_SHAPE_RESIZED:
	case GUI_TABLE_CONTENT_RESIZED:
	// would be nice to do this:
//	int b[4];
//	GetBounds(b);
//	int new_height = inBounds[3] - inBounds[1];
//	int old_height = b[3] - b[1];
//	int delta_y = new_height - old_height;
//	mScrollV -= delta_y;
		AlignContents();
		BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED, 0);
		Refresh();
		break;
	case GUI_TABLE_CONTENT_CHANGED:
		Refresh();
		break;
	}
}

void	GUI_Table::GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4])
{
	int	b[4];
	GetBounds(b);
	
	outVisibleBounds[0] = b[0];
	outVisibleBounds[1] = b[1];
	outVisibleBounds[2] = b[2];
	outVisibleBounds[3] = b[3];
	
	if (mGeometry == NULL)
	{
		outTotalBounds[0] = b[0];
		outTotalBounds[1] = b[1];
		outTotalBounds[2] = b[2];
		outTotalBounds[3] = b[3];
		return;
	}
	
	outTotalBounds[0] = b[0] - mScrollH;
	outTotalBounds[1] = b[1] - mScrollV;
	outTotalBounds[2] = b[0] - mScrollH + mGeometry->GetCellRight(mGeometry->GetColCount()-1);
	outTotalBounds[3] = b[1] - mScrollV + mGeometry->GetCellTop(mGeometry->GetRowCount()-1);
}

void	GUI_Table::ScrollH(float xOffset)
{
	mScrollH = xOffset;
}

void	GUI_Table::ScrollV(float yOffset)
{
	mScrollV = yOffset;
}

int		GUI_Table::MouseToCellX(int x)
{
	if (mGeometry == NULL) return -1;
	int	b[4];
	GetBounds(b);
	return mGeometry->ColForX(x - b[0] + mScrollH);
}


int		GUI_Table::MouseToCellY(int y)
{
	if (mGeometry == NULL) return -1;
	int	b[4];
	GetBounds(b);
	return mGeometry->RowForY(y - b[1] + mScrollV);
}


int		GUI_Table::CalcVisibleCells(int bounds[4])
{
	if (mGeometry == NULL) return 0;
		int	b[4];
		int l[4];
	GetBounds(b);
	
	int xc = mGeometry->GetColCount();
	int yc = mGeometry->GetRowCount();
	if (xc == 0 || yc == 0) return 0;
	
	l[0] = mScrollH;
	l[1] = mScrollV;
	l[2] = mScrollH + b[2] - b[0];
	l[3] = mScrollV + b[3] - b[1];
	
	bounds[0] = mGeometry->ColForX(l[0]);
	bounds[1] = mGeometry->RowForY(l[1]);
	bounds[2] = mGeometry->ColForX(l[2]-1)+1;
	bounds[3] = mGeometry->RowForY(l[3]-1)+1;
	
	if (bounds[0] < 0) 								bounds[0] = 0;
	if (bounds[1] < 0) 								bounds[1] = 0;
	if (bounds[2] < 0) 								bounds[2] = 0;
	if (bounds[3] < 0) 								bounds[3] = 0;
	if (bounds[0] >= xc)							bounds[0] = xc-1;
	if (bounds[1] >= yc)							bounds[1] = yc-1;
	if (bounds[2] > xc)								bounds[2] = xc;
	if (bounds[3] > yc)								bounds[3] = yc;

	if (bounds[0] >= bounds[2] ||
		bounds[1] >= bounds[3])				return 0;

	return 1;	
}

int		GUI_Table::CalcCellBounds(int x, int y, int bounds[4])
{
	if (mGeometry == NULL) return 0;
		int	b[4];
	GetBounds(b);
	
	bounds[0] = mGeometry->GetCellLeft  (x) + b[0] - mScrollH;
	bounds[1] = mGeometry->GetCellBottom(y) + b[1] - mScrollV;
	bounds[2] = mGeometry->GetCellRight (x) + b[0] - mScrollH;
	bounds[3] = mGeometry->GetCellTop   (y) + b[1] - mScrollV;

	return 1;
}

void		GUI_Table::SizeShowAll(void)
{
	if (!mGeometry) return;
	int b[4];
	GetBounds(b);
	b[2] = b[0] + mGeometry->GetCellLeft(mGeometry->GetColCount()) - mGeometry->GetCellLeft(0);
	b[3] = b[1] + mGeometry->GetCellBottom(mGeometry->GetRowCount()) - mGeometry->GetCellBottom(0);
	SetBounds(b);
}


/************************************************************************************************************
 * HEADER 
 ************************************************************************************************************/
#pragma mark -

 GUI_Header::GUI_Header() :
	mGeometry(NULL),
	mHeader(NULL),
	mTable(NULL)
{	
}

GUI_Header::~GUI_Header()
{
}

void		GUI_Header::SetGeometry(GUI_TableGeometry * inGeometry)
{
	mGeometry = inGeometry;
}
		
void		GUI_Header::SetHeader(GUI_TableHeader * inHeader)
{
	mHeader = inHeader;
}

void		GUI_Header::SetTable(GUI_Table * inTable)
{
	mTable = inTable;
}

void		GUI_Header::Draw(GUI_GraphState * state)
{
	if (mGeometry == NULL) return;
	if (mHeader == NULL) return;

	int me[4];
	GetVisibleBounds(me);
	
	glPushAttrib(GL_SCISSOR_BIT);
	glEnable(GL_SCISSOR_TEST);
	
	int cells[2], cellbounds[4];
	if (CalcVisibleCells(cells))
	for (int x = cells[0]; x < cells[1]; ++x)
	{
		if (CalcCellBounds(x,cellbounds))
		if (ClipTo(me, cellbounds))			
			mHeader->HeadDraw(cellbounds,x,state);
	}
	
	glPopAttrib();	
}

int			GUI_Header::MouseDown(int x, int y, int button)
{
	if (mGeometry == NULL) return 0;
	if (mHeader == NULL) return 0;
	mClickCellX = MouseToCellX(x);
	if (mClickCellX >= 0 &&
		mClickCellX < mGeometry->GetColCount())
	{
		int cellbounds[4];
		if (CalcCellBounds(mClickCellX, cellbounds))
		if (mHeader->HeadMouseDown(cellbounds, mClickCellX, x, y, button))
			return 1;
	}
	return 0;
}

void		GUI_Header::MouseDrag(int x, int y, int button)
{
	if (mGeometry == NULL) return;
	if (mHeader == NULL) return;
	int cellbounds[4];
	if (CalcCellBounds(mClickCellX, cellbounds))
		mHeader->HeadMouseDrag(cellbounds, mClickCellX, x, y, button);	
}

void		GUI_Header::MouseUp  (int x, int y, int button)
{
	if (mGeometry == NULL) return;
	if (mHeader == NULL) return;
	int cellbounds[4];
	if (CalcCellBounds(mClickCellX, cellbounds))
		mHeader->HeadMouseUp(cellbounds, mClickCellX, x, y, button);	
}

void		GUI_Header::ReceiveMessage(
				GUI_Broadcaster *		inSrc,
				int						inMsg,
				int						inParam)
{
	switch(inMsg) {
	case GUI_TABLE_SHAPE_RESIZED:
	case GUI_TABLE_CONTENT_RESIZED:
		Refresh();
		break;
	}

}
int		GUI_Header::MouseToCellX(int x)
{
	if (mGeometry == NULL) return -1;
	if (mTable == NULL) return -1;
	int	b[4];
	GetBounds(b);
	return mGeometry->ColForX(x - b[0] + mTable->GetScrollH());
}


int		GUI_Header::CalcVisibleCells(int bounds[2])
{
	if (mGeometry == NULL) return 0;
	if (mTable == NULL) return 0;

	int xc = mGeometry->GetColCount();
	if (xc == 0) return 0;
		
		int	b[4];
		int l[2];
	GetBounds(b);
	l[0] = mTable->GetScrollH();
	l[1] = mTable->GetScrollH() + b[2] - b[0];
	
	bounds[0] = mGeometry->ColForX(l[0]);
	bounds[1] = mGeometry->ColForX(l[1]-1)+1;
	
	if (bounds[0] < 0) 								bounds[0] = 0;
	if (bounds[1] < 0) 								bounds[1] = 0;
	if (bounds[0] >= xc)							bounds[0] = xc-1;
	if (bounds[1] > xc)								bounds[1] = xc;

	if (bounds[0] >= bounds[1])				return 0;
	return 1;	
}

int		GUI_Header::CalcCellBounds(int x, int bounds[4])
{
	if (mGeometry == NULL) return 0;
		int	b[4];
	GetBounds(b);
	
	bounds[0] = mGeometry->GetCellLeft  (x) + b[0] - mTable->GetScrollH();
	bounds[1] = b[1];
	bounds[2] = mGeometry->GetCellRight (x) + b[0] - mTable->GetScrollH();
	bounds[3] = b[3];

	return 1;
}

 
/************************************************************************************************************
* SIDE
************************************************************************************************************/
#pragma mark -

 GUI_Side::GUI_Side() :
	mGeometry(NULL),
	mSide(NULL),
	mTable(NULL)
{	
}

GUI_Side::~GUI_Side()
{
}

void		GUI_Side::SetGeometry(GUI_TableGeometry * inGeometry)
{
	mGeometry = inGeometry;
}
		
void		GUI_Side::SetSide(GUI_TableSide * inSide)
{
	mSide = inSide;
}

void		GUI_Side::SetTable(GUI_Table * inTable)
{
	mTable = inTable;
}

void		GUI_Side::Draw(GUI_GraphState * state)
{
	if (mGeometry == NULL) return;
	if (mSide == NULL) return;

	int me[4];
	GetVisibleBounds(me);

	glPushAttrib(GL_SCISSOR_BIT);
	glEnable(GL_SCISSOR_TEST);
	
	int cells[2], cellbounds[4];
	if (CalcVisibleCells(cells))
	for (int y = cells[0]; y < cells[1]; ++y)
	{
		if (CalcCellBounds(y,cellbounds))
		if (ClipTo(me, cellbounds))			
			mSide->SideDraw(cellbounds,y,state);
	}
	glPopAttrib();
}

int			GUI_Side::MouseDown(int x, int y, int button)
{
	if (mGeometry == NULL) return 0;
	if (mSide == NULL) return 0;
	mClickCellY = MouseToCellY(y);
	if (mClickCellY >= 0 &&
		mClickCellY < mGeometry->GetRowCount())
	{
		int cellbounds[4];
		if (CalcCellBounds(mClickCellY, cellbounds))
		if (mSide->SideMouseDown(cellbounds, mClickCellY, x, y, button))
			return 1;
	}
	return 0;
}

void		GUI_Side::MouseDrag(int x, int y, int button)
{
	if (mGeometry == NULL) return;
	if (mSide == NULL) return;
	int cellbounds[4];
	if (CalcCellBounds(mClickCellY, cellbounds))
		mSide->SideMouseDrag(cellbounds, mClickCellY, x, y, button);	
}

void		GUI_Side::MouseUp  (int x, int y, int button)
{
	if (mGeometry == NULL) return;
	if (mSide == NULL) return;
	int cellbounds[4];
	if (CalcCellBounds(mClickCellY, cellbounds))
		mSide->SideMouseUp(cellbounds, mClickCellY, x, y, button);	
}

void		GUI_Side::ReceiveMessage(
				GUI_Broadcaster *		inSrc,
				int						inMsg,
				int						inParam)
{
	switch(inMsg) {
	case GUI_TABLE_SHAPE_RESIZED:
	case GUI_TABLE_CONTENT_RESIZED:
		Refresh();
		break;
	}

}
int		GUI_Side::MouseToCellY(int y)
{
	if (mGeometry == NULL) return -1;
	if (mTable == NULL) return -1;
	int	b[4];
	GetBounds(b);
	return mGeometry->RowForY(y - b[1] + mTable->GetScrollV());
}


int		GUI_Side::CalcVisibleCells(int bounds[2])
{
	if (mGeometry == NULL) return 0;
	if (mTable == NULL) return 0;

	int yc = mGeometry->GetRowCount();
	if (yc == 0) return 0;
	
		int	b[4];
		int t[2];
	GetBounds(b);
	t[0] = mTable->GetScrollV();
	t[1] = mTable->GetScrollV() + b[3] - b[1];
	
	bounds[0] = mGeometry->RowForY(t[0]);
	bounds[1] = mGeometry->RowForY(t[1]-1)+1;
	
	
	if (bounds[0] < 0) 								bounds[0] = 0;
	if (bounds[1] < 0) 								bounds[1] = 0;
	if (bounds[0] >= yc)							bounds[0] = yc-1;
	if (bounds[1] > yc)								bounds[1] = yc;

	if (bounds[0] >= bounds[1])				return 0;

	return 1;	
}

int		GUI_Side::CalcCellBounds(int y, int bounds[4])
{
	if (mGeometry == NULL) return 0;
		int	b[4];
	GetBounds(b);
	
	bounds[0] = b[0];
	bounds[1] = mGeometry->GetCellBottom(y) + b[1] - mTable->GetScrollV();
	bounds[2] = b[2];
	bounds[3] = mGeometry->GetCellTop   (y) + b[1] - mTable->GetScrollV();

	return 1;
}

