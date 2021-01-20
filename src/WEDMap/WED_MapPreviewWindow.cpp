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

#include "WED_MapPreviewWindow.h"

#include "IDocPrefs.h"
#include "WED_Document.h"
#include "WED_MapPreviewPane.h"
#include "WED_Menus.h"
#include "WED_Messages.h"

static int bounds_default[4] = { 0, 0, 1024, 768 };

WED_MapPreviewWindow::WED_MapPreviewWindow(GUI_Commander * documentWindowCmdr, WED_Document * document)
	: GUI_Window("", xwin_style_resizable|xwin_style_centered|xwin_style_popup, bounds_default, nullptr),
	mMapPreviewPane(new WED_MapPreviewPane(this, document)), mDocumentWindowCmdr(documentWindowCmdr)
{
	SetDescriptor(std::string("3D preview - ") + document->GetFilePath());

	mMapPreviewPane->SetParent(this);
	mMapPreviewPane->Show();
	mMapPreviewPane->SetSticky(1, 1, 1, 1);
	int		win_b[4];
	GUI_Pane::GetBounds(win_b);
	mMapPreviewPane->SetBounds(win_b);
}

int	WED_MapPreviewWindow::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	return mMapPreviewPane->HandleKeyPress(inKey, inVK, inFlags);
}

int	WED_MapPreviewWindow::HandleCommand(int command)
{
	// We forward a few specific commands that are appropriate for this window to be
	// handled by the document window. All other commands should not be active while
	// this window has the focus as there would be a risk of the user making changes to
	// the scenery without being aware of it.
	switch (command)
	{
	case wed_TogglePreviewWindow:
	case wed_ShowMapAreaInPreviewWindow:
	case wed_CenterMapOnPreviewCamera:
		return mDocumentWindowCmdr->DispatchHandleCommand(command);
	default:
		return 0;
	}
}

bool WED_MapPreviewWindow::Closed()
{
	// Instead of actually closing, we just hide the window.
	// This will allow it to be re-opened later without needing to recreate the window again.
	Hide();
	return false;
}

void WED_MapPreviewWindow::FromPrefs(IDocPrefs * prefs)
{
	int x_loc, y_loc, width, height;
	XWin::GetWindowLoc(&x_loc, &y_loc);
	XWin::GetBounds(&width, &height);

	x_loc = prefs->ReadIntPref("map_preview_window/x_loc", x_loc);
	y_loc = prefs->ReadIntPref("map_preview_window/y_loc", y_loc);
	width = prefs->ReadIntPref("map_preview_window/width", width);
	height = prefs->ReadIntPref("map_preview_window/height", height);

	SetBoundsSafe(x_loc, y_loc, x_loc + width, y_loc + height);

	int visible = prefs->ReadIntPref("map_preview_window/visible", 0);
	if (visible)
		Show();
}

void WED_MapPreviewWindow::ToPrefs(IDocPrefs * prefs)
{
	int x_loc, y_loc, width, height;
	XWin::GetWindowLoc(&x_loc, &y_loc);
	XWin::GetBounds(&width, &height);

	prefs->WriteIntPref("map_preview_window/x_loc", x_loc);
	prefs->WriteIntPref("map_preview_window/y_loc", y_loc);
	prefs->WriteIntPref("map_preview_window/width", width);
	prefs->WriteIntPref("map_preview_window/height", height);
	prefs->WriteIntPref("map_preview_window/visible", IsVisible() ? 1 : 0);
}