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

#include "WED_DummyCamera.h"

#if APL
#include <OpenGL/gl.h>
#else
#include "glew.h"
#endif

// Constant pixel size to return in PixelSize() methods. An object of this size should always
// get rendered.
static const double DUMMY_PIXEL_SIZE = 100.0;

WED_DummyCamera::WED_DummyCamera()
{
}

bool WED_DummyCamera::PointVisible(const Point3& point) const
{
	return true;
}

bool WED_DummyCamera::BboxVisible(const Bbox3& bbox) const
{
	return true;
}

double WED_DummyCamera::PointDistance(const Point3& point) const
{
	return 0.0;
}

double WED_DummyCamera::PixelSize(double zCamera, double featureSize) const
{
	return DUMMY_PIXEL_SIZE;
}

double WED_DummyCamera::PixelSize(const Bbox3& bbox) const
{
	return DUMMY_PIXEL_SIZE;
}

double WED_DummyCamera::PixelSize(const Bbox3& bbox, double featureSize) const
{
	return DUMMY_PIXEL_SIZE;
}

double WED_DummyCamera::PixelSize(const Point3& position, double diameter) const
{
	return DUMMY_PIXEL_SIZE;
}

void WED_DummyCamera::PushMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

void WED_DummyCamera::PopMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void WED_DummyCamera::Translate(const Vector3& v)
{
	glMatrixMode(GL_MODELVIEW);
	glTranslated(v.dx, v.dy, v.dz);
}

void WED_DummyCamera::Scale(double sx, double sy, double sz)
{
	glMatrixMode(GL_MODELVIEW);
	glScaled(sx, sy, sz);
}

void WED_DummyCamera::Rotate(double deg, const Vector3& axis)
{
	glMatrixMode(GL_MODELVIEW);
	glRotated(deg, axis.dx, axis.dy, axis.dz);
}