/* 
 * Copyright (c) 2005, Laminar Research.
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
#ifndef OBJDRAW_H
#define OBJDRAW_H

// OBJ7:
// These APIs will handle merged tris/quads/lines for an OBJ7, but will
// not attempt to consolidate begin/end statements or other unnecessary state.
// This routine will attempt to change textures only when needed.

// OBJ8: 
// OBJ8 drawing will not eliminate unneeded state changes or consolidate
// draws, but for well-formed OBJ8s, the TRIs command should produce long runs.

struct XObj;
struct XObj8;

struct	ObjDrawFuncs_t {
	// These routines are called to set up OGL for a certain mode.
	void (* SetupPoly_f)(void * ref);
	void (* SetupLine_f)(void * ref);
	void (* SetupLight_f)(void * ref);
	void (* SetupMovie_f)(void * ref);
	void (* SetupPanel_f)(void * ref);
	// This is your multi-tex-coord routine.
	void (* TexCoord_f)(const float * st, void * ref);
	void (* TexCoordPointer_f)(int size, unsigned long type, long stride, const void * pointer, void * ref);
	// Return anim params here.
	float (* GetAnimParam)(const char * string, float v1, float v2, void * ref);
};

void	ObjDraw(const XObj& obj, float dist, ObjDrawFuncs_t * funcs, void * ref);
void	ObjDraw8(const XObj8& obj, float dist, ObjDrawFuncs_t * funcs, void * ref);

#endif
