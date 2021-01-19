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
#ifndef WED_PERSPECTIVECAMERA_H
#define WED_PERSPECTIVECAMERA_H

#include "WED_Camera.h"

/*
	WED_PerspectiveCamera

	An implementation of WED_Camera that uses a perspective projection.
*/

class WED_PerspectiveCamera : public WED_Camera {
public:
	// Initializes the camera with near and far clip planes at the given distances in meters.
	WED_PerspectiveCamera(double nearClip, double farClip);

	// Moves the camera to the given position.
	void MoveTo(const Point3& position);

	// Sets the camera's "forward" vector, which points into the screen.
	void SetForward(const Vector3& forward);

	// Sets the field of view and screen resolution.
	// `horiztontalFovDeg` is the horizontal field of view in degrees. `viewportWidth` and `viewportHeight`
	// are the size of the viewport in pixels.
	void SetFOV(double horizontalFovDeg, double viewportWidth, double viewportHeight);

	// Applies the camera's projection matrix to the current OpenGL matrix. Make sure glMatrixMode(GL_PROJECTION)
	// is called before calling this method.
	void ApplyProjectionMatrix();

	// Applies the camera's model-view matrix to the current OpenGL matrix. Make sure glMatrixMode(GL_MODELVIEW)
	// is called before calling this method.
	void ApplyModelViewMatrix();

	const Point3& Position() const
	{
		return mXform.position;
	}

	const Vector3& Forward() const
	{
		return mXform.forward;
	}

	const Vector3& Up() const
	{
		return mXform.up;
	}

	const Vector3& Right() const
	{
		return mXform.right;
	}

	// WED_Camera overrides.
	bool PointVisible(const Point3& point) const override;
	bool BboxVisible(const Bbox3& bbox) const override;
	double PointDistance(const Point3& point) const override;
	double PixelSize(double zCamera, double featureSize) const override;
	double PixelSize(const Bbox3& bbox) const override;
	double PixelSize(const Bbox3& bbox, double featureSize) const override;
	double PixelSize(const Point3& position, double diameter) const override;
	void PushMatrix() override;
	void PopMatrix() override;
	void Translate(const Vector3& v) override;
	void Scale(double sx, double sy, double sz) override;
	void Rotate(double deg, const Vector3& axis) override;

private:
	void UpdateFrustumPlanes() const;
	bool ModelViewMatrixConsistent();

	// Current model-view transformation. This is equivalent to the following matrix:
	//
	// /  rx  ry  rz 0 \  /  1  0  0 px \
	// |  ux  uy  uz 0 |  |  0  1  0 py |
	// | -fx -fy -fz 0 |  |  0  0  1 pz |
	// \  0   0   0  1 /  \  0  0  0 1  /
	//
	// Where
	// right    = (rx, ry, rz)
	// forward  = (fx, fy, fz)
	// up       = (ux, ry, uz)
	// position = (px, py, pz)
	struct Transformation
	{
		Point3 position;
		Vector3 forward, right, up;
	};

	static constexpr int kNumFrustumPlanes = 6;

	double mNearClip, mFarClip, mWidth, mHeight;
	double mViewportWidth, mViewportHeight;
	Transformation mXform;
	std::vector<Transformation> mXformStack;

	mutable bool mFrustumPlanesDirty;
	mutable Plane3 mFrustumPlanes[kNumFrustumPlanes];
};

#endif
