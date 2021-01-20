/*
 * Copyright (c) 2020, Laminar Research.
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

#ifndef WED_MapPreviewWindow_H
#define WED_MapPreviewWindow_H

#include "GUI_Commander.h"
#include "GUI_Listener.h"
#include "GUI_Window.h"
#include "IDocPrefs.h"

class GUI_Commander;
class IDocPrefs;
class WED_Document;
class WED_MapPreviewPane;

class WED_MapPreviewWindow : public GUI_Window
{
public:
	WED_MapPreviewWindow(GUI_Commander * documentWindowCmdr, WED_Document * document);

	int	HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags) override;
	int	HandleCommand(int command) override;

	bool Closed() override;

	bool ShouldDeferKeypress() override { return false; }

	void FromPrefs(IDocPrefs * prefs);
	void ToPrefs(IDocPrefs * prefs);

	WED_MapPreviewPane * MapPreviewPane() { return mMapPreviewPane; }

private:
	WED_MapPreviewPane * mMapPreviewPane;
	GUI_Commander * mDocumentWindowCmdr;
};

#endif
