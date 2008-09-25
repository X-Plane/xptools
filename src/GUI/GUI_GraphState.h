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

#ifndef GUI_GRAPHSTATE_H
#define GUI_GRAPHSTATE_H

class GUI_GraphState {
public:

	void		Init(void);
	void		Reset(void);

	void		EnableLighting(bool lighting);
	void		SetTexUnits(int count);
	void		EnableFog(bool fog);
	void		EnableAlpha(bool test, bool blend);
	void		EnableDepth(bool read, bool write);

	void		BindTex(int id, int unit);

	inline void	SetState(bool lit, int tex, bool fog, bool test, bool blend, bool read, bool write)
	{
		EnableLighting(lit);
		SetTexUnits(tex);
		EnableFog(fog);
		EnableAlpha(test,blend);
		EnableDepth(read,write);
	}

};


#endif
