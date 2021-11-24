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

#ifndef WED_MapPreviewPane_H
#define WED_MapPreviewPane_H

#include "CompGeomDefs3.h"
#include "GUI_Button.h"
#include "GUI_Commander.h"
#include "GUI_Listener.h"
#include "GUI_Pane.h"
#include "WED_MapZoomerNew.h"
#include "WED_PerspectiveCamera.h"

class IDocPrefs;
class WED_Document;
class WED_PreviewLayer;

class WED_MapPreviewPane : public GUI_Pane, public WED_MapZoomerNew, public GUI_Listener, public GUI_Commander {
public:

	WED_MapPreviewPane(GUI_Commander * cmdr, WED_Document * document);
	~WED_MapPreviewPane();

	void				SetBounds(int inBounds[4]) override;

	void				Draw(GUI_GraphState * state) override;

	void				ReceiveMessage(GUI_Broadcaster * inSrc,	intptr_t inMsg, intptr_t inParam) override;

	int					MouseDown(int x, int y, int button) override;
	void				MouseDrag(int x, int y, int button) override;
	void				MouseUp(int x, int y, int button) override;
	int					ScrollWheel(int x, int y, int dist, int axis) override;

	int					HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags) override;
	int					HandleCommand(int command) override;
	int					AcceptTakeFocus() override { return 1; }
	bool				ShouldDeferKeypress() override { return false; }

	void				DisplayExtent(const Bbox2& extent, double relativeDistance);
	Point2				CameraPositionLL() const;

	void				FromPrefs(IDocPrefs * prefs);
	void				ToPrefs(IDocPrefs * prefs);

private:
	void				StartMoving();
	void				HandleKeyMove();
	void				MoveCameraToXYZ(const Point3& xyz);
	void				SetForwardVector();
	void				UpdateMapVisibleArea();
	void				InitGL(int *b);
	void				FinishGL();

	WED_Document * mDocument;
	WED_PreviewLayer *	mPreviewLayer;
	WED_PerspectiveCamera mCamera;
	float mYaw, mPitch;
	float mX, mY;


	int mCameraLeftVk, mCameraRightVk, mCameraUpVk, mCameraDownVk, mCameraForwardVk, mCameraBackVk;
	GUI_KeyFlags mCameraFastModifier, mCameraSlowModifier;

	// When did the last drag event happen?
	float mTimeLastDrag;

	// Velocity with which we're dragging, in world coordinates.
	Vector3 mDragVelocity;

	// Is the camera moving?
	bool mMoving = false;

	// When did we last move the camera?
	float mTimeLastMove = 0.f;

	// Camera velocity in camera space.
	Vector3 mVelocity;
};

#endif
