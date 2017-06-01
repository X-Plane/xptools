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
#include "XWin.h"
#include "XUtils.h"
#include "AssertUtils.h"

#pragma mark -
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
// OBJ-C WINDOW SUB-CLASS
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// This sub-class handles:
// - Menu callbacks
// - Drag and drop callbacks (source and destination)
// - Timer callbacks
// We keep a pointer to our sub-view (not really needed since it is the content view)
// and also our owning window.
//

@implementation XWinCocoa
-(BOOL) canBecomeKeyWindow { return YES; }
-(BOOL) canBecomeMainWindow { return YES; }

- (void) setOwner:(XWin*)owner withView:(NSView *) view
{
	mOwner = owner;
	mView = view;
}

// Since we might be retained, the C++ window can go away before we do.
// Zap our owner flag to avoid calling a dead object.
- (void) killOwner
{
	mOwner = NULL;
}

- (NSView *) view
{
	return mView;
}

- (void) timerFired
{
	if(mOwner)
		mOwner->Timer();
}

// This is the command handler for menus created directly by XWin, e.g. for trivial apps like XGrinder/ObjView/RenderFarm.
// Thus the command is simply dispatched to the owner by index.
- (void) menuItemPicked:(id) sender
{
	NSMenuItem * item = sender;
	NSMenu * owner = [item representedObject];
	int idx = [owner indexOfItem:item];
	
	// If we're in a popup, we just store the result
	if(mOwner->mInPopup)
		mOwner->mPopupPick = idx;
	else
		mOwner->HandleMenuCmd(owner, idx);	// Otherwise pass to subclass
}

// MASSIVE HACK ALERT.  I don't normally hotwire things together like this, but in this case we gotta ship WED 1.5 and frankly this
// is going to be gross no matter what.
// Anyway: in order for menu items to be dispatched EVEN in a modal loop, menu items have to be sent to the first responder, and not
// to the app delegate.  (For some reason, menu items do go down the responder chain to the key window in a modal loop, but they won't
// go directly to the app delegate.)  We need this for cut/copy/paste to work in the gateway import dialog box.
//
// So...the nasty thing is that the ObjC window that GUI uses is secretly down inside XWin (mac edition).  But because ObjC uses squashy
// dispatch-by-name on selectors, we can _by luck_ put into the base window class the particular method name that ObjCUtils is going to use.
// We then tie it to a template method.
//
// GUI turns around and (1) uses ObjCUtils to build menu items targeting this secret path and (2) overrides the tmeplate method and finally
// GUI has access to menu picks in all modal situations.
//
// Note that this only works because WED _always_ has a window up - if we could close all windows, the application itself doesn't have
// a responder chain handler to match.  We can fix that someday (by explicitly subclassing the application I guess????) if we ever need it.
//
// It's worth noting that the command sent to GUI_Window is sent from the focus target up the _GUI_ command chain, regardless of what ObjC 
// thinks is going on.  So it is possible (if GUI is f---ed up) to have a command in one window act on another due to GUI bugs - the NS idea
// of key window is NOT enforced.
//
// Note that there are TWO menu implementations here but only one runs at a time:
// - If ALL menus go through XWin, then XWin creates the menu item, points at its own menuItemPicked selector, and then calls its own
//   public HandleMenuCmd handler.  This dispatches menus by indexand is frankly a pretty clean way to do menus.
//
// - If ALL menus go through GUI_Menus, then ObjCUtils is used to manually build the menu bar, targeting the hidden selector in XWin that
//   is sent to GUI_Window and up the GUI command chain.

- (void) menu_picked:(id) sender
{
	NSMenuItem * menu = sender;
	int cmd_id = [menu tag];
	mOwner->GotCommandHack(cmd_id);
}

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context
{
	return mOwner->mCurrentDragOps;
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
//	Debug code to dump what's coming in
//	NSArray<NSString *> * tl = [[sender draggingPasteboard] types];
//	for(int i = 0; i < [tl count]; ++i)
//		printf("%d: %s\n", i, [[tl objectAtIndex:i] UTF8String]);
//
//	NSArray<NSPasteboardItem *> * il = [[sender draggingPasteboard] pasteboardItems];
//	printf("%d items\n", [il count]);
//	for(int i = 0; i < [il count]; ++i)
//	{
//		NSPasteboardItem * item = [il objectAtIndex:i];
//		NSArray<NSString *> * tl = [item types];
//		for(int j = 0; j < [tl count]; ++j)
//			printf("%d,%d: %s\n", i, j, [[tl objectAtIndex:j] UTF8String]);
//	}
	
	

	if(!mOwner->mDefaultDND)
		return mOwner->AdvancedDragEntered(sender);

	NSArray *classes = [NSArray arrayWithObject:[NSURL class]];
	NSDictionary *options = [NSDictionary dictionaryWithObject:
		[NSNumber numberWithBool:YES] forKey:NSPasteboardURLReadingFileURLsOnlyKey];

	NSPasteboard * dpb = [sender draggingPasteboard];
	
	if(![dpb canReadObjectForClasses:classes options:options])
		return NSDragOperationNone;
	
	NSPoint p = [sender draggingLocation];
	int x = p.x;
	int y = [self frame].size.height - p.y;
	
	mOwner->DragEnter(x,y);
	return NSDragOperationCopy;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
	if(!mOwner->mDefaultDND)
		return mOwner->AdvancedDragUpdated(sender);

	NSArray *classes = [NSArray arrayWithObject:[NSURL class]];
	NSDictionary *options = [NSDictionary dictionaryWithObject:
		[NSNumber numberWithBool:YES] forKey:NSPasteboardURLReadingFileURLsOnlyKey];

	NSPasteboard * dpb = [sender draggingPasteboard];
	
	if(![dpb canReadObjectForClasses:classes options:options])
		return NSDragOperationNone;

	NSPoint p = [sender draggingLocation];
	int x = p.x;
	int y = [self frame].size.height - p.y;
	
	mOwner->DragEnter(x,y);
	return NSDragOperationCopy;
}

- (void)draggingExited:(nullable id <NSDraggingInfo>)sender
{
	if(!mOwner->mDefaultDND)
	{
		mOwner->AdvancedDragExited(sender);
		return;
	}
	
	NSArray *classes = [NSArray arrayWithObject:[NSURL class]];
	NSDictionary *options = [NSDictionary dictionaryWithObject:
		[NSNumber numberWithBool:YES] forKey:NSPasteboardURLReadingFileURLsOnlyKey];

	NSPasteboard * dpb = [sender draggingPasteboard];
	
	if(![dpb canReadObjectForClasses:classes options:options])
		return;

	mOwner->DragLeave();
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	if(!mOwner->mDefaultDND)
		return mOwner->AdvancedPerformDrop(sender);
	
	NSPoint p = [sender draggingLocation];
	int x = p.x;
	int y = [self frame].size.height - p.y;


	NSPasteboard *pasteboard = [sender draggingPasteboard];
	NSArray *classes = [NSArray arrayWithObject:[NSURL class]];
 
	NSDictionary *options = [NSDictionary dictionaryWithObject:
		[NSNumber numberWithBool:YES] forKey:NSPasteboardURLReadingFileURLsOnlyKey];
 
	NSArray *fileURLs =
		[pasteboard readObjectsForClasses:classes options:options];
	
	vector<string>	files;
	
	for(int i = 0; i < [fileURLs count]; ++i)
	{
		NSURL * url = [fileURLs objectAtIndex:i];
		NSString * path = [url path];
		files.push_back([path UTF8String]);
	}
	
	if(!files.empty())
		mOwner->ReceiveFiles(files, x,y);
	
	return YES;
}

- (void)draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
	// This is a hack. When we are dragging we go into a modal loop to emulate the modal drag semantics of the other OSes and old UI
	// kits.  We pump the event queue until we see that the drag is over, then we eject, and we skip up events while we do this.
	// There's some kind of weird race - I've never seen it but we have seen the evidence in bug WED-566. Basically we end up with
	// the drag session over but we are STILL inside "sendEvent" from inside the modal loop.  This was allowing for a re-dispach of
	// a mouse down INSIDE a mouse down, resulting in double-click-counting.
	//
	// To prevent this, we use a tri-state flag..."2" means we are done with the drag (and thus we SHOULD exit the loop) but we have not
	// YET exited the loop and unwound the stack, so we should not process down events.  Down events that hit this race window
	// will now be dropped.
	mOwner->mInDragOp = 2;
}

- (void)close
{
	XWin * o = mOwner;
	mOwner = NULL;
	delete mOwner;
}

@end

#pragma mark -
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
// OBJ-C VIEW SUB-CLASS
//----------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Our view handles:
// - View-type callbacks (E.g. all mouse and draw callbacks.
// - Since our view is (weirdly) our window delegate, it doe some window operations.
// - Tooltip callbacks
//

@interface XWinView : NSView<NSWindowDelegate> {
	XWin * mOwner;
};
- (void) setOwner:(XWin*)owner;
@end

@implementation XWinView

- (void) setOwner:(XWin*)owner
{
	mOwner = owner;
}

- (BOOL)windowShouldClose:(id)sender
{
	if(!mOwner)	return YES;
	if(mOwner->Closed())
		return YES;
	return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
	// Work around WED-851...mOwner is NULL if we get a resize message
	// on a hide due to being destroyed. 
	if(mOwner)
	if(!mOwner->mInInit)
	{
		int w, h;
		mOwner->GetBounds(&w,&h);
		mOwner->Resized(w,h);
	}
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(mOwner)
	if(!mOwner->mInInit)
	mOwner->Update(NULL);
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
	if(mOwner)
	if(!mOwner->mInInit)
		mOwner->Activate(1);
}

- (void)windowDidResignMain:(NSNotification *)notification
{
	if(mOwner)
	if(!mOwner->mInInit)
		mOwner->Activate(0);
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	// This kind of protection protects not only against our owner being destroyed while NS is alive by ref count,
	// but also against getting mouse events leaking through the modal drag loop, which happnes rarely but not never.
	if(mOwner && mOwner->mInDragOp == 0)
	mOwner->EventButtonDown(x,y,0, [theEvent modifierFlags] & NSControlKeyMask ? 1 : 0);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner && mOwner->mInDragOp == 0)
	mOwner->EventButtonDown(x,y,1, [theEvent modifierFlags] & NSControlKeyMask ? 1 : 0);
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner && mOwner->mInDragOp == 0)
	mOwner->EventButtonDown(x,y,2, [theEvent modifierFlags] & NSControlKeyMask ? 1 : 0);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner)
	mOwner->EventButtonMove(x,y);
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner)
	mOwner->EventButtonMove(x,y);
}

- (void)otherMouseDragged:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner)
	mOwner->EventButtonMove(x,y);
}

- (void)mouseUp:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner && mOwner->mInDragOp == 0)
	mOwner->EventButtonUp(x,y,0);
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner && mOwner->mInDragOp == 0)
	mOwner->EventButtonUp(x,y,1);
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner && mOwner->mInDragOp == 0)
	mOwner->EventButtonUp(x,y,2);
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	
	if(mOwner)
	mOwner->EventButtonMove(x, y);
}

- (void)flagsChanged:(NSEvent *)theEvent
{
	// Send a fake mouse moved so code can update based on mod-key changes
	if(mOwner)
	mOwner->EventButtonMove(mOwner->mLastX, mOwner->mLastY);
}

- (void)mouseEntered:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;
	if(mOwner)
	mOwner->EventButtonMove(x, y);
}


- (void)scrollWheel:(NSEvent *)theEvent
{
	NSPoint event_location = [theEvent locationInWindow];
	int x = event_location.x;
	int y = [self frame].size.height - event_location.y;

	CGEventRef real_event = [theEvent CGEvent];
	int dy = roundf(CGEventGetDoubleValueField(real_event, kCGScrollWheelEventDeltaAxis1));
	int dx = roundf(CGEventGetDoubleValueField(real_event, kCGScrollWheelEventDeltaAxis2));

	if(mOwner)
	{
		if(abs(dy) > abs(dx))
			mOwner->MouseWheel(x, y, dy, 0);
		else
			mOwner->MouseWheel(x, y, dx, 1);
	}
}

- (void) keyDown:(NSEvent *)theEvent
{
	int flags = 0;
	uint32_t char_code = 0, uni_code = 0;
	unsigned short key_code = [theEvent keyCode];
	NSString * keystr = [theEvent characters];
	if([keystr length] > 0)
		uni_code = char_code = [keystr characterAtIndex:0];
	if(char_code > 127)
		char_code = 0;

	long param1 = (key_code << 8) | (char_code & 0xFF);
	long param2 = 0;
	long msg = 0;
	
	if ([theEvent type] == NSKeyDown && ![theEvent isARepeat])
		msg = NSKeyDown;
	if ([theEvent type] == NSKeyUp)
		msg = NSKeyUp;
		
	if ([theEvent modifierFlags] & NSShiftKeyMask)
		param2 |= NSShiftKeyMask;
	if ([theEvent modifierFlags] & NSControlKeyMask)
		param2 |= NSControlKeyMask;

	if ([theEvent modifierFlags] & NSAlternateKeyMask)
		param2 |= NSAlternateKeyMask;

	if ([theEvent modifierFlags] & NSCommandKeyMask)
		param2 |= NSCommandKeyMask;

	if(mOwner)
	{
		if(!mOwner->KeyPressed(uni_code, msg, param1, param2))
		{
			NSPoint event_location = [[self window] convertScreenToBase:[NSEvent mouseLocation]];
		
			int x = event_location.x;
			int y = [self frame].size.height - event_location.y;
		
			if (char_code == '=')
				mOwner->MouseWheel(x, y, 1, 0);
			else if (char_code == '-')
				mOwner->MouseWheel(x, y, -1, 0);
		}
	}
}

// This proc is called when we get a tool tip request in the 'global' tool tip catcher...that's our cue to
// ask our client where the tool tip really is on the fly.
- (NSString *)view:(NSView *)view stringForToolTip:(NSToolTipTag)tag point:(NSPoint)point userData:(void *)data
{
	if(!mOwner) return NULL;
	mOwner->mToolTipLive = 1;
	int x = point.x;
	int y = [self frame].size.height - point.y;
	string msg;
	
	int got_one = mOwner->CalcHelpTip(x, y, mOwner->mToolTipBounds, msg);
//	if(got_one) printf("Calc help tip for %d %d got %d,%d -> %d,%d %s\n", x, y, mOwner->mToolTipBounds[0], mOwner->mToolTipBounds[1], mOwner->mToolTipBounds[2], mOwner->mToolTipBounds[3], msg.c_str());
//	else		printf("No help tip for %d %d\n", x,y);
	
	if(got_one)
	{
		// If we get a tool tip, we go build a -real- tool tip and add it.
		// This will cause NSWindow to go re-figure out what's up. The
		// tool tip's rect is set to the actual rect so that the OS can
		// position the tip to not obscure the information the user is
		// mousing over.

		NSRect r = NSMakeRect(
			mOwner->mToolTipBounds[0],
			[self frame].size.height - mOwner->mToolTipBounds[3],
			mOwner->mToolTipBounds[2] - mOwner->mToolTipBounds[0],
			mOwner->mToolTipBounds[3] - mOwner->mToolTipBounds[1]);

		// The tool tip does not retain its tip object, so since we are using
		// a dumb string, we store it ourselves.
		NSString * str = [[NSString alloc] initWithUTF8String:msg.c_str()];
		mOwner->mToolTipMem = str;
		
		// Make a simple static tool tip for the actual string with a rect based on what
		// our sub-class told us
		[self addToolTipRect:r owner:str userData:NULL];
		
		// The text for the global catcher is still NULL since its bounds are not useful for positioing.
		return NULL;
	}
	else
	{
		// If we did not get a hit, set our invalidation rect to be really small - that way, we can get a chance
		// to re-evaluate the tool tip if the mouse moves.
		mOwner->mToolTipBounds[0] = x;
		mOwner->mToolTipBounds[1] = y;
		mOwner->mToolTipBounds[2] = x+1;
		mOwner->mToolTipBounds[3] = y+1;
		return NULL;
	}
}

@end

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
// XWIN C++ STUFF
//----------------------------------------------------------------------------------------------------------------------------------------------------------------


void	XWin::initCommon(int dnd, const char * title, int attributes, int x, int y, int dx, int dy)
{
	mInDragOp = 0;
	mInMouseHandler = 0;
	mCurrentDragOps = 0;
	mLastX = mLastY = 0;
	mToolTipMem = NULL;
	mInInit = 1;
	mInPopup = 0;
	mDefaultDND = dnd;
	mToolTipLive = 0;
	
	mInDrag = -1;
	mWantFakeUp = 0;
	mIsControlClick = 0;
	
	mTimer = NULL;

	int mask = 0;
	
	switch(attributes & xwin_style_modal) {
	case xwin_style_thin:		break;
	case xwin_style_movable:	mask = NSTitledWindowMask|NSClosableWindowMask;		break;
	case xwin_style_resizable:	mask =	NSTitledWindowMask|NSClosableWindowMask|NSResizableWindowMask|NSMiniaturizableWindowMask; break;
	case xwin_style_modal:		mask = NSTitledWindowMask;			break;
	}
	
	float h = [[[NSScreen screens] objectAtIndex:0] frame].size.height;

	XWinCocoa * window;
	window = [[XWinCocoa alloc] initWithContentRect:NSMakeRect(x,h-y-dy,dx,dy)
									styleMask:mask
									backing:NSBackingStoreBuffered
									defer:TRUE];
	mWindow = window;
	
	[window setAcceptsMouseMovedEvents:YES];
	
	XWinView * view = [[XWinView alloc] initWithFrame:NSMakeRect(0,0,dx,dy)];
	
	// Register a tracking area - we need this to get mouse moved events.  NSTrackingInVisibleRect causes it to always
	// track the entire visible area of the NSView, so it won't need resizing.
	
	NSTrackingArea			*trackingArea	= [[NSTrackingArea alloc] initWithRect:NSZeroRect
																  options:NSTrackingMouseEnteredAndExited|NSTrackingMouseMoved|NSTrackingActiveInActiveApp|NSTrackingInVisibleRect
																	owner:view
																 userInfo:nil];
	[view addTrackingArea:trackingArea];
	[trackingArea release];
	
	[window setContentView:view];
	[window setContentMinSize:NSMakeSize(512,384)];
	[view setOwner:this];
	[window setOwner:this withView:view];
	[window setAcceptsMouseMovedEvents:YES];
	[view release];
	[window setDelegate:view];

	if(attributes & xwin_style_centered)
	{
		[window center];
	}
	if(attributes & xwin_style_fullscreen)
	{
		[window zoom:NULL];
	}
	if((attributes & xwin_style_modal) == xwin_style_modal)
	{
		// Since modals are popped up NOW, they have to be visible, otherwise NS puts
		// the window up and GUI thinks we're hidden and doesn't draw.
		DebugAssert(attributes & xwin_style_visible);
		[window setLevel:NSModalPanelWindowLevel];
	}
	if(attributes & xwin_style_visible)
	{
		[window makeKeyAndOrderFront:NSApp];
	}
	[window makeFirstResponder:view];
	
	// This is needed to fully set up the GL stack - only after we do a display will buffer swaps actually show anything!
	[view setNeedsDisplay:TRUE];

	[view addToolTipRect:[view frame] owner:view userData:NULL];

	if(mDefaultDND)
	{
		NSArray * pbtypes = [NSArray arrayWithObject:NSFilenamesPboardType];
		[window registerForDraggedTypes:pbtypes];
	}
	
	// On close, we call our C++, which will kill us.
	[mWindow setReleasedWhenClosed:NO];
	mInInit = 0;
	
	// This is slightly insane.  To get true modal behavior, we have to run a tight modal loop via a
	// call to NSApplication that doesn't return until modal is over.  This is normally what we want -
	// you run a modal loop and you want the asnwer NOW so you can proceed with linear code.
	//
	// But WED's not written that way - the import dialog boxes are modal mostly so that we don't have to cope
	// with the document massively changing while the import is happening.  But the code is written modeless -
	// that is, everything is done as a bunch of callbacks.
	//
	// So here's how we get modeless code flow with modal window logic in WED/GUI: we are going to run a modal
	// loop on NSApplication for this window, but not until we _fully_ exit this callback.
	// So typically this window ctor is called from some handler which is a callout from NS's menu handler from inside
	// the main run loop.  We use libdispatch to async queue our modal loop and quit, going all the way up the handler.
	//
	// THEN at the main loop, the block runs, runs the modal loop, and the window's close box will kill the modal loop.
	//
	// The rest of GUI doesn't care - it's still getting app call-outs.
	if((attributes & xwin_style_modal) == xwin_style_modal)
		dispatch_async(dispatch_get_main_queue(),^{
			[[NSApplication sharedApplication] runModalForWindow:mWindow];
		});
}


XWin::XWin(int default_dnd)
{
	float h = [[[NSScreen screens] objectAtIndex:0] frame].size.height;
	float w = [[[NSScreen screens] objectAtIndex:0] frame].size.width;
	
	h -= [[NSApp mainMenu] menuBarHeight];
	
	initCommon(default_dnd, "", xwin_style_thin|xwin_style_visible, 0, 0, w, h);

}


XWin::XWin(
	int				default_dnd,
	const char * 	inTitle,
	int				inAttributes,
	int				inX,
	int				inY,
	int				inWidth,
	int				inHeight)
{
	initCommon(default_dnd, inTitle, inAttributes, inX, inY, inWidth, inHeight);
}

XWin::~XWin()
{
	// Our window might be retained - if we are killed from a modal loop, the modal
	// loop might be retaining the window, but the C++ window is killed FROM our
	// loop via a button callback like 'cancel' or who knows what.  Or maybe the
	// auto release pool hasn't flushed.  Bottom line is, the window can't be ensured
	// to die synchronously, so we null out its owner so it can't call us, since as of
	// now WE are dead.
	[mWindow killOwner];
	[[mWindow contentView] setOwner:NULL];
	[mWindow setIsVisible:FALSE];
	
	// If we are a modal dialog box, there's a modal looop somewhere that needs killing.
	if([mWindow level] == NSModalPanelWindowLevel)
		[[NSApplication sharedApplication] stopModal];

	[mWindow release];
}

#pragma mark -

void			XWin::SetTitle(const char * inTitle)
{
	[mWindow setTitle:[NSString stringWithUTF8String:inTitle]];
}

void			XWin::SetFilePath(const char * inPath,bool modified)
{
	if(inPath)
		[mWindow setRepresentedFilename:[NSString stringWithUTF8String:inPath]];
	[mWindow setDocumentEdited:modified];
}


void	XWin::SetVisible(bool visible)
{
	if (visible)
	{
		[mWindow setIsVisible:TRUE];
	}
	else
	{
		[mWindow setIsVisible:FALSE];
	}
}

bool	XWin::GetVisible(void) const
{
	return [mWindow isVisible];
}

bool	XWin::GetActive(void) const
{
	return [mWindow isMainWindow];
}

void			XWin::MoveTo(int inX, int inY)
{
	float h = [[[NSScreen screens] objectAtIndex:0] frame].size.height;

	NSRect me_now = [mWindow frame];
	me_now.origin.y = h - inY;
	me_now.origin.x = inX;
	[mWindow setFrame:me_now display:YES];
}

void			XWin::Resize(int inWidth, int inHeight)
{
	NSRect me_now = [mWindow frame];

	me_now = [mWindow contentRectForFrameRect:me_now];

	me_now.size.height = inHeight;
	me_now.size.width = inWidth;

	me_now = [mWindow frameRectForContentRect:me_now];

	[mWindow setFrame:me_now display:YES];

}

void			XWin::ForceRefresh(void)
{
	[[mWindow view] setNeedsDisplay: YES ];
	if([[[mWindow view] subviews] count])
	{
		NSView * target = [[[mWindow view] subviews] objectAtIndex:0];
		// This is a hack - our OpenGL subclass puts ANOTHER view in - inval that too if it exists.
		[target setNeedsDisplay:YES];
	}
}

void			XWin::UpdateNow(void)
{
	[mWindow display];
}

void			XWin::SetTimerInterval(double seconds)
{
	if(mTimer) [mTimer invalidate];
	mTimer = NULL;
	
	if(seconds)
	{
		mTimer = [NSTimer scheduledTimerWithTimeInterval:seconds target:mWindow selector:@selector(timerFired) userInfo:NULL repeats:YES];
		[mTimer retain];
	}
}

void			XWin::GetBounds(int * outX, int * outY)
{
	NSRect me_now = [mWindow frame];

	me_now = [mWindow contentRectForFrameRect:me_now];

	if (outX) *outX = me_now.size.width;
	if (outY) *outY = me_now.size.height;
}

void			XWin::GetWindowLoc(int * outX, int * outY)
{
	float h = [[[NSScreen screens] objectAtIndex:0] frame].size.height;
	NSRect me_now = [mWindow frame];

	me_now = [mWindow contentRectForFrameRect:me_now];

	if (outX) *outX = me_now.origin.x;
	if (outY) *outY = h - me_now.origin.y;
}



void		XWin::GetMouseLoc(int * outX, int * outY)
{
	NSPoint mouseLoc = [NSEvent mouseLocation];
	
	NSRect me_now = [mWindow frame];
	me_now = [mWindow contentRectForFrameRect:me_now];
	
	mouseLoc = [mWindow convertScreenToBase:mouseLoc];
	
	if (outX) *outX = *outX = mouseLoc.x;
	if (outY) *outY = *outY = me_now.size.height - mouseLoc.y;
}

void		XWin::EventButtonDown(int x, int y, int button, int has_control_key)
{
	DebugAssert(mInMouseHandler == 0);
	++mInMouseHandler;
	
	mLastX = x;
	mLastY = y;
	ManageToolTipForMouse(x,y);
	if(mWantFakeUp)
	{
		DebugAssert(mInDrag != -1);
		this->ClickUp(x, y, mIsControlClick ? 1 : button);
		mInDrag = -1;
		mWantFakeUp = 0;
	}
	
	// We only allow one button down at a time - this prevents the problem of nested
	// edit events inside the UI.  So if we are in a drag, we simply stick our fingers
	// in our head and ignore other events.
	if(mInDrag == -1)
	{
		mInDrag = button;
		mIsControlClick = has_control_key;
		this->ClickDown(x, y, mIsControlClick ? 1 : button);
		
		// This fake-up case is a nasty hack for OS X: our up click is eaten by the DND code...
		// so the DND code may have issued a fake up _immediately_.  We do this now.  The click
		// code can't issue the up itself because it is INSIDE the down handler (not cool man,
		// not cool) but if it does an NS event post or libdispatch async it's too slow and a pile
		// of moved events slip through.
		if(mWantFakeUp)
		{
			mWantFakeUp = 0;
			this->ClickUp(x, y, mIsControlClick ? 1 : button);
			mInDrag = -1;
			mWantFakeUp = 0;
		}
	}
	--mInMouseHandler;
}

void		XWin::EventButtonUp  (int x, int y, int button					 )
{
	DebugAssert(mInMouseHandler==0);
	++mInMouseHandler;
	mLastX = x;
	mLastY = y;
	ManageToolTipForMouse(x,y);

	// The up click for events other than us are ignored too
	if(mInDrag == button)
	{
		mWantFakeUp = 0;
		mInDrag = -1;
		this->ClickUp(x, y, mIsControlClick ? 1 : button);
	}
	--mInMouseHandler;
}

void		XWin::EventButtonMove(int x, int y								 )
{
	mLastX = x;
	mLastY = y;
	ManageToolTipForMouse(x,y);

	// Drag and move are treated basically the same - we consider
	// whether WE have a button, not what the OS says.  This prevents
	// a ton of multiple-buttons-down-at-once bugs.
	if(mInDrag >= 0)
	{
		this->ClickDrag(x, y, mIsControlClick ? 1 : mInDrag);
		if(mWantFakeUp)
		{
			DebugAssert(mInMouseHandler==0);
			++mInMouseHandler;
			this->ClickUp(x, y, mIsControlClick ? 1 : mInDrag);
			mInDrag = -1;
			mWantFakeUp = 0;
			--mInMouseHandler;
		}
	}
	else
	{
		this->ClickMove(x, y);
	}
}

void		XWin::ManageToolTipForMouse(int x, int y)
{
	// This routine detect the case where the mouse has moved out of the
	// dynamic rect that is our tool tip.  If this happens, we go NUKE
	// all tool tips and our dynamic string memory and re-build the
	// tool-tip catcher.  The tool-tip catcher, having been rebuilt,
	// will then be re-queried when it's time to get a new tool tip,
	// giving us a chance to go grab a new dynamic rect.
	if(mToolTipLive)
		if(x < mToolTipBounds[0] ||
		   y < mToolTipBounds[1] ||
		   x >= mToolTipBounds[2] ||
		   y >= mToolTipBounds[3])
	{
		mToolTipLive = 0;
		NSView * view = [mWindow contentView];
		[view removeAllToolTips];
		[view addToolTipRect:[view frame] owner:view userData:NULL];

		NSString * r = (NSString *) mToolTipMem;
		if(r) [r release];
		mToolTipMem = NULL;
	}
}

void	XWin::DoMacDragAndDrop(int item_count,
								  void *	items[],
								  int		mac_drag_type)
{
	// On input we get an array of NSDraggingItems, each with a retain count for us to consume.
	// So first, build the drag data and release the refs.
	
	mCurrentDragOps = mac_drag_type;
	
	NSView * v = [mWindow contentView];
	
	NSMutableArray * a = [NSMutableArray arrayWithCapacity:item_count];
	for(int i = 0; i < item_count; ++i)
	{
		NSDraggingItem * item = (NSDraggingItem *) items[i];
		[a addObject:item];
		[item release];
	}
	
	// This launches the drag & drop session - note that this is async!
	
	mInDragOp = 1;
	[v beginDraggingSessionWithItems:a event:[[NSApplication sharedApplication] currentEvent] source:mWindow];
	
	// This is a blocking while loop that will run th event queue modally until we pop out of the drag,
	// since DnD is supposed to act synchronous.
	
	while(mInDragOp == 1)
	{
		NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

		NSEvent * e = [[NSApplication sharedApplication] nextEventMatchingMask:NSAnyEventMask
																	 untilDate:[NSDate distantFuture]
																		inMode:NSEventTrackingRunLoopMode dequeue:TRUE];

		[[NSApplication sharedApplication] sendEvent:e];
		[pool drain];
	}
	mInDragOp = 0;

	// This will force an up click IMMEDIATELY after us, without a down click - needed to ensure the DND op doesn't
	// play out as a drag on some other widget.
	mWantFakeUp = 1;
}


#pragma mark -

xmenu XWin::GetMenuBar(void)
{
	NSMenu * mm = [NSApp mainMenu];
	if(mm == NULL)
	{
		mm = [[NSMenu alloc] initWithTitle:@"Menu Bar"];
		[NSApp setMainMenu:mm];
		[mm release];
		
		NSMenuItem * app_menu_item = [[NSMenuItem alloc] initWithTitle:@"Application" action:NULL keyEquivalent:@""];
		NSMenu * app_menu = [[NSMenu alloc] initWithTitle:@"Application"];
		[app_menu_item setSubmenu:app_menu];
		[mm addItem:app_menu_item];
		
		NSMenuItem * quit_item = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
		[app_menu addItem:quit_item];
		
		[quit_item setTarget:[NSApplication sharedApplication]];
		
		[app_menu_item release];
		[app_menu release];
		[quit_item release];
		
	}
	return mm;
}

xmenu	XWin::CreateMenu(xmenu parent, int item, const char * inTitle)
{
	NSMenu * p = (NSMenu *) parent;
	NSMenu * new_menu = [[NSMenu alloc] initWithTitle:[NSString stringWithUTF8String:inTitle]];

	NSMenuItem * new_item =
		[[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:inTitle] action:NULL keyEquivalent:@""];
	
	if(item == -1)
		[p addItem:new_item];
	else
		[p insertItem:new_item atIndex:item];
	
	[new_item setSubmenu:new_menu];

	[new_menu autorelease];
	[new_item release];
	
	return new_menu;

}

int		XWin::AppendMenuItem(xmenu in_menu, const char * inTitle)
{
	NSMenu * menu = (NSMenu *) in_menu;
	NSMenuItem * new_item =
		[[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:inTitle] action:NULL keyEquivalent:@""];
	
	[menu addItem:new_item];
	
	[new_item setTag:[menu numberOfItems] - 1];
	
	[new_item setRepresentedObject:menu];
	[new_item setAction:@selector(menuItemPicked:)];
	[new_item release];
	
	return [menu numberOfItems] - 1;
}

int		XWin::AppendSeparator(xmenu in_menu)
{
	NSMenu * menu = (NSMenu *) in_menu;
	[menu addItem:[NSMenuItem separatorItem]];
	return [menu numberOfItems] - 1;
}

void	XWin::CheckMenuItem(xmenu in_menu, int item, bool inCheck)
{
	NSMenu * menu = (NSMenu *) in_menu;
	[[menu itemAtIndex:item] setState:(inCheck ? 1 : 0)];
}

void	XWin::EnableMenuItem(xmenu in_menu, int item, bool inEnable)
{
	NSMenu * menu = (NSMenu *) in_menu;
	[[menu itemAtIndex:item] setEnabled:(inEnable ? 1 : 0)];
}

int	XWin::TrackPopupCommands(xmenu in_menu, int mouse_x, int mouse_y, int button, int current)
{
	// The menu coming in might have come frmo GUI, in which case its handlers will be set to
	// who-the-hell-knows-what - probably the app delegate.
	// We reset its target to us, so that we can intercept the menu pick and return it to the user.
	NSMenu * menu = (NSMenu *) in_menu;
	[menu setDelegate:NULL];
	NSArray<NSMenuItem *> * items = [menu itemArray];
	for(int i = 0; i < [items count]; ++i)
	{
		NSMenuItem * item = [items objectAtIndex:i];
		[item setTarget:mWindow];
		[item setRepresentedObject:menu];
		[item setAction:@selector(menuItemPicked:)];
	}

	NSEvent * event = [NSApp currentEvent];
	mInPopup = 1;
	mPopupPick = -1;
	
	// The popup menu handler will -eat- the up event when the popup menu is released.  This has the unfortunate result of leaving
	// GUI's mouse tracking "dangling" with a current mouse-focus object.
	//
	// To avoid making EVERY pane have to cope with this, we set a flag indicating that an outstanding up event has not yet been
	// dispatched.  When we next get some other kind of mouse event, we'll fake an up click to end the previous dangling gesture.
	mWantFakeUp = 1;

	NSPoint pop_pt;
	pop_pt.x = mouse_x;
	pop_pt.y = [[mWindow contentView] frame].size.height - mouse_y;

	if(current == -1) current = 0;	// We need this in case we pop up a meun like equipment type where legitimately no one can be selected, but 'none' is not a choice.

	BOOL result = [menu popUpMenuPositioningItem:[[menu itemArray] objectAtIndex:current] atLocation:pop_pt inView:[mWindow contentView]];

	mInPopup = 0;
	return result ? mPopupPick : -1;
}

