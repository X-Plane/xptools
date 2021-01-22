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
#ifndef WED_DUMMYCAMERA_H
#define WED_DUMMYCAMERA_H

#include "WED_Camera.h"

/*
	WED_DummyCamera

	An implementation of WED_Camera that can be used when you need to supply a camera but
	don't want to cull any objects.

	The visibility and size methods return values that should always cause an object to
	be rendered. The methods for modifying the model-view transformation simply call their
	OpenGL counterparts.
*/

class WED_DummyCamera : public WED_Camera {
public:
	WED_DummyCamera();

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
};

#endif
