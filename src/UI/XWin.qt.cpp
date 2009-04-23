#include "XWin.h"

static bool				sIniting = false;
static std::map<QMainWindow*, XWin*>	sWindows;

XWin::XWin(
	int		default_dnd,
	const char*	inTitle,
	int		inAttributes,
	int		inX,
	int		inY,
	int		inWidth,
	int		inHeight,
	QWidget *parent) : QMainWindow(parent)
{
	sIniting = true;
	visible = false;
	active = false;
	sWindows[this] = this;
	memset(mDragging,0,sizeof(int)*BUTTON_DIM);
	mMouse.x = 0;
	mMouse.y = 0;
	SetTitle(inTitle);

	int x = (inAttributes & xwin_style_fullscreen) ? 0 : inX;
	int y = (inAttributes & xwin_style_fullscreen) ? 0 : inY;
	int w = (inAttributes & xwin_style_fullscreen) ? 1024 : inWidth;
	int h = (inAttributes & xwin_style_fullscreen) ? 768 : inHeight;
	MoveTo(x, y);
	Resize(w, h);

	sIniting = false;
}

XWin::XWin(int default_dnd, QWidget *parent) : QMainWindow(parent)
{
	sIniting = true;
	visible = false;
	active = false;
	sWindows[this] = this;
	memset(mDragging,0,sizeof(int)*BUTTON_DIM);
	mMouse.x = 0;
	mMouse.y = 0;

	sIniting = false;
}

XWin::~XWin()
{
	close();
	sWindows.erase(this);
}

void XWin::resizeEvent(QResizeEvent* e)
{
	Resized(e->size().width(), e->size().height());
}

void                    XWin::Resized(int inWidth, int inHeight)
{}

void                    XWin::SetTitle(const char * inTitle)
{
}

void                    XWin::MoveTo(int inX, int inY)
{
}

void                    XWin::Resize(int inWidth, int inHeight)
{
}

void                    XWin::ForceRefresh(void)
{
	Update(0);
}

void                    XWin::Update(XContext ctx)
{
}

void                    XWin::UpdateNow(void)
{
	ForceRefresh();
}

void                    XWin::SetVisible(bool visible)
{
	setVisible(visible);
}

bool XWin::GetVisible(void) const
{
	return isVisible();
}

bool XWin::GetActive(void) const
{
	return isActiveWindow();
}


void XWin::SetTimerInterval(double seconds)
{
    return;
}

void XWin::GetBounds(int * outX, int * outY)
{
	unsigned int width_return = 800;
	unsigned int height_return = 600;

	if (outX) *outX = width_return;
	if (outY) *outY = height_return;
}

void XWin::GetMouseLoc(int * outX, int * outY)
{
    if (outX) *outX = mMouse.x;
	if (outY) *outY = mMouse.y;
    return;
}

void	XWin::ReceiveFilesFromDrag(const vector<string>& inFiles)
{
	ReceiveFiles(inFiles, 0, 0);
	return;
}

xmenu XWin::CreateMenu(xmenu parent, int item, const char * inTitle)
{
    return 0;
}

int XWin::AppendMenuItem(xmenu menu, const char * inTitle)
{}

int XWin::AppendSeparator(xmenu menu)
{}

void XWin::CheckMenuItem(xmenu menu, int item, bool inCheck)
{}
void XWin::EnableMenuItem(xmenu menu, int item, bool inEnable)
{}

void XWin::DrawMenuBar(void)
{}

int XWin::TrackPopupCommands(xmenu in_menu, int mouse_x, int mouse_y, int current)
{
    return -1;
}
