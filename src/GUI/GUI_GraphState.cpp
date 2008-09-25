/*
 * Copyright (c) 2007, Laminar Research.
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

#include "GUI_GraphState.h"

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
#else
	#include <GL/gl.h>
	#include <GL/glext.h>
#endif

#include "XWinGL.h"

void		GUI_GraphState::Init(void)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glDisable(GL_DEPTH_TEST);
	glAlphaFunc		(GL_GREATER,0						);	// when ALPHA-TESTING  is enabled, we display the GREATEST alpha
	glBlendFunc		(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	// when ALPHA-BLENDING is enabled, we blend normally
	glDepthFunc		(GL_LEQUAL							);	// less than OR EQUAL plots on top
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	Reset();
}

void		GUI_GraphState::Reset(void)
{
	SetState(false,0,false,   false,false,   false,false);
}

void		GUI_GraphState::EnableLighting(bool lighting)
{
	if (lighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
}

void		GUI_GraphState::SetTexUnits(int count)
{
	for (int n = 0; n < 4; ++n)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+n);
		if (n < count)
			glEnable(GL_TEXTURE_2D);
		else
			glDisable(GL_TEXTURE_2D);
	}
	glActiveTextureARB(GL_TEXTURE0_ARB);
}

void		GUI_GraphState::EnableFog(bool fog)
{
	if (fog)
		glEnable(GL_FOG);
	else
		glDisable(GL_FOG);
}

void		GUI_GraphState::EnableAlpha(bool test, bool blend)
{
	if (test)
		glEnable(GL_ALPHA_TEST);
	else
		glDisable(GL_ALPHA_TEST);
	if (blend)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

}

void		GUI_GraphState::EnableDepth(bool read, bool write)
{
	if (read)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if (write)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}

void		GUI_GraphState::BindTex(int id, int unit)
{
	#if OPTIMIZE
		eliminate dupe bindings?
	#endif
	glActiveTextureARB(GL_TEXTURE0_ARB + unit);
	glBindTexture(GL_TEXTURE_2D, id);
	glActiveTextureARB(GL_TEXTURE0_ARB);

}
