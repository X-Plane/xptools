#include "XWin.h"

XWin::XWin(
	int		default_dnd,
	const char*	inTitle,
	int		inAttributes,
	int		inX,
	int		inY,
	int		inWidth,
	int		inHeight,
	QWidget *parent) : QMainWindow(parent), mInited(false)
{
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
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	if (default_dnd)
		setAcceptDrops(true);
	mInited = true;
}

XWin::XWin(int default_dnd, QWidget *parent) : QMainWindow(parent), mInited(false)
{
	memset(mDragging,0,sizeof(int)*BUTTON_DIM);
	mMouse.x = 0;
	mMouse.y = 0;
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	if (default_dnd)
		setAcceptDrops(true);
	mInited = true;
}

XWin::~XWin()
{
}

void XWin::closeEvent(QCloseEvent* e)
{
	if (Closed())
		e->accept();
	else
		e->ignore();
}

void XWin::resizeEvent(QResizeEvent* e)
{
	if (mInited) {
		Resized(e->size().width(), e->size().height());
	}
}

void XWin::mousePressEvent(QMouseEvent* e)
{
    unsigned int rbtn = e->button();
    int btn = 0;
    for (;(rbtn!=0)&&(btn<BUTTON_DIM);rbtn>>=1,btn++);
    if (btn==0 || btn > 3)  return;
    btn--;
	mMouse.x = e->x();
	mMouse.y = e->y();

    if(!mDragging[btn])
    {
        mDragging[btn]=1;
        ClickDown(mMouse.x, mMouse.y, btn);
    }
}

void XWin::mouseReleaseEvent(QMouseEvent* e)
{
    unsigned int rbtn = e->button();
    int btn = 0;
    for (;(rbtn!=0)&&(btn<BUTTON_DIM);rbtn>>=1,btn++);
    if (btn==0 || btn > 3) return;
    btn--;
	mMouse.x = e->x();
	mMouse.y = e->y();

    if(mDragging[btn])
			ClickUp(mMouse.x, mMouse.y, btn);
    mDragging[btn]=0;

}

void XWin::mouseMoveEvent(QMouseEvent* e)
{
	mMouse.x = e->x();
	mMouse.y = e->y();
	int bc=0;
	for(int b=0;b<BUTTON_DIM;++b) {
		if(mDragging[b]) {
			++bc;
			ClickDrag(mMouse.x, mMouse.y, b);
		}
	}
	if(bc==0)
		ClickMove(mMouse.x, mMouse.y);
}

void XWin::wheelEvent(QWheelEvent* e)
{
	mMouse.x = e->x();
	mMouse.y = e->y();
	MouseWheel(mMouse.x, mMouse.y, (e->delta() < 0) ? -1 : 1, 0);
}

void XWin::keyPressEvent(QKeyEvent* e)
{
	uint32_t utf32char = 0;
	if (e->text().size())
	{
		utf32char = e->text().toUcs4().at(0);
	}
	KeyPressed(utf32char, e->key(), 0, 0);
}

void XWin::keyReleaseEvent(QKeyEvent* e)
{}

void XWin::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasFormat("text/uri-list"))
		e->acceptProposedAction();
}

void XWin::dragLeaveEvent(QDragLeaveEvent* e)
{}

void XWin::dragMoveEvent(QDragMoveEvent* e)
{}

void XWin::dropEvent(QDropEvent* e)
{
	vector<string> inFiles;
	QList<QUrl> urls = e->mimeData()->urls();
	for (int i = 0; i < urls.size(); ++i) {
		if (urls.at(i).scheme() == "file")
			inFiles.push_back(urls.at(i).toLocalFile().toStdString());
	}
	ReceiveFilesFromDrag(inFiles);
}

void XWin::focusInEvent(QFocusEvent* e)
{
	Activate(1);
}

void XWin::focusOutEvent(QFocusEvent* e)
{
	Activate(0);
}

/* prevent pure virtual function calls. ben, we need to restructure this,
** it hinders us using deep inheritance schemes, typically needed by Qt
*/

void XWin::Resized(int inWidth, int inHeight)
{}

bool XWin::Closed(void)
{
	return true;
}

void XWin::Update(XContext ctx)
{}

void XWin::SetTitle(const char * inTitle)
{
	setWindowTitle(inTitle);
}

void XWin::MoveTo(int inX, int inY)
{
	move(inX, inY);
}

void XWin::Resize(int inWidth, int inHeight)
{
	resize(inWidth, inHeight);
}

void XWin::ForceRefresh(void)
{
	Update(0);
}

void XWin::UpdateNow(void)
{
	ForceRefresh();
}

void XWin::SetVisible(bool visible)
{
	setVisible(visible);
	if (visible)
		activateWindow();
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
	if (outX) *outX = size().width();
	if (outY) *outY = size().height();
}

void XWin::GetMouseLoc(int * outX, int * outY)
{
	if (outX) *outX = mMouse.x;
	if (outY) *outY = mMouse.y;
}

void XWin::ReceiveFilesFromDrag(const vector<string>& inFiles)
{
	ReceiveFiles(inFiles, 0, 0);
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
	if(!in_menu) return -1;

	QMouseEvent* e = new QMouseEvent(QEvent::MouseButtonRelease,
	QPoint(mouse_x, mouse_y), Qt::LeftButton, Qt::LeftButton,
	QApplication::keyboardModifiers());

	QCoreApplication::postEvent(this, e);

	QAction * aaction = in_menu->exec(this->mapToGlobal(QPoint(mouse_x,mouse_y)));
	return in_menu->actions().indexOf(aaction);
}
