/*
 * Copyright (c) 2021, Laminar Research.
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

#include "WED_PerspectiveCamera.h"

#include "XESConstants.h"

#if APL
#include <OpenGL/gl.h>
#else
#include "glew.h"
#endif

WED_PerspectiveCamera::WED_PerspectiveCamera(double nearClip, double farClip)
	: mNearClip(nearClip), mFarClip(farClip), mWidth(mNearClip), mHeight(mNearClip),
	mViewportWidth(1024), mViewportHeight(1024),
	mFrustumPlanesDirty(true)
{
	mXform.position = Point3(0, 0, 0);
	mXform.forward = Vector3(0, 1, 0);
	mXform.right = Vector3(1, 0, 0);
	mXform.up = Vector3(0, 0, 1);
}

void WED_PerspectiveCamera::MoveTo(const Point3& position)
{
	mXform.position = position;

	mFrustumPlanesDirty = true;
}

void WED_PerspectiveCamera::SetForward(const Vector3& forward)
{
	mXform.forward = forward;
	mXform.forward.normalize();
	mXform.right = mXform.forward.cross({ 0, 0, 1 });
	if (mXform.right.squared_length() < 1e-8)
		// mForward is almost collinear with (0, 0, 1), so do something slightly arbitrary to save the situation.
		mXform.right = mXform.forward.cross({ 1, 0, 0 });
	mXform.right.normalize();
	mXform.up = mXform.right.cross(mXform.forward);
	// Strictly, this normalization isn't needed, but we'll do it anyway.
	mXform.up.normalize();

	mFrustumPlanesDirty = true;
}

void WED_PerspectiveCamera::SetFOV(double horizontalFovDeg, double viewportWidth, double viewportHeight)
{
	mViewportWidth = viewportWidth;
	mViewportHeight = viewportHeight;

	if (viewportWidth == 0)
		return;

	double widthNormalized = 2 * sin(horizontalFovDeg * DEG_TO_RAD / 2);
	double heightNormalized = widthNormalized * viewportHeight / viewportWidth;

	mWidth = widthNormalized * mNearClip;
	mHeight = heightNormalized * mNearClip;

	mFrustumPlanesDirty = true;
}

void WED_PerspectiveCamera::ApplyProjectionMatrix()
{
	glLoadIdentity();
	glFrustum(-0.5 * mWidth, 0.5 * mWidth, -0.5 * mHeight, 0.5 * mHeight, mNearClip, mFarClip);
}

void WED_PerspectiveCamera::ApplyModelViewMatrix()
{
	double m[16] =
	{
		Right().dx, Right().dy, Right().dz, 0,
		Up().dx, Up().dy, Up().dz, 0,
		-Forward().dx, -Forward().dy, -Forward().dz, 0,
		0, 0, 0, 1
	};
	glLoadIdentity();
	glMultTransposeMatrixd(m);
	glTranslated(-Position().x, -Position().y, -Position().z);

	DebugAssert(ModelViewMatrixConsistent());
}

bool WED_PerspectiveCamera::PointVisible(const Point3& point) const
{
	UpdateFrustumPlanes();

	for (const auto& plane : mFrustumPlanes)
		if (!plane.on_normal_side(point))
			return false;

	return true;
}

bool WED_PerspectiveCamera::BboxVisible(const Bbox3& bbox) const
{
	UpdateFrustumPlanes();

	// For a bounding box to be invisible, all of its corners must be on the
	// non-visible side of at least one of the bounding planes.
	// This is a pretty simple test, but it's pretty expensive; optimize if
	// it turns out this is a bottleneck.

	for (const auto& plane : mFrustumPlanes) {
		bool visible = false;
		bbox.for_each_corner([&plane, &visible](const Point3& corner)
		{
			if (plane.on_normal_side(corner))
			{
				visible = true;
				return false;
			}
			return true;
		});
		if (!visible)
			return false;
	}

	return true;
}

double WED_PerspectiveCamera::PointDistance(const Point3& point) const
{
	return mXform.forward.dot(point - mXform.position);
}

double WED_PerspectiveCamera::PixelSize(double zCamera, double featureSize) const
{
	// If neede for performance, we could precompute some stuff so we only have
	// a single value that we divide by zCamera. Beyond that, we could turn this
	// into a "should this be visible" function that takes a minimum pixel size.
	// This would help allow us to avoid the division.
	return featureSize / zCamera * mNearClip / mWidth * mViewportWidth;
}

double WED_PerspectiveCamera::PixelSize(const Bbox3& bbox) const
{
	// This is a pretty crude approximation -- revisit if needed for performance.
	double diameter = sqrt((bbox.p2 - bbox.p1).squared_length());
	return PixelSize(bbox, diameter);
}

double WED_PerspectiveCamera::PixelSize(const Bbox3& bbox, double featureSize) const
{
	double zCameraMin = mFarClip;
	bbox.for_each_corner([this, &zCameraMin](const Point3 &corner)
	{
		double zCamera = PointDistance(corner);
		if (zCamera < zCameraMin)
			zCameraMin = zCamera;
		return true;
	});

	if (zCameraMin < mNearClip)
		zCameraMin = mNearClip;

	return PixelSize(zCameraMin, featureSize);
}

double WED_PerspectiveCamera::PixelSize(const Point3& position, double diameter) const
{
	// z coordinate of the point in camera coordinates.
	double zCamera = PointDistance(position);

	return PixelSize(zCamera, diameter);
}

void WED_PerspectiveCamera::PushMatrix()
{
	DebugAssert(ModelViewMatrixConsistent());

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	mXformStack.push_back(mXform);
}

void WED_PerspectiveCamera::PopMatrix()
{
	DebugAssert(ModelViewMatrixConsistent());

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	DebugAssert(!mXformStack.empty());
	if (mXformStack.empty()) return;
	mXform = mXformStack.back();
	mXformStack.pop_back();

	mFrustumPlanesDirty = true;

	DebugAssert(ModelViewMatrixConsistent());
}

void WED_PerspectiveCamera::Translate(const Vector3& v)
{
	DebugAssert(ModelViewMatrixConsistent());

	glMatrixMode(GL_MODELVIEW);
	glTranslated(v.dx, v.dy, v.dz);

	mXform.position -= v;

	mFrustumPlanesDirty = true;

	DebugAssert(ModelViewMatrixConsistent());
}

void WED_PerspectiveCamera::Scale(double sx, double sy, double sz)
{
	DebugAssert(ModelViewMatrixConsistent());

	glMatrixMode(GL_MODELVIEW);
	glScaled(sx, sy, sz);

	mXform.right.dx *= sx;
	mXform.up.dx *= sx;
	mXform.forward.dx *= sx;
	mXform.position.x /= sx;

	mXform.right.dy *= sy;
	mXform.up.dy *= sy;
	mXform.forward.dy *= sy;
	mXform.position.y /= sy;

	mXform.right.dz *= sz;
	mXform.up.dz *= sz;
	mXform.forward.dz *= sz;
	mXform.position.z /= sz;

	mFrustumPlanesDirty = true;

	DebugAssert(ModelViewMatrixConsistent());
}

void WED_PerspectiveCamera::Rotate(double deg, const Vector3& axis)
{
	DebugAssert(ModelViewMatrixConsistent());

	glMatrixMode(GL_MODELVIEW);
	glRotated(deg, axis.dx, axis.dy, axis.dz);

	double rad = deg * DEG_TO_RAD;

	double costh = cos(rad);
	double sinth = sin(rad);
	double ux = axis.dx;
	double uy = axis.dy;
	double uz = axis.dz;

	Vector3 col1(
		costh + ux * ux * (1 - costh),
		uy * ux * (1 - costh) + uz * sinth,
		uz * ux * (1 - costh) - uy * sinth);
	Vector3 col2(
		ux * uy * (1 - costh) - uz * sinth,
		costh + uy * uy * (1 - costh),
		uz * uy * (1 - costh) + ux * sinth
	);
	Vector3 col3(
		ux * uz * (1 - costh) + uy * sinth,
		uy * uz * (1 - costh) - ux * sinth,
		costh + uz * uz * (1 - costh)
	);

	Vector3 right = Right();
	Vector3 up = Up();
	Vector3 forward = Forward();
	Vector3 position = mXform.position - Point3(0, 0, 0);

	mXform.right = Vector3(right.dot(col1), right.dot(col2), right.dot(col3));
	mXform.up = Vector3(up.dot(col1), up.dot(col2), up.dot(col3));
	mXform.forward = Vector3(forward.dot(col1), forward.dot(col2), forward.dot(col3));
	mXform.position = Point3(position.dot(col1), position.dot(col2), position.dot(col3));

	mFrustumPlanesDirty = true;

	DebugAssert(ModelViewMatrixConsistent());
}

void WED_PerspectiveCamera::UpdateFrustumPlanes() const
{
	if (!mFrustumPlanesDirty) return;

	// Normals point towards inside of frustum.
	mFrustumPlanes[0] = Plane3(Position() + Forward() * mNearClip, Forward());
	mFrustumPlanes[1] = Plane3(Position() + Forward() * mFarClip, -Forward());
	mFrustumPlanes[2] = Plane3(Position(), -Right() * mNearClip + Forward() * (mWidth / 2));
	mFrustumPlanes[3] = Plane3(Position(), Right() * mNearClip + Forward() * (mWidth / 2));
	mFrustumPlanes[4] = Plane3(Position(), -Up() * mNearClip + Forward() * (mHeight / 2));
	mFrustumPlanes[5] = Plane3(Position(), Up() * mNearClip + Forward() * (mHeight / 2));

	mFrustumPlanesDirty = false;
}

static bool ApproxEqual(const Vector3& vec1, const Vector3& vec2, double epsilon = 1e-4)
{
	bool rval = (vec1 - vec2).squared_length() < epsilon*epsilon;
	if (!rval)
	{
		printf("vec1: (%lf, %lf, %lf), vec2: (%lf, %lf, %lf)\n", vec1.dx, vec1.dy, vec1.dz, vec2.dx, vec2.dy, vec2.dz);
	}
	return rval;
}

bool WED_PerspectiveCamera::ModelViewMatrixConsistent()
{
	float m[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);

	if (!ApproxEqual(mXform.right, Vector3(m[0], m[4], m[8])))
		return false;
	if (!ApproxEqual(mXform.up, Vector3(m[1], m[5], m[9])))
		return false;
	if (!ApproxEqual(mXform.forward, -Vector3(m[2], m[6], m[10])))
		return false;
	Vector3 preTranslation = Point3(0, 0, 0) - mXform.position;
	Vector3 postTranslation(preTranslation.dot(Right()), preTranslation.dot(Up()), -preTranslation.dot(Forward()));
	if (!ApproxEqual(postTranslation, Vector3(m[12], m[13], m[14]), 0.1))
		return false;
	if (!ApproxEqual(Vector3(0, 0, 0), Vector3(m[3], m[7], m[11])))
		return false;
	if (m[15] != 1.0)
		return false;

	return true;
}