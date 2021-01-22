/*
 * Copyright (c) 2007, Laminar Research.
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

#include "XWin.h"
#include <FL/filename.H>
#include "STLUtils.h"
#include <FL/names.h>

#define DEBUG_EVENTS 0
#define DEBUG_MENUS 0
#define DEBUG_DND 0

static void clearSubmenusRecursive(const Fl_Menu_Item *  menu)
{
	if(!menu) return;
	int sz = menu->size();
	for ( int t=0; t < sz ; t++)
	{
		Fl_Menu_Item * item = (Fl_Menu_Item *) &menu[t];
		if(!item->label()) break;

		if(item->flags&FL_SUBMENU_POINTER)
		{
			const Fl_Menu_Item * submenu = (const Fl_Menu_Item *) item->user_data();
			clearSubmenusRecursive(submenu);
			item->user_data(nullptr);
		}
		else
		{
			if(item->user_data_ != nullptr)
			{
				delete (xmenu_cmd *)item->user_data_;
				item->user_data_ = nullptr;
			}
		}

		if(item->text != nullptr)
		{
			free((void*)item->text);
			item->text = nullptr;
		}
	}
	delete [] menu;
}

XWin::XWin(
	int		default_dnd,
	const char*	inTitle,
	int		inAttributes,
	int		inX,
	int		inY,
	int		inWidth,
	int		inHeight)
	: Fl_Window(inX,inY,inWidth,inHeight,inTitle),
      mInited(false),mMenuBar(nullptr)
{
	mDragging    =-1;
	mWantFakeUp  = 0;
	mBlockEvents = 0;
	mMouse.x     = 0;
	mMouse.y     = 0;
	mTimer=0;

	// FLTK expects the titel label pointer points ever to valid data ,
	//copy_label makes a privat copy for the case the source for the text is lost
	copy_label(inTitle);

	if(inAttributes & xwin_style_centered)
	{
		this->position((Fl::w() - this->w())/2, (Fl::h() - this->h())/2 );
	}

	if(inAttributes & xwin_style_modal)	    set_modal();
	if(inAttributes & xwin_style_visible)	set_visible();
	if(inAttributes & xwin_style_resizable) resizable(this);

 	callback( window_cb );
	end();
	mInited = true;
}

XWin::XWin(int default_dnd) : Fl_Window(100,100), mInited(false),mMenuBar(nullptr)
{
	mDragging    =-1;
	mWantFakeUp  = 0;
	mBlockEvents = 0;
	mMouse.x     = 0;
	mMouse.y     = 0;
	mTimer=0;

	callback( window_cb );
	end();
	mInited = true;
}

XWin::~XWin()
{
//	if(mMenuBar)
//	{
//		clearSubmenusRecursive(mMenuBar->menu());
//	}
}

void XWin::ClearMenus(const Fl_Menu_Item *  menu)
{
	clearSubmenusRecursive(menu);
}

/**FLTK CALLBACK functs**/

/*FLTK draw callback*/
void XWin::draw()
{
	if(mInited)
	{
		Update(0);
		draw_children();
	}
}

inline int fltkBtnToXBtn(const int inButton )
{
	int btn = -1;
	switch (inButton)
	{
		 case FL_LEFT_MOUSE  : btn = 0; break;
		 case FL_RIGHT_MOUSE : btn = 1; break;
		 case FL_MIDDLE_MOUSE: btn = 2; break;
	}
	return btn;
}

static bool btnDown(int btn)
{
	switch(btn)
	{
		case 0:		return Fl::event_button1() != 0;
		case 1:		return Fl::event_button3() != 0;
		case 2:		return Fl::event_button2() != 0;
		default:	return false;
	}
}

/*FLTK event callback*/
int XWin::handle(int e)
{
	#if DEV && DEBUG_EVENTS
	string appnm("unknown");
	if(label()) appnm = label();
	#endif // DEV && DEBUG_EVENTS

	static bool want_menu_update = 1;

	/*handle menubar*/
	if( mMenuBar && ( Fl::event_inside(mMenuBar) || e == FL_SHORTCUT ) )
	{
		//mroe: if we detect a click on the menubar , thats the time before something is shown,
 		//We can e.g. update the menu content here;

		if( mMenuBar->callback() )
		if( e == FL_PUSH || (e == FL_SHORTCUT && want_menu_update ) )
		{
			 mMenuBar->do_callback(); //this updates the menus

			//mroe: sometimes the SHORTCUT is not handled by the menu when it first occurs.
			//We get this sc event up to three times. To avoid updating the whole menu every time,
			//the update is skipped until the shortcut is been handled or a key is released.
			 if(e == FL_SHORTCUT) want_menu_update = 0;
		}

		int result = mMenuBar->handle(e);

		if(result && e == FL_SHORTCUT) want_menu_update = 1;
		// ...
		if(result) return 1;
	}

	/*handle for GUI*/
   switch(e)
   {

	/*MOUSE events */
	case FL_PUSH:{
	#if DEV && DEBUG_EVENTS
		printf("FL_PUSH %s\n",appnm.c_str());
    #endif // DEV && DEBUG_EVENTS
		int btn  = fltkBtnToXBtn(Fl::event_button());
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
	#if DEV && DEBUG_EVENTS
		printf("FL_RELEASE %s\n",appnm.c_str());
	#endif // DEV && DEBUG_EVENTS
		int btn  = fltkBtnToXBtn(Fl::event_button());
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

		mMouse.x = Fl::event_x();
		mMouse.y = Fl::event_y();

		if(mBlockEvents) return 1;
		// Note: Can't use Fl::event_button() here because it is documented
		// to return garbage outside of FL_PUSH and FL_RELEASE.
		if(btnDown(mDragging))
		{
			ClickDrag(mMouse.x,mMouse.y,mDragging);
		}
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

	/*KEY events*/
	case FL_KEYDOWN:{
	#if DEV && DEBUG_EVENTS
		printf("FL_KEYDOWN \n");
	#endif // DEV && DEBUG_EVENTS
		if(Fl::event_command() || Fl::event_alt()) return 0;			  // propagate further for shortcuts
		uint32_t utf32char = 0;
		int l = Fl::event_length();
		if (l > 0)
		{
			const char * p = Fl::event_text();
			const char * e = p+l;
			utf32char = fl_utf8decode(p,e,&l);
		}
		KeyPressed(utf32char,Fl::event_key(), 0, 0);
	}
	return 1;
	case FL_KEYUP:{
	#if DEV && DEBUG_EVENTS
		printf("FL_KEYUP \n");
	#endif // DEV && DEBUG_EVENTS
		//if the shortcut is not handled by the menu we need a menu update with the next sc key stroke
		want_menu_update = 1;
	}
	return 1;
	case FL_SHORTCUT:{
	#if DEV && DEBUG_EVENTS
		printf("FL_SHORTCUT \n");
	#endif // DEV && DEBUG_EVENTS

	}
	return 0;

	/*WIDGET events */

	case FL_ACTIVATE:{
	#if DEV && DEBUG_EVENTS
		printf("FL_ACTIVATE %s\n",appnm.c_str());
	#endif // DEV && DEBUG_EVENTS
		Activate(true);
	}
	return 1;
	case FL_DEACTIVATE:{
	#if DEV && DEBUG_EVENTS
		printf("FL_DEACTIVATE %s\n",appnm.c_str());
	#endif // DEV && DEBUG_EVENTS
		Activate(false);
	}
	return 1;

	/*FOCUS events */
	case FL_FOCUS:{
	#if DEV && DEBUG_EVENTS
		printf("FL_FOCUS %s\n",appnm.c_str());
	#endif // DEV && DEBUG_EVENTS
		Activate(true);
	}
	return 1;
	case FL_UNFOCUS:{
	#if DEV && DEBUG_EVENTS
		printf("FL_UNFOCUS %s\n",appnm.c_str());
	#endif // DEV && DEBUG_EVENTS
		Activate(false);
	}
	return 1;
	case FL_ENTER:{
	#if DEV && DEBUG_EVENTS
		printf("FL_ENTER %s\n",appnm.c_str());
	#endif // DEV && DEBUG_EVENTS
	}
	return 1;
	case FL_LEAVE:{
	#if DEV && DEBUG_EVENTS
		printf("FL_LEAVE %s\n",appnm.c_str());
	#endif // DEV && DEBUG_EVENTS
	}
	return 1;

	/*DND events */
	case FL_DND_ENTER:
	case FL_DND_DRAG :
	case FL_DND_LEAVE:
	case FL_DND_RELEASE:
	return 1;
	case FL_PASTE:{
		//TODO: can carry a list of filenames , can become much more
		char c[2048];
		strncpy(c, Fl::event_text(), sizeof(c));
		#if FL_PATCH_VERSION < 1
		#error FLTK 1.3.0 is not supported , no fl_decode_uri
		#else
		fl_decode_uri(c);
		#endif
	#if DEV && DEBUG_EVENTS
		printf("XWin::handle FL_PASTE Win %s\n",c);
	#endif // DEV && DEBUG_EVENTS
		ReceiveFilesFromDrag(c);
	}
	return 1;

	}

	/*OTHER Window events */
 	return Fl_Window::handle(e);
}

/*FLTK resize callback*/
void XWin::resize(int x,int y,int w,int h)
{
	bool no_resize = ( w == this->w() && h == this->h() );
	bool not_moved = ( x == this->x() && y == this->y() );
	if( no_resize & not_moved ) return;

	int  mbar_h = mMenuBar ? mMenuBar->h() : 0;
	Fl_Window::resize(x,y,w,h);

	if(no_resize) return;
	if(mMenuBar && this->resizable() == this)
	{
	   mMenuBar->size(w,mbar_h);
	}
	Resized(w,h);
}

/*FLTK  menu callback*/
void XWin::menu_cb(Fl_Widget *w, void * data)
{
	Fl_Menu_Bar *bar = (Fl_Menu_Bar*)w;		        // Get the menubar widget
	const Fl_Menu_Item *item = bar->mvalue();		// Get the menu item that was picked
	Fl_Menu_Item * m=(Fl_Menu_Item *) item;
	#if DEV && DEBUG_MENUS
	printf("cb %d %s\n",item->first(),item->first()->label());
	#endif // DEV && DEBUG_MENUS
	xmenu_cmd* cmd = (xmenu_cmd*) data;
	if(!cmd) return;
	#if DEV && DEBUG_MENUS
	printf("menu %d %d item %d cmd %d \n",item->value(),m->value(),cmd->data,cmd->cmd);
	#endif // DEV && DEBUG_MENUS
	XWin* win = (XWin*) bar->parent();
	win->HandleMenuCmd((xmenu )cmd->data,cmd->cmd);
}

/*FLTK  menubar callback*/
void XWin::menubar_cb(Fl_Widget *w, void * data)
{
	if(!w ) return;
	#if DEV && DEBUG_MENUS
	printf("menubar_cb called \n");
	#endif // DEV && DEBUG_MENUS
	//Fl_Menu_Bar * bar = (Fl_Menu_Bar *) w;
}

/*FLTK window about to close callback*/
void XWin::window_cb(Fl_Widget *widget, void *)
{
	XWin * w = (XWin *)widget;
	if (!w->Closed() ) return;
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

/** xptool GUI   **/

/* prevent pure virtual function calls. ben, we need to restructure this,
** it hinders us using deep inheritance schemes
*/

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
	this->copy_label(inTitle);
}

void XWin::SetFilePath(const char * inPath,bool modified)
{
}

void XWin::MoveTo(int inX, int inY)
{
	this->position(inX,inY);
}

void XWin::Resize(int inWidth, int inHeight)
{
	this->size(inWidth,inHeight + GetMenuBarHeight());
}

void XWin::ForceRefresh(void)
{
	flush();
}

void XWin::UpdateNow(void)
{
	Fl::check();
}

void XWin::SetVisible(bool visible)
{
	if(visible)
	{
		show();
	}
	else
	{
		hide();
	}
}

bool XWin::GetVisible(void) const
{
	return visible();
}

bool XWin::GetActive(void) const
{
    return active_r();
}

void XWin::SetTimerInterval(double seconds)
{
    if(seconds)
    {
        if(mTimer)
            Fl::remove_timeout(timeout_cb,this);

        Fl::add_timeout(seconds,timeout_cb,this);
        mTimer=seconds;
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
	if (outX) *outX = this->x();
	if (outY) *outY = this->y();
	//printf("WindowLoc l=%d t=%d\n", x(), y());
}

void XWin::GetDesktop(int bounds[4])
{
	bounds[0] = bounds[1] = 32000;
	bounds[2] = bounds[3] = 0;
	int num_screens = Fl::screen_count();
	for (int s = 0; s < num_screens; ++s)
	{
		int X,Y,W,H;
		Fl::screen_xywh(X, Y, W, H, s);
		//printf("Screen %d: l=%d t=%d w=%d h=%d\n", s, X, Y, W, H);
		bounds[0] = min(bounds[0], X);
		bounds[1] = min(bounds[1], Y);
		bounds[2] = max(bounds[2], X+W);
		bounds[3] = max(bounds[1], Y+H);
	}
	//printf("Total Desktop l=%d t=%d r=%d b=%d\n", bounds[0], bounds[1], bounds[2], bounds[3]);
}

void XWin::GetMouseLoc(int * outX, int * outY)
{
	if (outX) *outX = mMouse.x;
	if (outY) *outY = mMouse.y;
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
				#if DEV && DEBUG_DND
				printf("file s %s\n",files.back().c_str());
				#endif // DEV && DEBUG_DND
			}
		}
	}

	if(files.size() > 0)
			ReceiveFiles(files, 0, 0);
}

xmenu XWin::GetMenuBar(void)
{
	if(!mMenuBar)
	{
		int font_height = max(14,this->labelsize());
		mMenuBar = new Fl_Menu_Bar(0,0,w(),font_height + 10);
		mMenuBar->textsize(font_height);

		int mbar_h = mMenuBar->h();
		Fl_Widget * resizable_widget = this->resizable();
		resizable(this);
		this->size(this->w(),this->h() + mbar_h);
		this->insert(*mMenuBar,0);

		for(int i = 1; i < this->children() ; ++i)
		{
			int new_y =  this->child(i)->y() + mbar_h;
			int new_h =  this->child(i)->h() - mbar_h;
			resizable(child(i));
			this->child(i)->resize(this->child(i)->x(),new_y,this->child(i)->w(),new_h);
		}

		this->resizable(resizable_widget);
		init_sizes();

		mMenuBar->callback(menubar_cb);
		mMenuBar->box(FL_FLAT_BOX );
		mMenuBar->down_box(FL_GTK_THIN_DOWN_BOX);
		int r=84,g=133,b=198;// giving the menubar the same selection color as GUI widgets
		mMenuBar->selection_color(fl_rgb_color(r,g,b));
		//mMenuBar->textfont(0);

		//mroe: thats to get an empty initalized menu
		//mMenuBar->add("test",0,nullptr,nullptr);
		//mMenuBar->remove(0);
		Fl_Menu_Item * new_menu = new Fl_Menu_Item[MENU_ARRAY_SIZE * sizeof(Fl_Menu_Item )];
		memset(new_menu,0,MENU_ARRAY_SIZE * sizeof(Fl_Menu_Item ));
		mMenuBar->menu(new_menu);
	}
	return mMenuBar->menu();
}

int XWin::GetMenuBarHeight(void)
{
	if (mMenuBar != nullptr) return mMenuBar->h();
	return 0;
}

xmenu XWin::CreateMenu(xmenu parent, int item, const char * inTitle)
{
	if(!parent || parent->size() > MENU_ARRAY_SIZE-1) return NULL;

	Fl_Menu_Item * new_menu = new Fl_Menu_Item[MENU_ARRAY_SIZE * sizeof(Fl_Menu_Item )];
	memset(new_menu,0,MENU_ARRAY_SIZE * sizeof(Fl_Menu_Item ));

	Fl_Menu_Item * p = (Fl_Menu_Item *) parent;
	int idx= p->insert(item,inTitle,0,nullptr,(void*)new_menu,FL_SUBMENU_POINTER);

	return new_menu;
}

int XWin::AppendMenuItem(xmenu menu, const char * inTitle)
{
	if(!menu || menu->size() > MENU_ARRAY_SIZE-1) return -1;
	xmenu_cmd * cmd = new xmenu_cmd();

	Fl_Menu_Item * m = (Fl_Menu_Item *) menu;
	int idx = m->add(inTitle,0,menu_cb,cmd);

	cmd->data = m;
	cmd->cmd  = idx;

	return idx;
}

int XWin::AppendSeparator(xmenu menu)
{
	if(!menu || menu->size() > MENU_ARRAY_SIZE-1) return -1;

	Fl_Menu_Item * m	= (Fl_Menu_Item *) menu ;
	Fl_Menu_Item * last = (Fl_Menu_Item *) m + (m->size()-2);
	last->flags = last->flags|FL_MENU_DIVIDER;
	char buf[256];
	strcpy(buf,last->label());													//the item title must be unique
	strcat(buf,"_");															// adding '_' to the last name does this
	return m->add(buf,0,NULL,NULL,FL_MENU_INVISIBLE|FL_MENU_DIVIDER);
}

void XWin::CheckMenuItem(xmenu menu, int item, bool inCheck)
{
	if(!menu) return;
	Fl_Menu_Item * m = (Fl_Menu_Item *) menu + item;

	m->flags |= FL_MENU_TOGGLE ;
	inCheck ? m->set() : m->clear();
}

void XWin::EnableMenuItem(xmenu menu, int item, bool inEnable)
{
	/* #warning : XWin::EnableMenuItem not implemented yet */
	fprintf(stderr,"XWin::EnableMenuItem not implemented\n");
}

void XWin::DrawMenuBar(void)
{
}

int XWin::TrackPopupCommands(xmenu in_menu, int mouse_x, int mouse_y, int button, int current)
{
	if(!in_menu) return -1;

	int idx = -1;
	const Fl_Menu_Item * item = in_menu->popup(mouse_x,mouse_y,0,0,mMenuBar);

	if(item)
	   idx = in_menu->size() - item->size();

	if(mDragging == button)
	{
		mWantFakeUp = 1;
	}

	return  idx;
}
