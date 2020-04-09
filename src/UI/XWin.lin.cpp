#include "XWin.h"
#include <FL/filename.H>
#include "STLUtils.h"

XWin::XWin(
	int		default_dnd,
	const char*	inTitle,
	int		inAttributes,
	int		inX,
	int		inY,
	int		inWidth,
	int		inHeight)
	: Fl_Window(inX,inY,inWidth,inHeight,inTitle),
      mInited(false),mBar(nullptr)
{
	mDragging    =-1;
	mWantFakeUp  = 0;
	mBlockEvents = 0;
	mMouse.x     = 0;
	mMouse.y     = 0;
	mTimer=0;

	if(inAttributes & xwin_style_centered)
	{
		this->position((Fl::w() - this->w())/2, (Fl::h() - this->h())/2 );
	}

	if(inAttributes & xwin_style_modal)	    set_modal();
	if(inAttributes & xwin_style_visible)	set_visible();
	if(inAttributes & xwin_style_resizable) resizable(this);

//	if (default_dnd)
//		setAcceptDrops(true);
 	callback( window_cb );
	mInited = true;
	printf("Xwin ctor\n");
}

XWin::XWin(int default_dnd) : Fl_Window(100,100), mInited(false),mBar(nullptr)
{
	mDragging    =-1;
	mWantFakeUp  = 0;
	mBlockEvents = 0;
	mMouse.x     = 0;
	mMouse.y     = 0;

	mTimer=0;
	resizable(this);
	callback( window_cb );
	mInited = true;
}

XWin::~XWin()
{
	printf("Xwin dtor\n");
}

/*FLTK draw callback*/
void XWin::draw()
{
	if(mInited)
	{
		Update(0);
		draw_children();
	}
}


//void XWin::timerEvent(QTimerEvent* e)
//{
//	Timer();
//}
//

/*FLTK event callback*/
int XWin::handle(int e)
{

	/*handle menubar*/
	if(mBar &&  (Fl::event_y() <= mBar->h() && e == FL_PUSH ))
		if(mBar->handle(e))
		{
			//FIXME:mroe this to get the window indicatingg active again
			//hide();show();
			return 1;
		}

	switch(e)
	{
		/*MOUSE events */
		case FL_PUSH:{
			int btn = Fl::event_button()-1;
			printf("push\n");
			mMouse.x = Fl::event_x();
			mMouse.y = Fl::event_y();

			if(mBlockEvents) return 1;

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
		return 1;
		case FL_RELEASE:{
			int btn =Fl::event_button()-1;
			printf("release\n");
			mMouse.x = Fl::event_x();
			mMouse.y = Fl::event_y();

			if(mBlockEvents) return 1;

			if(mDragging == btn)
			{
				mDragging = -1;
				ClickUp(mMouse.x, mMouse.y, btn);
			}
		}
		return 1;
		case FL_DRAG:{
 			ClickDrag(Fl::event_x(),Fl::event_y(),mDragging);
		}
		 return 1;
		case FL_MOVE:{

			if(Fl::event_x() == mMouse.x && Fl::event_y() == mMouse.y) return 1;

			mMouse.x = Fl::event_x();
			mMouse.y = Fl::event_y();

			ClickMove(mMouse.x, mMouse.y);
		}
		return 1;
		case FL_MOUSEWHEEL:{
			mMouse.x = Fl::event_x();
			mMouse.y = Fl::event_y();
      		MouseWheel(mMouse.x, mMouse.y, (Fl::event_dy() < 0) ? 1 : -1, 0);
		}
		return 1;
		case FL_KEYDOWN:{
			uint32_t utf32char = 0;
			int l = Fl::event_length();
	     	if (l > 0)
			{
				int len;
				const char * p = Fl::event_text();
				const char * e = p+l;
				utf32char = fl_utf8decode(p,e,&l);
			}
			KeyPressed(utf32char,Fl::event_key(), 0, 0);
		}
		return 1;
		case FL_KEYUP:{
		}
		return 1;
		/*WIDGET events */
		case FL_ACTIVATE:{
			printf("FL_ACTIVATE \n");
		}
		return 1;
		case FL_DEACTIVATE:{
			printf("FL_DEACTIVATE \n");
		}
		return 1;

		/*FOCUS events */
		case FL_FOCUS:{
			 Fl_Window::set_active();
			 printf("FL_FOCUS \n");
		}
		return 1;
		case FL_UNFOCUS:{
			 printf("FL_UNFOCUS \n");
		}
		return 1;

		/*DND events */
		case FL_DND_ENTER:
		case FL_DND_DRAG :
		case FL_DND_LEAVE:
		case FL_DND_RELEASE:
		return 1;
		case FL_PASTE:{
			 char c[200];
			 strncpy(c, Fl::event_text(), sizeof(c));
			 fl_decode_uri(c);
			 printf("XWin::handle FL_PASTE Win %s\n",c);
			 ReceiveFilesFromDrag(c);
		}
		return 1;
		case FL_SHORTCUT:

		return 0;

		/*OTHER Window events */
		default:
		   return Fl_Window::handle(e);
	}
}

/*FLTK resize callback*/
void XWin::resize(int x,int y,int w,int h)
{
	printf(" XWin::resize inited %d\n",mInited);
    bool is_move_only = ( w == this->w() && h == this->h() );
	Fl_Widget::resize(x,y,w,h);
	if(is_move_only || !mInited) return;
	printf(" XWin::resize others \n");
	if(mBar) mBar->size(w,mBar->h());
	Resized(w,h);
}


static void clearOwnMenusRecursive(const Fl_Menu_Item * parent)
{
	for ( int t=0; t<parent->size(); t++)
	{
		const Fl_Menu_Item * menu = parent + t;

		if(menu->label() && menu->flags&FL_SUBMENU_POINTER)
		{
			const Fl_Menu_Item * submenu = (const Fl_Menu_Item *) menu->user_data();
			clearOwnMenusRecursive(submenu);

			for (int i=0; i<submenu->size(); i++)
			{
				const Fl_Menu_Item * m = submenu + i;

				if(!m->label()) break;

				if(m->text != nullptr)
					free((void*)m->text);

				if(m->user_data_ != nullptr)
					delete (xmenu_cmd *) m->user_data_;

			}

			delete [] submenu;
			((Fl_Menu_Item *) menu)->user_data(NULL);
		}
	}
}



/**FLTK CALLBACK functs**/

/*FLTK  menu callback*/
void XWin::menu_cb(Fl_Widget *w, void * data)
{
	Fl_Menu_Bar *bar = (Fl_Menu_Bar*)w;		        // Get the menubar widget
	const Fl_Menu_Item *item = bar->mvalue();		// Get the menu item that was picked
	Fl_Menu_Item * m=(Fl_Menu_Item *) item;
	printf("cb %d %s\n",item->first(),item->first()->label());
	xmenu_cmd* cmd = (xmenu_cmd*) data;
	if(!cmd) return;
	printf("menu %d %d item %d cmd %d \n",item->value(),m->value(),cmd->menu,cmd->cmd);
	XWin* win = (XWin*) bar->parent();
	win->HandleMenuCmd(cmd->menu,cmd->cmd);
}

/*FLTK window about to close callback*/
void XWin::window_cb(Fl_Widget *widget, void *)
{
	XWin * w = (XWin *)widget;
	if (!w->Closed() ) return;

	if(w->mBar) clearOwnMenusRecursive(w->mBar->menu());

	w->hide();
}

/*FLTK timeout callback*/
void XWin::timeout_cb(void * data)
{
	if(!data) return;
	XWin * w = (XWin *) data;
	w->Timer();
    Fl::repeat_timeout(w->mTimer,timeout_cb,w);
}


/* prevent pure virtual function calls. ben, we need to restructure this,
** it hinders us using deep inheritance schemes, typically needed by Qt
*/

void XWin::Activate(int inActive)
{
	printf("XWin::Activate %d\n",inActive);
	inActive ? this->show() : this->clear_active();
}

void XWin::Resized(int inWidth, int inHeight)
{
}

bool XWin::Closed(void)
{
	return true;
}

void XWin::Update(XContext ctx)
{
}

void XWin::SetTitle(const char * inTitle)
{
	this->label(inTitle);
}

void XWin::SetFilePath(const char * inPath,bool modified)
{
}

void XWin::MoveTo(int inX, int inY)
{
    //TODO: check for multi screen settings*/
	this->position(inX,inY);
}

void XWin::Resize(int inWidth, int inHeight)
{
	this->size(inWidth,inHeight);
}

void XWin::ForceRefresh(void)
{
	flush();
}

void XWin::UpdateNow(void)
{
	redraw();
}

void XWin::SetVisible(bool visible)
{
  if(mInited)visible ? show() : hide();
}

bool XWin::GetVisible(void) const
{
	return visible();
}

bool XWin::GetActive(void) const
{
	return active();
}

void XWin::SetTimerInterval(double seconds)
{
	if(seconds)
	{
		if(mTimer)
		{
			Fl::remove_timeout(timeout_cb,this);
			Fl::add_timeout(seconds,timeout_cb,this);
		}
		else
		{
			Fl::add_timeout(seconds,timeout_cb,this);
			mTimer=seconds;
		}
	}
	else
	{
		Fl::remove_timeout(timeout_cb,this);
		mTimer=0;
	}
}

void XWin::GetBounds(int * outX, int * outY)
{
	if (outX) *outX = this->w();
	if (outY) *outY = this->h() - GetMenuBarHeight();
}

void XWin::GetWindowLoc(int * outX, int * outY)
{
	//TODO: check for multi screen settings
	if (outX) *outX = this->x();
	if (outY) *outY = this->y();
}

void XWin::GetDesktop(int bounds[4])
{
//	QDesktopWidget * dt = QApplication::desktop();
//	int num_screens = dt->screenCount();
//
//	bounds[0] = bounds[1] = 32000;
//	bounds[2] = bounds[3] = 0;
//	for (int s = 0; s < num_screens; ++s)
//	{
//		QRect screen = dt->availableGeometry(s);
////		printf("Screen %d: l=%d t=%d w=%d h=%d\n", s, screen.left(), screen.top(), screen.width(), screen.height());
//		bounds[0] = min(bounds[0], screen.left());
//		bounds[1] = min(bounds[1], screen.top());
//		bounds[2] = max(bounds[2], screen.right());
//		bounds[3] = max(bounds[1], screen.bottom());
//	}
//	//printf("Primary screen is %d, I'm on screen %d\n", dt->primaryScreen(), dt->screenNumber(this));
//	//printf("Desktop l=%d t=%d r=%d b=%d\n", bounds[0], bounds[1], bounds[2], bounds[3]);
//	//printf("Desktop union size w=%d h=%d\n", dt-> dt->width(), dt->height());

}


void XWin::GetMouseLoc(int * outX, int * outY)
{
	if (outX) *outX = Fl::event_x();
	if (outY) *outY = Fl::event_y();
}

void XWin::ReceiveFilesFromDrag(const string& inFiles)
{
	vector<string>	files;
    auto b = inFiles.begin();
	while(b != inFiles.end())
	{
		while(b != inFiles.end() && *b == '\n') ++b;
		auto m1(b);
		while(b != inFiles.end() && *b != '\n') ++b;
		auto m2(b);

		if(m1 != m2)
		{
			string path(m1,m2);
			size_t pos = path.find("file://");      //only filepaths
			if(pos != std::string::npos)
			{
				files.push_back(path.substr(7));
				printf("file s %s\n",files.back().c_str());
			}
		}
	}

	if(files.size() > 0)
			ReceiveFiles(files, 0, 0);
}

xmenu XWin::GetMenuBar(void)
{
	if(!mBar)
	{
		mBar = new Fl_Menu_Bar(0,0,w(),labelsize() + 10);
	    add(mBar);
		this->size(this->w(),this->h() + mBar->h());
    	mBar->labelfont(FL_HELVETICA);
		mBar->selection_color(FL_BLUE);
		//mroe: thats to get an empty initalized menu
		mBar->add("",0,nullptr,nullptr);
		mBar->remove(0);
	}
	return (Fl_Menu_Item *) mBar->menu();
}

int XWin::GetMenuBarHeight(void)
{
	if (mBar != nullptr) return mBar->h();
	return 0;
}

xmenu XWin::CreateMenu(xmenu parent, int item, const char * inTitle)
{
	if(!parent) return NULL;

	xmenu new_menu = new Fl_Menu_Item[20*sizeof(Fl_Menu_Item )];
	memset(new_menu,0,20*sizeof(Fl_Menu_Item ));

	int idx= parent->insert(item,inTitle,0,nullptr,(void*)new_menu,FL_SUBMENU_POINTER);
	return new_menu;
}

int XWin::AppendMenuItem(xmenu menu, const char * inTitle)
{
	if(!menu) return -1;

	xmenu_cmd * cmd = new xmenu_cmd;

	int idx = menu->add(inTitle,0,menu_cb,cmd);
	cmd->menu = menu;
	cmd->cmd  = idx;

	return idx;
}

int XWin::AppendSeparator(xmenu menu)
{
	if(!menu) return -1;

	Fl_Menu_Item * last = menu + (menu->size()-2);
    last->flags = last->flags|FL_MENU_DIVIDER;
	char buf[256];
	strcpy(buf,last->label());
	strcat(buf,"_");
	return  menu->add(buf,0,NULL,NULL,FL_MENU_INVISIBLE|FL_MENU_DIVIDER);
}

void XWin::CheckMenuItem(xmenu menu, int item, bool inCheck)
{
	if(!menu) return;
	Fl_Menu_Item * m = menu + item;

	m->flags |= FL_MENU_TOGGLE ;
	inCheck ? m->set() : m->clear();
}

void XWin::EnableMenuItem(xmenu menu, int item, bool inEnable)
{

	printf("enable \n");
//   QAction * aact = menu->actions().at(item);
//	 aact->setEnabled(inEnable);
}

void XWin::DrawMenuBar(void)
{
}

int XWin::TrackPopupCommands(xmenu in_menu, int mouse_x, int mouse_y, int button, int current)
{
	if(!in_menu) return -1;
//	QAction * aaction = in_menu->exec(this->mapToGlobal(QPoint(mouse_x,mouse_y)));
//
//	if(mDragging == button)
//	{
//		mWantFakeUp = 1;
//	}
//
//	return in_menu->actions().indexOf(aaction);
	return -1;
}
