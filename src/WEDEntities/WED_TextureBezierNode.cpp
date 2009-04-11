/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_TextureBezierNode.h"

DEFINE_PERSISTENT(WED_TextureBezierNode)
TRIVIAL_COPY(WED_TextureBezierNode, WED_GISPoint_Bezier)

WED_TextureBezierNode::WED_TextureBezierNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i),
	mS(this,"S","WED_texturenode","s",0.0,5,4),
	mT(this,"T","WED_texturenode","t",0.0,5,4),
	mScL(this,"S (Ctrl Low)", "WED_texturenode_bezier","sc_lo", 0.0,5,4),
	mTcL(this,"T (Ctrl Low)", "WED_texturenode_bezier","tc_lo", 0.0,5,4),
	mScH(this,"S (Ctrl Hi)", "WED_texturenode_bezier","sc_hi", 0.0,5,4),
	mTcH(this,"T (Ctrl Hi)", "WED_texturenode_bezier","tc_hi", 0.0,5,4)
{
}

WED_TextureBezierNode::~WED_TextureBezierNode()
{
}


bool		WED_TextureBezierNode::HasUV(void) const
{
	return true;
}

void		WED_TextureBezierNode::SetUV(const Point2& st)
{
	if (st.x() != mS.value || st.y() != mT.value)
	{
		mS = st.x();
		mT = st.y();
	}
}

void		WED_TextureBezierNode::GetUV(	   Point2& st) const
{
	st.x_ = mS.value;
	st.y_ = mT.value;
}


void		WED_TextureBezierNode::SetUVLo(const Point2& st)
{
	if (st.x() != mScL.value || st.y() != mTcL.value)
	{
		mScL = st.x();
		mTcL = st.y();
	}
}

void		WED_TextureBezierNode::GetUVLo(	   Point2& st) const
{
	st.x_ = mScL.value;
	st.y_ = mTcL.value;
}

void		WED_TextureBezierNode::SetUVHi(const Point2& st)
{
	if (st.x() != mScH.value || st.y() != mTcH.value)
	{
		mScH = st.x();
		mTcH = st.y();
	}
}

void		WED_TextureBezierNode::GetUVHi(	   Point2& st) const
{
	st.x_ = mScH.value;
	st.y_ = mTcH.value;
}
