/*
 * Copyright (c) 2004, Laminar Research.
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
#include "OE_ProjectionMgr.h"

#include "OE_TexProjector.h"
#include "OE_CubeDeformer.h"
#include "OE_Globals.h"

#include "MatrixUtils.h"

OE_ProjectionMgr::OE_ProjectionMgr()
{
	mCurProjector = 0;

	mTexture.s1 = mTexture.t1 = 0.0;
	mTexture.s2 = mTexture.t2 = 1.0;
	mDeformer = new OE_CubeDeformer;
	mProjector[0] = new OE_PlanarProjector;
	mProjector[1] = new OE_CylinderProjector;
	mProjector[2] = new OE_SphereProjector;
}

OE_ProjectionMgr::~OE_ProjectionMgr()
{
	delete mDeformer;
	delete mProjector[0];
	delete mProjector[1];
	delete mProjector[2];
}

void		OE_ProjectionMgr::DrawProjectionSetup(void)
{
	mDeformer->DrawDeformer();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	mDeformer->ApplyTransform();
	mProjector[mCurProjector]->DrawProjector();
	glPopMatrix();
}

bool		OE_ProjectionMgr::TrackClick(
	OE_Zoomer3d *	zoomer,
	int				bounds[4],
	XPLMMouseStatus status,
	int 			x,
	int 			y,
	int 			button)
{
	bool	result =  mDeformer->TrackClick(zoomer, bounds, status, x, y, button);
	if (result)
		ApplyToObj();
	return result;
}

const XObj&		OE_ProjectionMgr::GetPreviewObj(void)
{
	return mPreviewObj;
}

OE_Texture_t&	OE_ProjectionMgr::GetTexture(void)
{
	return mTexture;
}

void		OE_ProjectionMgr::ApplyTexture(void)
{
	if (gObjects.empty()) return;

	ApplyToObj();

	gObjects[gLevelOfDetail] = mPreviewObj;
}

void		OE_ProjectionMgr::SetProjector(int p)
{
	mCurProjector = p;
	ApplyToObj();
}

int			OE_ProjectionMgr::GetProjector(void)
{
	return mCurProjector;
}


void		OE_ProjectionMgr::HandleNotification(int catagory, int message, void * param)
{
	ApplyToObj();
}

void		OE_ProjectionMgr::ApplyToObj(void)
{
	if (gObjects.empty())
		return;
	GLdouble matrix[16], matrix_i[16];
	mDeformer->GetTransform(matrix);
	invertMatrix(matrix_i, matrix);
	mPreviewObj = gObjects[gLevelOfDetail];
	for (set<int>::iterator cmdNum = gSelection.begin(); cmdNum != gSelection.end(); ++cmdNum)
	{
		XObjCmd& cmd = mPreviewObj.cmds[*cmdNum];
		for (vector<vec_tex>::iterator st = cmd.st.begin(); st != cmd.st.end(); ++st)
		{
			GLdouble	xyz[4], xyz_t[4];
			xyz[0] = st->v[0];
			xyz[1] = st->v[1];
			xyz[2] = st->v[2];
			xyz[3] = 1.0;
			double	s, t;
			multMatrixVec(xyz_t, matrix_i, xyz);
			mProjector[mCurProjector]->ProjectVertex(xyz_t[0], xyz_t[1], xyz_t[2], s, t);
			s = mTexture.s1 + s * (mTexture.s2 - mTexture.s1);
			t = mTexture.t1 + t * (mTexture.t2 - mTexture.t1);
			st->st[0] = s;
			st->st[1] = t;
		}
	}
}
