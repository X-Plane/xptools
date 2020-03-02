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
	mDragging    =-1;
	mWantFakeUp  = 0;
	mBlockEvents = 0;
	mMouse.x     = 0;
	mMouse.y     = 0;
	SetTitle(inTitle);

//	printf("New WinGeo %d %d\n",inX, inY);

	if(inAttributes & xwin_style_centered)
		this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));
	else
		MoveTo(inX, inY);
	
	Resize(inWidth, inHeight);
	
	if(inAttributes & xwin_style_modal)
	{
		this->setWindowFlags(windowFlags()| Qt::Dialog);
		this->setWindowModality(Qt::ApplicationModal);
	}

	if( !(inAttributes & xwin_style_resizable) )
	{
		this->setFixedSize(this->size());
	}
	// TODO:mroe: 
	// Some KDE-Style's want to use all 'empty' areas of a window to move it .
	// They try to detect them when the mouse-events comes through to the Mainwindow, and grab the mouse.  
	// It's clearly a mess , doing such stuff from outside.
	// Since XWin is our mainwindow and handles all events , that is not that good for us.
	// The preferred way would be :  the gl-window should have his own event-funcs. 
	// But what comes next?
	// Anyway, since many have this problem, 'Oxygen' has a property introduced what is now also accepted by qt-curve. 
	// Perhaps it becomes a standard.
	setProperty( "_kde_no_window_grab", true );
	//mroe: WA_DeleteOnClose is not set by default , we need this to get all our destr called
	//and the entire thing really removed from memory
	setAttribute(Qt::WA_DeleteOnClose, true);

	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	if (default_dnd)
		setAcceptDrops(true);
	mTimer=0;
	mInited = true;
}

XWin::XWin(int default_dnd, QWidget *parent) : QMainWindow(parent), mInited(false)
{
	mDragging    =-1;
	mWantFakeUp  = 0;
	mBlockEvents = 0;
	mMouse.x     = 0;
	mMouse.y     = 0;

	setProperty( "_kde_no_window_grab", true );

	setAttribute(Qt::WA_DeleteOnClose, true);

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	if (default_dnd)
		setAcceptDrops(true);
	mTimer=0;
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

	if(mBlockEvents) return;

	if(mDragging == -1)
	{
		mDragging = btn;
		ClickDown(mMouse.x, mMouse.y, btn);
		
		if(mWantFakeUp)
		{
			int btn = mDragging;
			mDragging = -1;
			ClickUp(mMouse.x, mMouse.y, btn);
			mWantFakeUp = 0;
		}
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

	if(mBlockEvents) return;

	if(mDragging == btn)
	{
		mDragging = -1;
		ClickUp(mMouse.x, mMouse.y, btn);
	}
}

void XWin::mouseMoveEvent(QMouseEvent* e)
{
	mMouse.x = e->x();
	mMouse.y = e->y();

	//mroe: We need the above calls , 
	// also to get the event proceeded for the dragdetect in GUI_Windows::IsDrag.
	// Seems the event is droped if the function does nothing.
	// Having thecurrent mouse-position ever is not that bad at all.
	if(mBlockEvents) return;

	if((mDragging >= 0 ) && (mDragging < BUTTON_DIM))
	{
		ClickDrag(mMouse.x, mMouse.y,mDragging);
		
		if(mWantFakeUp)
		{
			int btn = mDragging;
			mDragging = -1;
			ClickUp(mMouse.x, mMouse.y, btn);
			mWantFakeUp = 0;
		}
	}
	else
	{
		ClickMove(mMouse.x, mMouse.y);
	}
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

void XWin::timerEvent(QTimerEvent* e)
{
	Timer();
}

void XWin::focusInEvent(QFocusEvent* e)
{
	if(e->reason()==Qt::ActiveWindowFocusReason)
		Activate(1);
}

void XWin::focusOutEvent(QFocusEvent* e)
{
	if(e->reason()==Qt::ActiveWindowFocusReason)
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
	setWindowTitle(QString::fromUtf8(inTitle));
}

void XWin::SetFilePath(const char * inPath,bool modified)
{
}

void XWin::MoveTo(int inX, int inY)
{
	QPoint myPos(inX, inY);
	
	QDesktopWidget * dt = QApplication::desktop();
	QRect myScreen = dt->availableGeometry(dt->screenNumber(this));
	//printf("MoveTo %d %d. Im on screen %d which is at %d %d\n",inX, inY, dt->screenNumber(this), myScreen.left(), myScreen.top());

// About the WindowManager vs QT idiocracy under X11
// Upon initial placement, the WM hasn't decided yet on which screen to place the new widget.
// So if you move the winow in Qt to some coordinate, that is meant as a coordinate relative to the *default* screen.
// But is that coordinate is *outside* the area of the default screen, the WM will notice that and move the window 
// automatically to the screen where the (relative) coordinates point to. Qt then reads back the coordinates after
// placement to stay in sync with what the WM did. But stupid Qt4 mis-understand those coordinates as still relative to 
// the default screen - which isn't true. So we counteract this here. In QT5 this should not be neccesary any more.

	if(myScreen.contains(inX,inY))
	{
		//printf("Its is within myScreen - substract my screen offset\n");
		myPos -= myScreen.topLeft();                       // account for actual monitor we're on
		setGeometry(myPos.x(), myPos.y() - GetMenuBarHeight(), width(), height());
	}
	else
	{
		myScreen = dt->availableGeometry(-1);
		//printf("Its outside myScreen - rather substract default screen offset %d %d\n", myScreen.left(), myScreen.top());
		myPos -= myScreen.topLeft();                       // account for actual monitor we're on
		setGeometry(myPos.x(), myPos.y() - GetMenuBarHeight(), width(), height());
	}
}

void XWin::Resize(int inWidth, int inHeight)
{
	int w = inWidth;
	int h = inHeight + GetMenuBarHeight();
	//mroe:   to resize a non-resizable window
	if( (this->minimumSize() == this->size()) && (this->maximumSize() == this->size()) )
		this->setFixedSize(w, h);
	else
		this->resize(w, h);
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
	if(visible)
	{
		raise();
		activateWindow();
	}
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
	 if (seconds)
	 {
	 	if (mTimer)
		{
			killTimer(mTimer);
			mTimer = startTimer(seconds);
		}
		else
		  mTimer = startTimer(seconds);
	 }
	 else
	 {
		killTimer(mTimer);
		mTimer=0;
	 }
}

void XWin::GetBounds(int * outX, int * outY)
{
	if (outX) *outX = this->size().width();
	if (outY) *outY = this->size().height() - GetMenuBarHeight();
}

void XWin::GetWindowLoc(int * outX, int * outY)
{
	QDesktopWidget * dt = QApplication::desktop();
	QRect myScreen = dt->availableGeometry(dt->screenNumber(this));

	QPoint absPos = geometry().topLeft() + QPoint(0,GetMenuBarHeight()) + myScreen.topLeft(); // does not include window decorations - deliberately !
		
	// printf("GetLoc rel pos is x=%d y=%d, myScreen is %d, absolute pos %d %d\n", geometry().x(), geometry().y()+GetMenuBarHeight(), dt->screenNumber(this), absPos.x(), absPos.y());
		
	if (outX) *outX = absPos.x();
	if (outY) *outY = absPos.y();
}

void XWin::GetDesktop(int bounds[4])
{
	QDesktopWidget * dt = QApplication::desktop();
	int num_screens = dt->screenCount();
	
	bounds[0] = bounds[1] = 32000;
	bounds[2] = bounds[3] = 0;
	for (int s = 0; s < num_screens; ++s)
	{
		QRect screen = dt->availableGeometry(s);
//		printf("Screen %d: l=%d t=%d w=%d h=%d\n", s, screen.left(), screen.top(), screen.width(), screen.height());
		bounds[0] = min(bounds[0], screen.left());
		bounds[1] = min(bounds[1], screen.top());
		bounds[2] = max(bounds[2], screen.right());
		bounds[3] = max(bounds[1], screen.bottom());
	}
	//printf("Primary screen is %d, I'm on screen %d\n", dt->primaryScreen(), dt->screenNumber(this));
	//printf("Desktop l=%d t=%d r=%d b=%d\n", bounds[0], bounds[1], bounds[2], bounds[3]);
	//printf("Desktop union size w=%d h=%d\n", dt-> dt->width(), dt->height());
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

void XWin::onMenuAction(QAction* a)
{
	QMenu * amenu = (QMenu *) a->parent();
	int cmd = amenu->actions().indexOf(a);
	HandleMenuCmd(amenu,cmd);
}

xmenu XWin::GetMenuBar(void)
{
	QMenu* amenu = (QMenu*) this->menuBar();
	return (QMenu*) this->menuBar();
}

int XWin::GetMenuBarHeight(void)
{
	int mb_height = 0;
	QWidget* mb = layout()->menuBar();
	if(mb) mb_height = mb->sizeHint().height();
	return mb_height;
}

xmenu XWin::CreateMenu(xmenu parent, int item, const char * inTitle)
{
	QMenu * newmenu = new QMenu(inTitle,parent);
	//FIXME:mroe is parent->window() always the mainwindow ?
	connect(newmenu,SIGNAL(triggered(QAction*)),parent->window(),SLOT(onMenuAction(QAction*)));
	if (item == -1)
	    parent->addMenu(newmenu);
	else
		parent->actions().at(item)->setMenu(newmenu);
	return newmenu;
}

int XWin::AppendMenuItem(xmenu menu, const char * inTitle)
{
	QAction * aact = menu->addAction(inTitle);
	return  menu->actions().count()-1;
}

int XWin::AppendSeparator(xmenu menu)
{
	menu->addSeparator();
	return  menu->actions().count()-1;
}

void XWin::CheckMenuItem(xmenu menu, int item, bool inCheck)
{
   	 QAction * aact = menu->actions().at(item);
	 aact->setCheckable(inCheck);
	 aact->setChecked(inCheck);
}

void XWin::EnableMenuItem(xmenu menu, int item, bool inEnable)
{
   	 QAction * aact = menu->actions().at(item);
	 aact->setEnabled(inEnable);
}

void XWin::DrawMenuBar(void)
{}

int XWin::TrackPopupCommands(xmenu in_menu, int mouse_x, int mouse_y, int button, int current)
{
	if(!in_menu) return -1;
	QAction * aaction = in_menu->exec(this->mapToGlobal(QPoint(mouse_x,mouse_y)));
	
	if(mDragging == button)
	{
		mWantFakeUp = 1;
	}
	
	return in_menu->actions().indexOf(aaction);
}
