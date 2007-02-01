#ifndef GUI_MESSAGES_H
#define GUI_MESSAGES_H

enum {

	GUI_CONTROL_VALUE_CHANGED = 1000,
	GUI_SCROLL_CONTENT_SIZE_CHANGED,				// Sent by scroll pain when number of pixels in the content has changed

	GUI_TABLE_SHAPE_RESIZED,						// Column sizes have changed (sent by geometry)
	GUI_TABLE_CONTENT_RESIZED,						// Number of rows changed (sent by content - note - geometry MUST "know" the new size BEFORE this is sent!
	GUI_TABLE_CONTENT_CHANGED,						// Content of table changed but geometry the same (sent by content)

	GUI_APP_MESSAGES = 2000

};

#endif
