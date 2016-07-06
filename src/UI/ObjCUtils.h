//
//  ObjCUtils.h
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/1/15.
//
//

#ifndef ObjCUtils_h
#define ObjCUtils_h



#if APL

#ifdef __cplusplus
extern "C" {
#endif

	// An app can sigin up to receive C callbacks from the NSApplication's delegate.  Info is a ptr passed to each one.
	struct app_callbacks {
		void *		info;
		void (*		menu_item_pick)(void * info, int cmd);
		void (*		menu_item_update)(void * info, int cmd, char * io_name, int * io_check, int * io_enable);
		void  (*	try_quit)(void * info);				// Return 1 to allow quit, 0 to abort quitting.
	};
	
	// Call this first - it sets up a memory pool, etc.
	void init_ns_stuff();
	
	// For menu-based apps, pass in app callbacks and a NIB file for menu support.  Do this AFTER init but BEFORE menu calls.
	void set_delegate(const app_callbacks * cbs, const char * menu_nib_file);

	// Runs the app, returning at app quit.  Use in main()
	void run_app();
	void stop_app();

	// Blocks until the mouse moves or is released.
	int run_event_tracking_until_move_or_up();	// returns 1 if mouse still down

	// Force run_app to exit - call from a callback
	void quit_app();

	// Simple drawing
	void erase_a_rect(int x, int y, int w, int h);
	void draw_text(int x, int y, int w, int h, const char * msg);

	// Modifier-key-polling
	int has_optionkey();
	int has_shiftkey();
	int has_controlkey();
	int has_capslock();

	// Returns the height of teh content area of an NSWindow *
	int get_ns_window_height(void * window);
	
	// Given a list of C-string pasteboard types, registers the window to DND them
	void register_drag_types_for_window(void * window, int count, const char * types[]);

	// Access to default theme cursors
	void set_left_right_cursor();
	void set_up_down_cursor();
	void set_arrow_cursor();

	//------------
	// CLIPBOARD
	//------------
	const char *get_pasteboard_text_type();
	int			clipboard_has_type(const char * type);
	int			count_clipboard_formats();
	const char *get_nth_clipboard_format(int n);
	int			get_clipboard_data_size(const char * type);
	int			copy_data_of_type(const char * type, void * out_buffer, int max_size);
	void		clear_clipboard();
	void		add_data_of_type(const char * type, const void * data, int size);
	
	
	//------------
	// DRAG N DROP
	//------------
	int			count_drag_items(void * ns_dragging_info);
	int			nth_drag_item_has_type(void * ns_dragging_info, int n, const char * type);
	int			nth_drag_item_get_size(void * ns_dragging_info, int n, const char * type);
	int			nth_drag_item_get_data(void * ns_dragging_info, int n, const char * type, int size, void * data);

	int			get_drag_operations(void * ns_dragging_info);
	void		get_drag_location(void * ns_dragging_info, int * x, int * y);
	int			drag_has_option_key(void * ns_dragging_info);
	
	// This returns an NSDraggingItem* with a retain count of 1 - something has to release it to avoid a leak.
	void *		create_drag_item_with_data(int type_count, const char * types[], int sizes[], const void * data[], const int bounds[4]);
	
	//------------
	// MENU BAR
	//------------
	void *		get_menu_bar();
	void *		create_menu(const char * title, void * parent, int parent_item);
	void		add_menu_item(void * menu,
							const char * title,
							int			cmd,
							int			checked,
							int			enabled,
							const char *shortcut,
							int			flags);
	void		add_separator(void * menu);
	void		clear_menu(void * menu);

	
#ifdef __cplusplus
}
#endif

#endif


#endif /* ObjCUtils_h */
