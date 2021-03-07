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

#include "WED_MapPreviewPane.h"

#include <proj_api.h>
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_Prefs.h"
#include "IGIS.h"
#include "IDocPrefs.h"
#include "MathUtils.h"
#include "WED_Colors.h"
#include "WED_Document.h"
#include "WED_FacadePreview.h"
#include "WED_Menus.h"
#include "WED_Messages.h"
#include "WED_PerspectiveCamera.h"
#include "WED_PreviewLayer.h"
#include "WED_Thing.h"
#include "WED_ToolUtils.h"

#include <cmath>
#include <limits>
#include <stdlib.h>
#include <time.h>

#if APL
#include <OpenGL/gl.h>
#else
#include "glew.h"
#endif

// display Frames Per Second. Will peg CPU/GPU load at 100%, only useable for diaganostic purposes.
#define SHOW_FPS 0

static constexpr double DRAW_DISTANCE = 50e3;

struct DrawVisStats
{
	int numCullTests = 0;
	int numCulled = 0;
	int numSizeTests = 0;
	int numTooSmall = 0;
	int numSizeTestsComposite = 0;
	int numTooSmallComposite = 0;
};

static void DrawVisFor(WED_MapLayer * layer, const Bbox2& bounds, const WED_MapZoomerNew& zoomer, IGISEntity * what, GUI_GraphState * g, int depth, DrawVisStats * stats)
{
	const float TOO_SMALL_TO_GO_IN = 20.0;
	const float MIN_PIXELS_TO_DRAW = 5.0;

#if 0
	if (!layer->IsVisibleNow(what))	return;

	Bbox3 bboxLL = what->GetVisibleBounds();
	Point2 p1 = projection.LLToXY(Point2(bboxLL.xmin(), bboxLL.ymin()));
	Point2 p2 = projection.LLToXY(Point2(bboxLL.xmax(), bboxLL.ymax()));
	Bbox3 bboxXY(p1.x(), p1.y(), bboxLL.zmin(), p2.x(), p2.y(), bboxLL.zmax());

#if DEV
	++stats->numCullTests;
#endif
	if (!camera.BboxVisible(bboxXY))
	{
#if DEV
		++stats->numCulled;
#endif
		return;
	}

	// TODO: Clean up
#if DEV
	++stats->numSizeTests;
#endif
	if (camera.PixelSize(bboxXY) < MIN_PIXELS_TO_DRAW)
	{
#if DEV
		++stats->numTooSmall;
#endif
		return;
	}

	IGISComposite * c;

	if (layer->DrawEntityVisualization(false, what, g, false))
		if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
		{
#if DEV
			++stats->numSizeTestsComposite;
#endif
			if (camera.PixelSize(bboxXY) > TOO_SMALL_TO_GO_IN || (p1 == p2) || depth == 0)	// Why p1 == p2?  If the composite contains ONLY ONE POINT it is zero-size.  We'd LOD out.  But if
			{																				// it contains one thing then we might as well ALWAYS draw it - it's relatively cheap!
				int t = c->GetNumEntities();												// Depth == 0 means we draw ALL top level objects -- good for airports.
				for (int n = t - 1; n >= 0; --n)
					DrawVisFor(layer, projection, camera, c->GetNthEntity(n), g, depth + 1, stats);
			}
			else
			{
#if DEV
				++stats->numTooSmallComposite;
#endif
			}
		}
#else
#if DEV
	++stats->numCullTests;
#endif
	if(!what->Cull(bounds))
	{
#if DEV
		++stats->numCulled;
#endif
		return;
	}

	IGISComposite * c;

	if (layer->DrawEntityVisualization(false, what, g, false))
		if (what->GetGISClass() == gis_Composite && (c = SAFE_CAST(IGISComposite, what)) != NULL)
		{
#if DEV
			++stats->numSizeTestsComposite;
#endif
			Bbox2 bboxLL;
			what->GetBounds(gis_Geo, bboxLL);
			Point2 p1 = zoomer.LLToPixel(bboxLL.p1);
			Point2 p2 = zoomer.LLToPixel(bboxLL.p2);
			if (zoomer.PixelSize(bboxLL) > TOO_SMALL_TO_GO_IN || (p1 == p2) || depth == 0)	// Why p1 == p2?  If the composite contains ONLY ONE POINT it is zero-size.  We'd LOD out.  But if
			{																				// it contains one thing then we might as well ALWAYS draw it - it's relatively cheap!
				int t = c->GetNumEntities();												// Depth == 0 means we draw ALL top level objects -- good for airports.
				for (int n = t - 1; n >= 0; --n)
					DrawVisFor(layer, bounds, zoomer, c->GetNthEntity(n), g, depth + 1, stats);
			}
			else
			{
#if DEV
				++stats->numTooSmallComposite;
#endif
			}
		}
#endif

}

static int GetVkPref(const char * key, int defaultVk)
{
	int vk = atoi(GUI_GetPrefString("preferences", key, ""));
	if (vk == 0)
		return defaultVk;
	else
		return vk;
}

static GUI_KeyFlags GetModifierPref(const char * key, GUI_KeyFlags defaultModifier)
{
	string val = GUI_GetPrefString("preferences", key, "");
	if (val == "Shift")
		return gui_ShiftFlag;
	if (val == "Control")
		return gui_ControlFlag;
	if (val == "OptionAlt")
		return gui_OptionAltFlag;
	return defaultModifier;
}

WED_MapPreviewPane::WED_MapPreviewPane(GUI_Commander * cmdr, WED_Document * document)
	: GUI_Commander(cmdr),
	  mDocument(document),
//	  mCamera(std::make_unique<WED_PerspectiveCamera>(0.5, DRAW_DISTANCE)),
	  mYaw(0.f),
	  mPitch(-10.f)
{
	// We set up the WED_MapZoomerNew as follows:
	// - The "ppm" value is set to 1, so "pixels" correspond to meters.
	// - The "lat/lon center" position corresponds to the position of the camera
	// - The "pixel center" position is always at 0, 0
	// The latter two points together imply that the camera is always at x=0, y=0 in
	// OpenGL coordinates. This is important for floating-point precision: The closer
	// objects are to the camera, the more precision we need.

	mCamera = new WED_PerspectiveCamera(0.5, DRAW_DISTANCE);
	this->cam = mCamera;

	mPreviewLayer = new WED_PreviewLayer(this, this, document);

	this->SetPixelBounds(-1,-1,1,1);
	this->SetPPM(1.0);

	mDocument->GetArchive()->AddListener(this);

#if 0
	WED_PreviewLayer::Options options;
	options.clearDepthBuffer = false;
	options.drawFacadeWalls = true;
	options.drawFacadeOutline = false;
	options.drawSkeletonIfLineTooThin = false;
	mPreviewLayer->SetOptions(options);
#endif
	SetForwardVector();

	Bbox2 box;
	WED_Thing * wrl = WED_GetWorld(mDocument);
	IGISEntity * ent = dynamic_cast<IGISEntity *>(wrl);
	if (!ent) return;
	ent->GetBounds(gis_Geo, box);

	DisplayExtent(box, 0.5);

{
	Point3  p = mCamera->Position();
	Vector3 d = mCamera->Forward();

    printf("Cam pos %11.5lf %.5lf %.5lf dir %.5lf %.5lf %.5lf\n", p.x, p.y, p.z, d.dx, d.dy, d.dz);

	this->SetPixelBounds(-1,-1,1,1);
    printf("Zoom PPM %11.5lf Orig %11.5lf %11.5lf Unit %11.5lf %11.5lf\n", this->GetPPM(),
                    this->LonToXPixel(0), this->LatToYPixel(0),
                    this->LonToXPixel(1), this->LatToYPixel(1));
}

	mCameraLeftVk = GetVkPref("CameraLeftVk", GUI_VK_LEFT);
	mCameraRightVk = GetVkPref("CameraRightVk", GUI_VK_RIGHT);
	mCameraUpVk = GetVkPref("CameraUpVk", GUI_VK_UP);
	mCameraDownVk = GetVkPref("CameraDownVk", GUI_VK_DOWN);
	mCameraForwardVk = GetVkPref("CameraForwardVk", GUI_VK_PERIOD);
	mCameraBackVk = GetVkPref("CameraBackVk", GUI_VK_COMMA);
	mCameraFastModifier = GetModifierPref("CameraFastModifier", gui_ShiftFlag);
	mCameraSlowModifier = GetModifierPref("CameraSlowModifier", gui_ControlFlag);
}

WED_MapPreviewPane::~WED_MapPreviewPane()
{
}

void WED_MapPreviewPane::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);

	if (inBounds[0] == inBounds[2] || inBounds[1] == inBounds[3])
		return;

	mCamera->SetFOV(45.0, inBounds[2] - inBounds[0], inBounds[3] - inBounds[1]);
}

void WED_MapPreviewPane::Draw(GUI_GraphState * state)
{
	IGISEntity * base = dynamic_cast<IGISEntity *>(WED_GetWorld(mDocument));
	if (!base)
		return;

	HandleKeyMove();

	int b[4];
	GetBounds(b);

	InitGL(b);

	Bbox2 b_geo;
	this->GetMapVisibleBounds(b_geo.p1.x_, b_geo.p1.y_, b_geo.p2.x_, b_geo.p2.y_);

	DrawVisStats stats;
	DrawVisFor(mPreviewLayer, b_geo, *this, base, state, 0, &stats);

#if 1
{
	Point3  p = mCamera->Position();
	Vector3 d = mCamera->Forward();

    printf("Cam pos %11.5lf %.5lf %.5lf dir %.5lf %.5lf %.5lf\n", p.x, p.y, p.z, d.dx, d.dy, d.dz);

    printf("Zoom PPM %11.5lf Orig %11.5lf %11.5lf Unit %11.5lf %11.5lf\n", this->GetPPM(),
                    this->LonToXPixel(0), this->LatToYPixel(0),
                    this->LonToXPixel(1), this->LatToYPixel(1));

}
#if DEV
	printf("%d / %d culled, %d / %d too small, %d / %d too small (composite)\n",
		stats.numCulled, stats.numCullTests, stats.numTooSmall, stats.numSizeTests,
		stats.numTooSmallComposite, stats.numSizeTestsComposite);
#endif
#endif

	// Draw the sky.
	glClearColor(0.6, 0.6, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	state->EnableDepth(true, true);         // turn on z-buffering - otherwise we can't clear the z-buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	// Draw a ground plane.
	state->SetState(false, 0, false, false, true, true, true);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.f, 1.f);
	glColor4f(0.2, 0.4, 0.2, 1.0);
	// We don't strictly need this, as we always keep the camera at the origin,
	// but we'll do it anyway in case we ever decide to break that invariant.
	Point3 position = mCamera->Position();
	glFrontFace(GL_CCW);
	glBegin(GL_QUADS);
	glVertex2f(position.x - DRAW_DISTANCE, position.y - DRAW_DISTANCE);
	glVertex2f(position.x + DRAW_DISTANCE, position.y - DRAW_DISTANCE);
	glVertex2f(position.x + DRAW_DISTANCE, position.y + DRAW_DISTANCE);
	glVertex2f(position.x - DRAW_DISTANCE, position.y + DRAW_DISTANCE);
	glEnd();
	glFrontFace(GL_CW);
	glDisable(GL_POLYGON_OFFSET_FILL);

	mPreviewLayer->DrawVisualization(false, state);

	FinishGL();

	// Draw "north" arrow
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(25, 25, 0);
	glRotatef(mYaw, 0, 0, 1);
	state->SetState(0, 0, 0, 1, 1, 0, 0);
	float * white = WED_Color_RGBA(wed_pure_white);
	glColor4fv(white);
	glBegin(GL_LINES);
	glVertex2i(0, -10);
	glVertex2i(0, 10);
	glVertex2i(-3, 7);
	glVertex2i(0, 10);
	glVertex2i(3, 7);
	glVertex2i(0, 10);
	glEnd();
	GUI_FontDraw(state, font_UI_Basic, white, 0, 12, "N", align_Center);
	glPopMatrix();

#if 0
#if DEV
	const FacadeStats& facade_stats = GetFacadeStats();
	printf("%d / %d walls big enough, %d / %d LODs drawn\n",
		facade_stats.numWallsBigEnough, facade_stats.numWallsTested,
		facade_stats.numLODsDrawn, facade_stats.numLODsTested);
	ResetFacadeStats();
#endif
#endif

#if SHOW_FPS
	static clock_t  last_time = 0;
	static float	fps = 0.0f;
	static int		cycle = 0;
	++cycle;
	if (cycle > 20)
	{
		clock_t now = clock();
		fps = (20.0 * CLOCKS_PER_SEC) / ((float)(now - last_time));
		last_time = now;
		cycle = 0;
	}
	char fpsText[100];
	sprintf(fpsText, "%6.1f FPS ", fps);

	GUI_FontDraw(state, font_UI_Basic, white, b[0] + 30, b[1] + 5, fpsText);
	Refresh();
#endif
}

void WED_MapPreviewPane::ReceiveMessage(GUI_Broadcaster * inSrc,	intptr_t inMsg, intptr_t inParam)
{
	if(inMsg == msg_ArchiveChanged || inMsg == msg_ArchiveChangedEphemerally)
		Refresh();
}

int	WED_MapPreviewPane::MouseDown(int x, int y, int button)
{
	if (!IsFocused())
		TakeFocus();

	mX = x;
	mY = y;

	return 1;
}

void WED_MapPreviewPane::MouseDrag(int x, int y, int button)
{
	if (button == 1)
	{
		float dx = x - mX;
		float dy = y - mY;
		mX = x;
		mY = y;
		mYaw += dx * 0.2;
		mPitch += dy * 0.2;

		mPitch = fltlim(mPitch, -85, 85);
		mYaw = fltwrap(mYaw, -180, 180);

		SetForwardVector();
		Refresh();
	}
}

int WED_MapPreviewPane::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (inVK == mCameraLeftVk || inVK == mCameraRightVk || inVK == mCameraUpVk || inVK == mCameraDownVk ||
		inVK == mCameraForwardVk || inVK == mCameraBackVk)
	{
		StartMoving();
		return 1;
	}

	return 0;
}

int WED_MapPreviewPane::HandleCommand(int command)
{
	// Some commands have shortcuts that conflict with default movement commands (e.g. Ctrl+Up
	// for "move upwards slowly"), so check whether one of the movement keys is pressed.
	if (IsKeyPressedNow(mCameraLeftVk) |
		IsKeyPressedNow(mCameraRightVk) |
		IsKeyPressedNow(mCameraUpVk) |
		IsKeyPressedNow(mCameraDownVk) |
		IsKeyPressedNow(mCameraForwardVk) |
		IsKeyPressedNow(mCameraBackVk))
	{
		StartMoving();
		return 1;
	}
	return 0;
}

void WED_MapPreviewPane::DisplayExtent(const Bbox2& extent, double relativeDistance)
{
	if (extent.is_empty() || extent.is_null()) return;

	Point2 centerLL = extent.centroid();

	this->SetMapLogicalBounds(extent.p1.x(), extent.p1.y(), extent.p2.x(), extent.p2.y());

	Point2 p1XY = this->LLToPixel(extent.p1);
	Point2 p2XY = this->LLToPixel(extent.p2);

	Vector3 forwardWithoutZ = mCamera->Forward();
	forwardWithoutZ.dz = 0;
	forwardWithoutZ.normalize();

	double distance = sqrt(Vector2(p1XY, p2XY).squared_length()) * relativeDistance;

	MoveCameraToXYZ(Point3(0, 0, distance * 0.2) - forwardWithoutZ * distance);

	mPitch = -10.f;
	SetForwardVector();

	Refresh();
}

Point2 WED_MapPreviewPane::CameraPositionLL() const
{
	Point3 position = mCamera->Position();
	return this->PixelToLL(Point2(position.x, position.y));
}

void WED_MapPreviewPane::FromPrefs(IDocPrefs * prefs)
{
	const double qnan = std::numeric_limits<double>::quiet_NaN();

	double camera_lon = prefs->ReadDoublePref("map_preview_window/camera_lon", qnan);
	double camera_lat = prefs->ReadDoublePref("map_preview_window/camera_lat", qnan);
	double camera_agl = prefs->ReadDoublePref("map_preview_window/camera_agl", qnan);
	double camera_yaw = prefs->ReadDoublePref("map_preview_window/camera_yaw", qnan);
	double camera_pitch = prefs->ReadDoublePref("map_preview_window/camera_pitch", qnan);

	if (!std::isnan(camera_lon) && !std::isnan(camera_lat) && !std::isnan(camera_agl) && !std::isnan(camera_yaw) && !std::isnan(camera_pitch))
	{
		this->SetMapLogicalBounds(camera_lon, camera_lat, camera_lon, camera_lat);

		mCamera->MoveTo(Point3(0, 0, camera_agl));

		mYaw = camera_yaw;
		mPitch = camera_pitch;
		SetForwardVector();
	}
	else
	{
		WED_Thing * wrl = WED_GetWorld(mDocument);
		IGISEntity * ent = dynamic_cast<IGISEntity *>(wrl);
		Bbox2 box(-180, -90, 180, 90);
		if (ent)
			ent->GetBounds(gis_Geo, box);

		box.p1.x_ = prefs->ReadDoublePref("map/west", box.p1.x_);
		box.p1.y_ = prefs->ReadDoublePref("map/south", box.p1.y_);
		box.p2.x_ = prefs->ReadDoublePref("map/east", box.p2.x_);
		box.p2.y_ = prefs->ReadDoublePref("map/north", box.p2.y_);

		DisplayExtent(box, 0.5);
	}
}

void WED_MapPreviewPane::ToPrefs(IDocPrefs * prefs)
{
	Point3 position = mCamera->Position();
	Point2 positionLL = this->PixelToLL(Point2(position.x, position.y));
	prefs->WriteDoublePref("map_preview_window/camera_lon", positionLL.x());
	prefs->WriteDoublePref("map_preview_window/camera_lat", positionLL.y());
	prefs->WriteDoublePref("map_preview_window/camera_agl", position.z);
	prefs->WriteDoublePref("map_preview_window/camera_yaw", mYaw);
	prefs->WriteDoublePref("map_preview_window/camera_pitch", mPitch);
}

void WED_MapPreviewPane::StartMoving()
{
	if (!mMoving) {
		mMoving = true;
		mTimeLastMove = GetTimeNow();
		Refresh();
	}
}

void WED_MapPreviewPane::HandleKeyMove()
{
	if (!mMoving)
		return;

	GUI_KeyFlags flags = GetModifiersNow();
	double speed = 50.0; // m/s
	if (flags & mCameraFastModifier)
		speed *= 10.0;
	else if (flags & mCameraSlowModifier)
		speed *= 0.1;

	Vector3 desiredVelocity;
	if (IsKeyPressedNow(mCameraLeftVk))
		desiredVelocity.dx -= speed;
	if (IsKeyPressedNow(mCameraRightVk))
		desiredVelocity.dx += speed;
	if (IsKeyPressedNow(mCameraUpVk))
		desiredVelocity.dz += speed;
	if (IsKeyPressedNow(mCameraDownVk))
		desiredVelocity.dz -= speed;
	if (IsKeyPressedNow(mCameraForwardVk))
		desiredVelocity.dy += speed;
	if (IsKeyPressedNow(mCameraBackVk))
		desiredVelocity.dy -= speed;

	float mTime = GetTimeNow();
	double elapsedTime = mTime - mTimeLastMove;
	mTimeLastMove = mTime;

	constexpr double halfDecayTime = 0.1; // s
	mVelocity += (desiredVelocity - mVelocity) * (1.0 - pow(0.5, elapsedTime / halfDecayTime));

	// Velocity in XYZ space.
	Vector3 xyzVelocity =
		mCamera->Right() * mVelocity.dx +
		mCamera->Forward() * mVelocity.dy +
		mCamera->Up() * mVelocity.dz;

	// Limit the elapsed time so that we don't get any unexpectedly big jumps in position
	// when there is a big pause (maybe due to resource loading) that prevents refreshes
	// from happening regularly.
	Point3 position = mCamera->Position() + xyzVelocity * min(elapsedTime, 0.2);
	if (position.z < 1.5)
		position.z = 1.5;
	MoveCameraToXYZ(position);

	constexpr double minVelocity = 0.5; // m/s
	if (desiredVelocity != Vector3() || mVelocity.squared_length() >= minVelocity * minVelocity)
		Refresh();
	else
	{
		mVelocity = Vector3();
		mMoving = false;
	}
}

void WED_MapPreviewPane::MoveCameraToXYZ(const Point3& xyz)
{
	Point2 ll = this->PixelToLL({ xyz.x, xyz.y });

	this->SetMapLogicalBounds(ll.x(), ll.y(), ll.x(), ll.y());

	mCamera->MoveTo({ 0, 0, xyz.z });

	UpdateMapVisibleArea();
}

void WED_MapPreviewPane::SetForwardVector()
{
	Vector3 forward(sin(mYaw * DEG_TO_RAD) * cos(mPitch * DEG_TO_RAD), cos(mYaw * DEG_TO_RAD) * cos(mPitch * DEG_TO_RAD), sin(mPitch * DEG_TO_RAD));
	mCamera->SetForward(forward);
	UpdateMapVisibleArea();
}

void WED_MapPreviewPane::UpdateMapVisibleArea()
{
	// Project view frustum to the ground plane and find a bounding box around it.
	Bbox2 visArea;
	for (const auto& corner : mCamera->FrustumCorners())
		visArea += Point2(corner.x, corner.y);

	this->SetPixelBounds(visArea.xmin(), visArea.ymin(), visArea.xmax(), visArea.ymax());

	// SetPixelBounds() changes the "pixel center" position, so reset it to (0, 0).
	this->SetPixelCenter(0.0, 0.0);
}

void WED_MapPreviewPane::InitGL(int *b)
{
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(b[0], b[1], b[2] - b[0], b[3] - b[1]);

	// Set up lighting.
	GLfloat ambient_color[4] = { 2, 2, 2, 2 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_color);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, false);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	mCamera->ApplyProjectionMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	mCamera->ApplyModelViewMatrix();
}

void WED_MapPreviewPane::FinishGL()
{
	glDisable(GL_LIGHTING);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}
