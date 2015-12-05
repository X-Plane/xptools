//
//  ObjCUtils.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/1/15.
//
//

#include "ObjCUtils.h"
#import <AppKit/AppKit.h>
#include <CoreServices/CoreServices.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------
//  APP DELEGATE
//------------------------------------------------------------------------------------------------------------------------------------------------
//
// We create our own app delegate.  It is used as a menu delegate to handle commands
// and app delegate for app-quit callbacks.

static app_callbacks	g_callbacks = { 0 };
static NSString *		s_nib_file = NULL;

@interface objc_delegate : NSObject <NSApplicationDelegate, NSMenuDelegate>

- (void) menu_picked:(id) sender;

@end

@implementation objc_delegate

- (NSInteger)numberOfItemsInMenu:(NSMenu *)menu
{
	// If we don't do this, updateItem won't be called.
	return [[menu itemArray] count];
}

- (BOOL)menu:(NSMenu*)menu updateItem:(NSMenuItem*)item atIndex:(NSInteger)index shouldCancel:(BOOL)shouldCancel
{
//	printf("Update: %s\n", [[item title] UTF8String]);
	if(shouldCancel)	return NO;
	if([item isSeparatorItem])
		return YES;
	if(g_callbacks.menu_item_update)
	{
		char buf[1024];
		int check = [item state];
		const char * old_name = [[item title] UTF8String];
		strncpy(buf,old_name, sizeof(buf));
		int enable = [item isEnabled];
		g_callbacks.menu_item_update(g_callbacks.info, [item tag], buf, &check, &enable);
		[item setTitle:[NSString stringWithUTF8String:buf]];
		[item setState: check ? 1 : 0];
		[item setEnabled:enable ? YES : NO];
	}
	return YES;
}

- (void) menu_picked:(id) sender
{
	NSMenuItem * menu = sender;
	int cmd_id = [menu tag];
	if(g_callbacks.menu_item_pick)
		g_callbacks.menu_item_pick(g_callbacks.info, cmd_id);
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	if(g_callbacks.can_quit)
		return g_callbacks.can_quit(g_callbacks.info) ? NSTerminateNow : NSTerminateCancel;
	return NSTerminateNow;
}


@end

static objc_delegate * s_delegate = NULL;


#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------------------
// INITIALIZATION
//------------------------------------------------------------------------------------------------------------------------------------------------


void init_ns_stuff()
{
	// Since we don't have main local scope, we just let the pool run forever.
	static NSAutoreleasePool * forever_pool = [[NSAutoreleasePool alloc] init];
	NSApplication * me = [NSApplication sharedApplication];
}

void set_delegate(const app_callbacks * cbs, const char * menu_nib_file)
{
	memcpy(&g_callbacks, cbs, sizeof(g_callbacks));
	
	if(menu_nib_file)
	{
		if(s_nib_file)
			[s_nib_file release];
		s_nib_file = [[NSString alloc] initWithUTF8String:menu_nib_file];
	}
	
	if(s_delegate == NULL)
	{
		s_delegate = [[objc_delegate alloc] init];
		[[NSApplication sharedApplication] setDelegate:s_delegate];
	}
}


void run_app()
{
	NSApplication * me = [NSApplication sharedApplication];
	[me run];
}

int run_event_tracking_until_move_or_up()
{
	bool done = false;
	while(!done)
	{
		NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

		NSEvent * e = [[NSApplication sharedApplication] nextEventMatchingMask:NSAnyEventMask
																	 untilDate:[NSDate distantFuture]
																		inMode:NSEventTrackingRunLoopMode dequeue:TRUE];
		
		NSEventType t = [e type];
		
		if(t >= NSLeftMouseDown && t <= NSMouseExited)
			done = true;
		if (t == NSOtherMouseDown || t == NSOtherMouseUp || t == NSOtherMouseDragged)
			done = true;
		
		if(t == NSLeftMouseUp || t == NSRightMouseUp || t == NSOtherMouseUp)
		{
			// If we found the up even that ends this, clients want to see it _after_ we return, so that they can NOT get an up from inside
			// their down event handler (since we are typically below the call-out for down clicks on the stack.
			// So instead of sending out the message synchronously, we put it back on the event Q and return.
			// We might have been albe to also solve this by not using dequeue.
			[[NSApplication sharedApplication] postEvent:e atStart:YES];
			[pool drain];
			return 0;
		}
		
		[[NSApplication sharedApplication] sendEvent:e];
		[pool drain];
	}
	return 1;
}


void quit_app()
{
	[[NSApplication sharedApplication] terminate:NULL];
}

#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------------------
// BASIC WINDOWS, DRAWING AND UI ACCESSORS
//------------------------------------------------------------------------------------------------------------------------------------------------

void erase_a_rect(int x, int y, int w, int h)
{
	[[NSColor colorWithWhite:1.0 alpha:1.0] setFill];
	NSRect r = NSMakeRect(x, y, w, h);
	NSRectFill(r);
}

void draw_text(int x, int y, int w, int h, const char * msg)
{
	NSRect r = NSMakeRect(x, y, w, h);
	NSString * s = [NSString stringWithUTF8String:msg];
	[s drawWithRect:r options:NSStringDrawingUsesLineFragmentOrigin attributes:NULL context:NULL];
}

int has_optionkey()
{
	return ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0;
}

int has_shiftkey()
{
	return ([NSEvent modifierFlags] & NSShiftKeyMask) != 0;
}

int has_controlkey()
{
	return ([NSEvent modifierFlags] & (NSControlKeyMask | NSCommandKeyMask)) != 0;
}

int has_capslock()
{
	return ([NSEvent modifierFlags] & NSAlphaShiftKeyMask) != 0;
}

int get_ns_window_height(void * window)
{
	NSWindow * w = (NSWindow *) window;
	NSRect r = [w frame];
	r = [w contentRectForFrameRect:r];
	return r.size.height;
}

void register_drag_types_for_window(void * window, int count, const char * types[])
{
	NSMutableArray * a = [NSMutableArray arrayWithCapacity:count];
	for(int i = 0; i < count; ++i)
	{
		[a addObject:[NSString stringWithUTF8String:types[i]]];
	}
	
	NSWindow * w= (NSWindow *) window;
	[w registerForDraggedTypes:a];
}


void set_left_right_cursor()
{
	[[NSCursor resizeLeftRightCursor] set];
}

void set_up_down_cursor()
{
	[[NSCursor resizeUpDownCursor] set];
}

void set_arrow_cursor()
{
	[[NSCursor arrowCursor] set];
}

#pragma mark -
//---------------------------------------------------------------------------------------------------------
// PASTEBOARD
//---------------------------------------------------------------------------------------------------------

const char * get_pasteboard_text_type()
{
	return [NSPasteboardTypeString UTF8String];
}

int			clipboard_has_type(const char * type)
{
	NSPasteboard * pb = [NSPasteboard generalPasteboard];
	NSArray<NSPasteboardItem *> * items = [pb pasteboardItems];
	if ([items count] != 1)	return false;
	NSPasteboardItem * i = [items objectAtIndex:0];

	NSArray<NSString *>	* types = [i types];
	for(int i = 0; i < [types count]; ++i)
	{
		const char * the_type = [[types objectAtIndex:i] UTF8String];
		if(strcmp(type, the_type) == 0)
			return 1;
	}
	return 0;
}

int			count_clipboard_formats()
{
	NSPasteboard * pb = [NSPasteboard generalPasteboard];
	NSArray<NSPasteboardItem *>	* items = [pb pasteboardItems];
	if ([items count] != 1)	return 0;
	NSPasteboardItem * i = [items objectAtIndex:0];

	NSArray<NSString *>	* types = [i types];
	return [types count];
}

const char *get_nth_clipboard_format(int n)
{
	NSPasteboard * pb = [NSPasteboard generalPasteboard];
	NSArray<NSPasteboardItem *>	* items = [pb pasteboardItems];
	if ([items count] != 1)	return 0;
	NSPasteboardItem * i = [items objectAtIndex:0];

	NSArray<NSString *> * types = [i types];
	if(n < 0 || n >= [types count])
		return NULL;
	
	return [[types objectAtIndex:n] UTF8String];
}

int			get_clipboard_data_size(const char * type)
{
	NSPasteboard * pb = [NSPasteboard generalPasteboard];
	NSArray<NSPasteboardItem *>	* items = [pb pasteboardItems];
	if ([items count] != 1)	return 0;
	NSPasteboardItem * i = [items objectAtIndex:0];
	NSData * d = [i dataForType:[NSString stringWithUTF8String:type]];
	if(d == NULL) return 0;
	return [d length];
}

int			copy_data_of_type(const char * type, void * out_buffer, int max_size)
{
	NSPasteboard * pb = [NSPasteboard generalPasteboard];
	NSArray<NSPasteboardItem *>	* items = [pb pasteboardItems];
	if ([items count] != 1)	return 0;
	NSPasteboardItem * i = [items objectAtIndex:0];
	NSData * d = [i dataForType:[NSString stringWithUTF8String:type]];
	if ([d length] != max_size)
		return 0;
	[d getBytes:out_buffer length:max_size];
	return max_size;
}

void		clear_clipboard()
{
	NSPasteboard * pb = [NSPasteboard generalPasteboard];
	[pb clearContents];
}

void		add_data_of_type(const char * type, const void * data, int size)
{
	NSData * d = [NSData dataWithBytes:data length:size];
	NSPasteboard * pb = [NSPasteboard generalPasteboard];
	[pb setData:d forType:[NSString stringWithUTF8String:type]];
}

#pragma mark -
//---------------------------------------------------------------------------------------------------------
// DRAG AND DROP
//---------------------------------------------------------------------------------------------------------

// This is TOTALLY non-obvious: while the pasteboard can take any Pboard type, the paste board ITEMs take
// only UTIs.  This weirdo core services UT adapter layer changes the types on the fly, but this means that
// if we are doing type inspection on a PB item, we need to go get the ACTUAL UTI that the OS made up.
//
// This routine does that, while keeping public UTIs like public.utf8-plain-text alone.
static NSString * get_uti_for_type(const char * in_type)
{
	NSString * our_type = [NSString stringWithUTF8String:in_type];
	if(strcmp(in_type, [NSPasteboardTypeString UTF8String]) == 0)
	{
		return our_type;
	}
	else
	{
		CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(
								kUTTagClassNSPboardType,
								(CFStringRef) our_type,
								kUTTypeData);
		NSString * ret = [NSString stringWithString:(NSString *) uti];
		CFRelease(uti);
		return ret;
	}
}


int			count_drag_items(void * ns_dragging_info)
{
	id<NSDraggingInfo> info = (id<NSDraggingInfo>) ns_dragging_info;
	NSPasteboard * pb = [info draggingPasteboard];
	NSArray<NSPasteboardItem *> * pbi = [pb pasteboardItems];
	
	return [pbi count];
}

int			nth_drag_item_has_type(void * ns_dragging_info, int n, const char * type)
{
	id<NSDraggingInfo> info = (id<NSDraggingInfo>) ns_dragging_info;
	NSPasteboard * pb = [info draggingPasteboard];
	NSArray<NSPasteboardItem *> * pbi = [pb pasteboardItems];
	NSPasteboardItem * item = [pbi objectAtIndex:n];
	
	NSString * t = get_uti_for_type(type);
	
	if([item availableTypeFromArray:[NSArray arrayWithObject:t]] != NULL)
		return 1;
	return 0;
}

int			nth_drag_item_get_size(void * ns_dragging_info, int n, const char * type)
{
	id<NSDraggingInfo> info = (id<NSDraggingInfo>) ns_dragging_info;
	NSPasteboard * pb = [info draggingPasteboard];
	NSArray<NSPasteboardItem *> * pbi = [pb pasteboardItems];
	NSPasteboardItem * item = [pbi objectAtIndex:n];
	
	NSString * t = get_uti_for_type(type);
	
	NSData * data = [item dataForType:t];
	if(data == NULL) return 0;
	
	return [data length];
}

int		nth_drag_item_get_data(void * ns_dragging_info, int n, const char * type, int size, void * buffer)
{
	id<NSDraggingInfo> info = (id<NSDraggingInfo>) ns_dragging_info;
	NSPasteboard * pb = [info draggingPasteboard];
	NSArray<NSPasteboardItem *> * pbi = [pb pasteboardItems];
	NSPasteboardItem * item = [pbi objectAtIndex:n];
	
	NSString * t = get_uti_for_type(type);
	
	NSData * data = [item dataForType:t];
	if(data == NULL) return 0;
	
	if(size != [data length]) return 0;

	[data getBytes:buffer length:size];
	
	return size;
}

int			get_drag_operations(void * ns_dragging_info)
{
	id<NSDraggingInfo> info = (id<NSDraggingInfo>) ns_dragging_info;
	return [info draggingSourceOperationMask];
	
}

void		get_drag_location(void * ns_dragging_info, int * x, int * y)
{
	id<NSDraggingInfo> info = (id<NSDraggingInfo>) ns_dragging_info;
	NSPoint p = [info draggingLocation];
	*x = p.x;
	*y = p.y;
}

int			drag_has_option_key(void * ns_dragging_info)
{
	id<NSDraggingInfo> info = (id<NSDraggingInfo>) ns_dragging_info;
	return 0;
}

		
void *		create_drag_item_with_data(int tc, const char * types[], int sizes[], const void * data[], const int bounds[4])
{
	NSPasteboardItem * pb = [[NSPasteboardItem alloc] init];
	
	for(int i = 0; i < tc; ++i)
	{
		NSString * our_type = get_uti_for_type(types[i]);
		[pb setData:[NSData dataWithBytes:data[i] length:sizes[i]] forType:our_type];
	}
//	Debug code for UTIs
//	printf("--created--\n");
//	
//	NSArray<NSString *> * tl = [pb types];
//	for(int i = 0; i < [tl count]; ++i)
//	{
//		printf("%d %s\n", i, [[tl objectAtIndex:i] UTF8String]);
//	}
//	printf("--created--\n");
	
	
	NSDraggingItem * item = [[NSDraggingItem alloc] initWithPasteboardWriter:pb];
	[pb release];
	
	NSImage * image = [[NSImage alloc] initWithSize:NSMakeSize(bounds[2]-bounds[0], bounds[3]-bounds[1])];
	[image lockFocus];
	
	[[NSColor colorWithWhite:0.5 alpha:1.0] setFill];
	NSRect r = NSMakeRect(0, 0, bounds[2]-bounds[0], bounds[3]-bounds[1]);
	NSFrameRectWithWidth(r, 2);
	
	[image unlockFocus];
	
	
	[item setDraggingFrame:NSMakeRect(bounds[0], bounds[1], bounds[2]-bounds[0],bounds[3]-bounds[1]) contents:image];
	[image release];
	return item;
}

#pragma mark -
//---------------------------------------------------------------------------------------------------------
// MENUS
//---------------------------------------------------------------------------------------------------------

void *		get_menu_bar()
{
	NSApplication * app = [NSApplication sharedApplication];
	NSMenu * main_bar = [app mainMenu];
	if(main_bar == NULL)
	{
		if(s_nib_file)
		{
			NSNib *mainNib = [[NSNib alloc] initWithNibNamed:s_nib_file bundle:[NSBundle mainBundle]];
			[mainNib instantiateNibWithOwner:app topLevelObjects:nil];

			main_bar = [app mainMenu];
			NSMenu * app_menu = [[main_bar itemAtIndex:0] submenu];

			[app_menu setDelegate:s_delegate];
			[[app_menu itemWithTag:1000] setAction:@selector(menu_picked:)];
			[[app_menu itemWithTag:1001] setAction:@selector(menu_picked:)];

		}
		else
		{
			main_bar = [[[NSMenu alloc] initWithTitle:@"Menu bar"] autorelease];
			[app setMainMenu:main_bar];
			[main_bar setDelegate:s_delegate];
		}
	}
	
	return main_bar;
}

void *		create_menu(const char * title, void * parent, int parent_item)
{
	NSMenu * new_menu = [[[NSMenu alloc] initWithTitle:[NSString stringWithUTF8String:title]] autorelease];
	[new_menu setAutoenablesItems:NO];
	[new_menu setDelegate:s_delegate];
	if(parent != NULL)
	{
		if(parent == [[NSApplication sharedApplication] mainMenu])
		{
			NSMenuItem * parent_item = [[[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:title] action:NULL keyEquivalent:@""] autorelease];
			[parent_item setSubmenu:new_menu];
			NSMenu * mbar = [[NSApplication sharedApplication] mainMenu];
			if(strcmp(title,"Help") == 0)
			[mbar insertItem:parent_item atIndex:[[mbar itemArray] count]];
			else
			[mbar insertItem:parent_item atIndex:[[mbar itemArray] count] - 1];
		}
		else
		{
			NSMenu * p = (NSMenu *) parent;
			NSMenuItem * item = [p itemAtIndex:parent_item];
			[item setSubmenu:new_menu];
		}
	}
	else
		[new_menu retain];
	return new_menu;
}

void		add_menu_item(void * menu,
						const char * title,
						int			cmd,
						int			checked,
						int			enabled,
						const char * shortcut,
						int			flags)
{
	NSMenu * m = (NSMenu *) menu;
	
	NSMenuItem * item = [[[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:title]
												    action:@selector(menu_picked:)
											 keyEquivalent:[NSString stringWithUTF8String:shortcut]] autorelease];
	
	int mods = 0;
	if(flags & 1)
		mods |= NSShiftKeyMask;
	if(flags & 2)
		mods |= NSAlternateKeyMask;
	if(flags & 4)
		mods |= NSCommandKeyMask;
	
	[item setKeyEquivalentModifierMask:mods];
	
	[m addItem:item];
	[item setTag:cmd];
	[item setState: checked ? 1 : 0];
	[item setEnabled:enabled ? YES : NO];
	
	
	[item setTarget:s_delegate];
}
						
void		add_separator(void * menu)
{
	NSMenu * m = (NSMenu *) menu;
	[m addItem:[NSMenuItem separatorItem]];
}

void		clear_menu(void * menu)
{
	NSMenu * m = (NSMenu *) menu;
	[m removeAllItems];
}


#ifdef __cplusplus
}
#endif

