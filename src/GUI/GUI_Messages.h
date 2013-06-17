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

#ifndef GUI_MESSAGES_H
#define GUI_MESSAGES_H

enum {

	GUI_CONTROL_VALUE_CHANGED = 1000,
	GUI_SCROLL_CONTENT_SIZE_CHANGED,				// Sent by scroll pane when number of pixels in the content has changed
	GUI_TEXT_FIELD_TEXT_CHANGED,
	GUI_FILTER_FIELD_CHANGED,

	GUI_TABLE_SHAPE_RESIZED,						// Column sizes have changed (sent by geometry)
	GUI_TABLE_CONTENT_RESIZED,						// Number of rows changed (sent by content - note - geometry MUST "know" the new size BEFORE this is sent!
	GUI_TABLE_CONTENT_CHANGED,						// Content of table changed but geometry the same (sent by content)
	GUI_FILTER_MENU_CHANGED,
	GUI_APP_MESSAGES = 2000

};

#endif
