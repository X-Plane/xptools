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
#ifndef WED_CAMERA_H
#define WED_CAMERA_H

#include "CompGeomDefs3.h"

/*
	WED_Camera

	A camera combines a position and orientation in space with an orthogonal or
	perspective projection.

	It provides methods to query whether an object is visible in the camera's field of
	view, what distance it is from the camera, and what its size in pixels will be on
	the screen. It also provides methods to modify the current model-view transformation.
*/

class WED_Camera {
public:
	// Returns whether a point or bounding box (in XYZ coordinates) is (partially) visible.
	virtual bool PointVisible(const Point3& point) const = 0;
	virtual bool BboxVisible(const Bbox3& bbox) const = 0;

	// Returns the distance of the point from the camera's center of projection (if the camera uses a
	// perspective projection) or 0.0 if the camera uses an orthogonal projection.
	virtual double PointDistance(const Point3& point) const = 0;

	// Returns the size in pixels that a feature of the given size at the given distance from the camera
	// will have on screen.
	virtual double PixelSize(double zCamera, double featureSize) const = 0;

	// Returns the size in pixels that an object with the given bounding box (in XYZ coordinates)
	// will have on screen.
	virtual double PixelSize(const Bbox3& bbox) const = 0;

	// Returns the maximum size in pixels that a feature of the given size (in XYZ coordinates) somewhere
	// within the given bounding box (in XYZ coordinates) will have on screen.
	virtual double PixelSize(const Bbox3& bbox, double featureSize) const = 0;

	// Returns the size in pixels that an object at the given position (in XYZ coordinates) and
	// with the given diameter (in XYZ coordinates) will have on screen.
	virtual double PixelSize(const Point3& position, double diameter) const = 0;

	// These methods modify the model-view transformation in an analogous way to their OpenGL counterparts.
	// An implementation should call these OpenGL counterparts and, additionally, update its own
	// transformation state if necessary.
	virtual void PushMatrix() = 0;
	virtual void PopMatrix() = 0;
	virtual void Translate(const Vector3& v) = 0;
	virtual void Scale(double sx, double sy, double sz) = 0;
	virtual void Rotate(double deg, const Vector3& axis) = 0;
};

#endif
